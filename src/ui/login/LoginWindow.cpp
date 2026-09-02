#include "ui/login/LoginWindow.h"

#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QStackedWidget>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>

#include "core/Session.h"
#include "network/ApiClient.h"
#include "third_party/qrcodegen/qrcodegen.hpp"
#include "ui/components/button/button.h"
#include "ui/components/dialog/dialog.h"

namespace {
const QString kDemoPassword = QStringLiteral("123456");  // 后端种子账号密码

// 文字链接（绿色，无下划线），点击触发回调 —— 用于登录方式切换
const char kLinkStyle[] =
    "<a href=\"switch\" style=\"color:#07C160; text-decoration:none;\">%1</a>";
}

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("XinChat - 登录"));
    setFixedSize(400, 470);

    // 两个登录页面放在一个栈里，通过底部文字链接切换（不是页签）
    m_stack = new QStackedWidget(this);
    m_passwordPage = createPasswordPage();
    m_qrPage = createQrPage();
    m_stack->addWidget(m_passwordPage);
    m_stack->addWidget(m_qrPage);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    // 后端登录结果（账号登录 / 扫码登录共用同一套回调）
    connect(&ApiClient::instance(), &ApiClient::loginSucceeded,
            this, &LoginWindow::onLoginSucceeded);
    connect(&ApiClient::instance(), &ApiClient::loginFailed,
            this, &LoginWindow::onLoginFailed);
}

QLabel *LoginWindow::createSwitchLink(const QString &text,
                                      const std::function<void()> &onActivated) const
{
    auto *link = new QLabel(QString::fromLatin1(kLinkStyle).arg(text));
    link->setAlignment(Qt::AlignCenter);
    link->setCursor(Qt::PointingHandCursor);
    link->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    connect(link, &QLabel::linkActivated, link,
            [onActivated](const QString & /*href*/) { onActivated(); });
    return link;
}

// ---------- 页面一：账号密码登录 ----------

QWidget *LoginWindow::createPasswordPage()
{
    auto *page = new QWidget(this);

    auto *title = new QLabel(QStringLiteral("XinChat"), page);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPixelSize(26);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_usernameEdit = new QLineEdit(page);
    m_usernameEdit->setPlaceholderText(QStringLiteral("用户名 / 邮箱"));

    m_passwordEdit = new QLineEdit(page);
    m_passwordEdit->setPlaceholderText(QStringLiteral("密码"));
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_loginButton = new Button(QStringLiteral("登  录"), page);
    connect(m_loginButton, &Button::clicked, this, &LoginWindow::onLoginClicked);

    // 回车键快速提交
    connect(m_usernameEdit, &QLineEdit::returnPressed,
            this, &LoginWindow::onLoginClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed,
            this, &LoginWindow::onLoginClicked);

    // 底部文字链接：切换到扫码登录（微信风格）
    auto *qrLink = createSwitchLink(QStringLiteral("扫码登录"),
                                    [this]() { showQrPage(); });

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 40, 40, 24);
    layout->addStretch(1);
    layout->addWidget(title);
    layout->addSpacing(16);
    layout->addWidget(m_usernameEdit);
    layout->addSpacing(10);
    layout->addWidget(m_passwordEdit);
    layout->addSpacing(20);
    layout->addWidget(m_loginButton);
    layout->addSpacing(12);
    layout->addStretch(2);
    layout->addWidget(qrLink);
    return page;
}

// ---------- 页面二：扫码登录（演示模式） ----------

QWidget *LoginWindow::createQrPage()
{
    auto *page = new QWidget(this);

    auto *title = new QLabel(QStringLiteral("使用 XinChat 手机端扫码登录"), page);
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont = title->font();
    titleFont.setPixelSize(15);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_qrStatus = new QLabel(QStringLiteral("请使用手机 XinChat 扫描下方二维码"), page);
    m_qrStatus->setAlignment(Qt::AlignCenter);
    m_qrStatus->setStyleSheet(QStringLiteral("color: #666666;"));

    m_qrImage = new QLabel(page);
    m_qrImage->setFixedSize(200, 200);
    m_qrImage->setAlignment(Qt::AlignCenter);
    m_qrImage->setStyleSheet(QStringLiteral("border: 1px solid #DDDDDD;"));

    m_qrSceneLabel = new QLabel(page);
    m_qrSceneLabel->setAlignment(Qt::AlignCenter);
    m_qrSceneLabel->setStyleSheet(QStringLiteral("color: #999999; font-size: 11px;"));

    // 模拟手机端：当前"手机"上已登录的账号
    m_demoAccountCombo = new QComboBox(page);
    m_demoAccountCombo->addItem(QStringLiteral("管理员一 (admin1)"), QStringLiteral("admin1"));
    m_demoAccountCombo->addItem(QStringLiteral("管理员二 (admin2)"), QStringLiteral("admin2"));

    m_demoScanButton = new Button(QStringLiteral("模拟手机扫码"), page);
    connect(m_demoScanButton, &Button::clicked,
            this, &LoginWindow::onDemoScanClicked);

    auto *row = new QHBoxLayout;
    row->addWidget(m_demoAccountCombo, 1);
    row->addWidget(m_demoScanButton, 1);

    // 底部文字链接：切回账号登录（微信风格）
    auto *passwordLink = createSwitchLink(QStringLiteral("账号登录"),
                                          [this]() { showPasswordPage(); });

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 24, 40, 24);
    layout->addWidget(title);
    layout->addSpacing(8);
    layout->addWidget(m_qrStatus);
    layout->addSpacing(6);
    layout->addWidget(m_qrImage, 0, Qt::AlignHCenter);
    layout->addSpacing(4);
    layout->addWidget(m_qrSceneLabel);
    layout->addSpacing(10);
    layout->addLayout(row);
    layout->addSpacing(6);
    layout->addStretch(1);
    layout->addWidget(passwordLink);

    // 二维码 30 秒自动刷新（模拟真实产品的过期机制）
    m_qrRefreshTimer = new QTimer(this);
    m_qrRefreshTimer->setInterval(30000);
    connect(m_qrRefreshTimer, &QTimer::timeout, this, &LoginWindow::refreshQrCode);
    m_qrRefreshTimer->start();

    refreshQrCode();  // 生成初始二维码
    return page;
}

