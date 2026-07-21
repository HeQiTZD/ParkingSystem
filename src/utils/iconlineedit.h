#ifndef ICONLINEEDIT_H
#define ICONLINEEDIT_H
#include <QLineEdit>
#include <QIcon>
class IconLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit IconLineEdit(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setIconPath(const QString &path);

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
#endif