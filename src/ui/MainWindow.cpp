#include "ui/MainWindow.h"

#include <QAbstractItemView>
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

#include "core/UserManager.h"
#include "ui/widgets/ConversationItem.h"
#include "ui/widgets/MessageBubble.h"

namespace {
constexpr int kConversationItemHeight = 64;
constexpr int kNavWidth = 64;
constexpr int kConversationListWidth = 260;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_userManager = new UserManager(this);

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

    // 业务信号 -> 界面刷新
    connect(m_userManager, &UserManager::messageAdded,
            this, &MainWindow::onMessageAdded);
    connect(m_userManager, &UserManager::conversationUpdated,
            this, &MainWindow::onConversationUpdated);

    rebuildConversationList();

    // 默认选中第一个会话
    if (m_conversationList->count() > 0) {
        m_conversationList->setCurrentRow(0);
    }
}

void MainWindow::setCurrentUser(const QString &username)
{
    m_userManager->setCurrentUser(username);
    setWindowTitle(QStringLiteral("XinChat - %1").arg(username));
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
    auto *label = new QLabel(QStringLiteral("通讯录\n\n（阶段 3 加入好友列表/分组管理）"), page);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    return page;
}

QWidget *MainWindow::createSettingsPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    auto *label = new QLabel(QStringLiteral("设置\n\n（主题、账号信息等，后续迭代）"), page);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    return page;
}

void MainWindow::rebuildConversationList()
{
    m_conversationList->clear();
    const QList<Conversation> &conversations = m_userManager->conversations();
    for (const Conversation &conv : conversations) {
        auto *item = new QListWidgetItem(m_conversationList);
        item->setData(Qt::UserRole, conv.friendName);
        item->setSizeHint(QSize(0, kConversationItemHeight));
        m_conversationList->setItemWidget(item, new ConversationItem(conv));
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
    m_currentFriend = item->data(Qt::UserRole).toString();
    m_chatHeader->setText(m_currentFriend);
    reloadMessages();
}

void MainWindow::reloadMessages()
{
    m_messageList->clear();
    const QList<ChatMessage> &messages = m_userManager->messagesFor(m_currentFriend);
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
    if (m_currentFriend.isEmpty()) {
        return;
    }
    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        return;
    }
    m_userManager->sendMessage(m_currentFriend, text);
    m_inputEdit->clear();
}

void MainWindow::onMessageAdded(const QString &friendName, const ChatMessage &message)
{
    if (friendName == m_currentFriend) {
        appendMessage(message);
    }
}

void MainWindow::onConversationUpdated(const QString &friendName,
                                       const QString & /*lastMessage*/,
                                       const QString & /*timeText*/)
{
    if (QListWidgetItem *item = conversationItemOf(friendName)) {
        const QList<Conversation> &all = m_userManager->conversations();
        for (const Conversation &conv : all) {
            if (conv.friendName == friendName) {
                m_conversationList->removeItemWidget(item);
                item->setSizeHint(QSize(0, kConversationItemHeight));
                m_conversationList->setItemWidget(item, new ConversationItem(conv));
                break;
            }
        }
    }
}

QListWidgetItem *MainWindow::conversationItemOf(const QString &friendName) const
{
    for (int i = 0; i < m_conversationList->count(); ++i) {
        QListWidgetItem *item = m_conversationList->item(i);
        if (item->data(Qt::UserRole).toString() == friendName) {
            return item;
        }
    }
    return nullptr;
}
