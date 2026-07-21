#ifndef DATELINEEDIT_H
#define DATELINEEDIT_H
#include <QLineEdit>
#include <QIcon>
class DateLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit DateLineEdit(QWidget *parent = nullptr);
    void setLeftIcon(const QIcon &icon);
    void setRightIcon(const QIcon &icon);
    void setIconSize(const QSize &size);
    void setRightIconSize(const QSize &size);
signals:
    void rightIconClicked();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
private:
    void updateTextMargins();
    QRect rightIconRect() const;
    QIcon m_leftIcon;
    QIcon m_rightIcon;
    QSize m_iconSize;
    QSize m_rightIconSize;
    int m_iconSpacing = 6;
    int m_textSpacing = 3;
};
#endif