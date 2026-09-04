#include "ui/MainWindow.h"

#include <QAbstractItemView>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
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
    m_navList->addItem(QStringLiteral("消息"));
    m_navList->addItem(QStringLiteral("通讯录"));
    m_navList->addItem(QStringLiteral("设置"));
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
    connect(m_chat, &ChatManager::contactRequestError,
            this, &MainWindow::onContactRequestError);
    connect(m_chat, &ChatManager::friendsChanged,
            this, &MainWindow::onFriendsLoaded);
    connect(m_chat, &ChatManager::friendSearchResultsChanged,
            this, &MainWindow::onUsersSearchLoaded);
    connect(m_chat, &ChatManager::friendRequestSent,
            this, &MainWindow::onFriendRequestSent);
    connect(m_chat, &ChatManager::conversationOpened,
            this, &MainWindow::onConversationOpened);

    // 主题变化：刷新导航图标与列表项颜色
    connect(&Theme::instance(), &Theme::themeChanged,
            this, &MainWindow::onThemeChanged);
}

// 当前用户
void MainWindow::setCurrentUser(const QString &displayName)
{
    m_currentUserName = displayName;
    setWindowTitle(QStringLiteral("XinChat - %1").arg(displayName));
    if (m_settingsUserLabel) {
        m_settingsUserLabel->setText(QStringLiteral("当前用户：%1").arg(displayName));
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_started) {
        m_started = true;
        // 建立连接
        m_chat->startRealtime();
        // 加载消息列表
        m_chat->loadSessions();
        m_chat->loadFriends();
    }
}

// 聊天列表
QWidget *MainWindow::createChatPage()
{
    // 会话列表（中栏）
    m_conversationList = new QListWidget(this);
    m_conversationList->setObjectName(QStringLiteral("conversationList"));
    m_conversationList->setFixedWidth(kConversationListWidth);
    m_conversationList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_conversationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 监听当前选择的会话
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

    // 输入消息框
    m_inputEdit = new QTextEdit(this);
    m_inputEdit->setObjectName(QStringLiteral("inputEdit"));
    m_inputEdit->setFixedHeight(110);
    m_inputEdit->setPlaceholderText(QStringLiteral("请输入消息"));

    // 发送按钮
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

// 好友界面
QWidget *MainWindow::createContactsPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 24, 28, 24);
    layout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("通讯录"), page);
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto *searchRow = new QHBoxLayout;
    m_friendSearchEdit = new QLineEdit(page);
    m_friendSearchEdit->setPlaceholderText(QStringLiteral("输入用户名搜索好友"));
    m_friendSearchButton = new QPushButton(QStringLiteral("搜索"), page);
    m_friendSearchButton->setFixedWidth(80);
    connect(m_friendSearchButton, &QPushButton::clicked,
            this, &MainWindow::onSearchFriendsClicked);
    connect(m_friendSearchEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onSearchFriendsClicked);
    searchRow->addWidget(m_friendSearchEdit, 1);
    searchRow->addWidget(m_friendSearchButton);
    layout->addLayout(searchRow);

    m_contactStatus = new QLabel(page);
    m_contactStatus->setStyleSheet(QStringLiteral("color: #888888;"));
    layout->addWidget(m_contactStatus);

    m_searchResultList = new QListWidget(page);
    m_searchResultList->setObjectName(QStringLiteral("contactSearchResults"));
    m_searchResultList->setSelectionMode(QAbstractItemView::NoSelection);
    m_searchResultList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_searchResultList->setVisible(false);
    layout->addWidget(m_searchResultList);

    auto *friendsTitle = new QLabel(QStringLiteral("我的好友"), page);
    QFont friendsTitleFont = friendsTitle->font();
    friendsTitleFont.setBold(true);
    friendsTitle->setFont(friendsTitleFont);
    layout->addWidget(friendsTitle);

    m_friendList = new QListWidget(page);
    m_friendList->setObjectName(QStringLiteral("friendsList"));
    m_friendList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_friendList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_friendList, &QListWidget::itemClicked,
            this, &MainWindow::onFriendItemClicked);
    layout->addWidget(m_friendList, 1);
    return page;
}

