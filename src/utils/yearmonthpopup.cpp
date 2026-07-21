#include "yearmonthpopup.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDate>
static const QString NAV_BTN_STYLE =
    "QPushButton { background: transparent; border: 1px solid #ccc; border-radius: 4px; color: #555; font-size: 11px; font-weight: bold; }"
    "QPushButton:hover { background: #f0f0f0; border-color: #999; color: #333; }"
    "QPushButton:pressed { background: #e0e0e0; }";
static const QString MONTH_BTN_NORMAL =
    "QPushButton { background: #f5f5f5; border: 1px solid #ddd; border-radius: 4px; color: #333; font-size: 10px; }"
    "QPushButton:hover { background: #e8e8e8; border-color: #bbb; }"
    "QPushButton:pressed { background: #d8d8d8; }";
static const QString MONTH_BTN_SELECTED =
    "QPushButton { background: #333; border: none; border-radius: 4px; color: #fff; font-size: 10px; font-weight: bold; }"
    "QPushButton:pressed { background: #555; }";
static const QString MONTH_BTN_CURRENT =
    "QPushButton { background: transparent; border: 2px solid #2196F3; border-radius: 4px; color: #2196F3; font-size: 10px; font-weight: bold; }"
    "QPushButton:hover { background: #e3f2fd; }"
    "QPushButton:pressed { background: #bbdefb; }";
YearMonthPopup::YearMonthPopup(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setFixedSize(230, 180);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(4);
    QHBoxLayout *yearLayout = new QHBoxLayout;
    yearLayout->setSpacing(2);
    auto createNavBtn = [this](const QString &text) -> QPushButton* {
        auto *btn = new QPushButton(text, this);
        btn->setFixedSize(26, 26);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(NAV_BTN_STYLE);
        return btn;
    };
    QPushButton *yearJumpLeft  = createNavBtn("\u00AB");
    QPushButton *yearStepLeft  = createNavBtn("\u2039");
    QPushButton *yearStepRight = createNavBtn("\u203A");
    QPushButton *yearJumpRight = createNavBtn("\u00BB");
    m_yearLabel = new QLabel(this);
    m_yearLabel->setAlignment(Qt::AlignCenter);
    m_yearLabel->setFixedWidth(70);
    m_yearLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #333;");
    yearLayout->addStretch();
    yearLayout->addWidget(yearJumpLeft);
    yearLayout->addWidget(yearStepLeft);
    yearLayout->addWidget(m_yearLabel);
    yearLayout->addWidget(yearStepRight);
    yearLayout->addWidget(yearJumpRight);
    yearLayout->addStretch();
    mainLayout->addLayout(yearLayout);
    QFrame *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #ddd; max-height: 1px;");
    mainLayout->addWidget(sep);
    QGridLayout *monthGrid = new QGridLayout;
    monthGrid->setSpacing(2);
    QDate today = QDate::currentDate();
    for (int i = 0; i < 12; ++i) {
        m_monthBtns[i] = new QPushButton(QString("%1\u6708").arg(i + 1), this);
        m_monthBtns[i]->setFixedSize(46, 28);
        m_monthBtns[i]->setCursor(Qt::PointingHandCursor);
        int row = i / 4;
        int col = i % 4;
        monthGrid->addWidget(m_monthBtns[i], row, col);
        connect(m_monthBtns[i], &QPushButton::clicked, this, [this, i]() {
            onMonthClicked(i + 1);
        });
    }
    mainLayout->addLayout(monthGrid);
    connect(yearJumpLeft, &QPushButton::clicked, this, [this]() { onYearChanged(-5); });
    connect(yearStepLeft, &QPushButton::clicked, this, [this]() { onYearChanged(-1); });
    connect(yearStepRight, &QPushButton::clicked, this, [this]() { onYearChanged(1); });
    connect(yearJumpRight, &QPushButton::clicked, this, [this]() { onYearChanged(5); });
    m_year = QDate::currentDate().year();
    m_month = QDate::currentDate().month();
    updateYearLabel();
    updateMonthButtons();
}
void YearMonthPopup::setYearMonth(int year, int month)
{
    m_year = year;
    m_month = month;
    updateYearLabel();
    updateMonthButtons();
}
void YearMonthPopup::updateYearLabel()
{
    m_yearLabel->setText(QString("%1\u5E74").arg(m_year));
}
void YearMonthPopup::updateMonthButtons()
{
    QDate today = QDate::currentDate();
    int sysYear = today.year();
    int sysMonth = today.month();
    for (int i = 0; i < 12; ++i) {
        int month = i + 1;
        bool isSystemMonth = (m_year == sysYear && month == sysMonth);
        bool isSelected = (month == m_month);
        if (isSelected) {
            m_monthBtns[i]->setStyleSheet(MONTH_BTN_SELECTED);
        } else if (isSystemMonth) {
            m_monthBtns[i]->setStyleSheet(MONTH_BTN_CURRENT);
        } else {
            m_monthBtns[i]->setStyleSheet(MONTH_BTN_NORMAL);
        }
    }
}
void YearMonthPopup::onYearChanged(int delta)
{
    m_year += delta;
    updateYearLabel();
}
void YearMonthPopup::onMonthClicked(int month)
{
    m_month = month;
    emit yearMonthSelected(m_year, m_month);
    accept();
}