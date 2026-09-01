#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

#include "core/models.h"

// 用户/会话/消息的业务管理层。
//
// 当前阶段（客户端 UI）用内存假数据驱动界面；
// 阶段 4 接入网络后，本类的 sendMessage 等实现将替换为
// 调用 network/TcpClient 收发真实消息，界面层无需改动。
class UserManager : public QObject {
    Q_OBJECT

public:
    explicit UserManager(QObject *parent = nullptr);

    QString currentUser() const;
    void setCurrentUser(const QString &name);

    const QList<Conversation> &conversations() const;
    const QList<ChatMessage> &messagesFor(const QString &friendName) const;

    // 发送一条消息（当前为演示实现：本地追加 + 延时自动回复）
    void sendMessage(const QString &friendName, const QString &text);

signals:
    // 新消息产生（自己发的或对方回复的）
    void messageAdded(const QString &friendName, const ChatMessage &message);
    // 会话列表某一行需要刷新
    void conversationUpdated(const QString &friendName,
                             const QString &lastMessage,
                             const QString &timeText);

private:
    void appendMessage(const QString &friendName, const QString &text, bool isSelf);
    void touchConversation(const QString &friendName, const QString &lastMessage);
    void simulateReply(const QString &friendName);  // 演示用自动回复

    QString m_currentUser;
    QList<Conversation> m_conversations;
    QHash<QString, QList<ChatMessage>> m_messages;  // friendName -> 历史消息
};
