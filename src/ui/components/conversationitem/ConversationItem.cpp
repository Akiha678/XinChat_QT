#include "ui/components/conversationitem/ConversationItem.h"

#include <QColor>
#include <QFontMetrics>
#include <QPainter>
#include <QRect>

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

    // 圆形头像（色块 + 昵称首字）
    const QRect avatarRect(12, (height() - kAvatarSize) / 2, kAvatarSize, kAvatarSize);
    p.setPen(Qt::NoPen);
    p.setBrush(m_conv.avatarColor);
    p.drawEllipse(avatarRect);

    QFont avatarFont = font();
    avatarFont.setBold(true);
    avatarFont.setPixelSize(16);
    p.setFont(avatarFont);
    p.setPen(Qt::white);
    p.drawText(avatarRect, Qt::AlignCenter, m_conv.friendName.left(1));

    // 昵称（加粗）
    QFont nameFont = font();
    nameFont.setPixelSize(15);
    nameFont.setBold(true);
    p.setFont(nameFont);
    p.setPen(QColor(0x33, 0x33, 0x33));
    p.drawText(QRect(66, 10, width() - 66 - 60, 20),
               Qt::AlignLeft | Qt::AlignVCenter, m_conv.friendName);

    // 最后一条消息（灰色小字，超长省略）
    QFont msgFont = font();
    msgFont.setPixelSize(12);
    p.setFont(msgFont);
    p.setPen(QColor(0x99, 0x99, 0x99));
    const QFontMetrics msgFm(msgFont);
    const QString elided = msgFm.elidedText(m_conv.lastMessage, Qt::ElideRight,
                                            width() - 66 - 16);
    p.drawText(QRect(66, 34, width() - 66 - 16, 18),
               Qt::AlignLeft | Qt::AlignVCenter, elided);

    // 时间（右上角）
    p.setPen(QColor(0xBB, 0xBB, 0xBB));
    p.drawText(QRect(width() - 56, 10, 44, 16),
               Qt::AlignRight | Qt::AlignVCenter, m_conv.timeText);
}
