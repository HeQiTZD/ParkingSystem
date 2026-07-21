#include "circleprogress.h"
#include <QPainter>
#include <QPen>
#include <QFont>
CircleProgress::CircleProgress(QWidget *parent)
    : QWidget(parent)
    , m_progress(70)
    , m_userText("已用空间")
{
}
void CircleProgress::setProgress(double progress)
{
    m_progress = progress;
    update();
}
void CircleProgress::setUsedText(const QString &text)
{
    m_userText = text;
    update();
}
QSize CircleProgress::sizeHint() const
{
    return QSize(150, 150);
}
QSize CircleProgress::minimumSizeHint() const
{
    return QSize(80, 80);
}
void CircleProgress::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width(), height());
    int penWidth = side / 12;
    if (penWidth < 6) penWidth = 6;
    if (penWidth > 15) penWidth = 15;
    int margin = penWidth / 2 + 2;
    QRectF rect(margin, margin, side - 2 * margin, side - 2 * margin);

    QPen bgPen(QColor(232, 237, 243), penWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(bgPen);
    painter.drawArc(rect, 0, 360 * 16);

    QPen progressPen(QColor(59, 130, 246), penWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progressPen);
    double startAngle = 90 * 16;
    double spanAngle = -(m_progress / 100.0) * 360 * 16;
    painter.drawArc(rect, startAngle, spanAngle);

    QFont percentFont("Arial", 10, QFont::Bold);
    painter.setFont(percentFont);
    painter.setPen(QColor(30, 58, 95));
    QString percentText = QString::number(static_cast<int>(m_progress)) + "%";
    QRectF textRect = rect.adjusted(0, -side * 0.20, 0, 0);
    painter.drawText(textRect, Qt::AlignCenter, percentText);

    QFont descFont("Arial", 10, QFont::Normal);
    painter.setFont(descFont);
    painter.setPen(QColor(107, 114, 128));
    QRectF descRect = rect.adjusted(0, side * 0.12, 0, 0);
    painter.drawText(descRect, Qt::AlignCenter, m_userText);
}