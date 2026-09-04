#pragma once

#include <QObject>

#include "ui/MainWindow.h"
#include "ui/login/LoginWindow.h"

// 应用编排层：负责窗口生命周期和登录态切换，不承载具体业务逻辑。
// 登录、主窗口各自只关心自身 UI，通过信号与本类协作。
class ApplicationController : public QObject {
    Q_OBJECT

public:
    explicit ApplicationController(QObject *parent = nullptr);

    void show();

private slots:
    void onLoginSucceeded(const QString &displayName);
    void onLogoutRequested();

private:
    LoginWindow m_loginWindow;
    MainWindow m_mainWindow;
};
