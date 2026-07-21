#include "datelineedit.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
DateLineEdit::DateLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_iconSize(24, 24)
    , m_rightIconSize(24, 24)
{
    setMouseTracking(true);
    updateTextMargins();
}
void DateLineEdit::setLeftIcon(const QIcon &icon)
{
    m_leftIcon = icon;
    update();
}
void DateLineEdit::setRightIcon(const QIcon &icon)
{
    m_rightIcon = icon;
    update();
}
void DateLineEdit::setIconSize(const QSize &size)
{
    m_iconSize = size;
    updateTextMargins();
    update();
}
void DateLineEdit::setRightIconSize(const QSize &size)
{
    m_rightIconSize = size;
    updateTextMargins();
    update();
}
void DateLineEdit::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!m_leftIcon.isNull()) {
        int x = m_iconSpacing;
        int y = (height() - m_iconSize.height()) / 2;
        m_leftIcon.paint(&painter, QRect(x, y, m_iconSize.width(), m_iconSize.height()));
    }
    if (!m_rightIcon.isNull()) {
        int x = width() - m_iconSpacing - m_rightIconSize.width();
        int y = (height() - m_rightIconSize.height()) / 2;
        m_rightIcon.paint(&painter, QRect(x, y, m_rightIconSize.width(), m_rightIconSize.height()));
    }
}
void DateLineEdit::mousePressEvent(QMouseEvent *event)
{
    if (!m_rightIcon.isNull() && rightIconRect().contains(event->position().toPoint())) {
        emit rightIconClicked();
        return;
    }
    QLineEdit::mousePressEvent(event);
}
void DateLineEdit::mouseMoveEvent(QMouseEvent *event)
{
    bool onRightIcon = rightIconRect().contains(event->position().toPoint());
    setCursor(onRightIcon ? Qt::PointingHandCursor : Qt::ArrowCursor);
    QLineEdit::mouseMoveEvent(event);
}
QRect DateLineEdit::rightIconRect() const
{
    int x = width() - m_iconSpacing - m_rightIconSize.width();
    int y = (height() - m_rightIconSize.height()) / 2;
    return QRect(x, y, m_rightIconSize.width(), m_rightIconSize.height());
}
void DateLineEdit::updateTextMargins()
{
    int leftMargin = m_iconSpacing + m_iconSize.width() + m_textSpacing;
    int rightMargin = m_iconSpacing + m_rightIconSize.width() + m_textSpacing;
    setTextMargins(leftMargin, 0, rightMargin, 0);
}