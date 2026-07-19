#include "customdatechooser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

const QString CustomDateChooser::WEEKDAY_NAMES[7] = {
    QStringLiteral("\u4E00"), QStringLiteral("\u4E8C"), QStringLiteral("\u4E09"),
    QStringLiteral("\u56DB"), QStringLiteral("\u4E94"), QStringLiteral("\u516D"),
    QStringLiteral("\u65E5")
};

CustomDateChooser::CustomDateChooser(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(280, 280);

    m_selectedDate = QDate::currentDate();
    m_currentYear = m_selectedDate.year();
    m_currentMonth = m_selectedDate.month();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    initTopWidget(mainLayout);

    QWidget *middleWidget = new QWidget(this);
    QHBoxLayout *middleLayout = new QHBoxLayout(middleWidget);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(0);

    initCalendarGrid(middleLayout);
    initTimeSelector(middleLayout);

    mainLayout->addWidget(middleWidget, 1);

    initBottomWidget(mainLayout);

    updateCalendar();
}

void CustomDateChooser::initTopWidget(QVBoxLayout *parentLayout)
{
    QWidget *topWidget = new QWidget(this);
    topWidget->setObjectName("TopBar");
    topWidget->setFixedHeight(30);
    topWidget->setStyleSheet("background: #f5f5f5; border-bottom: 1px solid #eee;");

    QHBoxLayout *hbox = new QHBoxLayout(topWidget);
    hbox->setContentsMargins(8, 0, 8, 0);
    hbox->setSpacing(4);

    m_yearMonthLabel = new QPushButton(topWidget);
    m_yearMonthLabel->setObjectName("YearMonthBtn");
    m_yearMonthLabel->setCursor(Qt::PointingHandCursor);
    m_yearMonthLabel->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: #333; font-size: 12px; font-weight: bold; padding: 2px 6px; border-radius: 4px; }"
        "QPushButton:hover { background: #e8e8e8; }"
        "QPushButton:pressed { background: #d8d8d8; }");

    hbox->addWidget(m_yearMonthLabel);
    hbox->addStretch();

    auto createNavBtn = [topWidget](const QString &text) -> QPushButton* {
        auto *btn = new QPushButton(text, topWidget);
        btn->setObjectName("NavBtn");
        btn->setFixedSize(22, 22);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { background: transparent; border: 1px solid #ccc; border-radius: 3px; color: #555; font-size: 11px; font-weight: bold; }"
            "QPushButton:hover { background: #e8e8e8; border-color: #999; color: #333; }"
            "QPushButton:pressed { background: #ddd; }");
        return btn;
    };

    QPushButton *yearJumpLeft  = createNavBtn("\u00AB");
    QPushButton *yearStepLeft  = createNavBtn("\u2039");
    QPushButton *yearStepRight = createNavBtn("\u203A");
    QPushButton *yearJumpRight = createNavBtn("\u00BB");

    hbox->addWidget(yearJumpLeft);
    hbox->addWidget(yearStepLeft);
    hbox->addWidget(yearStepRight);
    hbox->addWidget(yearJumpRight);

    connect(m_yearMonthLabel, &QPushButton::clicked, this, &CustomDateChooser::onYearLabelClicked);
    connect(yearJumpLeft,  &QPushButton::clicked, this, [this]() { onYearChanged(-12); });
    connect(yearStepLeft,  &QPushButton::clicked, this, [this]() { onYearChanged(-1); });
    connect(yearStepRight, &QPushButton::clicked, this, [this]() { onYearChanged(1); });
    connect(yearJumpRight, &QPushButton::clicked, this, [this]() { onYearChanged(12); });

    parentLayout->addWidget(topWidget);
}

void CustomDateChooser::initCalendarGrid(QHBoxLayout *parentLayout)
{
    QWidget *calendarWidget = new QWidget(this);
    calendarWidget->setObjectName("CalendarWidget");
    calendarWidget->setStyleSheet("background: #ffffff;");

    QVBoxLayout *calLayout = new QVBoxLayout(calendarWidget);
    calLayout->setContentsMargins(2, 1, 2, 1);
    calLayout->setSpacing(0);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setSpacing(0);

    for (int i = 0; i < 7; ++i) {
        QLabel *label = new QLabel(WEEKDAY_NAMES[i], calendarWidget);
        label->setAlignment(Qt::AlignCenter);
        label->setFixedHeight(20);
        if (i >= 5) {
            label->setStyleSheet("color: #2196F3; font-size: 10px; font-weight: 600; background: transparent;");
        } else {
            label->setStyleSheet("color: #666; font-size: 10px; font-weight: 500; background: transparent;");
        }
        headerLayout->addWidget(label);
    }
    calLayout->addLayout(headerLayout);

    QGridLayout *dayGrid = new QGridLayout;
    dayGrid->setSpacing(0);

    for (int i = 0; i < 42; ++i) {
        m_dayBtns[i] = new CalendarDayBtn(calendarWidget);
        int row = i / 7;
        int col = i % 7;
        dayGrid->addWidget(m_dayBtns[i], row, col);
        connect(m_dayBtns[i], &QPushButton::clicked, this, [this, i]() { onDayBtnClicked(i); });
    }

    calLayout->addLayout(dayGrid);

    parentLayout->addWidget(calendarWidget, 1);
}

