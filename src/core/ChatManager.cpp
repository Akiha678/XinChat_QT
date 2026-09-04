#include "core/ChatManager.h"

#include <QDateTime>
#include <QDebug>

#include "core/Session.h"
#include "network/ApiClient.h"
#include "network/WsClient.h"

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
{
    ApiClient &api = ApiClient::instance();
    connect(&api, &ApiClient::sessionsLoaded,
            this, &ChatManager::onSessionsLoaded);
    connect(&api, &ApiClient::messagesLoaded,
            this, &ChatManager::onMessagesLoaded);
    connect(&api, &ApiClient::messageSent,
            this, &ChatManager::onMessageSent);
    connect(&api, &ApiClient::chatRequestFailed,
            this, &ChatManager::requestError);
    connect(&api, &ApiClient::conversationCreated,
            this, &ChatManager::onConversationCreated);

    WsClient &ws = WsClient::instance();
    connect(&ws, &WsClient::messageCreated,
            this, &ChatManager::onIncomingMessage);
}

void ChatManager::startRealtime()
{
    WsClient::instance().start();
}

void ChatManager::reset()
{
    WsClient::instance().stop();
    m_sessions.clear();
    m_messages.clear();
    m_knownIds.clear();
    m_openSessionId = 0;
    m_sessionsLoaded = false;
}

void ChatManager::loadSessions()
{
    ApiClient::instance().fetchSessions();
}

void ChatManager::openSession(qint64 sessionId)
{
    m_openSessionId = sessionId;
    ApiClient::instance().fetchMessages(sessionId, 1, 50);
}

void ChatManager::openConversationWith(qint64 friendId)
{
    if (friendId <= 0) {
        return;
    }
    ApiClient::instance().createDirectConversation(friendId);
}

void ChatManager::sendText(qint64 sessionId, const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }
    ApiClient::instance().sendMessage(sessionId, text);
}

const QList<Conversation> &ChatManager::conversations() const
{
    return m_sessions;
}

const QList<ChatMessage> &ChatManager::messagesOf(qint64 sessionId) const
{
    static const QList<ChatMessage> kEmpty;
    auto it = m_messages.constFind(sessionId);
    return it == m_messages.constEnd() ? kEmpty : it.value();
}

int ChatManager::unreadTotal() const
{
    int total = 0;
    for (const Conversation &c : m_sessions) {
        total += c.unreadCount;
    }
    return total;
}

void ChatManager::onSessionsLoaded(const QList<Conversation> &sessions)
{
    if (!Session::instance().isLoggedIn()) {
        return;
    }
    m_sessions = sessions;
    sortSessions();
    m_sessionsLoaded = true;
    emit sessionsChanged();
}

void ChatManager::onMessagesLoaded(qint64 sessionId,
                                   const QList<ChatMessage> &messages,
                                   int total)
{
    if (!Session::instance().isLoggedIn()) {
        return;
    }
    Q_UNUSED(total);
    // 后端按时间倒序（最新在前），转成时间升序便于 UI 从上到下渲染
    QList<ChatMessage> ascending = messages;
    std::reverse(ascending.begin(), ascending.end());

    QList<ChatMessage> &cache = m_messages[sessionId];
    cache = ascending;
    for (const ChatMessage &msg : ascending) {
        m_knownIds.insert(msg.id);
    }

    // 打开会话时，把收到的未读消息标记为已读（type==1 且 status==0）
    if (sessionId == m_openSessionId) {
        QList<qint64> unreadIds;
        for (const ChatMessage &msg : ascending) {
            if (!msg.isSelf && !msg.isRead) {
                unreadIds.append(msg.id);
            }
        }
        ApiClient::instance().markMessagesRead(unreadIds);
        if (Conversation *conv = conversation(sessionId)) {
            conv->unreadCount = 0;
        }
    }
    emit sessionMessagesChanged(sessionId);
}

void ChatManager::onMessageSent(const ChatMessage &message)
{
    if (!Session::instance().isLoggedIn()) {
        return;
    }
    appendMessage(message);  // 服务端会推 WS 回显，这里用 ID 去重兜底
}

void ChatManager::onIncomingMessage(const ChatMessage &message)
{
    if (!Session::instance().isLoggedIn()) {
        return;
    }
    appendMessage(message);
}

void ChatManager::onConversationCreated(const Conversation &conversation)
{
    if (!Session::instance().isLoggedIn()) {
        return;
    }
    if (conversation.id <= 0) {
        emit requestError(QStringLiteral("打开聊天"), QStringLiteral("服务器返回的会话无效"));
        return;
    }
    Conversation *existing = this->conversation(conversation.id);
    if (existing) {
        *existing = conversation;
    } else {
        m_sessions.append(conversation);
    }
    sortSessions();
    emit sessionsChanged();
    emit conversationOpened(conversation);
}

void ChatManager::appendMessage(const ChatMessage &message)
{
    if (message.id == 0 || m_knownIds.contains(message.id)) {
        return;  // 去重（REST 回执 与 WS 回显可能先后到达）
    }
    m_knownIds.insert(message.id);

    m_messages[message.sessionId].append(message);

    // 更新会话行：预览 + 时间，新消息会话置顶；自己发的消息不增加未读
    if (Conversation *conv = conversation(message.sessionId)) {
        conv->preview = message.text;
        conv->lastMessageAt = message.timestamp;
        if (!message.isSelf && message.sessionId != m_openSessionId) {
            conv->unreadCount += 1;
        }
    }
    sortSessions();
    emit messageAdded(message);
    emit sessionsChanged();  // 列表顺序/预览变化，由界面重建行（保留选中项）
}

void ChatManager::sortSessions()
{
    std::sort(m_sessions.begin(), m_sessions.end(),
              [](const Conversation &a, const Conversation &b) {
                  return a.lastMessageAt > b.lastMessageAt;
              });
}

Conversation *ChatManager::conversation(qint64 sessionId)
{
    for (Conversation &c : m_sessions) {
        if (c.id == sessionId) {
            return &c;
        }
    }
    return nullptr;
}

const Conversation *ChatManager::conversation(qint64 sessionId) const
{
    for (const Conversation &c : m_sessions) {
        if (c.id == sessionId) {
            return &c;
        }
    }
    return nullptr;
}
