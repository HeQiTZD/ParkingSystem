#ifndef CUSTOMDATECHOOSER_H
#define CUSTOMDATECHOOSER_H

#include <QWidget>
#include <QPushButton>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "calendardaybtn.h"
#include "yearmonthpopup.h"
#include "timeselector.h"

class CustomDateChooser : public QWidget
{
    Q_OBJECT
public:
    explicit CustomDateChooser(QWidget *parent = nullptr);

    QDate selectedDate() const { return m_selectedDate; }
    QTime selectedTime() const { return QTime(m_timeSelector->hour(), m_timeSelector->minute()); }
    QDateTime selectedDateTime() const { return QDateTime(m_selectedDate, selectedTime()); }

    void setSelectedDate(const QDate &date);
    void setSelectedDateTime(const QDateTime &dt);

signals:
    void dateTimeConfirmed(const QDateTime &dateTime);
    void dateTimeChanged(const QDateTime &dateTime);

private slots:
    void onYearLabelClicked();
    void onYearMonthSelected(int year, int month);
    void onYearChanged(int delta);
    void onDayBtnClicked(int index);
    void onTodayClicked();
    void onNowClicked();
    void onClearClicked();
    void onConfirmClicked();
    void onCancelClicked();
    void onTimeChanged(int hour, int minute);

private:
    void initTopWidget(QVBoxLayout *parentLayout);
    void initCalendarGrid(QHBoxLayout *parentLayout);
    void initTimeSelector(QHBoxLayout *parentLayout);
    void initBottomWidget(QVBoxLayout *parentLayout);
    void updateCalendar();

    QDate m_selectedDate;
    int m_currentYear;
    int m_currentMonth;
    QPushButton *m_yearMonthLabel;
    CalendarDayBtn *m_dayBtns[42];
    TimeSelector *m_timeSelector;

    static const QString WEEKDAY_NAMES[7];
};

#endif
