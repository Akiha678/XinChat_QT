#pragma once

#include <QPushButton>
#include <QString>

class Button : public QPushButton {
    Q_OBJECT

public:
    explicit Button(const QString &text = {}, QWidget *parent = nullptr);

    void setLoading(bool loading, const QString &loadingText = QString());
    bool isLoading() const;

private:
    QString m_normalText;   // 正常态文案（进入加载态前的文本）
    bool m_loading = false;
};
