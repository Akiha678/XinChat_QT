#include "ui/components/button/button.h"

// 封装Button组件

Button::Button(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_normalText(text)
{
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(36);

    setStyleSheet(QStringLiteral(R"(
        QPushButton {
            background-color: #07C160;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 15px;
        }
        QPushButton:hover { background-color: #06AD56; }
        QPushButton:pressed { background-color: #059A4D; }
        QPushButton:disabled { background-color: #A8D8B8; }
    )"));
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
