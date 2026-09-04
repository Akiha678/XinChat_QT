#include "network/ApiClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include "core/Format.h"
#include "core/Session.h"

namespace {

QString apiBaseUrl()
{
    // 默认连接本机后端；可用环境变量覆盖，便于联调其他环境
    const QByteArray env = qgetenv("XINCHAT_API_BASE");
    if (!env.isEmpty()) {
        return QString::fromUtf8(env);
    }
    return QStringLiteral("http://127.0.0.1:8080");
}

// 从响应体解析 JSON 对象；解析失败返回空对象
QJsonObject parseBodyObject(const QByteArray &raw, bool *ok)
{
    if (ok) {
        *ok = false;
    }
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return QJsonObject();
    }
    if (ok) {
        *ok = true;
    }
    return doc.object();
}

QJsonArray parseBodyArray(const QByteArray &raw, bool *ok)
{
    if (ok) {
        *ok = false;
    }
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        return QJsonArray();
    }
    if (ok) {
        *ok = true;
    }
    return doc.array();
}

// 业务是否失败：HTTP>=400，或 HTTP200 但业务 code != 1000（NetworkResponse.failure）
bool isBusinessError(int httpStatus, const QJsonObject &obj)
{
    if (httpStatus >= 400) {
        return true;
    }
    return obj.contains(QStringLiteral("code"))
           && obj.value(QStringLiteral("code")).toInt() != 1000;
}

// 提取错误描述（兼容 NetworkResponse / Spring ErrorResponse）
QString extractErrorMessage(int httpStatus, const QJsonObject &obj)
{
    QString message = obj.value(QStringLiteral("message")).toString();
    if (message.isEmpty()) {
        message = obj.value(QStringLiteral("error")).toString();
    }
    if (message.isEmpty()) {
        message = QStringLiteral("请求失败（HTTP %1）").arg(httpStatus);
    }
    return message;
}

// 取 NetworkResponse.data；无 data 字段时返回空对象
QJsonObject unwrapData(const QJsonObject &obj)
{
    const QJsonValue data = obj.value(QStringLiteral("data"));
    if (data.isObject()) {
        return data.toObject();
    }
    return QJsonObject();
}

QJsonArray unwrapDataArray(const QJsonObject &obj)
{
    const QJsonValue data = obj.value(QStringLiteral("data"));
    if (data.isArray()) {
        return data.toArray();
    }
    return QJsonArray();
}

UserSummary parseUserSummary(const QJsonObject &u)
{
    UserSummary user;
    user.id = u.value(QStringLiteral("id")).toVariant().toLongLong();
    user.name = u.value(QStringLiteral("name")).toString();
    if (user.name.isEmpty()) {
        user.name = u.value(QStringLiteral("displayName")).toString();
    }
    user.username = u.value(QStringLiteral("username")).toString();
    user.email = u.value(QStringLiteral("email")).toString();
    user.avatarColor = u.value(QStringLiteral("avatarColor")).toInt();
    return user;
}

FriendRequest parseFriendRequest(const QJsonObject &r)
{
    FriendRequest request;
    request.id = r.value(QStringLiteral("id")).toVariant().toLongLong();
    request.requester = parseUserSummary(r.value(QStringLiteral("requester")).toObject());
    request.addressee = parseUserSummary(r.value(QStringLiteral("addressee")).toObject());
    request.status = r.value(QStringLiteral("status")).toString();
    request.message = r.value(QStringLiteral("message")).toString();
    request.createdAt = xc::parseIsoInstant(r.value(QStringLiteral("createdAt")).toString());
    return request;
}

// MsgResponse -> ChatMessage
ChatMessage parseMessage(const QJsonObject &m)
{
    ChatMessage msg;
    msg.id = m.value(QStringLiteral("id")).toVariant().toLongLong();
    msg.sessionId = m.value(QStringLiteral("sessionId")).toVariant().toLongLong();
    msg.senderId = m.value(QStringLiteral("userId")).toVariant().toLongLong();
    msg.nickName = m.value(QStringLiteral("nickName")).toString();
    msg.isSelf = m.value(QStringLiteral("type")).toInt() == 0;  // 0=本人发送
    msg.isRead = m.value(QStringLiteral("status")).toInt() == 1;
    msg.timestamp = xc::parseIsoInstant(m.value(QStringLiteral("createTime")).toString());

    const QJsonObject content = m.value(QStringLiteral("content")).toObject();
    msg.text = content.value(QStringLiteral("data")).toString();
    if (msg.text.isEmpty()) {
        msg.text = content.value(QStringLiteral("text")).toString();  // 兼容旧格式
    }
    return msg;
}

