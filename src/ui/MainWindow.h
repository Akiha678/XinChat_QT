#pragma once

#include <QMainWindow>
#include <QString>

#include "core/models.h"

class ChatManager;
class Button;
class QLabel;
class QListWidget;
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

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void onNavChanged(int row);
    void onConversationSelected(QListWidgetItem *item);
    void onSendClicked();
    void onToggleTheme();

    void onSessionsChanged();
    void onSessionMessagesChanged(qint64 sessionId);
    void onMessageAdded(const ChatMessage &message);
    void onRequestError(const QString &operation, const QString &message);
    void onThemeChanged();

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

    ChatManager *m_chat = nullptr;

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_stack = nullptr;

    QListWidget *m_conversationList = nullptr;
    QListWidget *m_messageList = nullptr;
    QLabel *m_chatHeader = nullptr;
    QTextEdit *m_inputEdit = nullptr;
    Button *m_themeToggle = nullptr;

    qint64 m_currentSessionId = 0;
    QString m_currentUserName;
    bool m_started = false;
};
