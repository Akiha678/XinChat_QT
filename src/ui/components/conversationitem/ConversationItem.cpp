#include "ui/components/conversationitem/ConversationItem.h"

#include <QColor>
#include <QFontMetrics>
#include <QPainter>
#include <QRect>

#include "core/Format.h"
#include "ui/color/Theme.h"

namespace {
constexpr int kItemHeight = 64;
constexpr int kAvatarSize = 42;
}

ConversationItem::ConversationItem(const Conversation &conv, QWidget *parent)
    : QWidget(parent)
    , m_conv(conv)
{
}

QSize ConversationItem::sizeHint() const
{
    return QSize(0, kItemHeight);
}

void ConversationItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const Theme::Palette &theme = Theme::instance().palette();

    // 圆形头像（色板色 + 名称首字）
    const QRect avatarRect(12, (height() - kAvatarSize) / 2, kAvatarSize, kAvatarSize);
    p.setPen(Qt::NoPen);
    p.setBrush(xc::avatarColor(m_conv.colorSeed));
    p.drawEllipse(avatarRect);

    QFont avatarFont = font();
    avatarFont.setBold(true);
    avatarFont.setPixelSize(16);
    p.setFont(avatarFont);
    p.setPen(Qt::white);
    p.drawText(avatarRect, Qt::AlignCenter, m_conv.name.left(1));

    // 未读角标：头像右上角红色圆 + 数字
    if (m_conv.unreadCount > 0) {
        const QString countText =
            m_conv.unreadCount > 99 ? QStringLiteral("99+")
                                    : QString::number(m_conv.unreadCount);
        const QFontMetrics countFm(font());
        const int badgeW = qMax(18, countFm.horizontalAdvance(countText) + 8);
        const QRect badgeRect(avatarRect.right() - badgeW / 2 + 4,
                              avatarRect.top() - 2, badgeW, 18);
        p.setBrush(theme.danger);
        p.drawRoundedRect(badgeRect, 9, 9);
        p.setPen(Qt::white);
        QFont badgeFont = font();
        badgeFont.setPixelSize(11);
        p.setFont(badgeFont);
        p.drawText(badgeRect, Qt::AlignCenter, countText);
    }

    // 名称（加粗）
    QFont nameFont = font();
    nameFont.setPixelSize(15);
    nameFont.setBold(true);
    p.setFont(nameFont);
    p.setPen(theme.textStrong);
    p.drawText(QRect(66, 10, width() - 66 - 60, 20),
               Qt::AlignLeft | Qt::AlignVCenter, m_conv.name);

    // 最近一条消息（小字，超长省略）
    QFont msgFont = font();
    msgFont.setPixelSize(12);
    p.setFont(msgFont);
    p.setPen(theme.textSecondary);
    const QFontMetrics msgFm(msgFont);
    const QString elided = msgFm.elidedText(m_conv.preview, Qt::ElideRight,
                                            width() - 66 - 16);
    p.drawText(QRect(66, 34, width() - 66 - 16, 18),
               Qt::AlignLeft | Qt::AlignVCenter, elided);

    // 时间（右上角）
    p.setPen(theme.textMuted);
    p.drawText(QRect(width() - 56, 10, 44, 16),
               Qt::AlignRight | Qt::AlignVCenter,
               xc::formatConversationTime(m_conv.lastMessageAt));
}
