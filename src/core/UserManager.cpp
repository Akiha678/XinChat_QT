#include "core/UserManager.h"

#include <QRandomGenerator>
#include <QTime>
#include <QTimer>

UserManager::UserManager(QObject *parent)
    : QObject(parent)
{
    // 演示用假数据（阶段 4 接入网络/数据库后替换为真实数据）
    m_conversations = {
        { QStringLiteral("张三"),     QStringLiteral("好的，明天见"), QStringLiteral("10:24"), QColor(0x5B, 0xC2, 0x8F) },
        { QStringLiteral("李四"),     QStringLiteral("文件我发你了"), QStringLiteral("09:12"), QColor(0xE8, 0x9C, 0x4E) },
        { QStringLiteral("王五"),     QStringLiteral("[图片]"),       QStringLiteral("昨天"),  QColor(0x6A, 0x9B, 0xE8) },
        { QStringLiteral("产品讨论群"), QStringLiteral("周报记得提交"), QStringLiteral("周一"),  QColor(0xB0, 0x7C, 0xDE) },
    };

    // 预置一段聊天记录，让界面打开就有内容可看
    appendMessage(QStringLiteral("张三"), QStringLiteral("你好，我是张三"), false);
    appendMessage(QStringLiteral("张三"), QStringLiteral("方案文档发我一份？"), false);
    appendMessage(QStringLiteral("张三"), QStringLiteral("好的，马上发你"), true);
}

QString UserManager::currentUser() const
{
    return m_currentUser;
}

void UserManager::setCurrentUser(const QString &name)
{
    m_currentUser = name;
}

const QList<Conversation> &UserManager::conversations() const
{
    return m_conversations;
}

const QList<ChatMessage> &UserManager::messagesFor(const QString &friendName) const
{
    static const QList<ChatMessage> kEmpty;
    auto it = m_messages.constFind(friendName);
    return it == m_messages.constEnd() ? kEmpty : it.value();
}

void UserManager::sendMessage(const QString &friendName, const QString &text)
{
    appendMessage(friendName, text, true);
    touchConversation(friendName, text);

    // —— 演示逻辑：延时模拟对方自动回复 ——
    // 阶段 4 接入网络后，这里改为把消息交给 TcpClient 发送，
    // 由服务端/对方客户端触发 messageAdded。
    const int delay = 600 + QRandomGenerator::global()->bounded(700);
    QTimer::singleShot(delay, this, [this, friendName]() { simulateReply(friendName); });
}

void UserManager::appendMessage(const QString &friendName, const QString &text, bool isSelf)
{
    ChatMessage msg;
    msg.senderName = friendName;
    msg.text = text;
    msg.isSelf = isSelf;
    msg.timestamp = QDateTime::currentDateTime();
    m_messages[friendName].append(msg);
    emit messageAdded(friendName, msg);
}

void UserManager::touchConversation(const QString &friendName, const QString &lastMessage)
{
    for (Conversation &c : m_conversations) {
        if (c.friendName == friendName) {
            c.lastMessage = lastMessage;
            c.timeText = QTime::currentTime().toString(QStringLiteral("HH:mm"));
            emit conversationUpdated(c.friendName, c.lastMessage, c.timeText);
            return;
        }
    }
}

void UserManager::simulateReply(const QString &friendName)
{
    static const QStringList replies = {
        QStringLiteral("收到~"),
        QStringLiteral("好的"),
        QStringLiteral("哈哈哈哈"),
        QStringLiteral("稍等，我看一下"),
        QStringLiteral("在忙，晚点回你"),
    };
    const QString reply = replies.at(QRandomGenerator::global()->bounded(replies.size()));
    appendMessage(friendName, reply, false);
    touchConversation(friendName, reply);
}