// ConversationResponse -> Conversation
Conversation parseConversation(const QJsonObject &c)
{
    Conversation conv;
    conv.id = c.value(QStringLiteral("id")).toVariant().toLongLong();
    conv.peerId = c.value(QStringLiteral("peerId")).toVariant().toLongLong();
    conv.name = c.value(QStringLiteral("name")).toString();
    conv.preview = c.value(QStringLiteral("preview")).toString();
    conv.lastMessageAt =
        xc::parseIsoInstant(c.value(QStringLiteral("lastMessageAt")).toString());
    conv.unreadCount = c.value(QStringLiteral("unreadCount")).toInt();
    conv.colorSeed = c.value(QStringLiteral("colorSeed")).toInt();
    return conv;
}

}  // namespace

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
}

ApiClient &ApiClient::instance()
{
    static ApiClient s_instance;
    return s_instance;
}

QString ApiClient::baseUrl()
{
    return apiBaseUrl();
}

QNetworkRequest ApiClient::makeAuthedRequest(const QString &path) const
{
    QNetworkRequest request(QUrl(apiBaseUrl() + path));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    request.setTransferTimeout(10000);
    const QString &token = Session::instance().token();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization",
                             QStringLiteral("Bearer %1").arg(token).toUtf8());
    }
    return request;
}

// ---------- 登录 ----------

void ApiClient::login(const QString &username, const QString &password)
{
    QJsonObject body;
    body.insert(QStringLiteral("username"), username);
    body.insert(QStringLiteral("password"), password);

    QNetworkRequest request(QUrl(apiBaseUrl() + QStringLiteral("/auth/login")));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    request.setTransferTimeout(10000);

    QNetworkReply *reply = m_nam->post(request,
                                       QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleLoginReply(reply); });
}

void ApiClient::handleLoginReply(QNetworkReply *reply)
{
    reply->deleteLater();

    const int httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();

    // 非 HTTP 状态码的网络层错误（服务器未启动、连接被拒、超时等）
    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit loginFailed(0, 0,
                         QStringLiteral("无法连接服务器（%1）").arg(reply->errorString()));
        return;
    }

    bool parseOk = false;
    const QJsonObject obj = parseBodyObject(raw, &parseOk);
    if (!parseOk) {
        emit loginFailed(httpStatus, 0, QStringLiteral("服务器返回数据格式错误"));
        return;
    }

    if (isBusinessError(httpStatus, obj)) {
        emit loginFailed(httpStatus, obj.value(QStringLiteral("code")).toInt(),
                         extractErrorMessage(httpStatus, obj));
        return;
    }

    // 成功：直接返回 LoginResponse（顶层含 accessToken），或包在 data 里，两种都兼容
    QJsonObject data = obj;
    if (!obj.contains(QStringLiteral("accessToken"))) {
        data = unwrapData(obj);
    }
    if (!data.contains(QStringLiteral("accessToken"))) {
        emit loginFailed(httpStatus, 0, QStringLiteral("登录响应缺少 accessToken"));
        return;
    }

    LoginResult result;
    result.user.id = data.value(QStringLiteral("id")).toVariant().toLongLong();
    result.user.username = data.value(QStringLiteral("username")).toString();
    result.user.displayName = data.value(QStringLiteral("displayName")).toString();
    result.user.email = data.value(QStringLiteral("email")).toString();
    result.user.avatarColor = data.value(QStringLiteral("avatarColor")).toInt();
    result.accessToken = data.value(QStringLiteral("accessToken")).toString();
    result.expiresAt = xc::parseIsoInstant(data.value(QStringLiteral("expiresAt")).toString());

    emit loginSucceeded(result);
}

// ---------- 聊天接口 ----------

void ApiClient::fetchSessions()
{
    QNetworkRequest request = makeAuthedRequest(QStringLiteral("/chat/session"));
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleSessionsReply(reply); });
}

