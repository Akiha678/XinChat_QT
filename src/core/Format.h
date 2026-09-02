#pragma once

// 跨层共享的小工具：时间解析/格式化、头像色板。
// 纯 inline 实现，头文件即用，无需编译单元。

#include <QColor>
#include <QDateTime>
#include <QString>

namespace xc {

// 解析 ISO-8601 时间字符串（如 "2026-08-28T08:00:23.732Z"）。
// Qt 只支持毫秒精度，这里截断多余小数位并去掉 Z 后缀；失败返回无效时间。
inline QDateTime parseIsoInstant(const QString &text)
{
    if (text.isEmpty()) {
        return QDateTime();
    }
    QString t = text;
    if (t.endsWith(QLatin1Char('Z'))) {
        t.chop(1);
    }
    const int dot = t.indexOf(QLatin1Char('.'));
    if (dot >= 0) {
        t = t.left(dot + 1) + t.mid(dot + 1).left(3);
    }
    QDateTime dt = QDateTime::fromString(t, QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"));
    if (!dt.isValid()) {
        dt = QDateTime::fromString(text, Qt::ISODate);
    }
    return dt;
}

// 会话列表时间显示：今天 -> "HH:mm"；昨天 -> "昨天"；今年 -> "M月d日"；更早 -> "yyyy/M/d"
inline QString formatConversationTime(const QDateTime &dt)
{
    if (!dt.isValid()) {
        return QString();
    }
    const QDateTime local = dt.toLocalTime();
    const QDateTime now = QDateTime::currentDateTime();
    if (local.date() == now.date()) {
        return local.toString(QStringLiteral("HH:mm"));
    }
    if (local.date() == now.date().addDays(-1)) {
        return QStringLiteral("昨天");
    }
    if (local.date().year() == now.date().year()) {
        return local.toString(QStringLiteral("M月d日"));
    }
    return local.toString(QStringLiteral("yyyy/M/d"));
}

// 头像颜色：由后端 colorSeed(整数) 映射到固定色板（微信风格）
inline QColor avatarColor(int seed)
{
    static const QColor kPalette[] = {
        QColor(0x5B, 0xC2, 0x8F),  // 0 绿
        QColor(0xE8, 0x9C, 0x4E),  // 1 橙
        QColor(0x6A, 0x9B, 0xE8),  // 2 蓝
        QColor(0xB0, 0x7C, 0xDE),  // 3 紫
        QColor(0xE8, 0x6A, 0x8A),  // 4 粉
        QColor(0x4E, 0xB8, 0xC2),  // 5 青
        QColor(0xC2, 0xA0, 0x4E),  // 6 棕
        QColor(0x8A, 0xB0, 0x5B),  // 7 橄榄
    };
    constexpr int kCount = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));
    return kPalette[((seed % kCount) + kCount) % kCount];
}

}  // namespace xc
