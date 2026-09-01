#include "network/TcpClient.h"

#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
{
}

// TODO(阶段 4): 基于 QTcpSocket::connectToHost 实现
void TcpClient::connectToServer(const QString &host, quint16 port)
{
    Q_UNUSED(host);
    Q_UNUSED(port);
    qDebug() << "[TcpClient] connectToServer 未实现（阶段 4 接入网络后可用）";
}

void TcpClient::disconnectFromServer()
{
    qDebug() << "[TcpClient] disconnectFromServer 未实现";
}

bool TcpClient::isConnected() const
{
    return m_connected;
}

void TcpClient::sendMessage(const ChatMessage &message)
{
    Q_UNUSED(message);
    qDebug() << "[TcpClient] sendMessage 未实现";
}
