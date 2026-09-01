#pragma once

#include <QMainWindow>
#include <QString>

#include "core/models.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QTextEdit;
class UserManager;

// 主窗口：左侧导航栏 + 会话列表 + 聊天区 三栏布局（微信风格）
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setCurrentUser(const QString &username);

private slots:
    void onNavChanged(int row);
    void onConversationSelected(QListWidgetItem *item);
    void onSendClicked();
    void onMessageAdded(const QString &friendName, const ChatMessage &message);
    void onConversationUpdated(const QString &friendName,
                               const QString &lastMessage,
                               const QString &timeText);

private:
    QWidget *createChatPage();
    QWidget *createContactsPage();
    QWidget *createSettingsPage();
    void rebuildConversationList();
    void reloadMessages();
    void appendMessage(const ChatMessage &message);
    QListWidgetItem *conversationItemOf(const QString &friendName) const;

    UserManager *m_userManager = nullptr;

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_stack = nullptr;

    QListWidget *m_conversationList = nullptr;
    QListWidget *m_messageList = nullptr;
    QLabel *m_chatHeader = nullptr;
    QTextEdit *m_inputEdit = nullptr;

    QString m_currentFriend;
};
