#include "ui/components/messagebubble/MessageBubble.h"

#include <QColor>
#include <QFontMetrics>
#include <QPainter>
#include <QRect>

#include "ui/color/Theme.h"

namespace {
constexpr int kMarginLeftRight = 12;  // 气泡距控件左右边缘
constexpr int kPadding = 12;          // 气泡内边距
constexpr int kCornerRadius = 8;
constexpr int kMaxTextWidth = 320;    // 单条消息最大文本宽度
}

MessageBubble::MessageBubble(const QString &text, Role role, QWidget *parent)
    : QWidget(parent)
    , m_text(text)
    , m_role(role)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

QSize MessageBubble::sizeHint() const
{
    QFontMetrics fm(font());
    const QRect r = fm.boundingRect(QRect(0, 0, kMaxTextWidth, 10000),
                                    Qt::TextWordWrap, m_text);
    return QSize(r.width() + kPadding * 2 + kMarginLeftRight * 2,
                 r.height() + kPadding * 2 + 8);
}

void MessageBubble::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QFontMetrics fm(font());
    const int maxTextWidth = qMax(120, width() - kMarginLeftRight * 2 - kPadding * 2);
    const QRect textRect = fm.boundingRect(QRect(0, 0, maxTextWidth, 10000),
                                           Qt::TextWordWrap, m_text);

    const int bubbleWidth = textRect.width() + kPadding * 2;
    const int bubbleHeight = textRect.height() + kPadding * 2;

    QRect bubbleRect;
    if (m_role == Self) {
        bubbleRect = QRect(width() - bubbleWidth - kMarginLeftRight, 4,
                           bubbleWidth, bubbleHeight);
    } else {
        bubbleRect = QRect(kMarginLeftRight, 4, bubbleWidth, bubbleHeight);
    }

    // 颜色取自主题色板（自己=主题绿气泡，对方=面板色气泡）
    const Theme::Palette &palette = Theme::instance().palette();
    const bool self = (m_role == Self);
    p.setPen(Qt::NoPen);
    p.setBrush(self ? palette.bubbleSelf : palette.bubbleOther);
    p.drawRoundedRect(bubbleRect, kCornerRadius, kCornerRadius);

    p.setPen(self ? palette.bubbleSelfText : palette.bubbleOtherText);
    p.drawText(bubbleRect.adjusted(kPadding, kPadding, -kPadding, -kPadding),
               Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignVCenter, m_text);
}
