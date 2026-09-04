#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

#include "core/models.h"

// 聊天数据管理层（真实后端数据）：
//  - 会话列表（GET /chat/session）
//  - 会话消息（POST /chat/message/page，内存缓存）
//  - 发送消息（POST /chat/session/{id}/message）
//  - 实时接收（WS /ws/chat message.created，自动去重，含自己发送的回显）
//
// 界面层只与本类交互，不直接接触网络细节。
class ChatManager : public QObject {
    Q_OBJECT

public:
    explicit ChatManager(QObject *parent = nullptr);

    // 登录成功后调用：建立实时连接（会话列表由 MainWindow 触发 loadSessions）
    void startRealtime();
    void reset();  // 退出登录时清理当前用户的内存状态并停止实时连接

    void loadSessions();
    void loadFriends();
    void searchFriends(const QString &username);
    void addFriend(qint64 userId, const QString &message = QString());
    // 打开会话：拉取历史消息（最新一页），并把未读消息标记为已读
    void openSession(qint64 sessionId);
    // 创建或复用与好友的单聊会话。
    void openConversationWith(qint64 friendId);
    void sendText(qint64 sessionId, const QString &text);

    const QList<Conversation> &conversations() const;   // 按最近消息时间倒序
    const QList<ChatMessage> &messagesOf(qint64 sessionId) const;  // 时间升序
    int unreadTotal() const;

signals:
    void sessionsChanged();                          // 会话列表已刷新/顺序变化
    void sessionMessagesChanged(qint64 sessionId);   // 该会话消息缓存已更新
    void messageAdded(const ChatMessage &message);   // 收到新消息（含自己发送回显）
    void requestError(const QString &operation, const QString &message);
    void contactRequestError(const QString &operation, const QString &message);
    void conversationOpened(const Conversation &conversation);
    void friendsChanged(const QList<UserSummary> &friends);
    void friendSearchResultsChanged(const QList<UserSummary> &users);
    void friendRequestSent(const FriendRequest &request);

private:
    void onSessionsLoaded(const QList<Conversation> &sessions);
    void onMessagesLoaded(qint64 sessionId, const QList<ChatMessage> &messages, int total);
    void onMessageSent(const ChatMessage &message);
    void onIncomingMessage(const ChatMessage &message);
    void onConversationCreated(const Conversation &conversation);
    void onFriendsLoaded(const QList<UserSummary> &friends);
    void onFriendSearchResultsLoaded(const QList<UserSummary> &users);
    void onFriendRequestSent(const FriendRequest &request);
    void appendMessage(const ChatMessage &message);   // 去重后入缓存并更新会话行
    void sortSessions();
    Conversation *conversation(qint64 sessionId);
    const Conversation *conversation(qint64 sessionId) const;

    QList<Conversation> m_sessions;
    QHash<qint64, QList<ChatMessage>> m_messages;  // sessionId -> 时间升序
    QSet<qint64> m_knownIds;                        // 已见过的消息 ID（去重）
    qint64 m_openSessionId = 0;
    bool m_sessionsLoaded = false;
};