void CustomDateChooser::initTimeSelector(QHBoxLayout *parentLayout)
{
    QWidget *timeWidget = new QWidget(this);
    timeWidget->setObjectName("TimeArea");
    timeWidget->setFixedWidth(100);
    timeWidget->setStyleSheet("background: #fafafa; border-left: 1px solid #eee;");

    m_timeSelector = new TimeSelector(timeWidget);

    QVBoxLayout *tLayout = new QVBoxLayout(timeWidget);
    tLayout->setContentsMargins(1, 1, 1, 1);
    tLayout->setSpacing(0);
    tLayout->addWidget(m_timeSelector);

    connect(m_timeSelector, &TimeSelector::timeChanged, this, &CustomDateChooser::onTimeChanged);

    parentLayout->addWidget(timeWidget);
}

void CustomDateChooser::initBottomWidget(QVBoxLayout *parentLayout)
{
    QWidget *bottomWidget = new QWidget(this);
    bottomWidget->setObjectName("BottomBar");
    bottomWidget->setFixedHeight(30);
    bottomWidget->setStyleSheet("background: #ffffff; border-top: 1px solid #eee;");

    QHBoxLayout *hbox = new QHBoxLayout(bottomWidget);
    hbox->setContentsMargins(8, 0, 8, 0);
    hbox->setSpacing(6);

    auto createLeftBtn = [](const QString &text, QWidget *parent) -> QPushButton* {
        auto *btn = new QPushButton(text, parent);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: #2196F3; font-size: 12px; font-weight: 500; padding: 2px 6px; border-radius: 4px; }"
            "QPushButton:hover { background: #e3f2fd; }"
            "QPushButton:pressed { background: #bbdefb; }");
        return btn;
    };

    QPushButton *clearBtn  = createLeftBtn("\u6E05\u7A7A", bottomWidget);
    QPushButton *nowBtn    = createLeftBtn("\u73B0\u5728", bottomWidget);
    QPushButton *todayBtn  = createLeftBtn("\u4ECA\u5929", bottomWidget);

    QPushButton *cancelBtn = new QPushButton("\u53D6\u6D88", bottomWidget);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(
        "QPushButton { background: #f5f5f5; border: 1px solid #ddd; color: #333; font-size: 12px; font-weight: 500; padding: 2px 12px; border-radius: 4px; }"
        "QPushButton:hover { background: #e8e8e8; border-color: #bbb; }"
        "QPushButton:pressed { background: #ddd; }");

    QPushButton *confirmBtn = new QPushButton("\u786E\u8BA4", bottomWidget);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(
        "QPushButton { background: #2196F3; border: none; color: #fff; font-size: 12px; font-weight: 600; padding: 3px 16px; border-radius: 4px; }"
        "QPushButton:hover { background: #1E88E5; }"
        "QPushButton:pressed { background: #1976D2; }");

    hbox->addWidget(clearBtn);
    hbox->addWidget(nowBtn);
    hbox->addWidget(todayBtn);
    hbox->addStretch();
    hbox->addWidget(cancelBtn);
    hbox->addWidget(confirmBtn);

    connect(clearBtn, &QPushButton::clicked, this, &CustomDateChooser::onClearClicked);
    connect(nowBtn, &QPushButton::clicked, this, &CustomDateChooser::onNowClicked);
    connect(todayBtn, &QPushButton::clicked, this, &CustomDateChooser::onTodayClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &CustomDateChooser::onCancelClicked);
    connect(confirmBtn, &QPushButton::clicked, this, &CustomDateChooser::onConfirmClicked);

    parentLayout->addWidget(bottomWidget);
}

