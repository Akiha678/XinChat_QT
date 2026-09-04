#pragma once

#include <QList>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

#include "core/models.h"

class QNetworkAccessManager;
class QNetworkReply;

// 后端 REST API 客户端（基于 QNetworkAccessManager）。
// 采用单例：登录窗口、聊天等模块共用同一个网络管理器。
//
// 已实现接口：
//   - POST /auth/login                   用户名密码登录
//   - GET  /chat/session                 会话列表（需登录态）
//   - POST /chat/message/page            会话消息分页
//   - POST /chat/session/{id}/message    发送消息
//   - POST /chat/message/read            标记已读
//   - GET  /contact/users/search         搜索用户
//   - GET  /contact/friends              好友列表
//   - POST /contact/friend-requests      发送好友申请
//   - POST /chat/conversation             创建单聊会话
// 需要登录态的请求会自动携带 Authorization: Bearer <token>（token 取自 Session）。
class ApiClient : public QObject {
    Q_OBJECT

public:
    static ApiClient &instance();

    // 登录（无需登录态）
    void login(const QString &username, const QString &password);

    // ---- 聊天接口（需登录态） ----
    void fetchSessions();                              // 会话列表
    void fetchMessages(qint64 sessionId, int page = 1, int size = 50);
    void sendMessage(qint64 sessionId, const QString &content);
    void markMessagesRead(const QList<qint64> &messageIds);

    // ---- 好友接口（需登录态） ----
    void fetchFriends();
    void searchUsers(const QString &username);
    void sendFriendRequest(qint64 addresseeId, const QString &message = QString());

    // 创建/复用与好友的单聊会话（需登录态）
    void createDirectConversation(qint64 friendId);

    // 服务端基础地址（可用环境变量 XINCHAT_API_BASE 覆盖）
    static QString baseUrl();

signals:
    void loginSucceeded(const LoginResult &result);
    // httpStatus: HTTP 状态码（网络错误时为 0）；code: 业务 code；message: 错误描述
    void loginFailed(int httpStatus, int code, const QString &message);

    // ---- 聊天结果 ----
    void sessionsLoaded(const QList<Conversation> &sessions);
    void messagesLoaded(qint64 sessionId,
                        const QList<ChatMessage> &messages,  // 后端最新在前，未排序
                        int total);
    void messageSent(const ChatMessage &message);
    // 聊天类请求失败（operation: 如"会话列表"；message: 错误描述）
    void chatRequestFailed(const QString &operation, const QString &message);

    // ---- 好友结果 ----
    void friendsLoaded(const QList<UserSummary> &friends);
    void usersSearchLoaded(const QList<UserSummary> &users);
    void friendRequestSent(const FriendRequest &request);
    void conversationCreated(const Conversation &conversation);
    void contactRequestFailed(const QString &operation, const QString &message);

private:
    explicit ApiClient(QObject *parent = nullptr);

    void handleLoginReply(QNetworkReply *reply);
    void handleSessionsReply(QNetworkReply *reply);
    void handleMessagesReply(qint64 sessionId, QNetworkReply *reply);
    void handleSendReply(QNetworkReply *reply);
    void handleFriendsReply(QNetworkReply *reply);
    void handleSearchUsersReply(QNetworkReply *reply);
    void handleFriendRequestReply(QNetworkReply *reply);
    void handleCreateConversationReply(QNetworkReply *reply);

    QNetworkRequest makeAuthedRequest(const QString &path) const;

    QNetworkAccessManager *m_nam = nullptr;
};
