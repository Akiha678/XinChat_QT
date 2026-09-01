#pragma once

#include <QSize>
#include <QString>
#include <QWidget>

// 聊天气泡：自己发的消息靠右（微信绿底），对方消息靠左（白底）。
// 使用 paintEvent 自绘，宽度跟随所在列表宽度变化。
class MessageBubble : public QWidget {
    Q_OBJECT

public:
    enum Role { Self, Other };

    MessageBubble(const QString &text, Role role, QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_text;
    Role m_role;
};
