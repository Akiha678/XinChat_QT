#include <QApplication>
#include <QObject>

#include "ui/login/LoginWindow.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("XinChat"));
    app.setOrganizationName(QStringLiteral("XinChat"));

    // 全局样式（微信风格：绿色主色调）
    app.setStyleSheet(QStringLiteral(R"(
        * {
            font-family: "PingFang SC", "Helvetica Neue", "Microsoft YaHei", sans-serif;
        }
        QWidget { font-size: 14px; }
        QLineEdit {
            border: 1px solid #DDDDDD;
            border-radius: 6px;
            padding: 8px 10px;
            background: #FFFFFF;
        }
        QLineEdit:focus { border: 1px solid #07C160; }
        QPushButton {
            background-color: #07C160;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 9px 0;
        }
        QPushButton:hover { background-color: #06AD56; }
        QPushButton:pressed { background-color: #059A4D; }
        QListWidget { border: none; background: #F7F7F7; outline: none; }
        QListWidget::item { border: none; }
        QListWidget::item:selected { background: #E4F5EC; }

        #navList { background: #2E2E2E; }
        #navList::item { color: #BBBBBB; padding: 4px 0; }
        #navList::item:hover { background: #3A3A3A; }
        #navList::item:selected { background: #07C160; color: #FFFFFF; }
        #navList::item:selected:hover { background: #07C160; }

        #conversationList::item { border-bottom: 1px solid #ECECEC; }

        #messageList { background: #F5F5F5; }
        #messageList::item { border: none; background: transparent; }

        #chatHeader {
            background: #FFFFFF;
            border-bottom: 1px solid #E5E5E5;
            font-size: 16px;
            font-weight: bold;
        }
        #inputEdit {
            border: none;
            background: #FFFFFF;
            padding: 8px;
            font-size: 14px;
        }

        QSplitter::handle { background: #E5E5E5; }
        QScrollBar:vertical { background: transparent; width: 8px; }
        QScrollBar::handle:vertical {
            background: #C8C8C8;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )"));

    LoginWindow login;
    MainWindow mainWindow;

    // 登录成功 -> 显示主窗口
    QObject::connect(&login, &LoginWindow::loginSucceeded, &mainWindow,
                     [&](const QString &username) {
                         mainWindow.setCurrentUser(username);
                         mainWindow.show();
                         login.close();
                     });

    login.show();
    return app.exec();
}
