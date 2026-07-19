#ifndef TIMESELECTOR_H
#define TIMESELECTOR_H

#include <QWidget>
#include <QPushButton>

class TimeColumn : public QWidget
{
    Q_OBJECT
public:
    explicit TimeColumn(int start, int end, QWidget *parent = nullptr);

    void setSelected(int value);
    int selectedValue() const { return m_selected; }

signals:
    void valueSelected(int value);

private slots:
    void onBtnClicked(int value);

private:
    int m_selected = 0;
    QList<QPushButton*> m_btns;
    QList<int> m_values;
};

class TimeSelector : public QWidget
{
    Q_OBJECT
public:
    explicit TimeSelector(QWidget *parent = nullptr);

    void setTime(int hour, int minute);

    int hour() const;
    int minute() const;

signals:
    void timeChanged(int hour, int minute);

private slots:
    void onHourSelected(int value);
    void onMinuteSelected(int value);

private:
    TimeColumn *m_hourColumn;
    TimeColumn *m_minuteColumn;
};

#endif