// 设置界面
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

    m_settingsUserLabel = new QLabel(page);
    m_settingsUserLabel->setObjectName(QStringLiteral("settingsUserLabel"));
    m_settingsUserLabel->setText(QStringLiteral("当前用户：%1").arg(m_currentUserName));

    m_themeToggle = new Button(QString(), page);
    connect(m_themeToggle, &Button::clicked, this, &MainWindow::onToggleTheme);
    updateThemeToggleText();

    m_logoutButton = new Button(QStringLiteral("退出登录"), page);
    connect(m_logoutButton, &Button::clicked, this, &MainWindow::onLogoutClicked);

    layout->addWidget(title);
    layout->addSpacing(20);
    layout->addWidget(m_settingsUserLabel);
    layout->addSpacing(30);
    layout->addWidget(m_themeToggle);
    layout->addSpacing(12);
    layout->addWidget(m_logoutButton);
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

// 加载消息方法
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

void MainWindow::onFriendsLoaded(const QList<UserSummary> &friends)
{
    m_friends = friends;
    rebuildFriendList();
    if (m_contactStatus) {
        m_contactStatus->setText(friends.isEmpty() ? QStringLiteral("暂无好友，可搜索用户名添加")
                                                   : QString());
    }
}

void MainWindow::rebuildFriendList()
{
    if (!m_friendList) {
        return;
    }
    m_friendList->clear();
    for (const UserSummary &friendUser : m_friends) {
        auto *item = new QListWidgetItem(m_friendList);
        item->setData(Qt::UserRole, QVariant::fromValue<qint64>(friendUser.id));
        item->setSizeHint(QSize(0, 58));

        auto *row = new QWidget(m_friendList);
        // 让列表视图接收点击事件，点击好友整行都能打开聊天。
        row->setAttribute(Qt::WA_TransparentForMouseEvents);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(10, 4, 10, 4);
        auto *info = new QLabel(row);
        const QString displayName = friendUser.name.isEmpty()
                                        ? friendUser.username
                                        : friendUser.name;
        const QString account = friendUser.username.isEmpty()
                                    ? friendUser.email
                                    : friendUser.username;
        info->setText(QStringLiteral("%1\n%2").arg(displayName, account));
        rowLayout->addWidget(info, 1);
        m_friendList->setItemWidget(item, row);
    }
}

void MainWindow::onUsersSearchLoaded(const QList<UserSummary> &users)
{
    if (m_friendSearchButton) {
        m_friendSearchButton->setEnabled(true);
    }
    rebuildSearchResults(users);
    if (m_contactStatus) {
        m_contactStatus->setText(users.isEmpty() ? QStringLiteral("没有找到匹配的用户")
                                                 : QStringLiteral("找到 %1 位用户").arg(users.size()));
    }
}

void MainWindow::rebuildSearchResults(const QList<UserSummary> &users)
{
    if (!m_searchResultList) {
        return;
    }
    m_searchResultList->clear();
    m_searchResultList->setVisible(!users.isEmpty());

    for (const UserSummary &user : users) {
        auto *item = new QListWidgetItem(m_searchResultList);
        item->setData(Qt::UserRole, QVariant::fromValue<qint64>(user.id));
        item->setSizeHint(QSize(0, 58));

        auto *row = new QWidget(m_searchResultList);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(10, 4, 10, 4);
        auto *info = new QLabel(row);
        const QString displayName = user.name.isEmpty() ? user.username : user.name;
        info->setText(QStringLiteral("%1\n%2").arg(displayName, user.username));
        rowLayout->addWidget(info, 1);
        auto *addButton = new QPushButton(QStringLiteral("添加"), row);
        addButton->setFixedWidth(64);
        rowLayout->addWidget(addButton);
        connect(addButton, &QPushButton::clicked, this, [this, user, addButton]() {
            addButton->setEnabled(false);
            if (m_contactStatus) {
                m_contactStatus->setText(QStringLiteral("正在发送好友申请…"));
            }
            m_chat->addFriend(user.id);
        });
        m_searchResultList->setItemWidget(item, row);
    }
}

