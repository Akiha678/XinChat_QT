#include "app/ApplicationController.h"

#include "core/Session.h"

ApplicationController::ApplicationController(QObject *parent)
    : QObject(parent)
{
    connect(&m_loginWindow, &LoginWindow::loginSucceeded,
            this, &ApplicationController::onLoginSucceeded);
    connect(&m_mainWindow, &MainWindow::logoutRequested,
            this, &ApplicationController::onLogoutRequested);
}

void ApplicationController::show()
{
    m_loginWindow.show();
}

void ApplicationController::onLoginSucceeded(const QString &displayName)
{
    m_mainWindow.setCurrentUser(displayName);
    m_mainWindow.show();
    m_loginWindow.hide();
}

void ApplicationController::onLogoutRequested()
{
    m_mainWindow.resetForLogout();
    Session::instance().clear();
    m_mainWindow.hide();
    m_loginWindow.resetForLogout();
    m_loginWindow.show();
    m_loginWindow.raise();
    m_loginWindow.activateWindow();
}
