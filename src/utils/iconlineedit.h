#ifndef ICONLINEEDIT_H
#define ICONLINEEDIT_H

#include <QLineEdit>
#include <QIcon>

class IconLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit IconLineEdit(QWidget *parent = nullptr);

    // 设置图标（从资源路径加载，如 ":/icons/calendar_today.svg"）
    void setIcon(const QIcon &icon);
    void setIconPath(const QString &path);

    // 设置图标大小和间距
    void setIconSize(const QSize &size);
    void setIconSpacing(int spacing);
    int iconSpacing() const;

    void setTextSpacing(int spacing);
    int textSpacing() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QIcon m_icon;
    QSize m_iconSize;
    int m_iconSpacing;
    int m_textSpacing;

    void updateTextMargins();
};

#endif // ICONLINEEDIT_H
