#include "vehicleentryexitwidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QFont>
#include <QStyleOption>
#include <QtMath>
#include <QPainterPath>

// 颜色常量对齐文档 §8 验收清单(背景白 #FFFFFF,字体/标签色 WCAG AA)
// 自绘不设 objectName,QSS 选色器失效 — 颜色在 paintEvent 中硬编码
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
    // 必须允许滚轮事件(默认 QWidget 已接收,但显式声明醒目)
    setFocusPolicy(Qt::WheelFocus);
    // 控件尺寸跟随占位 QWidget(由 Designer 提升时确定最小高)
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

// ===== 公共 API =====
void VehicleEntryExitWidget::setEntries(const QList<VehicleEntry> &entries)
{
    m_entries = entries;
    if (m_entries.size() > m_maxCount)
        m_entries = m_entries.mid(0, m_maxCount);
    m_scrollY = 0;   // 重置滚动(避免视口留在底部看到空白)
    update();
}

// 头部插入(最新在上) — 由 MainWindow 的 onEntrySearchButton/onExitSearchButton 成功后调用
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

// ===== 私有工具 — 滚动范围计算 =====
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

// ===== wheelEvent — 滚轮自接管滚动 =====
// 不创建任何滚动条控件 → 仅调整 m_scrollY(像素偏移),paintEvent 内部用偏移绘制
void VehicleEntryExitWidget::wheelEvent(QWheelEvent *event)
{
    if (m_entries.isEmpty() || maxScrollY() == 0) {
        event->ignore();
        return;
    }
    // angleDelta().y():上滚为正,下滚为负;乘以系数控制滚动速度
    int delta = event->angleDelta().y();
    m_scrollY = qBound(0, m_scrollY - delta, maxScrollY());
    update();
    event->accept();
}

// ===== paintEvent — 整个控件的所有视觉内容 =====
// 顺序:剪贴板背景 → 每条记录(车牌+时间+标签)+ 分隔线 → 溢出提示渐变
void VehicleEntryExitWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // 1. 画卡片圆角背景(填充)
    p.setPen(Qt::NoPen);
    p.setBrush(kColorCardBg);
    p.drawRoundedRect(rect(), kCardRadius, kCardRadius);

    // 1.5 边框描边(直角,与原始卡片形状一致) — #E5E7EB 与分隔线同色
    // 用 QRectF 并内缩 0.5px 以对齐像素网格,避免 HiDPI 下半像素模糊
    p.setPen(QPen(kColorSeparator, 1));
    p.setBrush(Qt::NoBrush);
    QRectF borderRect = rect();
    borderRect.adjust(0.5, 0.5, -0.5, -0.5);
    p.drawRect(borderRect);

    if (m_entries.isEmpty()) {
        // 扩展点:可在这里画空态占位(暂无记录);简单起见先留白
        return;
    }

    // 2. 裁剪到圆角矩形内(避免绘制溢出)
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), kCardRadius, kCardRadius);
    p.setClipPath(clipPath);

    // 3. 遍历每条记录绘制
    const int rowStride = kRowHeight + kSeparator;
    for (int i = 0; i < m_entries.size(); ++i) {
        const VehicleEntry &e = m_entries[i];
        int y = i * rowStride - m_scrollY;

        // 剪掉视口外的行(显著减少绘制开销,10 条数据下不明显,但习惯保留)
        if (y + kRowHeight < 0 || y > height()) continue;

        // 3.1 车牌 — 17sp Medium,深灰
        QFont f = font();
        f.setPixelSize(kFontSizePlate);
        f.setWeight(QFont::Medium);
        p.setFont(f);
        p.setPen(kColorPlateText);
        p.drawText(kTextPadding, y + 24, e.plate);   // 基线 y+24 让文字在行内视觉居中偏上

        // 3.2 时间 — 12sp Normal,中灰(对齐 §8 修正)
        f.setPixelSize(kFontSizeTime);
        f.setWeight(QFont::Normal);
        p.setFont(f);
        p.setPen(kColorTimeText);
        p.drawText(kTextPadding, y + 48, e.timeStr());

        // 3.3 状态标签 — 圆角矩形 + 居中字
        QRect badge(width() - kTextPadding - kBadgeWidth, y + (kRowHeight - kBadgeHeight) / 2,
                   kBadgeWidth, kBadgeHeight);
        p.setPen(Qt::NoPen);
        p.setBrush(e.isEntry() ? kColorInBadgeBg : kColorOutBadgeBg);
        p.drawRect(badge);
        p.setFont(QFont(font().family(), kFontSizeBadge, QFont::Medium));
        p.setPen(e.isEntry() ? kColorInBadgeText : kColorOutBadgeText);
        p.drawText(badge, Qt::AlignCenter, e.isEntry() ? QStringLiteral("入库") : QStringLiteral("出库"));

        // 3.4 分隔线 — 最后一行不画(避免贴在卡片下边缘)
        if (i + 1 < m_entries.size()) {
            p.setPen(QPen(kColorSeparator, 1));
            p.drawLine(kTextPadding, y + kRowHeight,
                       width() - kTextPadding, y + kRowHeight);
        }
    }

    // 4. 顶部/底部渐变提示溢出(因为没有滚动条,这是用户唯一的可滚动视觉线索)
    drawEdgeFade(p);
}

// 顶部/底部渐变 — 当 m_scrollY>0 时顶部渐变;当 m_scrollY<maxScrollY 时底部渐变
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
