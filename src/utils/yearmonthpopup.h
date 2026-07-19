#ifndef YEARMONTHPOPUP_H
#define YEARMONTHPOPUP_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>

class YearMonthPopup : public QDialog
{
    Q_OBJECT
public:
    explicit YearMonthPopup(QWidget *parent = nullptr);

    void setYearMonth(int year, int month);

signals:
    void yearMonthSelected(int year, int month);

private slots:
    void onYearChanged(int delta);
    void onMonthClicked(int month);

private:
    void updateYearLabel();
    void updateMonthButtons();

    int m_year;
    int m_month;
    QLabel *m_yearLabel;
    QPushButton *m_monthBtns[12];
};

#endif
