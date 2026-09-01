#pragma once

#include <QObject>
#include <QString>

#include "core/models.h"

class QNetworkAccessManager;

// 后端 REST API 客户端（基于 QNetworkAccessManager）。
// 采用单例：登录窗口、聊天、通讯录等模块共用同一个网络管理器。
//
// 当前实现：
//   - POST /auth/login  用户名密码登录（返回 LoginResponse）
// 后续阶段在此扩展注册、会话、消息等接口。
class ApiClient : public QObject {
    Q_OBJECT

public:
    static ApiClient &instance();

    // 异步登录，结果通过 loginSucceeded / loginFailed 信号返回
    void login(const QString &username, const QString &password);

    // 服务端基础地址（可用环境变量 XINCHAT_API_BASE 覆盖）
    static QString baseUrl();

signals:
    void loginSucceeded(const LoginResult &result);
    // httpStatus: HTTP 状态码（网络错误时为 0）；code: 业务 code；message: 错误描述
    void loginFailed(int httpStatus, int code, const QString &message);

private:
    explicit ApiClient(QObject *parent = nullptr);

    void handleLoginReply(class QNetworkReply *reply);

    QNetworkAccessManager *m_nam = nullptr;
};