void CustomDateChooser::updateCalendar()
{
    m_yearMonthLabel->setText(QString("%1\u5E74%2\u6708 \u25BE")
                             .arg(m_currentYear)
                             .arg(m_currentMonth, 2, 10, QChar('0')));

    QDate firstDay(m_currentYear, m_currentMonth, 1);
    int startDayOfWeek = firstDay.dayOfWeek();
    int offset = startDayOfWeek - 1;
    QDate prevMonth = firstDay.addMonths(-1);
    int prevMonthDays = prevMonth.daysInMonth();

    for (int i = 0; i < offset; ++i) {
        int day = prevMonthDays - offset + i + 1;
        QDate date(prevMonth.year(), prevMonth.month(), day);
        m_dayBtns[i]->setDay(date, CalendarDayBtn::Disabled);
    }

    int daysInMonth = firstDay.daysInMonth();
    QDate today = QDate::currentDate();

    for (int i = 0; i < daysInMonth; ++i) {
        int day = i + 1;
        QDate date(m_currentYear, m_currentMonth, day);
        CalendarDayBtn::DayType type;

        bool isWeekend = (date.dayOfWeek() == Qt::Saturday || date.dayOfWeek() == Qt::Sunday);

        if (date == m_selectedDate) {
            type = CalendarDayBtn::Selected;
        } else if (date == today) {
            type = CalendarDayBtn::Today;
        } else if (isWeekend) {
            type = CalendarDayBtn::Weekend;
        } else {
            type = CalendarDayBtn::Normal;
        }

        m_dayBtns[offset + i]->setDay(date, type);
    }

    int filled = offset + daysInMonth;
    QDate nextMonth = firstDay.addMonths(1);
    for (int i = filled; i < 42; ++i) {
        int day = i - filled + 1;
        QDate date(nextMonth.year(), nextMonth.month(), day);
        m_dayBtns[i]->setDay(date, CalendarDayBtn::Disabled);
    }
}

void CustomDateChooser::setSelectedDate(const QDate &date)
{
    m_selectedDate = date;
    m_currentYear = date.year();
    m_currentMonth = date.month();
    updateCalendar();
}

void CustomDateChooser::setSelectedDateTime(const QDateTime &dt)
{
    m_selectedDate = dt.date();
    m_currentYear = m_selectedDate.year();
    m_currentMonth = m_selectedDate.month();
    m_timeSelector->setTime(dt.time().hour(), dt.time().minute());
    updateCalendar();
}

void CustomDateChooser::onYearLabelClicked()
{
    YearMonthPopup popup(this);
    popup.setYearMonth(m_currentYear, m_currentMonth);

    QPoint pos = m_yearMonthLabel->mapToGlobal(QPoint(0, m_yearMonthLabel->height() + 4));
    popup.move(pos);

    connect(&popup, &YearMonthPopup::yearMonthSelected, this, &CustomDateChooser::onYearMonthSelected);
    popup.exec();
}

void CustomDateChooser::onYearMonthSelected(int year, int month)
{
    m_currentYear = year;
    m_currentMonth = month;
    updateCalendar();
}

void CustomDateChooser::onYearChanged(int delta)
{
    m_currentMonth += delta;
    while (m_currentMonth > 12) {
        m_currentMonth -= 12;
        m_currentYear++;
    }
    while (m_currentMonth < 1) {
        m_currentMonth += 12;
        m_currentYear--;
    }
    updateCalendar();
}

void CustomDateChooser::onDayBtnClicked(int index)
{
    CalendarDayBtn *btn = m_dayBtns[index];
    if (btn->dayType() == CalendarDayBtn::Disabled)
        return;

    m_selectedDate = btn->date();
    m_currentYear = m_selectedDate.year();
    m_currentMonth = m_selectedDate.month();
    updateCalendar();
    emit dateTimeChanged(selectedDateTime());
}

void CustomDateChooser::onTodayClicked()
{
    QDate today = QDate::currentDate();
    m_selectedDate = today;
    m_currentYear = today.year();
    m_currentMonth = today.month();
    updateCalendar();
}

void CustomDateChooser::onNowClicked()
{
    QDateTime now = QDateTime::currentDateTime();
    m_selectedDate = now.date();
    m_currentYear = m_selectedDate.year();
    m_currentMonth = m_selectedDate.month();
    m_timeSelector->setTime(now.time().hour(), now.time().minute());
    updateCalendar();
}

void CustomDateChooser::onClearClicked()
{
    m_selectedDate = QDate();
    m_timeSelector->setTime(0, 0);
    updateCalendar();
}

void CustomDateChooser::onConfirmClicked()
{
    emit dateTimeConfirmed(selectedDateTime());
}

void CustomDateChooser::onCancelClicked()
{
    m_selectedDate = QDate();
    m_currentYear = QDate::currentDate().year();
    m_currentMonth = QDate::currentDate().month();
    m_timeSelector->setTime(0, 0);
    updateCalendar();
}

void CustomDateChooser::onTimeChanged(int hour, int minute)
{
    Q_UNUSED(hour);
    Q_UNUSED(minute);
    if (m_selectedDate.isValid())
        emit dateTimeChanged(selectedDateTime());
}
