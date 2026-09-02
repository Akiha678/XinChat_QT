#include "ui/color/Theme.h"

#include <QApplication>
#include <QStringList>

namespace {

constexpr QColor rgb(int r, int g, int b)
{
    return QColor(r, g, b);
}

}  // namespace

Theme::Palette Theme::lightPalette()
{
    Palette p;
    p.primary          = rgb(0x07, 0xC1, 0x60);
    p.primaryHover     = rgb(0x06, 0xAD, 0x56);
    p.primaryPressed   = rgb(0x05, 0x9A, 0x4D);
    p.primaryDisabled  = rgb(0xA8, 0xD8, 0xB8);
    p.navBg            = rgb(0x2E, 0x2E, 0x2E);
    p.navHoverBg       = rgb(0x3A, 0x3A, 0x3A);
    p.navActiveBg      = rgb(0x07, 0xC1, 0x60);
    p.navText          = rgb(0xBB, 0xBB, 0xBB);
    p.navTextActive    = rgb(0xFF, 0xFF, 0xFF);
    p.listBg           = rgb(0xF7, 0xF7, 0xF7);
    p.listSelectedBg   = rgb(0xE4, 0xF5, 0xEC);
    p.itemBorder       = rgb(0xEC, 0xEC, 0xEC);
    p.chatBg           = rgb(0xF5, 0xF5, 0xF5);
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
    p.link             = rgb(0x07, 0xC1, 0x60);
    p.danger           = rgb(0xFA, 0x51, 0x51);
    return p;
}

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
    p.navTextActive    = rgb(0xFF, 0xFF, 0xFF);
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
    p.link             = rgb(0x3A, 0xD4, 0x7F);
    p.danger           = rgb(0xE5, 0x48, 0x4D);
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

    rules << QStringLiteral("#navList { background: %1; }").arg(css(p.navBg));
    rules << QStringLiteral("#navList::item { color: %1; padding: 4px 0; }").arg(css(p.navText));
    rules << QStringLiteral("#navList::item:hover { background: %1; }").arg(css(p.navHoverBg));
    rules << QStringLiteral("#navList::item:selected { background: %1; color: %2; }")
                 .arg(css(p.navActiveBg), css(p.navTextActive));
    rules << QStringLiteral("#navList::item:selected:hover { background: %1; }").arg(css(p.navActiveBg));

    rules << QStringLiteral("#conversationList::item { border-bottom: 1px solid %1; }")
                 .arg(css(p.itemBorder));

    rules << QStringLiteral("#messageList { background: %1; }").arg(css(p.chatBg));
    rules << "#messageList::item { border: none; background: transparent; }";

    rules << QStringLiteral("#chatHeader { background: %1; border-bottom: 1px solid %2;"
                            " font-size: 16px; font-weight: bold; }")
                 .arg(css(p.headerBg), css(p.headerBorder));
    rules << QStringLiteral("#inputEdit { border: none; background: %1; padding: 8px;"
                            " font-size: 14px; }")
                 .arg(css(p.inputBg));

    rules << QStringLiteral("QSplitter::handle { background: %1; }").arg(css(p.splitterHandle));
    rules << "QScrollBar:vertical { background: transparent; width: 8px; }";
    rules << QStringLiteral("QScrollBar::handle:vertical { background: %1; border-radius: 4px;"
                            " min-height: 30px; }")
                 .arg(css(p.scrollbarHandle));
    rules << "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }";

    return rules.join(QLatin1Char('\n'));
}
