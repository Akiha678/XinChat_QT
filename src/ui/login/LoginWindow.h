#pragma once

#include <QWidget>
#include <QString>

#include "core/models.h"

class QLabel;
class QLineEdit;
class Button;

// 登录窗口：调用后端 POST /auth/login 完成真实登录。
// 成功 -> 写入 Session 并发出 loginSucceeded；失败 -> 弹窗提示。
class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    // 参数为登录成功后的展示名（昵称优先，其次用户名）
    void loginSucceeded(const QString &displayName);

private slots:
    void onLoginClicked();
    void onLoginSucceeded(const LoginResult &result);
    void onLoginFailed(int httpStatus, int code, const QString &message);

private:
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    Button *m_loginButton = nullptr;
};
