#pragma once

#include <QString>
#include <QWidget>

#include <functional>

#include "core/models.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QStackedWidget;
class QTimer;
class Button;

// 登录窗口，含两种登录方式（微信风格：窗口底部文字链接切换，非页签）：
//   1. 账号登录：表单页，底部文字链接"扫码登录"
//   2. 扫码登录（演示模式）：客户端本地生成二维码，内置"模拟手机端"面板
//      （后端暂无扫码接口，扫描/确认均为模拟；确认后走真实登录获取 token），
//      底部文字链接"账号登录"可切回
class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    // 参数为登录成功后的展示名（昵称优先，其次用户名）
    void loginSucceeded(const QString &displayName);

private slots:
    // ---- 账号密码登录 ----
    void onLoginClicked();
    void onLoginSucceeded(const LoginResult &result);
    void onLoginFailed(int httpStatus, int code, const QString &message);

    // ---- 扫码登录（演示） ----
    void onDemoScanClicked();   // 模拟手机端：扫码 -> 确认登录
    void refreshQrCode();       // 重新生成场景 ID 与二维码

    // ---- 页面切换（文字链接） ----
    void showPasswordPage();
    void showQrPage();

private:
    QWidget *createPasswordPage();
    QWidget *createQrPage();
    QLabel *createSwitchLink(const QString &text,
                             const std::function<void()> &onActivated) const;
    void setQrStatus(const QString &text);   // 更新扫码状态提示
    void resetQrDemo();                       // 回到"等待扫码"状态
    void renderQrCode(const QString &content);  // 文本 -> 二维码图片

    // 页面容器
    QStackedWidget *m_stack = nullptr;
    QWidget *m_passwordPage = nullptr;
    QWidget *m_qrPage = nullptr;

    // 账号密码页
    QLineEdit *m_usernameEdit = nullptr;
    QLineEdit *m_passwordEdit = nullptr;
    Button *m_loginButton = nullptr;

    // 扫码页（演示）
    QLabel *m_qrImage = nullptr;
    QLabel *m_qrStatus = nullptr;
    QLabel *m_qrSceneLabel = nullptr;
    QComboBox *m_demoAccountCombo = nullptr;
    Button *m_demoScanButton = nullptr;
    QTimer *m_qrRefreshTimer = nullptr;
    QString m_sceneId;
    bool m_scanned = false;     // 模拟手机是否已扫码（未确认）
    bool m_confirming = false;  // 是否正在执行确认登录
};
