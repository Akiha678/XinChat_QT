#include "ui/color/Theme.h"

#include <QApplication>
#include <QStringList>

namespace {

constexpr QColor rgb(int r, int g, int b)
{
    return QColor(r, g, b);
}

}  // namespace

// 浅色主题
Theme::Palette Theme::lightPalette()
{
    Palette p;
    // 背景颜色 - 浅色模式
    p.screenBg = rgb(0xF1, 0xF4, 0xFA);
    p.whiteBg = rgb(0xFF, 0xFF, 0xFF);
    p.contentBg = rgb(0xF8, 0xF8, 0xF8);
    
    p.primary          = rgb(0x00, 0x52, 0xD9);
    p.primaryHover     = p.primary; // 鼠标悬浮时主题颜色
    p.primaryPressed   = p.primary; // 按钮点击时主题颜色
    p.primaryDisabled  = p.primary; // 按钮不可点击时颜色
    // 左侧导航
    p.navBg            = rgb(0xD, 0xA6, 0xB1);  // 导航栏背景颜色
    p.navHoverBg       = rgb(0xC6, 0xC6, 0xC6); // 鼠标悬浮背景颜色
    p.navActiveBg      = rgb(0x07, 0xC1, 0x60); // 
    p.navText          = rgb(0x8B, 0x8B, 0x8B); // 未选中字体颜色
    p.navTextActive    = p.primary; // 选中字体颜色
    // 列表
    p.listBg           = rgb(0xF7, 0xF7, 0xF7); // 列表背景
    p.listSelectedBg   = rgb(0xDD, 0xDD, 0xDD); // 选中列表背景颜色
    p.itemBorder       = rgb(0xEC, 0xEC, 0xEC);
    p.chatBg           = rgb(0xF5, 0xF5, 0xF5); // 聊天界面背景颜色
    p.fieldBg          = rgb(0xFF, 0xFF, 0xFF);
    p.fieldBorder      = rgb(0xDD, 0xDD, 0xDD);
    p.headerBg         = rgb(0xFF, 0xFF, 0xFF);
    p.headerBorder     = rgb(0xE5, 0xE5, 0xE5);
    p.inputBg          = rgb(0xFF, 0xFF, 0xFF);
    
    p.textStrong       = rgb(0x33, 0x33, 0x33);
    p.textSecondary    = rgb(0x99, 0x99, 0x99);
    p.textMuted        = rgb(0xBB, 0xBB, 0xBB);
    p.textOnPrimary    = rgb(0xFF, 0xFF, 0xFF);
    
    p.splitterHandle   = rgb(0xE5, 0xE5, 0xE5);
    p.scrollbarHandle  = rgb(0xC8, 0xC8, 0xC8);
    p.buttonNeutralBg  = rgb(0xF0, 0xF0, 0xF0);
    p.buttonNeutralText= rgb(0x33, 0x33, 0x33);
    
    p.bubbleSelf       = rgb(0x95, 0xEC, 0x69);
    p.bubbleSelfText   = rgb(0x1A, 0x1A, 0x1A);
    p.bubbleOther      = rgb(0xFF, 0xFF, 0xFF);
    p.bubbleOtherText  = rgb(0x33, 0x33, 0x33);
    
    p.link             = p.primary; // 文字链接
    p.danger           = rgb(0xFA, 0x51, 0x51);
    return p;
}

// 深色主题
Theme::Palette Theme::darkPalette()
{
    Palette p;
    p.primary          = rgb(0x07, 0xC1, 0x60);
    p.primaryHover     = rgb(0x17, 0xD1, 0x70);
    p.primaryPressed   = rgb(0x05, 0x9A, 0x4D);
    p.primaryDisabled  = rgb(0x3E, 0x7A, 0x5C);
    p.navBg            = rgb(0x17, 0x17, 0x1A);
    p.navHoverBg       = rgb(0x23, 0x23, 0x27);
    p.navActiveBg      = rgb(0x07, 0xC1, 0x60);
    p.navText          = rgb(0x8A, 0x8A, 0x8F);
    p.navTextActive    = p.primary;
    p.listBg           = rgb(0x23, 0x23, 0x27);
    p.listSelectedBg   = rgb(0x2E, 0x5C, 0x45);
    p.itemBorder       = rgb(0x33, 0x33, 0x3A);
    p.chatBg           = rgb(0x1B, 0x1B, 0x1F);
    p.fieldBg          = rgb(0x2A, 0x2A, 0x2F);
    p.fieldBorder      = rgb(0x3A, 0x3A, 0x3F);
    p.headerBg         = rgb(0x23, 0x23, 0x27);
    p.headerBorder     = rgb(0x33, 0x33, 0x3A);
    p.inputBg          = rgb(0x23, 0x23, 0x27);
    p.textStrong       = rgb(0xE6, 0xE6, 0xE6);
    p.textSecondary    = rgb(0xB0, 0xB0, 0xB5);
    p.textMuted        = rgb(0x7A, 0x7A, 0x80);
    p.textOnPrimary    = rgb(0xFF, 0xFF, 0xFF);
    
    p.splitterHandle   = rgb(0x33, 0x33, 0x3A);
    p.scrollbarHandle  = rgb(0x55, 0x55, 0x5C);
    p.buttonNeutralBg  = rgb(0x3A, 0x3A, 0x3F);
    p.buttonNeutralText= rgb(0xE6, 0xE6, 0xE6);
    
    p.bubbleSelf       = rgb(0x2C, 0x7A, 0x4E);
    p.bubbleSelfText   = rgb(0xF2, 0xF2, 0xF2);
    p.bubbleOther      = rgb(0x38, 0x38, 0x3D);
    p.bubbleOtherText  = rgb(0xE6, 0xE6, 0xE6);

    p.link             = rgb(0x00, 0x52, 0xD9); // 文字链接
    p.danger           = rgb(0xE5, 0x48, 0x4D); // 未读消息
    return p;
}

