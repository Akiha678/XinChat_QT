#pragma once

#include <QColor>
#include <QDateTime>
#include <QString>

// 一条聊天消息
struct ChatMessage {
    QString senderName;   // 发送者昵称
    QString text;         // 消息内容（后续可扩展 type 字段支持图片/文件/语音）
    bool isSelf = false;  // 是否是自己发出的
    QDateTime timestamp;  // 发送时间
};

// 会话列表里的一行（好友/群）
struct Conversation {
    QString friendName;    // 好友或群名称
    QString lastMessage;   // 最后一条消息预览
    QString timeText;      // 显示时间，如 "10:24" / "昨天"
    QColor avatarColor;    // 头像底色（暂无头像图片，用首字 + 颜色代替）
};

// 当前登录用户信息（对应后端 LoginResponse）
struct UserInfo {
    qint64 id = 0;             // 用户 ID
    QString username;          // 登录账号（可能为邮箱）
    QString displayName;       // 昵称
    QString email;             // 邮箱
    int avatarColor = 0;       // 头像色值（后端为 int 索引）
};

// 登录成功后的结果
struct LoginResult {
    UserInfo user;
    QString accessToken;       // 登录令牌（后续请求放入 Authorization 头）
    QDateTime expiresAt;       // 令牌过期时间；无效时间表示未知
};
