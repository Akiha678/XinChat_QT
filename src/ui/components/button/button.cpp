#include "ui/components/button/button.h"

#include "ui/color/Theme.h"

Button::Button(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_normalText(text)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(36);

    // 主题切换时自动重建样式
    connect(&Theme::instance(), &Theme::themeChanged,
            this, [this]() { rebuildStyle(); });
    rebuildStyle();
}

void Button::rebuildStyle()
{
    const Theme::Palette &p = Theme::instance().palette();
    const QString primary = Theme::toCss(p.primary);
    const QString hover = Theme::toCss(p.primaryHover);
    const QString pressed = Theme::toCss(p.primaryPressed);
    const QString disabled = Theme::toCss(p.primaryDisabled);
    const QString onPrimary = Theme::toCss(p.textOnPrimary);

    setStyleSheet(QStringLiteral(R"(
        QPushButton {
            background-color: %1;
            color: %2;
            border: none;
            border-radius: 6px;
            font-size: 15px;
        }
        QPushButton:hover { background-color: %3; }
        QPushButton:pressed { background-color: %4; }
        QPushButton:disabled { background-color: %5; }
    )")
                      .arg(primary, onPrimary, hover, pressed, disabled));
}

void Button::setLoading(bool loading, const QString &loadingText)
{
    m_loading = loading;
    setEnabled(!loading);
    setText(loading ? (loadingText.isEmpty() ? QStringLiteral("请稍候...") : loadingText)
                    : m_normalText);
}

bool Button::isLoading() const
{
    return m_loading;
}
