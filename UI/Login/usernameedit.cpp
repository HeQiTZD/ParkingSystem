#include "usernameedit.h"
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
UsernameEdit::UsernameEdit(QWidget *parent) : QLineEdit(parent), m_hasFocus(false)
{
    setFixedHeight(41);
    setStyleSheet("border: none; background: transparent;");
}
void UsernameEdit::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int penWidth = 2;
    int halfPen = penWidth / 2;

    QPainterPath bgPath;
    bgPath.addRoundedRect(rect(), 8, 8);
    painter.setClipPath(bgPath);
    QColor bgColor = m_hasFocus ? QColor(0xE3F2FD) : QColor(0xF5F5F5);
    painter.fillPath(bgPath, bgColor);

    QPen borderPen(m_hasFocus ? QColor(0x2196F3) : QColor(0xE0E0E0));
    borderPen.setWidth(penWidth);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(halfPen, halfPen, -halfPen, -halfPen), 8, 8);

    painter.setClipRect(rect());
    QPixmap icon(":/icons/icon-person");
    if (!icon.isNull()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(12, (height() - 20) / 2, 20, 20, icon);
    }

    painter.setPen(m_hasFocus ? QColor(0x333) : QColor(0x999));
    QFont font("Microsoft YaHei", 12);
    painter.setFont(font);
    QRect textRect = rect().adjusted(42, 0, -10, 0);
    painter.drawText(textRect, Qt::AlignVCenter, text());
}
void UsernameEdit::focusInEvent(QFocusEvent *event)
{
    m_hasFocus = true;
    QLineEdit::focusInEvent(event);
    update();
}
void UsernameEdit::focusOutEvent(QFocusEvent *event)
{
    m_hasFocus = false;
    QLineEdit::focusOutEvent(event);
    update();
}