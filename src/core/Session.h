#pragma once

#include "core/models.h"

// 全局会话：保存登录结果（用户信息 + 访问令牌）。
// 登录成功后由 LoginWindow 写入，聊天/通讯录等模块从 Token 读取鉴权信息。
class Session {
public:
    static Session &instance();

    void setLogin(const LoginResult &login);
    void clear();

    bool isLoggedIn() const;
    const LoginResult &login() const;
    const QString &token() const;

private:
    Session() = default;

    LoginResult m_login;
};
