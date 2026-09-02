#pragma once

#include <QObject>
#include <QString>

#include "core/models.h"

class QTimer;
class QWebSocket;

// 实时通道客户端：连接后端 WS /ws/chat（携带 Authorization: Bearer <token>）。
// 服务端只做事件推送（消息写入仍走 REST）。
// 解析推送的 message.created 事件并转成 ChatMessage 发出。
// 断线后自动按固定间隔重连。
class WsClient : public QObject {
    Q_OBJECT

public:
    static WsClient &instance();

    // 开始连接（登录成功后调用）；内部自动断线重连
    void start();
    void stop();
    bool isConnected() const;

signals:
    void connected();
    void disconnected(const QString &reason);
    // 服务端推送的新消息（含自己发送的回显）
    void messageCreated(const ChatMessage &message);

private:
    explicit WsClient(QObject *parent = nullptr);

    void openSocket();
    void handleTextMessage(const QString &payload);

    QWebSocket *m_socket = nullptr;
    QTimer *m_reconnectTimer = nullptr;
    bool m_stopped = true;
};
