#include <QApplication>
#include <QObject>

#include "ui/color/Theme.h"
#include "core/Session.h"
#include "ui/login/LoginWindow.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("XinChat"));
    app.setOrganizationName(QStringLiteral("XinChat"));

    // 全局样式：由主题组件（src/ui/color/Theme）按色板生成
    Theme::instance().apply();

    LoginWindow login;
    MainWindow mainWindow;

    // 登录成功 -> 显示主窗口
    QObject::connect(&login, &LoginWindow::loginSucceeded, &mainWindow,
                     [&](const QString &username) {
                         mainWindow.setCurrentUser(username);
                         mainWindow.show();
                         login.close();
                     });

    // 退出登录 -> 清理状态并返回登录窗口
    QObject::connect(&mainWindow, &MainWindow::logoutRequested, &login,
                     [&]() {
                         mainWindow.resetForLogout();
                         Session::instance().clear();
                         mainWindow.hide();
                         login.resetForLogout();
                         login.show();
                         login.raise();
                         login.activateWindow();
                     });

    login.show();
    return app.exec();
}