// ---------- 页面切换 ----------

void LoginWindow::showPasswordPage()
{
    m_stack->setCurrentWidget(m_passwordPage);
}

void LoginWindow::showQrPage()
{
    m_stack->setCurrentWidget(m_qrPage);
    refreshQrCode();  // 每次进入扫码页都换一个新二维码（防过期）
}

// ---------- 扫码状态与二维码渲染 ----------

void LoginWindow::setQrStatus(const QString &text)
{
    m_qrStatus->setText(text);
}

void LoginWindow::resetQrDemo()
{
    m_scanned = false;
    m_confirming = false;
    m_demoAccountCombo->setEnabled(true);
    m_demoScanButton->setLoading(false);  // 恢复"模拟手机扫码"文案
    setQrStatus(QStringLiteral("请使用手机 XinChat 扫描下方二维码"));
}

void LoginWindow::refreshQrCode()
{
    if (m_confirming) {
        return;  // 确认登录过程中不刷新二维码
    }
    // 每次刷新生成新的场景 ID（真实实现中后端据此识别扫码会话）
    m_sceneId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    resetQrDemo();
    m_qrSceneLabel->setText(
        QStringLiteral("场景ID: %1（30秒自动刷新）").arg(m_sceneId.left(8)));
    renderQrCode(QStringLiteral("xinchat://scan/login?scene=%1").arg(m_sceneId));
}

void LoginWindow::renderQrCode(const QString &content)
{
    const qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(
        content.toUtf8().constData(), qrcodegen::QrCode::Ecc::MEDIUM);

    constexpr int kQuietZone = 4;  // 二维码四周静区（模块数）
    const int moduleCount = qr.getSize();
    const int targetPx = 190;      // 目标图片尺寸
    const int modulePx = qMax(1, targetPx / (moduleCount + kQuietZone * 2));
    const int imagePx = modulePx * (moduleCount + kQuietZone * 2);

    QImage image(imagePx, imagePx, QImage::Format_RGB32);
    image.fill(Qt::white);
    for (int y = 0; y < moduleCount; ++y) {
        for (int x = 0; x < moduleCount; ++x) {
            if (!qr.getModule(x, y)) {
                continue;
            }
            const int px = (x + kQuietZone) * modulePx;
            const int py = (y + kQuietZone) * modulePx;
            for (int dy = 0; dy < modulePx; ++dy) {
                for (int dx = 0; dx < modulePx; ++dx) {
                    image.setPixelColor(px + dx, py + dy, Qt::black);
                }
            }
        }
    }
    m_qrImage->setPixmap(QPixmap::fromImage(image));
}

void LoginWindow::onDemoScanClicked()
{
    if (m_confirming) {
        return;
    }

    if (!m_scanned) {
        // 第一步：模拟手机扫码成功
        m_scanned = true;
        m_demoAccountCombo->setEnabled(false);  // 扫码后锁定所选账号
        m_demoScanButton->setText(QStringLiteral("在手机上确认登录"));
        setQrStatus(QStringLiteral("已扫码，请在手机上确认登录"));
        return;
    }

    // 第二步：模拟手机确认 -> 走真实登录换取 token
    m_confirming = true;
    m_demoScanButton->setLoading(true, QStringLiteral("确认中..."));
    setQrStatus(QStringLiteral("已确认，正在登录..."));

    const QString account = m_demoAccountCombo->currentData().toString();
    ApiClient::instance().login(account, kDemoPassword);
}

// ---------- 共用：登录结果处理 ----------

void LoginWindow::onLoginClicked()
{
    if (m_loginButton->isLoading()) {
        return;  // 防重复提交
    }
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        Dialog::warning(this, QStringLiteral("提示"),
                        QStringLiteral("请输入用户名和密码"));
        return;
    }

    m_loginButton->setLoading(true, QStringLiteral("登录中..."));

    ApiClient::instance().login(username, password);
}

void LoginWindow::onLoginSucceeded(const LoginResult &result)
{
    // 两种登录方式共用的收尾：恢复按钮状态
    m_loginButton->setLoading(false);
    m_demoScanButton->setLoading(false);
    m_confirming = false;

    // 保存登录态，供后续鉴权接口使用
    Session::instance().setLogin(result);

    const QString name = result.user.displayName.isEmpty()
                             ? result.user.username
                             : result.user.displayName;
    emit loginSucceeded(name);
}

void LoginWindow::onLoginFailed(int /*httpStatus*/, int /*code*/, const QString &message)
{
    m_loginButton->setLoading(false);
    m_demoScanButton->setLoading(false);
    m_confirming = false;
    resetQrDemo();  // 扫码登录失败则回到"等待扫码"状态

    Dialog::warning(this, QStringLiteral("登录失败"), message);
}
