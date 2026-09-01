#pragma once

#include <QWidget>

#include "core/models.h"

// 会话列表项：圆形头像(色块+首字) + 昵称 + 最后消息 + 时间
class ConversationItem : public QWidget {
    Q_OBJECT

public:
    explicit ConversationItem(const Conversation &conv, QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Conversation m_conv;
};
