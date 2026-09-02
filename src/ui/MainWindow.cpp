#include "ui/MainWindow.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollBar>
#include <QSize>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include "core/ChatManager.h"
#include "ui/color/Theme.h"
#include "ui/components/button/button.h"
#include "ui/components/conversationitem/ConversationItem.h"
#include "ui/components/messagebubble/MessageBubble.h"

namespace {
constexpr int kConversationItemHeight = 64;
constexpr int kNavWidth = 64;
constexpr int kConversationListWidth = 260;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_chat = new ChatManager(this);

    setWindowTitle(QStringLiteral("XinChat"));
    resize(1000, 700);

    // ---- 左侧导航栏 ----
    m_navList = new QListWidget(this);
    m_navList->setObjectName(QStringLiteral("navList"));
    m_navList->setFixedWidth(kNavWidth);
    m_navList->setSpacing(2);
    m_navList->addItem(QStringLiteral("💬\n消息"));
    m_navList->addItem(QStringLiteral("👥\n通讯录"));
    m_navList->addItem(QStringLiteral("⚙️\n设置"));
    for (int i = 0; i < m_navList->count(); ++i) {
        m_navList->item(i)->setSizeHint(QSize(kNavWidth, 56));
    }
    m_navList->setCurrentRow(0);
    connect(m_navList, &QListWidget::currentRowChanged,
            this, &MainWindow::onNavChanged);

    // ---- 右侧页面栈（聊天 / 通讯录 / 设置） ----
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createChatPage());
    m_stack->addWidget(createContactsPage());
    m_stack->addWidget(createSettingsPage());

    auto *central = new QWidget(this);
    auto *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(m_navList);
    rootLayout->addWidget(m_stack, 1);
    setCentralWidget(central);

    // 数据层信号 -> 界面刷新
    connect(m_chat, &ChatManager::sessionsChanged,
            this, &MainWindow::onSessionsChanged);
    connect(m_chat, &ChatManager::sessionMessagesChanged,
            this, &MainWindow::onSessionMessagesChanged);
    connect(m_chat, &ChatManager::messageAdded,
            this, &MainWindow::onMessageAdded);
    connect(m_chat, &ChatManager::requestError,
            this, &MainWindow::onRequestError);

    // 主题变化：刷新导航图标与列表项颜色
    connect(&Theme::instance(), &Theme::themeChanged,
            this, &MainWindow::onThemeChanged);
}

void MainWindow::setCurrentUser(const QString &displayName)
{
    m_currentUserName = displayName;
    setWindowTitle(QStringLiteral("XinChat - %1").arg(displayName));
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_started) {
        m_started = true;
        // 登录完成：建立实时通道并拉取会话列表
        m_chat->startRealtime();
        m_chat->loadSessions();
    }
}

QWidget *MainWindow::createChatPage()
{
    // 会话列表（中栏）
    m_conversationList = new QListWidget(this);
    m_conversationList->setObjectName(QStringLiteral("conversationList"));
    m_conversationList->setFixedWidth(kConversationListWidth);
    m_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_conversationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_conversationList, &QListWidget::currentItemChanged,
            this, [this](QListWidgetItem *current, QListWidgetItem *previous) {
                Q_UNUSED(previous);
                if (current) {
                    onConversationSelected(current);
                }
            });

    // 聊天区（右栏）
    m_chatHeader = new QLabel(QStringLiteral("请选择会话"), this);
    m_chatHeader->setObjectName(QStringLiteral("chatHeader"));
    m_chatHeader->setFixedHeight(50);
    m_chatHeader->setAlignment(Qt::AlignCenter);

    m_messageList = new QListWidget(this);
    m_messageList->setObjectName(QStringLiteral("messageList"));
    m_messageList->setSelectionMode(QAbstractItemView::NoSelection);
    m_messageList->setFocusPolicy(Qt::NoFocus);
    m_messageList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_messageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageList->setSpacing(6);

    m_inputEdit = new QTextEdit(this);
    m_inputEdit->setObjectName(QStringLiteral("inputEdit"));
    m_inputEdit->setFixedHeight(110);
    m_inputEdit->setPlaceholderText(QStringLiteral("输入消息，Ctrl+Enter 发送"));

    auto *sendButton = new QPushButton(QStringLiteral("发送"), this);
    sendButton->setFixedWidth(80);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);

    auto *inputRow = new QHBoxLayout;
    inputRow->setContentsMargins(12, 8, 12, 8);
    inputRow->addWidget(m_inputEdit, 1);
    inputRow->addWidget(sendButton, 0, Qt::AlignBottom);

    auto *chatArea = new QWidget(this);
    auto *chatLayout = new QVBoxLayout(chatArea);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);
    chatLayout->addWidget(m_chatHeader);
    chatLayout->addWidget(m_messageList, 1);
    chatLayout->addLayout(inputRow);

    // 中栏 + 右栏 分栏
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_conversationList);
    splitter->addWidget(chatArea);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({kConversationListWidth, 740});
    return splitter;
}

QWidget *MainWindow::createContactsPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    auto *label = new QLabel(QStringLiteral("通讯录\n\n（好友列表后续接入 GET /contact/friends）"), page);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    return page;
}

