#include <QApplication>

#include "app/ApplicationController.h"
#include "ui/color/Theme.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("XinChat"));
    app.setOrganizationName(QStringLiteral("XinChat"));

    // 全局样式：由主题组件（src/ui/color/Theme）按色板生成
    Theme::instance().apply();

    ApplicationController controller;
    controller.show();
    return app.exec();
}
