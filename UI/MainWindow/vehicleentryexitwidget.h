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
    static constexpr int kRowHeight   = 64;     // 行高(px)
    static constexpr int kSeparator   = 1;      // 分隔线高(px)
    static constexpr int kBadgeWidth  = 56;     // 状态标签宽(px)
    static constexpr int kBadgeHeight = 24;     // 状态标签高(px)
    static constexpr int kTextPadding = 12;     // 左右 padding(px)
    static constexpr int kFontSizePlate = 17;   // 车牌号字号(px)
    static constexpr int kFontSizeTime  = 12;   // 时间字号(px)
    static constexpr int kFontSizeBadge = 12;   // 状态标签字号(px)
    static constexpr int kCardRadius   = 12;    // 卡片圆角(px)

    static const QColor kColorCardBg;          // #FFFFFF  卡片背景
    static const QColor kColorPlateText;       // #1F2937  车牌文字(深灰)
    static const QColor kColorTimeText;        // #6B7280  时间文字(中灰,AA 达标;正文档 §8 修正)
    static const QColor kColorSeparator;       // #E5E7EB  分隔线
    static const QColor kColorInBadgeBg;       // #2563EB  入库标签底(蓝)
    static const QColor kColorInBadgeText;     // #FFFFFF  入库标签字(白)
    static const QColor kColorOutBadgeBg;      // #F3F4F6  出库标签底(浅灰)
    static const QColor kColorOutBadgeText;    // #4B5563  出库标签字(深灰)
    static const QColor kColorFadeEdge;        // 顶部/底部渐变提示可滚动(白→透明)

    QList<VehicleEntry> m_entries;
    int m_maxCount = 10;    // 默认 10 条
    int m_scrollY  = 0;     // 滚动偏移(px);0 = 顶部,>0 = 向下滚动

    // 私有工具函数
    int  contentHeight() const;          // m_entries 实际高度
    int  maxScrollY() const;             // 允许的最大 m_scrollY
    void clampScrollY();                 // 把 m_scrollY 夹到合法范围
    void drawEdgeFade(QPainter &p);      // 顶部/底部淡入淡出提示溢出
};

#endif