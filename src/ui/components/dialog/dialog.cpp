#include "ui/components/dialog/dialog.h"

#include <QMessageBox>
#include <QPushButton>

namespace {

const QString kOkText = QStringLiteral("确定");
const QString kCancelText = QStringLiteral("取消");

// 主按钮绿色，次按钮灰色
QPushButton *styleButton(QMessageBox &box, const QString &text, bool primary)
{
    auto *btn = box.addButton(text, primary ? QMessageBox::AcceptRole
                                            : QMessageBox::RejectRole);
    btn->setStyleSheet(primary
        ? QStringLiteral("background-color:#07C160;color:#FFFFFF;border:none;"
                         "border-radius:6px;padding:6px 16px;min-width:72px;")
        : QStringLiteral("background-color:#F0F0F0;color:#333333;border:none;"
                         "border-radius:6px;padding:6px 16px;min-width:72px;"));
    return btn;
}

}  // namespace

void Dialog::warning(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(QMessageBox::Warning, title, text, QMessageBox::NoButton, parent);
    styleButton(box, kOkText, /*primary=*/true);
    box.exec();
}

void Dialog::info(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(QMessageBox::Information, title, text, QMessageBox::NoButton, parent);
    styleButton(box, kOkText, /*primary=*/true);
    box.exec();
}

bool Dialog::confirm(QWidget *parent, const QString &title, const QString &text)
{
    QMessageBox box(QMessageBox::Question, title, text, QMessageBox::NoButton, parent);
    styleButton(box, kOkText, /*primary=*/true);
    styleButton(box, kCancelText, /*primary=*/false);
    return box.exec() == QMessageBox::Ok;
}