QWidget *MainWindow::createSettingsPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(60, 40, 60, 40);

    auto *title = new QLabel(QStringLiteral("设置"), page);
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *userLabel = new QLabel(page);
    userLabel->setObjectName(QStringLiteral("settingsUserLabel"));
    userLabel->setText(QStringLiteral("当前用户：%1").arg(m_currentUserName));

    m_themeToggle = new Button(QString(), page);
    connect(m_themeToggle, &Button::clicked, this, &MainWindow::onToggleTheme);
    updateThemeToggleText();

    layout->addWidget(title);
    layout->addSpacing(20);
    layout->addWidget(userLabel);
    layout->addSpacing(30);
    layout->addWidget(m_themeToggle);
    layout->addStretch();
    return page;
}

void MainWindow::setChatHeader(const QString &text)
{
    m_chatHeader->setText(text);
}

void MainWindow::rebuildConversationList()
{
    const qint64 keepSessionId = m_currentSessionId;

    m_conversationList->clear();
    const QList<Conversation> &conversations = m_chat->conversations();
    for (const Conversation &conv : conversations) {
        auto *item = new QListWidgetItem(m_conversationList);
        item->setData(Qt::UserRole, QVariant::fromValue<qint64>(conv.id));
        item->setSizeHint(QSize(0, kConversationItemHeight));
        m_conversationList->setItemWidget(item, new ConversationItem(conv));
    }

    if (conversations.isEmpty()) {
        setChatHeader(QStringLiteral("暂无会话"));
        m_messageList->clear();
        return;
    }

    // 恢复之前的选中会话，否则默认选第一个（触发拉取消息）
    QListWidgetItem *target = nullptr;
    if (keepSessionId != 0) {
        target = conversationItemOf(keepSessionId);
    }
    if (!target) {
        target = m_conversationList->item(0);
    }
    if (target) {
        m_conversationList->setCurrentItem(target);
    }
}

void MainWindow::onNavChanged(int row)
{
    m_stack->setCurrentIndex(row);
}

void MainWindow::onConversationSelected(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    m_currentSessionId = item->data(Qt::UserRole).toLongLong();
    const QList<Conversation> &all = m_chat->conversations();
    for (const Conversation &conv : all) {
        if (conv.id == m_currentSessionId) {
            setChatHeader(conv.name);
            break;
        }
    }
    m_chat->openSession(m_currentSessionId);
}

void MainWindow::reloadMessages()
{
    m_messageList->clear();
    const QList<ChatMessage> &messages = m_chat->messagesOf(m_currentSessionId);
    for (const ChatMessage &msg : messages) {
        appendMessage(msg);
    }
}

void MainWindow::appendMessage(const ChatMessage &message)
{
    auto *bubble = new MessageBubble(message.text,
                                     message.isSelf ? MessageBubble::Self
                                                    : MessageBubble::Other);
    auto *item = new QListWidgetItem(m_messageList);
    item->setSizeHint(bubble->sizeHint());
    m_messageList->setItemWidget(item, bubble);
    m_messageList->scrollToBottom();
}

void MainWindow::onSendClicked()
{
    if (m_currentSessionId == 0) {
        return;
    }
    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }
    m_chat->sendText(m_currentSessionId, text);
    m_inputEdit->clear();
}

void MainWindow::onSessionsChanged()
{
    rebuildConversationList();
}

void MainWindow::onSessionMessagesChanged(qint64 sessionId)
{
    if (sessionId == m_currentSessionId) {
        reloadMessages();
    }
}

void MainWindow::onMessageAdded(const ChatMessage &message)
{
    if (message.sessionId == m_currentSessionId) {
        appendMessage(message);
    }
}

void MainWindow::onRequestError(const QString &operation, const QString &message)
{
    qWarning() << "[MainWindow]" << operation << "失败:" << message;
    // 简单提示：标题栏短暂提示或仅在无会话时展示错误
    if (m_chat->conversations().isEmpty()) {
        setChatHeader(QStringLiteral("%1失败：%2").arg(operation, message));
    }
}

QListWidgetItem *MainWindow::conversationItemOf(qint64 sessionId) const
{
    for (int i = 0; i < m_conversationList->count(); ++i) {
        QListWidgetItem *item = m_conversationList->item(i);
        if (item->data(Qt::UserRole).toLongLong() == sessionId) {
            return item;
        }
    }
    return nullptr;
}

// ---------- 主题相关 ----------

void MainWindow::refreshItemWidgets()
{
    const auto refresh = [](QListWidget *list) {
        for (int i = 0; i < list->count(); ++i) {
            if (QWidget *w = list->itemWidget(list->item(i))) {
                w->update();  // 气泡/会话行的 paintEvent 会读取新色板
            }
        }
    };
    refresh(m_conversationList);
    refresh(m_messageList);
}

void MainWindow::updateThemeToggleText()
{
    if (!m_themeToggle) {
        return;
    }
    m_themeToggle->setText(
        Theme::instance().scheme() == Theme::Scheme::Dark
            ? QStringLiteral("切换到浅色模式")
            : QStringLiteral("切换到深色模式"));
}

void MainWindow::onToggleTheme()
{
    Theme::instance().setScheme(
        Theme::instance().scheme() == Theme::Scheme::Dark ? Theme::Scheme::Light
                                                          : Theme::Scheme::Dark);
}

void MainWindow::onThemeChanged()
{
    refreshItemWidgets();
    updateThemeToggleText();
}
