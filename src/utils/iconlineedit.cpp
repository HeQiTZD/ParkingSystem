#include "iconlineedit.h"
#include <QPainter>
#include <QPaintEvent>
IconLineEdit::IconLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , m_iconSize(24, 24)
    , m_iconSpacing(6)
    , m_textSpacing(3)
{

    updateTextMargins();
}
void IconLineEdit::setIcon(const QIcon &icon)
{

    m_icon = icon;
    update();
}
void IconLineEdit::setIconPath(const QString &path)
{

    setIcon(QIcon(path));
}
void IconLineEdit::setIconSize(const QSize &size)
{

    m_iconSize = size;
    updateTextMargins();
    update();
}
void IconLineEdit::setIconSpacing(int spacing)
{

    m_iconSpacing = spacing;
    updateTextMargins();
    update();
}
int IconLineEdit::iconSpacing() const
{
    return m_iconSpacing;
}
void IconLineEdit::setTextSpacing(int spacing)
{
    m_textSpacing = spacing;
    updateTextMargins();
    update();
}
int IconLineEdit::textSpacing() const
{
    return m_textSpacing;
}
void IconLineEdit::paintEvent(QPaintEvent *event)
{

    QLineEdit::paintEvent(event);

    if(m_icon.isNull()){
        return;
    }

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    int x = m_iconSpacing;
    int y = (height() - m_iconSize.height()) / 2;
    QRect iconRect(x, y, m_iconSize.width(), m_iconSize.height());

    m_icon.paint(&painter, iconRect);

}
void IconLineEdit::updateTextMargins()
{

    int leftMargin = m_iconSpacing + m_iconSize.width() + m_textSpacing;

    setTextMargins(leftMargin, 0, 0, 0);
}