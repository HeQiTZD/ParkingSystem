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
    , m_opaAnim(new QPropertyAnimation(this, "opacity", this))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
    , m_opacity(0.0)
{
    // 无边框 + 置顶 + 透明背景
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint
                  | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    resize(kToastWidth, kToastHeight);

    // 挂透明度效果器（必须在 show() 之前）
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);

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

    setStyleSheet(QString::fromLatin1(kToastQss));
}

// ── 根据队列索引计算绝对屏幕坐标 ──────────────────────────
QPoint ToastWidget::targetPosition(int row) const
{
    QWidget *p = parentWidget();
    if (p) {
        int parentCenterX = p->frameGeometry().left() + p->frameGeometry().width() / 2;
        return QPoint(parentCenterX - kToastWidth / 2,
                      p->frameGeometry().top() + kMarginTop + row * (kToastHeight + kSpacing));
    }
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect geo = screen->availableGeometry();
    return QPoint(geo.center().x() - kToastWidth / 2,
                  geo.top() + kMarginTop + row * (kToastHeight + kSpacing));
}

// ── 显示一条 Toast ──────────────────────────────────────────
void ToastWidget::showImmediatly(const QString &msg, ToastType type, int durationMs)
{
    buildUi(msg,type);

    if(durationMs <= 0){
        durationMs = (type == Failure || type == Warning) ? 3500 : 3000;
    }

    // ① 淘汰超限（先淘汰队尾的旧 Toast,避免被挤开）
    //    用 takeLast()(尾部):prepend 后新 Toast 在队首,最旧的在队尾。
    while(s_active.size() >= kMaxActive){
        ToastWidget *oldest = s_active.takeLast();
        if(oldest && oldest != this){
            oldest->slideOut([oldest]() {oldest->close(); });
        }
    }

    // ② 入队首（新来的索引 0 = 排在最上面，顶部永远显示最新 Toast）
    s_active.prepend(this);
    const int myRow = 0;

    if(parentWidget()){
        parentWidget()->installEventFilter(this);
    }

    // ③ 计算并记录目标位置（顶部居中，新来的在最上面）
    m_targetPos = targetPosition(myRow);

    // ④ 入场起点：目标位置上方 kSlideOffset 处（在已有 Toast 上缘之外，不遮挡）
    move(m_targetPos - QPoint(0, kSlideOffset));
    QWidget::show();
    raise();

    // ⑤ 把已有 Toast 整体下移一行,为新 Toast 腾出顶部位置
    restackAll();

    // ⑥ 入场：从目标位置上方滑入 + 淡入
    slideIn();

    // ⑦ 定时消失
    QTimer::singleShot(durationMs, this, [this]() {
        if(isVisible()){
            slideOut([this]() {close();});
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

// ── 属性读写 ─────────────────────────────────────────────────
void ToastWidget::setOpacity(qreal o)
{
    m_opacity = o;
    m_opacityEffect->setOpacity(o);
}

// ── 动画 ─────────────────────────────────────────────────────
void ToastWidget::slideIn()
{
    // 位置：从目标位置上方 kSlideOffset 滑入（OutCubic 先快后慢，自然刹车）
    //       顶部入场,与「最新 Toast 在顶部」的堆叠语义一致。
    QPoint start = m_targetPos - QPoint(0, kSlideOffset);
    m_posAnim->setDuration(kSlideInMs);
    m_posAnim->setStartValue(start);
    m_posAnim->setEndValue(m_targetPos);
    m_posAnim->setEasingCurve(QEasingCurve::OutCubic);

    // 透明度：0 → 1
    m_opaAnim->setDuration(kSlideInMs);
    m_opaAnim->setStartValue(0.0);
    m_opaAnim->setEndValue(1.0);

    // 起点在已有 Toast 上方，不被遮挡，因此入场全程可见
    move(start);
    m_posAnim->start();
    m_opaAnim->start();
}

void ToastWidget::slideOut(const std::function<void()> &onFinished)
{
    s_active.removeOne(this);

    // 透明度：1 → 0（淡出），位置不动
    m_opaAnim->setDuration(kFadeMs);
    m_opaAnim->setStartValue(1.0);
    m_opaAnim->setEndValue(0.0);

    // 先淡出 → 关闭 → 再让剩余 Toast 整体上移补位（不留空白）
    connect(m_opaAnim, &QPropertyAnimation::finished, this, [this, onFinished]() {
        onFinished();
        restackAll();
    });
    m_opaAnim->start();
}

void ToastWidget::restackAll()
{
    // 把全体活跃 Toast 重排到它们当前在队列中对应的行。
    //   - 起点 = t->pos()(实时位置):正在入场/离场途中也不会跳回;
    //   - 终点 = targetPosition(i)(绝对坐标):确定且唯一,不会累积漂移。
    // 调用场景:
    //   1) 新 Toast 入队首后 → 已有 Toast 整体下移腾出顶部;
    //   2) 任一 Toast 消失后 → 其下的 Toast 整体上移补位(不留空白)。
    const int total = s_active.size();
    for (int i = 0; i < total; ++i) {
        ToastWidget *t = s_active.at(i);
        QPoint end = targetPosition(i);
        if (t->pos() == end) continue;      // 已到位,跳过
        QPropertyAnimation *slide = new QPropertyAnimation(t, "pos", t);
        slide->setDuration(kPushMs);
        slide->setStartValue(t->pos());
        slide->setEndValue(end);
        slide->setEasingCurve(QEasingCurve::OutCubic);
        slide->start(QAbstractAnimation::DeleteWhenStopped);
    }
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
