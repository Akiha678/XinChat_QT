#pragma once

#include <QColor>
#include <QObject>
#include <QString>

// 主题色板：项目内所有颜色集中于此，避免散落硬编码。
//
//   - palette() 取当前方案的语义化颜色（浅色/深色两套）
//   - 代码画图用：painter.setPen(Theme::instance().palette().primary);
//   - 切换主题：Theme::instance().setScheme(Scheme::Dark)
//     → 自动重建全局 QSS 并发出 themeChanged，各组件自行刷新
//   - 组件内拼 QSS 用 Theme::toCss(color) 得到 "#RRGGBB"
class Theme : public QObject {
    Q_OBJECT

public:
    enum class Scheme { Light, Dark };

    // 语义化颜色集合（一份完整色板）
    struct Palette {
        // 品牌主色
        QColor primary;     
        QColor primaryHover;
        QColor primaryPressed;
        QColor primaryDisabled;
        // 左侧导航
        QColor navBg;
        QColor navHoverBg;
        QColor navActiveBg;
        QColor navText;
        QColor navTextActive;
        // 列表/面板
        QColor listBg;
        QColor listSelectedBg;
        QColor itemBorder;
        QColor chatBg;          // 消息区背景
        // 输入框/头部
        QColor fieldBg;
        QColor fieldBorder;
        QColor headerBg;
        QColor headerBorder;
        QColor inputBg;
        // 文字
        QColor textStrong;
        QColor textSecondary;
        QColor textMuted;
        QColor textOnPrimary;   // 主色按钮上的文字
        // 控件细节
        QColor splitterHandle;
        QColor scrollbarHandle;
        QColor buttonNeutralBg; // 次按钮/取消按钮
        QColor buttonNeutralText;
        // 聊天气泡
        QColor bubbleSelf;
        QColor bubbleSelfText;
        QColor bubbleOther;
        QColor bubbleOtherText;
        // 其他
        QColor link;            // 文字链接
        QColor danger;          // 未读角标/错误
    };

    static Palette lightPalette();
    static Palette darkPalette();

    static Theme &instance();
    // QColor -> "#RRGGBB"（QSS 用）
    static QString toCss(const QColor &color);

    Scheme scheme() const { return m_scheme; }
    // 切换主题：重建全局 QSS 并发出 themeChanged
    void setScheme(Scheme scheme);

    const Palette &palette() const { return m_palette; }
    // 用当前色板重建全局样式（main 启动时调用一次）
    void apply();

signals:
    void themeChanged();

private:
    explicit Theme(QObject *parent = nullptr);

    QString buildStyleSheet(const Palette &p) const;

    Palette m_palette = lightPalette();
    Scheme m_scheme = Scheme::Light;
};