void ApiClient::handleSessionsReply(QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();

    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit chatRequestFailed(QStringLiteral("会话列表"),
                               QStringLiteral("无法连接服务器"));
        return;
    }
    bool parseOk = false;
    const QJsonObject obj = parseBodyObject(raw, &parseOk);
    if (!parseOk || isBusinessError(httpStatus, obj)) {
        emit chatRequestFailed(QStringLiteral("会话列表"),
                               parseOk ? extractErrorMessage(httpStatus, obj)
                                       : QStringLiteral("服务器返回数据格式错误"));
        return;
    }

    QList<Conversation> sessions;
    const QJsonArray arr = unwrapDataArray(obj);
    for (const QJsonValue &v : arr) {
        sessions.append(parseConversation(v.toObject()));
    }
    emit sessionsLoaded(sessions);
}

void ApiClient::fetchMessages(qint64 sessionId, int page, int size)
{
    QJsonObject body;
    body.insert(QStringLiteral("sessionId"),
                QJsonValue::fromVariant(QVariant::fromValue<qint64>(sessionId)));
    body.insert(QStringLiteral("page"), page);
    body.insert(QStringLiteral("size"), size);

    QNetworkRequest request =
        makeAuthedRequest(QStringLiteral("/chat/message/page"));
    QNetworkReply *reply = m_nam->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, sessionId, reply]() {
        handleMessagesReply(sessionId, reply);
    });
}

void ApiClient::handleMessagesReply(qint64 sessionId, QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();

    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit chatRequestFailed(QStringLiteral("消息记录"),
                               QStringLiteral("无法连接服务器"));
        return;
    }
    bool parseOk = false;
    const QJsonObject obj = parseBodyObject(raw, &parseOk);
    if (!parseOk || isBusinessError(httpStatus, obj)) {
        emit chatRequestFailed(QStringLiteral("消息记录"),
                               parseOk ? extractErrorMessage(httpStatus, obj)
                                       : QStringLiteral("服务器返回数据格式错误"));
        return;
    }

    QList<ChatMessage> messages;
    const QJsonObject data = unwrapData(obj);  // {list:[...], pagination:{...}}
    const QJsonArray arr = data.value(QStringLiteral("list")).toArray();
    for (const QJsonValue &v : arr) {
        messages.append(parseMessage(v.toObject()));
    }
    const int total =
        data.value(QStringLiteral("pagination")).toObject()
            .value(QStringLiteral("total")).toInt();
    emit messagesLoaded(sessionId, messages, total);
}

void ApiClient::sendMessage(qint64 sessionId, const QString &content)
{
    QJsonObject body;
    body.insert(QStringLiteral("content"), content);

    QNetworkRequest request = makeAuthedRequest(
        QStringLiteral("/chat/session/%1/message").arg(sessionId));
    QNetworkReply *reply = m_nam->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleSendReply(reply); });
}

void ApiClient::handleSendReply(QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();

    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit chatRequestFailed(QStringLiteral("发送消息"),
                               QStringLiteral("无法连接服务器"));
        return;
    }
    bool parseOk = false;
    const QJsonObject obj = parseBodyObject(raw, &parseOk);
    if (!parseOk || isBusinessError(httpStatus, obj)) {
        emit chatRequestFailed(QStringLiteral("发送消息"),
                               parseOk ? extractErrorMessage(httpStatus, obj)
                                       : QStringLiteral("服务器返回数据格式错误"));
        return;
    }
    emit messageSent(parseMessage(unwrapData(obj)));
}

void ApiClient::markMessagesRead(const QList<qint64> &messageIds)
{
    if (messageIds.isEmpty()) {
        return;
    }
    QJsonArray ids;
    for (qint64 id : messageIds) {
        ids.append(QJsonValue::fromVariant(QVariant::fromValue<qint64>(id)));
    }
    QJsonObject body;
    body.insert(QStringLiteral("ids"), ids);

    QNetworkRequest request =
        makeAuthedRequest(QStringLiteral("/chat/message/read"));
    QNetworkReply *reply = m_nam->post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

// ---------- 好友接口 ----------

void ApiClient::fetchFriends()
{
    QNetworkReply *reply = m_nam->get(makeAuthedRequest(QStringLiteral("/contact/friends")));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleFriendsReply(reply); });
}

void ApiClient::searchUsers(const QString &username)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("username"), username);
    const QString path = QStringLiteral("/contact/users/search?") + query.toString(QUrl::FullyEncoded);
    QNetworkReply *reply = m_nam->get(makeAuthedRequest(path));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleSearchUsersReply(reply); });
}

