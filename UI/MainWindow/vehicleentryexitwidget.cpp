#include "vehicleentryexitwidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QFont>
#include <QStyleOption>
#include <QtMath>
#include <QPainterPath>
const QColor VehicleEntryExitWidget::kColorCardBg       = 0xFFFFFF;
const QColor VehicleEntryExitWidget::kColorPlateText    = 0x1F2937;
const QColor VehicleEntryExitWidget::kColorTimeText      = 0x6B7280;
const QColor VehicleEntryExitWidget::kColorSeparator     = 0xE5E7EB;
const QColor VehicleEntryExitWidget::kColorInBadgeBg     = 0x2563EB;
const QColor VehicleEntryExitWidget::kColorInBadgeText   = 0xFFFFFF;
const QColor VehicleEntryExitWidget::kColorOutBadgeBg    = 0xF3F4F6;
const QColor VehicleEntryExitWidget::kColorOutBadgeText  = 0x4B5563;
const QColor VehicleEntryExitWidget::kColorFadeEdge      = 0xFFFFFF;
VehicleEntryExitWidget::VehicleEntryExitWidget(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::WheelFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
void VehicleEntryExitWidget::setEntries(const QList<VehicleEntry> &entries)
{
    m_entries = entries;
    if (m_entries.size() > m_maxCount)
        m_entries = m_entries.mid(0, m_maxCount);
    m_scrollY = 0;
    update();
}
void VehicleEntryExitWidget::prependEntry(const VehicleEntry &entry)
{
    m_entries.prepend(entry);
    while (m_entries.size() > m_maxCount)
        m_entries.removeLast();
    clampScrollY();
    update();
}
void VehicleEntryExitWidget::clear()
{
    m_entries.clear();
    m_scrollY = 0;
    update();
}
void VehicleEntryExitWidget::setMaxCount(int n)
{
    m_maxCount = qMax(1, n);
    clampScrollY();
    update();
}
int VehicleEntryExitWidget::contentHeight() const
{
    if (m_entries.isEmpty()) return 0;
    return m_entries.size() * kRowHeight
         + (m_entries.size() - 1) * kSeparator;
}
int VehicleEntryExitWidget::maxScrollY() const
{
    return qMax(0, contentHeight() - height());
}
void VehicleEntryExitWidget::clampScrollY()
{
    m_scrollY = qBound(0, m_scrollY, maxScrollY());
}
void VehicleEntryExitWidget::wheelEvent(QWheelEvent *event)
{
    if (m_entries.isEmpty() || maxScrollY() == 0) {
        event->ignore();
        return;
    }
    int delta = event->angleDelta().y();
    m_scrollY = qBound(0, m_scrollY - delta, maxScrollY());
    update();
    event->accept();
}
void VehicleEntryExitWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    p.setPen(Qt::NoPen);
    p.setBrush(kColorCardBg);
    p.drawRoundedRect(rect(), kCardRadius, kCardRadius);

    p.setPen(QPen(kColorSeparator, 1));
    p.setBrush(Qt::NoBrush);
    QRectF borderRect = rect();
    borderRect.adjust(0.5, 0.5, -0.5, -0.5);
    p.drawRect(borderRect);
    if (m_entries.isEmpty()) {
        return;
    }

    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), kCardRadius, kCardRadius);
    p.setClipPath(clipPath);

    const int rowStride = kRowHeight + kSeparator;
    for (int i = 0; i < m_entries.size(); ++i) {
        const VehicleEntry &e = m_entries[i];
        int y = i * rowStride - m_scrollY;

        if (y + kRowHeight < 0 || y > height()) continue;

        QFont f = font();
        f.setPixelSize(kFontSizePlate);
        f.setWeight(QFont::Medium);
        p.setFont(f);
        p.setPen(kColorPlateText);
        p.drawText(kTextPadding, y + 24, e.plate);

        f.setPixelSize(kFontSizeTime);
        f.setWeight(QFont::Normal);
        p.setFont(f);
        p.setPen(kColorTimeText);
        p.drawText(kTextPadding, y + 48, e.timeStr());

        QRect badge(width() - kTextPadding - kBadgeWidth, y + (kRowHeight - kBadgeHeight) / 2,
                   kBadgeWidth, kBadgeHeight);
        p.setPen(Qt::NoPen);
        p.setBrush(e.isEntry() ? kColorInBadgeBg : kColorOutBadgeBg);
        p.drawRect(badge);
        p.setFont(QFont(font().family(), kFontSizeBadge, QFont::Medium));
        p.setPen(e.isEntry() ? kColorInBadgeText : kColorOutBadgeText);
        p.drawText(badge, Qt::AlignCenter, e.isEntry() ? QStringLiteral("入库") : QStringLiteral("出库"));

        if (i + 1 < m_entries.size()) {
            p.setPen(QPen(kColorSeparator, 1));
            p.drawLine(kTextPadding, y + kRowHeight,
                       width() - kTextPadding, y + kRowHeight);
        }
    }

    drawEdgeFade(p);
}
void VehicleEntryExitWidget::drawEdgeFade(QPainter &p)
{
    const int fadeH = 12;
    if (m_scrollY > 0) {
        QLinearGradient topFade(0, 0, 0, fadeH);
        topFade.setColorAt(0.0, kColorFadeEdge);
        topFade.setColorAt(1.0, Qt::transparent);
        p.fillRect(0, 0, width(), fadeH, topFade);
    }
    int maxSY = maxScrollY();
    if (m_scrollY < maxSY) {
        QLinearGradient bottomFade(0, height() - fadeH, 0, height());
        bottomFade.setColorAt(0.0, Qt::transparent);
        bottomFade.setColorAt(1.0, kColorFadeEdge);
        p.fillRect(0, height() - fadeH, width(), fadeH, bottomFade);
    }
}