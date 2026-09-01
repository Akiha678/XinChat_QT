#include "network/ApiClient.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

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

// 解析 ISO-8601 时间字符串，如 "2026-09-08T05:40:05.554935Z"
// Qt 只支持毫秒精度，这里截断多余小数位并去掉 Z 后缀
QDateTime parseIsoInstant(const QString &text)
{
    QString t = text;
    if (t.endsWith(QLatin1Char('Z'))) {
        t.chop(1);
    }
    const int dot = t.indexOf(QLatin1Char('.'));
    if (dot >= 0) {
        t = t.left(dot + 1) + t.mid(dot + 1).left(3);
    }
    QDateTime dt = QDateTime::fromString(t, QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"));
    if (!dt.isValid()) {
        dt = QDateTime::fromString(text, Qt::ISODate);
    }
    return dt;
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

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit loginFailed(httpStatus, 0, QStringLiteral("服务器返回数据格式错误"));
        return;
    }
    const QJsonObject obj = doc.object();

    // 业务失败判定（两种情况）：
    // 1) HTTP 4xx/5xx —— Spring ErrorResponse {"status":401,"error":"Unauthorized","message":"..."}
    // 2) HTTP 200 但 body 携带业务错误码（本后端 /auth 下返回 NetworkResponse.failure，
    //    形如 {"data":null,"code":401,"message":"用户名或密码错误"}）
    const bool hasBusinessCode = obj.contains(QStringLiteral("code"));
    const bool businessError = hasBusinessCode
                               && obj.value(QStringLiteral("code")).toInt() != 1000;
    if (httpStatus >= 400 || businessError) {
        QString message = obj.value(QStringLiteral("message")).toString();
        if (message.isEmpty()) {
            message = obj.value(QStringLiteral("error")).toString();
        }
        if (message.isEmpty()) {
            message = QStringLiteral("登录失败（HTTP %1）").arg(httpStatus);
        }
        const int bizCode = businessError ? obj.value(QStringLiteral("code")).toInt() : 0;
        emit loginFailed(httpStatus, bizCode, message);
        return;
    }

    // 成功：直接返回 LoginResponse（顶层含 accessToken），
    // 或包在 NetworkResponse.data 里，两种都兼容
    QJsonObject data = obj;
    if (!obj.contains(QStringLiteral("accessToken"))) {
        const QJsonValue wrapped = obj.value(QStringLiteral("data"));
        if (wrapped.isObject()) {
            data = wrapped.toObject();
        }
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
    result.expiresAt = parseIsoInstant(data.value(QStringLiteral("expiresAt")).toString());

    emit loginSucceeded(result);
}
