#pragma once

#include <QObject>
#include <QString>

#include "core/models.h"

// 网络层接口（阶段 4 实现）。
// 当前为占位实现：只定义客户端与服务端的通信契约。
// 接入时基于 QTcpSocket / QWebSocket 完成收发与心跳。
class TcpClient : public QObject {
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    // 发送一条消息到服务端（由服务端转发给接收方）
    void sendMessage(const ChatMessage &message);

signals:
    void connected();
    void disconnected();
    void messageReceived(const ChatMessage &message);
    void errorOccurred(const QString &error);

private:
    bool m_connected = false;
};
