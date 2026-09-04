#pragma once

#include <QString>
#include <QWidget>

#include <functional>

#include "core/models.h"

class QLabel;
class QLineEdit;
class QStackedWidget;
class QTimer;
class Button;

// 登录窗口，含账号密码登录和二维码展示页。
// 二维码页仅展示二维码，二维码按固定时间自动刷新。
class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

    // 退出登录后重置登录表单并回到账户登录页。
    void resetForLogout();

signals:
    // 参数为登录成功后的展示名（昵称优先，其次用户名）
    void loginSucceeded(const QString &displayName);

private slots:
    // ---- 账号密码登录 ----
    void onLoginClicked();
    void onLoginSucceeded(const LoginResult &result);
    void onLoginFailed(int httpStatus, int code, const QString &message);

    // ---- 扫码登录 ----
    void refreshQrCode();       // 重新生成场景 ID 与二维码

    // ---- 页面切换（文字链接） ----
    void showPasswordPage();
    void showQrPage();

private:
    QWidget *createPasswordPage();
    QWidget *createQrPage();
    QLabel *createSwitchLink(const QString &text,
                             const std::function<void()> &onActivated) const;
    void renderQrCode(const QString &content);  // 文本 -> 二维码图片

    // 页面容器
    QStackedWidget *m_stack = nullptr;
    QWidget *m_passwordPage = nullptr;
    QWidget *m_qrPage = nullptr;

    // 账号密码页
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    Button *m_loginButton = nullptr;

    // 扫码页（仅展示二维码）
    QLabel *m_qrImage = nullptr;
    QTimer *m_qrRefreshTimer = nullptr;
    QString m_sceneId;
};
