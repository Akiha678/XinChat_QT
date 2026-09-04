#pragma once

#include <QColor>
#include <QDateTime>
#include <QString>

// 一条聊天消息（对应后端 MsgResponse）
struct ChatMessage {
    qint64 id = 0;           // 消息 ID
    qint64 sessionId = 0;    // 所属会话 ID
    qint64 senderId = 0;     // 发送者用户 ID
    QString nickName;        // 发送者昵称
    QString text;            // 文本内容（content.data）
    bool isSelf = false;     // 是否自己发送（后端 type==0）
    bool isRead = false;     // 是否已读（status==1）
    QDateTime timestamp;     // 发送时间（createTime）
};

// 会话（对应后端 ConversationResponse）
struct Conversation {
    qint64 id = 0;           // 会话 ID（发消息/拉历史用）
    qint64 peerId = 0;       // 对端用户 ID
    QString name;            // 会话展示名（单聊 = 对方昵称）
    QString preview;         // 最近一条消息预览
    QDateTime lastMessageAt; // 最近消息时间
    int unreadCount = 0;     // 未读数
    int colorSeed = 0;       // 头像颜色种子（整数索引，客户端映射为色板）
};

// 当前登录用户信息（对应后端 LoginResponse）
struct UserInfo {
    qint64 id = 0;             // 用户 ID
    QString username;          // 登录账号（可能为邮箱）
    QString displayName;       // 昵称
    QString email;             // 邮箱
    int avatarColor = 0;       // 头像色值（后端为 int 索引）
};

// 用户摘要（对应后端 UserSummaryResponse，用于通讯录及好友搜索）。
struct UserSummary {
    qint64 id = 0;
    QString name;
    QString username;
    QString email;
    int avatarColor = 0;
};

// 好友申请（对应后端 FriendRequestResponse）。
struct FriendRequest {
    qint64 id = 0;
    UserSummary requester;
    UserSummary addressee;
    QString status;
    QString message;
    QDateTime createdAt;
};

// 登录成功后的结果
struct LoginResult {
    UserInfo user;
    QString accessToken;       // 登录令牌（后续请求放入 Authorization 头）
    QDateTime expiresAt;       // 令牌过期时间；无效时间表示未知
};