void ApiClient::sendFriendRequest(qint64 addresseeId, const QString &message)
{
    QJsonObject body;
    body.insert(QStringLiteral("addresseeId"), QJsonValue::fromVariant(QVariant::fromValue<qint64>(addresseeId)));
    if (!message.trimmed().isEmpty()) {
        body.insert(QStringLiteral("message"), message.trimmed());
    }
    QNetworkReply *reply = m_nam->post(
        makeAuthedRequest(QStringLiteral("/contact/friend-requests")),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleFriendRequestReply(reply); });
}

void ApiClient::createDirectConversation(qint64 friendId)
{
    QJsonObject body;
    body.insert(QStringLiteral("friendId"), QJsonValue::fromVariant(QVariant::fromValue<qint64>(friendId)));
    QNetworkReply *reply = m_nam->post(
        makeAuthedRequest(QStringLiteral("/chat/conversation")),
        QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply]() { handleCreateConversationReply(reply); });
}

void ApiClient::handleFriendsReply(QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit contactRequestFailed(QStringLiteral("好友列表"), QStringLiteral("无法连接服务器"));
        return;
    }
    bool objectOk = false;
    const QJsonObject obj = parseBodyObject(raw, &objectOk);
    QJsonArray arr;
    if (objectOk) {
        if (isBusinessError(httpStatus, obj)) {
            emit contactRequestFailed(QStringLiteral("好友列表"), extractErrorMessage(httpStatus, obj));
            return;
        }
        arr = unwrapDataArray(obj);
    } else {
        bool arrayOk = false;
        arr = parseBodyArray(raw, &arrayOk);
        if (!arrayOk) {
            emit contactRequestFailed(QStringLiteral("好友列表"), QStringLiteral("服务器返回数据格式错误"));
            return;
        }
    }
    QList<UserSummary> friends;
    for (const QJsonValue &value : arr) {
        friends.append(parseUserSummary(value.toObject()));
    }
    emit friendsLoaded(friends);
}

void ApiClient::handleSearchUsersReply(QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit contactRequestFailed(QStringLiteral("搜索好友"), QStringLiteral("无法连接服务器"));
        return;
    }
    bool objectOk = false;
    const QJsonObject obj = parseBodyObject(raw, &objectOk);
    QJsonArray arr;
    if (objectOk) {
        if (isBusinessError(httpStatus, obj)) {
            emit contactRequestFailed(QStringLiteral("搜索好友"), extractErrorMessage(httpStatus, obj));
            return;
        }
        arr = unwrapDataArray(obj);
    } else {
        bool arrayOk = false;
        arr = parseBodyArray(raw, &arrayOk);
        if (!arrayOk) {
            emit contactRequestFailed(QStringLiteral("搜索好友"), QStringLiteral("服务器返回数据格式错误"));
            return;
        }
    }
    QList<UserSummary> users;
    for (const QJsonValue &value : arr) {
        users.append(parseUserSummary(value.toObject()));
    }
    emit usersSearchLoaded(users);
}

void ApiClient::handleFriendRequestReply(QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    bool ok = false;
    const QJsonObject obj = parseBodyObject(raw, &ok);
    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit contactRequestFailed(QStringLiteral("添加好友"), QStringLiteral("无法连接服务器"));
    } else if (!ok || isBusinessError(httpStatus, obj)) {
        emit contactRequestFailed(QStringLiteral("添加好友"), ok ? extractErrorMessage(httpStatus, obj)
                                                                    : QStringLiteral("服务器返回数据格式错误"));
    } else {
        QJsonObject data = unwrapData(obj);
        if (data.isEmpty()) {
            data = obj;
        }
        emit friendRequestSent(parseFriendRequest(data));
    }
}

void ApiClient::handleCreateConversationReply(QNetworkReply *reply)
{
    reply->deleteLater();
    const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    bool ok = false;
    const QJsonObject obj = parseBodyObject(raw, &ok);
    if (httpStatus == 0 && reply->error() != QNetworkReply::NoError) {
        emit contactRequestFailed(QStringLiteral("打开聊天"), QStringLiteral("无法连接服务器"));
        return;
    }
    if (!ok || isBusinessError(httpStatus, obj)) {
        emit contactRequestFailed(QStringLiteral("打开聊天"), ok ? extractErrorMessage(httpStatus, obj)
                                                                    : QStringLiteral("服务器返回数据格式错误"));
        return;
    }
    QJsonObject data = unwrapData(obj);
    if (data.isEmpty()) {
        data = obj;
    }
    emit conversationCreated(parseConversation(data));
}
