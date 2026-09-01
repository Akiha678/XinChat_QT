#include "ui/login/LoginWindow.h"

#include <QFont>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/Session.h"
#include "network/ApiClient.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("XinChat - 登录"));
    setFixedSize(360, 300);

    auto *title = new QLabel(QStringLiteral("XinChat"), this);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPixelSize(26);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(QStringLiteral("用户名 / 邮箱"));

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText(QStringLiteral("密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_loginButton = new QPushButton(QStringLiteral("登  录"), this);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);

    // 回车键快速提交
    connect(m_usernameEdit, &QLineEdit::returnPressed,
            this, &LoginWindow::onLoginClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed,
            this, &LoginWindow::onLoginClicked);

    // 后端登录结果
    connect(&ApiClient::instance(), &ApiClient::loginSucceeded,
            this, &LoginWindow::onLoginSucceeded);
    connect(&ApiClient::instance(), &ApiClient::loginFailed,
            this, &LoginWindow::onLoginFailed);

    // 测试账号提示（后端种子数据）
    auto *hint = new QLabel(QStringLiteral("测试账号：admin1 / 123456"), this);
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet(QStringLiteral("color: #999999; font-size: 12px;"));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->addWidget(title);
    layout->addSpacing(16);
    layout->addWidget(m_usernameEdit);
    layout->addSpacing(10);
    layout->addWidget(m_passwordEdit);
    layout->addSpacing(20);
    layout->addWidget(m_loginButton);
    layout->addSpacing(8);
    layout->addWidget(hint);
}

void LoginWindow::onLoginClicked()
{
    if (m_loggingIn) {
        return;
    }
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("提示"),
                             QStringLiteral("请输入用户名和密码"));
        return;
    }

    m_loggingIn = true;
    m_loginButton->setEnabled(false);
    m_loginButton->setText(QStringLiteral("登录中..."));

    ApiClient::instance().login(username, password);
}

void LoginWindow::onLoginSucceeded(const LoginResult &result)
{
    m_loggingIn = false;
    m_loginButton->setEnabled(true);
    m_loginButton->setText(QStringLiteral("登  录"));

    // 保存登录态，供后续鉴权接口使用
    Session::instance().setLogin(result);

    const QString name = result.user.displayName.isEmpty()
                             ? result.user.username
                             : result.user.displayName;
    emit loginSucceeded(name);
}

void LoginWindow::onLoginFailed(int /*httpStatus*/, int /*code*/, const QString &message)
{
    m_loggingIn = false;
    m_loginButton->setEnabled(true);
    m_loginButton->setText(QStringLiteral("登  录"));

    QMessageBox::warning(this, QStringLiteral("登录失败"), message);
}
