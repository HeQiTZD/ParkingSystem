#ifndef VEHICLEENTRYEXITWIDGET_H
#define VEHICLEENTRYEXITWIDGET_H
#include <QWidget>
#include <QList>
#include "vehicleentry.h"
class VehicleEntryExitWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VehicleEntryExitWidget(QWidget *parent = nullptr);
    void setEntries(const QList<VehicleEntry> &entries);
    void prependEntry(const VehicleEntry &entry);
    void clear();
    void setMaxCount(int n);
signals:
    void entryClicked(const VehicleEntry &entry);
protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *evnet) override;
private:
    static constexpr int kRowHeight   = 64;
    static constexpr int kSeparator   = 1;
    static constexpr int kBadgeWidth  = 56;
    static constexpr int kBadgeHeight = 24;
    static constexpr int kTextPadding = 12;
    static constexpr int kFontSizePlate = 17;
    static constexpr int kFontSizeTime  = 12;
    static constexpr int kFontSizeBadge = 12;
    static constexpr int kCardRadius   = 12;
    static const QColor kColorCardBg;
    static const QColor kColorPlateText;
    static const QColor kColorTimeText;
    static const QColor kColorSeparator;
    static const QColor kColorInBadgeBg;
    static const QColor kColorInBadgeText;
    static const QColor kColorOutBadgeBg;
    static const QColor kColorOutBadgeText;
    static const QColor kColorFadeEdge;
    QList<VehicleEntry> m_entries;
    int m_maxCount = 10;
    int m_scrollY  = 0;

    int  contentHeight() const;
    int  maxScrollY() const;
    void clampScrollY();
    void drawEdgeFade(QPainter &p);
};
#endif