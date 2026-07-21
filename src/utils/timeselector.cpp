#include "timeselector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
static const QString TIME_BTN_NORMAL =
    "QPushButton { background: transparent; border: none; border-radius: 4px; color: #444; font-size: 11px; }"
    "QPushButton:hover { background: #e8f0fe; color: #1976D2; }"
    "QPushButton:pressed { background: #d0e4fc; }";
static const QString TIME_BTN_SELECTED =
    "QPushButton { background: #1976D2; border: none; border-radius: 4px; color: #fff; font-size: 11px; font-weight: bold; }"
    "QPushButton:pressed { background: #1565C0; }";
TimeColumn::TimeColumn(int start, int end, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);
    for (int v = start; v <= end; ++v) {
        m_values.append(v);
        auto *btn = new QPushButton(QString("%1").arg(v, 2, 10, QChar('0')), this);
        btn->setFixedSize(34, 24);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(TIME_BTN_NORMAL);
        m_btns.append(btn);
        connect(btn, &QPushButton::clicked, this, [this, v]() { onBtnClicked(v); });
        layout->addWidget(btn);
    }
    layout->addStretch();
    setSelected(start);
}
void TimeColumn::setSelected(int value)
{
    m_selected = value;
    for (int i = 0; i < m_values.size(); ++i) {
        if (m_values[i] == value) {
            m_btns[i]->setStyleSheet(TIME_BTN_SELECTED);
        } else {
            m_btns[i]->setStyleSheet(TIME_BTN_NORMAL);
        }
    }
}
void TimeColumn::onBtnClicked(int value)
{
    setSelected(value);
    emit valueSelected(value);
}
TimeSelector::TimeSelector(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->setSpacing(0);
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(8);
    QWidget *hourGroup = new QWidget(this);
    QVBoxLayout *hourLayout = new QVBoxLayout(hourGroup);
    hourLayout->setContentsMargins(0, 0, 0, 0);
    hourLayout->setSpacing(2);
    QLabel *hourHeader = new QLabel("\u65F6", hourGroup);
    hourHeader->setAlignment(Qt::AlignCenter);
    hourHeader->setFixedHeight(20);
    hourHeader->setStyleSheet("color: #888; font-size: 10px; font-weight: 600;");
    m_hourColumn = new TimeColumn(0, 23, hourGroup);
    QScrollArea *hourArea = new QScrollArea();
    hourArea->setObjectName("hourArea");
    hourArea->setWidgetResizable(true);
    hourArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hourArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    hourArea->setFrameShape(QFrame::NoFrame);
    hourArea->setFixedHeight(180);
    hourArea->setStyleSheet("QScrollArea { background: transparent; }");
    hourArea->setWidget(m_hourColumn);
    hourArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { width: 3px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #ccc; border-radius: 1px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");
    hourLayout->addWidget(hourHeader);
    hourLayout->addWidget(hourArea);
    QWidget *minuteGroup = new QWidget(this);
    QVBoxLayout *minuteLayout = new QVBoxLayout(minuteGroup);
    minuteLayout->setContentsMargins(0, 0, 0, 0);
    minuteLayout->setSpacing(2);
    QLabel *minuteHeader = new QLabel("\u5206", minuteGroup);
    minuteHeader->setAlignment(Qt::AlignCenter);
    minuteHeader->setFixedHeight(20);
    minuteHeader->setStyleSheet("color: #888; font-size: 10px; font-weight: 600;");
    m_minuteColumn = new TimeColumn(0, 59, minuteGroup);
    QScrollArea *minuteArea = new QScrollArea();
    minuteArea->setObjectName("minuteArea");
    minuteArea->setWidgetResizable(true);
    minuteArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    minuteArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    minuteArea->setFrameShape(QFrame::NoFrame);
    minuteArea->setFixedHeight(180);
    minuteArea->setStyleSheet("QScrollArea { background: transparent; }");
    minuteArea->setWidget(m_minuteColumn);
    minuteArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical { width: 3px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #ccc; border-radius: 1px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");
    minuteLayout->addWidget(minuteHeader);
    minuteLayout->addWidget(minuteArea);
    contentLayout->addWidget(hourGroup);
    contentLayout->addWidget(minuteGroup);
    mainLayout->addLayout(contentLayout);
    connect(m_hourColumn, &TimeColumn::valueSelected, this, &TimeSelector::onHourSelected);
    connect(m_minuteColumn, &TimeColumn::valueSelected, this, &TimeSelector::onMinuteSelected);
}
void TimeSelector::setTime(int hour, int minute)
{
    m_hourColumn->setSelected(hour);
    m_minuteColumn->setSelected(minute);
}
int TimeSelector::hour() const
{
    return m_hourColumn->selectedValue();
}
int TimeSelector::minute() const
{
    return m_minuteColumn->selectedValue();
}
void TimeSelector::onHourSelected(int value)
{
    Q_UNUSED(value);
    emit timeChanged(hour(), minute());
}
void TimeSelector::onMinuteSelected(int value)
{
    Q_UNUSED(value);
    emit timeChanged(hour(), minute());
}