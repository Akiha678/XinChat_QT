#pragma once

#include <QString>

class QWidget;

// 可复用对话框工具组件：统一全应用的提示/确认弹窗样式。
// 无状态静态类，直接调用静态方法即可，无需 new 实例。
//
// 用法：
//   Dialog::warning(this, "登录失败", "用户名或密码错误");
//   if (Dialog::confirm(this, "退出", "确定要退出吗？")) { ... }
class Dialog {
public:
    static void warning(QWidget *parent, const QString &title, const QString &text);
    static void info(QWidget *parent, const QString &title, const QString &text);

    // 返回 true 表示用户点击了"确定"
    static bool confirm(QWidget *parent, const QString &title, const QString &text);
};
