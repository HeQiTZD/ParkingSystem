#include "calendardaybtn.h"
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>

CalendarDayBtn::CalendarDayBtn(QWidget *parent)
    : QPushButton(parent)
{
    setFixedSize(30, 30);
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void CalendarDayBtn::setDay(const QDate &date, DayType type)
{
    m_date = date;
    m_type = type;

    if (type == Disabled) {
        setEnabled(false);
        setCursor(Qt::ForbiddenCursor);
    } else {
        setEnabled(true);
        setCursor(Qt::PointingHandCursor);
    }
    m_hovered = false;
    m_pressed = false;
    update();
}

void CalendarDayBtn::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    if (m_type != Disabled) {
        m_hovered = true;
        update();
    }
}

void CalendarDayBtn::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hovered = false;
    m_pressed = false;
    update();
}

void CalendarDayBtn::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_type != Disabled) {
        m_pressed = true;
        update();
    }
    QPushButton::mousePressEvent(event);
}

void CalendarDayBtn::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    update();
    QPushButton::mouseReleaseEvent(event);
}

void CalendarDayBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect rect = this->rect();
    int radius = 5;

    // 绘制背景
    switch (m_type) {
    case Selected:
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(60, 60, 60));
        painter.drawRoundedRect(rect, radius, radius);
        break;
    case Today:
        // 无背景，后面画蓝色边框
        break;
    case Weekend:
        if (m_hovered) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(235, 245, 255));
            painter.drawRoundedRect(rect, radius, radius);
        } else if (m_pressed) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(215, 235, 250));
            painter.drawRoundedRect(rect, radius, radius);
        }
        break;
    case Normal:
        if (m_hovered) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(240, 240, 240));
            painter.drawRoundedRect(rect, radius, radius);
        } else if (m_pressed) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(220, 220, 220));
            painter.drawRoundedRect(rect, radius, radius);
        }
        break;
    case Disabled:
        // 无背景
        break;
    }

    //绘制今日边框
    if (m_type == Today) {
        painter.setPen(QPen(QColor(30, 120, 220), 1));
        painter.setBrush(Qt::NoBrush);
        int pad = 2;
        QRect borderRect(rect.x() + pad, rect.y() + pad, rect.width() - pad * 2, rect.height() - pad * 2);
        painter.drawRoundedRect(borderRect, radius - 1, radius - 1);
    }

    //绘制选中边框
    if (m_type == Selected) {
        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(Qt::NoBrush);
        int pad = 2;
        QRect borderRect(rect.x() + pad, rect.y() + pad, rect.width() - pad * 2, rect.height() - pad * 2);
        painter.drawRoundedRect(borderRect, radius - 1, radius - 1);
    }

    // 绘制文字
    QFont font = painter.font();
    font.setPointSize(9);
    QColor textColor;

    switch (m_type) {
    case Disabled:
        textColor = QColor(190, 190, 190);
        font.setWeight(QFont::Normal);
        break;
    case Selected:
        textColor = Qt::black;
        font.setWeight(QFont::Bold);
        break;
    case Today:
        textColor = QColor(30, 120, 220);
        font.setWeight(QFont::Bold);
        break;
    case Weekend:
        textColor = QColor(30, 120, 220);
        font.setWeight(QFont::Normal);
        break;
    case Normal:
    default:
        textColor = Qt::black;
        font.setWeight(QFont::Normal);
        break;
    }

    painter.setFont(font);
    painter.setPen(textColor);
    painter.drawText(rect, Qt::AlignCenter, QString::number(m_date.day()));
}
