#include "toastwidget.h"

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QScreen>
#include <QStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

// 内联 QSS
static const char *kToastQss = R"(
ToastWidget { background:transparent; }
QLabel[isIcon="true"] { font-size:18px; font-weight:bold; border-radius:16px; padding:2px; }
QLabel[isIcon="true"][severity="success"] { color:#3498db; background:#ebf3fd; }
QLabel[isIcon="true"][severity="error"]   { color:#e74c3c; background:#fdecea; }
QLabel[isIcon="true"][severity="warning"] { color:#f39c12; background:#fef5e7; }
QLabel[isIcon="true"][severity="info"]    { color:#3498db; background:#ebf3fd; }
QLabel#toastMsg { font-size:13px; color:#1a1a2e; padding:2px; }
QPushButton#toastClose { background:transparent; color:#bbb; font-size:14px; border:none; padding:0; }
QPushButton#toastClose:hover { color:#555; }
)";

// 静态成员初始化为空队列
QQueue<ToastWidget *> ToastWidget::s_active;

// ── 静态调用入口 ──────────────────────────────────────────────
void ToastWidget::show(QWidget *parent, const QString &msg,
                       ToastType type, int durationMs)
{
    // 没有父窗口时创建独立窗口（服务于 main.cpp 这种无 parent 场景）
    QWidget *realParent = parent;
    if (!realParent) {
        // 尝试取当前活动窗口
        realParent = QApplication::activeWindow();
    }

    ToastWidget *toast = new ToastWidget(realParent);
    if (realParent) {
        toast->setAttribute(Qt::WA_DeleteOnClose);
    }
    toast->showImmediatly(msg, type, durationMs);
}

// ── 构造 ──────────────────────────────────────────────────────
ToastWidget::ToastWidget(QWidget *parent)
    : QWidget(parent)
    , m_iconLabel(new QLabel(this))
    , m_msgLabel(new QLabel(this))
    , m_closeButton(new QPushButton(this))
    , m_posAnim(new QPropertyAnimation(this, "pos", this))
    , m_opaAnim(new QPropertyAnimation(this, "windowOpacity", this))
{
    // 无边框 + 置顶 + 透明背景
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint
                  | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(kToastWidth, kToastHeight);

    // ── 布局 ──────────────────────────────────────────────
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setProperty("isIcon", true);   // QSS 用

    m_msgLabel->setWordWrap(true);
    m_msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_msgLabel->setObjectName("toastMsg");

    m_closeButton->setFixedSize(18, 18);
    m_closeButton->setObjectName("toastClose");
    m_closeButton->setCursor(Qt::PointingHandCursor);
    connect(m_closeButton, &QPushButton::clicked, this, [this]() {
        slideOut([this]() { close(); });
    });

    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(14, 12, 14, 12);
    rootLayout->setSpacing(10);

    // 垂直居中放 icon
    QVBoxLayout *iconLayout = new QVBoxLayout;
    iconLayout->addStretch();
    iconLayout->addWidget(m_iconLabel);
    iconLayout->addStretch();
    rootLayout->addLayout(iconLayout);

    // 消息区域
    QVBoxLayout *msgLayout = new QVBoxLayout;
    msgLayout->addStretch();
    msgLayout->addWidget(m_msgLabel);
    msgLayout->addStretch();
    rootLayout->addLayout(msgLayout, 1);

    rootLayout->addWidget(m_closeButton, 0, Qt::AlignTop);

    setWindowOpacity(0.0);

    setStyleSheet(QString::fromLatin1(kToastQss));
}

// ── 显示一条 Toast ──────────────────────────────────────────
void ToastWidget::showImmediatly(const QString &msg, ToastType type, int durationMs)
{
    buildUi(msg, type);

    // 默认时长
    if (durationMs <= 0) {
        durationMs = (type == Failure || type == Warning) ? 3500 : 3000;
    }

    // 超出上限 → 销毁最旧的
    while (s_active.size() >= kMaxActive) {
        ToastWidget *oldest = s_active.dequeue();
        if (oldest && oldest != this) {
            oldest->slideOut([oldest]() { oldest->close(); });
        }
    }
    s_active.enqueue(this);

    // 监听父窗口销毁
    if (parentWidget()) {
        parentWidget()->installEventFilter(this);
    }

    // 计算起始位置
    QWidget *p = parentWidget();
    QPoint target;
    if (p) {
        target = QPoint(p->frameGeometry().right() - kToastWidth - kMarginRight,
                        p->frameGeometry().top() + kMarginTop);
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect geo = screen->availableGeometry();
        target = QPoint(geo.right() - kToastWidth - kMarginRight,
                        geo.top() + kMarginTop + (s_active.size() - 1) * (kToastHeight + kSpacing));
    }
    // 堆叠偏移
    target.setY(target.y() + (s_active.size() - 1) * (kToastHeight + kSpacing));

    move(target);
    QWidget::show();
    raise();
    slideIn();

    // 定时消失
    QTimer::singleShot(durationMs, this, [this]() {
        if (isVisible()) {
            slideOut([this]() { close(); });
        }
    });
}

// ── UI 构建 ──────────────────────────────────────────────────
void ToastWidget::buildUi(const QString &msg, ToastType type)
{
    m_msgLabel->setText(msg);

    // 图标字符 + 颜色
    QString iconChar, severity;
    switch (type) {
    case Success: iconChar = QStringLiteral("✓"); severity = "success"; break;
    case Failure: iconChar = QStringLiteral("✕"); severity = "error";   break;
    case Warning: iconChar = QStringLiteral("⚠"); severity = "warning"; break;
    case Info:    iconChar = QStringLiteral("ℹ"); severity = "info";    break;
    }
    m_iconLabel->setText(iconChar);
    m_iconLabel->setProperty("severity", severity);

    // 左侧竖条：给父 widget 设 severity 以便 QSS 用伪状态
    setProperty("severity", severity);
    setProperty("toastType", type);
    style()->unpolish(this);
    style()->polish(this);
}

// ── 动画 ─────────────────────────────────────────────────────
void ToastWidget::slideIn()
{
    // X 方向从屏幕外滑入
    QWidget *p = parentWidget();
    int targetX = p ? p->frameGeometry().right() - width() - kMarginRight
                    : QGuiApplication::primaryScreen()->availableGeometry().right() - width() - kMarginRight;
    int targetY = pos().y();
    int startX  = targetX + width() + 20;

    m_posAnim->setDuration(kSlideMs);
    m_posAnim->setStartValue(QPoint(startX, targetY));
    m_posAnim->setEndValue(QPoint(targetX, targetY));
    m_posAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_opaAnim->setDuration(kSlideMs);
    m_opaAnim->setStartValue(0.0);
    m_opaAnim->setEndValue(1.0);

    m_posAnim->start();
    m_opaAnim->start();
}

void ToastWidget::slideOut(const std::function<void()> &onFinished)
{
    // 从活跃队列移除
    s_active.removeOne(this);

    int startX = pos().x();
    int targetX = startX + width() + 20;

    m_posAnim->setDuration(kFadeMs);
    m_posAnim->setStartValue(QPoint(startX, pos().y()));
    m_posAnim->setEndValue(QPoint(targetX, pos().y()));
    m_posAnim->setEasingCurve(QEasingCurve::InCubic);

    m_opaAnim->setDuration(kFadeMs);
    m_opaAnim->setStartValue(1.0);
    m_opaAnim->setEndValue(0.0);

    connect(m_opaAnim, &QPropertyAnimation::finished, this, onFinished);
    m_posAnim->start();
    m_opaAnim->start();
}

// ── 父窗口销毁时自动清理 ─────────────────────────────────────
bool ToastWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Destroy) {
        s_active.removeOne(this);
        deleteLater();
    }
    return QWidget::eventFilter(obj, event);
}

// 绘制圆角卡片 + 左侧彩色竖条
void ToastWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 颜色
    QColor accent;
    switch (property("toastType").toInt()) {
    case Success: accent.setNamedColor("#3498db"); break;
    case Failure: accent.setNamedColor("#e74c3c"); break;
    case Warning: accent.setNamedColor("#f39c12"); break;
    case Info:    accent.setNamedColor("#3498db"); break;
    }

    // 背景
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawRoundedRect(rect().adjusted(4, 0, 0, 0), 10, 10);

    // 左侧竖条
    p.setBrush(accent);
    p.drawRoundedRect(0, 8, 4, height() - 16, 2, 2);

    // 阴影
    // 由 Qt 的 WA_TranslucentBackground + 外层偏移实现，此处简化为画深色半透底层
}
