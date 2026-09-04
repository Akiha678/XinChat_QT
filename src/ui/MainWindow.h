#pragma once

#include <QMainWindow>
#include <QList>
#include <QString>

#include "core/models.h"

class ChatManager;
class ContactManager;
class QEvent;
class Button;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QListWidgetItem;
class QStackedWidget;
class QTextEdit;

// 主窗口：左侧导航栏 + 会话列表 + 聊天区 三栏布局（微信风格）。
// 数据全部来自后端（ChatManager），无任何本地模拟数据。
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    // 登录成功由 LoginWindow 调用：设置标题并触发数据加载
    void setCurrentUser(const QString &displayName);
    // 清理当前用户数据，供退出登录后下次登录复用窗口。
    void resetForLogout();

signals:
    void logoutRequested();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onNavChanged(int row);
    void onConversationSelected(QListWidgetItem *item);
    void onSendClicked();
    void onToggleTheme();
    void onLogoutClicked();

    void onSessionsChanged();
    void onSessionMessagesChanged(qint64 sessionId);
    void onMessageAdded(const ChatMessage &message);
    void onRequestError(const QString &operation, const QString &message);
    void onThemeChanged();
    void onFriendsLoaded(const QList<UserSummary> &friends);
    void onUsersSearchLoaded(const QList<UserSummary> &users);
    void onFriendRequestSent(const FriendRequest &request);
    void onContactRequestError(const QString &operation, const QString &message);
    void onFriendItemClicked(QListWidgetItem *item);
    void onSearchFriendsClicked();
    void onConversationOpened(const Conversation &conversation);

private:
    QWidget *createChatPage();
    QWidget *createContactsPage();
    QWidget *createSettingsPage();
    void rebuildConversationList();   // 保留当前选中会话
    void reloadMessages();
    void appendMessage(const ChatMessage &message);
    QListWidgetItem *conversationItemOf(qint64 sessionId) const;
    void setChatHeader(const QString &text);
    void refreshItemWidgets();        // 主题变化后重绘列表项（气泡/会话行）
    void updateThemeToggleText();
    void rebuildFriendList();
    void rebuildSearchResults(const QList<UserSummary> &users);

    ChatManager *m_chat = nullptr;
    ContactManager *m_contacts = nullptr;

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_stack = nullptr;

    QListWidget *m_conversationList = nullptr;
    QListWidget *m_messageList = nullptr;
    QLabel *m_chatHeader = nullptr;
    QLabel *m_settingsUserLabel = nullptr;
    QTextEdit *m_inputEdit = nullptr;
    Button *m_themeToggle = nullptr;
    Button *m_logoutButton = nullptr;
    QLineEdit *m_friendSearchEdit = nullptr;
    QPushButton *m_friendSearchButton = nullptr;
    QLabel *m_contactStatus = nullptr;
    QListWidget *m_friendList = nullptr;
    QListWidget *m_searchResultList = nullptr;
    QList<UserSummary> m_friends;

    qint64 m_currentSessionId = 0;
    QString m_currentUserName;
    bool m_started = false;
};