Theme &Theme::instance()
{
    static Theme s_instance;
    return s_instance;
}

Theme::Theme(QObject *parent)
    : QObject(parent)
{
}

QString Theme::toCss(const QColor &color)
{
    return color.name(QColor::HexRgb).toUpper();
}

void Theme::setScheme(Scheme scheme)
{
    if (m_scheme == scheme) {
        return;
    }
    m_scheme = scheme;
    m_palette = (scheme == Scheme::Dark) ? darkPalette() : lightPalette();
    apply();
    emit themeChanged();
}

void Theme::apply()
{
    if (qApp) {
        qApp->setStyleSheet(buildStyleSheet(m_palette));
    }
}

QString Theme::buildStyleSheet(const Palette &p) const
{
    const auto css = [](const QColor &c) { return toCss(c); };

    QStringList rules;
    rules << QStringLiteral(R"(
        * {
            font-family: "PingFang SC", "Helvetica Neue", "Microsoft YaHei", sans-serif;
        }
        QWidget { font-size: 14px; })");

    rules << QStringLiteral("QLineEdit { border: 1px solid %1; border-radius: 6px;"
                            " padding: 8px 10px; background: %2; }")
                 .arg(css(p.fieldBorder), css(p.fieldBg));
    rules << QStringLiteral("QLineEdit:focus { border: 1px solid %1; }").arg(css(p.primary));

    rules << QStringLiteral("QPushButton { background-color: %1; color: %2; border: none;"
                            " border-radius: 6px; padding: 9px 0; }")
                 .arg(css(p.primary), css(p.textOnPrimary));
    rules << QStringLiteral("QPushButton:hover { background-color: %1; }").arg(css(p.primaryHover));
    rules << QStringLiteral("QPushButton:pressed { background-color: %1; }").arg(css(p.primaryPressed));

    rules << QStringLiteral("QListWidget { border: none; background: %1; outline: none; }")
                 .arg(css(p.listBg));
    rules << "QListWidget::item { border: none; }";
    rules << QStringLiteral("QListWidget::item:selected { background: %1; }")
                 .arg(css(p.listSelectedBg));

    rules << QStringLiteral("#navList { background: %1; }").arg(css(p.screenBg));
    rules << QStringLiteral("#mainSeparator { background: %1; border: none; }")
                 .arg(css(p.splitterHandle));
    rules << QStringLiteral("#navList::item { color: %1; padding: 4px 0; }").arg(css(p.navText));
    rules << QStringLiteral("#navList::item:hover { background: %1; }").arg(css(p.navHoverBg));
    // 选中态仅通过主题主色文字区分，不再绘制选中背景。
    rules << QStringLiteral("#navList::item:selected { background: transparent; color: %1; }")
                 .arg(css(p.navTextActive));
    rules << QStringLiteral("#navList::item:selected:hover { background: transparent; color: %1; }")
                 .arg(css(p.navTextActive));

    rules << QStringLiteral("#conversationList::item { border-bottom: 1px solid %1; }")
                 .arg(css(p.itemBorder));

    rules << QStringLiteral("#messageList { background: %1; }").arg(css(p.chatBg));
    rules << "#messageList::item { border: none; background: transparent; }";

    rules << QStringLiteral("#chatHeader { background: %1; border-bottom: 1px solid %2;"
                            " font-size: 16px; font-weight: bold; }")
                 .arg(css(p.headerBg), css(p.headerBorder));
    rules << QStringLiteral("#inputEdit { border: none; border-top: 1px solid %1;"
                            " background: %2; padding: 8px; font-size: 14px; }")
                 .arg(css(p.headerBorder), css(p.inputBg));

    rules << QStringLiteral("QSplitter::handle { background: %1; width: 1px; }")
                 .arg(css(p.splitterHandle));
    rules << "QScrollBar:vertical { background: transparent; width: 8px; }";
    rules << QStringLiteral("QScrollBar::handle:vertical { background: %1; border-radius: 4px;"
                            " min-height: 30px; }")
                 .arg(css(p.scrollbarHandle));
    rules << "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }";

    return rules.join(QLatin1Char('\n'));
}
