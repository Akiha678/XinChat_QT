#include "network/WsClient.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

#include "core/Format.h"
#include "core/Session.h"
#include "network/ApiClient.h"

namespace {

QString wsUrl()
{
    QString url = ApiClient::baseUrl();
    if (url.startsWith(QLatin1String("https://"))) {
        url.replace(0, 8, QStringLiteral("wss://"));
    } else if (url.startsWith(QLatin1String("http://"))) {
        url.replace(0, 7, QStringLiteral("ws://"));
    }
    return url + QStringLiteral("/ws/chat");
}

// 解析实时推送载荷（后端 ChatMessageResponse，与 REST 的 MsgResponse 字段不同）：
//   {id, conversationId, senderId, content(纯文本), type:"TEXT", createdAt}
// 不含发送者昵称与方向字段；方向由 senderId 与当前登录用户比对得出。
ChatMessage parseMessageEnvelope(const QJsonObject &m)
{
    ChatMessage msg;
    msg.id = m.value(QStringLiteral("id")).toVariant().toLongLong();
    msg.sessionId = m.value(QStringLiteral("conversationId")).toVariant().toLongLong();
    msg.senderId = m.value(QStringLiteral("senderId")).toVariant().toLongLong();
    msg.text = m.value(QStringLiteral("content")).toString();
    msg.timestamp = xc::parseIsoInstant(m.value(QStringLiteral("createdAt")).toString());
    const qint64 myId = Session::instance().login().user.id;
    msg.isSelf = (msg.senderId != 0) && (msg.senderId == myId);
    return msg;
}

}  // namespace

WsClient::WsClient(QObject *parent)
    : QObject(parent)
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(5000);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() { openSocket(); });
}

WsClient &WsClient::instance()
{
    static WsClient s_instance;
    return s_instance;
}

bool WsClient::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void WsClient::start()
{
    m_stopped = false;
    if (isConnected()) {
        return;
    }
    openSocket();
}

void WsClient::stop()
{
    m_stopped = true;
    m_reconnectTimer->stop();
    if (m_socket) {
        m_socket->abort();
    }
}

void WsClient::openSocket()
{
    if (m_stopped) {
        return;
    }
    if (Session::instance().token().isEmpty()) {
        qWarning() << "[WsClient] 未登录，跳过连接";
        return;
    }
    if (isConnected()) {
        return;
    }

    delete m_socket;
    m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_socket, &QWebSocket::connected, this, [this]() {
        m_reconnectTimer->stop();
        emit connected();
        qInfo() << "[WsClient] 已连接" << wsUrl();
    });
    connect(m_socket, &QWebSocket::disconnected, this, [this]() {
        qWarning() << "[WsClient] 连接断开，尝试重连...";
        if (!m_stopped) {
            m_reconnectTimer->start();
        }
    });
    connect(m_socket, &QWebSocket::textMessageReceived, this,
            [this](const QString &payload) { handleTextMessage(payload); });

    // 握手时携带 Authorization: Bearer <token>（与 REST 鉴权一致）
    const QUrl url(wsUrl());
    QNetworkRequest request{url};
    request.setRawHeader("Authorization",
                         QStringLiteral("Bearer %1").arg(Session::instance().token()).toUtf8());
    m_socket->open(request);
}

void WsClient::handleTextMessage(const QString &payload)
{
    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    if (!doc.isObject()) {
        return;
    }
    const QJsonObject obj = doc.object();
    const QString type = obj.value(QStringLiteral("type")).toString();
    if (type == QLatin1String("message.created")) {
        const QJsonObject data = obj.value(QStringLiteral("data")).toObject();
        if (!data.isEmpty()) {
            emit messageCreated(parseMessageEnvelope(data));
        }
    }
}