void MainWindow::onSearchFriendsClicked()
{
    const QString username = m_friendSearchEdit ? m_friendSearchEdit->text().trimmed() : QString();
    if (username.isEmpty()) {
        if (m_contactStatus) {
            m_contactStatus->setText(QStringLiteral("请输入用户名"));
        }
        if (m_searchResultList) {
            m_searchResultList->clear();
            m_searchResultList->setVisible(false);
        }
        return;
    }
    if (m_friendSearchButton) {
        m_friendSearchButton->setEnabled(false);
    }
    if (m_contactStatus) {
        m_contactStatus->setText(QStringLiteral("搜索中…"));
    }
    m_chat->searchFriends(username);
}

void MainWindow::onFriendRequestSent(const FriendRequest &request)
{
    if (m_contactStatus) {
        const QString name = request.addressee.name.isEmpty()
                                 ? request.addressee.username
                                 : request.addressee.name;
        m_contactStatus->setText(QStringLiteral("已向 %1 发送好友申请").arg(name));
    }
}

void MainWindow::onContactRequestError(const QString &operation, const QString &message)
{
    qWarning() << "[MainWindow]" << operation << "失败:" << message;
    if (operation == QStringLiteral("搜索好友") && m_friendSearchButton) {
        m_friendSearchButton->setEnabled(true);
    }
    if (operation == QStringLiteral("添加好友") && m_searchResultList) {
        for (QPushButton *button : m_searchResultList->findChildren<QPushButton *>()) {
            button->setEnabled(true);
        }
    }
    if (m_contactStatus) {
        m_contactStatus->setText(QStringLiteral("%1失败：%2").arg(operation, message));
    }
}

void MainWindow::onFriendItemClicked(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    const qint64 friendId = item->data(Qt::UserRole).toLongLong();
    if (friendId <= 0) {
        return;
    }
    if (m_contactStatus) {
        m_contactStatus->setText(QStringLiteral("正在打开聊天…"));
    }
    m_chat->openConversationWith(friendId);
}

void MainWindow::onConversationOpened(const Conversation &conversation)
{
    if (m_navList) {
        m_navList->setCurrentRow(0);
    }
    QListWidgetItem *item = conversationItemOf(conversation.id);
    if (!item) {
        rebuildConversationList();
        item = conversationItemOf(conversation.id);
    }
    if (item) {
        m_conversationList->setCurrentItem(item);
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

void MainWindow::onLogoutClicked()
{
    emit logoutRequested();
}

void MainWindow::resetForLogout()
{
    m_chat->reset();
    m_currentSessionId = 0;
    m_currentUserName.clear();
    m_started = false;
    setWindowTitle(QStringLiteral("XinChat"));
    if (m_settingsUserLabel) {
        m_settingsUserLabel->clear();
    }

    if (m_navList) {
        m_navList->setCurrentRow(0);
    }
    if (m_conversationList) {
        m_conversationList->clear();
    }
    if (m_messageList) {
        m_messageList->clear();
    }
    if (m_friendList) {
        m_friendList->clear();
    }
    if (m_searchResultList) {
        m_searchResultList->clear();
        m_searchResultList->setVisible(false);
    }
    m_friends.clear();
    if (m_friendSearchEdit) {
        m_friendSearchEdit->clear();
    }
    if (m_contactStatus) {
        m_contactStatus->clear();
    }
}

// 主题切换
void MainWindow::onThemeChanged()
{
    refreshItemWidgets();
    updateThemeToggleText();
}
