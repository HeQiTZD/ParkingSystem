#ifndef CALENDARDAYBTN_H
#define CALENDARDAYBTN_H
#include <QPushButton>
#include <QDate>
class CalendarDayBtn : public QPushButton
{
    Q_OBJECT
public:
    enum DayType {
        Normal,
        Disabled,
        Selected,
        Today,
        Weekend
    };
    explicit CalendarDayBtn(QWidget *parent = nullptr);
    void setDay(const QDate &date, DayType type);
    QDate date() const { return m_date; }
    DayType dayType() const { return m_type; }
protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    QDate m_date;
    DayType m_type = Normal;
    bool m_hovered = false;
    bool m_pressed = false;
};
#endif