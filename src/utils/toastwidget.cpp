#include "toastwidget.h"
#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QScreen>
#include <QStyle>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
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
QQueue<ToastWidget *> ToastWidget::s_active;
void ToastWidget::show(QWidget *parent, const QString &msg,
                       ToastType type, int durationMs)
{
    QWidget *realParent = parent;
    if (!realParent) {
        realParent = QApplication::activeWindow();
    }
    ToastWidget *toast = new ToastWidget(realParent);
    if (realParent) {
        toast->setAttribute(Qt::WA_DeleteOnClose);
    }
    toast->showImmediatly(msg, type, durationMs);
}
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
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint
                  | Qt::WindowStaysOnTopHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    resize(kToastWidth, kToastHeight);

    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);

    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setProperty("isIcon", true);
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

    QVBoxLayout *iconLayout = new QVBoxLayout;
    iconLayout->addStretch();
    iconLayout->addWidget(m_iconLabel);
    iconLayout->addStretch();
    rootLayout->addLayout(iconLayout);

    QVBoxLayout *msgLayout = new QVBoxLayout;
    msgLayout->addStretch();
    msgLayout->addWidget(m_msgLabel);
    msgLayout->addStretch();
    rootLayout->addLayout(msgLayout, 1);
    rootLayout->addWidget(m_closeButton, 0, Qt::AlignTop);
    setStyleSheet(QString::fromLatin1(kToastQss));
}
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
void ToastWidget::showImmediatly(const QString &msg, ToastType type, int durationMs)
{
    buildUi(msg,type);
    if(durationMs <= 0){
        durationMs = (type == Failure || type == Warning) ? 3500 : 3000;
    }

    while(s_active.size() >= kMaxActive){
        ToastWidget *oldest = s_active.takeLast();
        if(oldest && oldest != this){
            oldest->slideOut([oldest]() {oldest->close(); });
        }
    }

    s_active.prepend(this);
    const int myRow = 0;
    if(parentWidget()){
        parentWidget()->installEventFilter(this);
    }

    m_targetPos = targetPosition(myRow);

    move(m_targetPos - QPoint(0, kSlideOffset));
    QWidget::show();
    raise();

    restackAll();

    slideIn();

    QTimer::singleShot(durationMs, this, [this]() {
        if(isVisible()){
            slideOut([this]() {close();});
        }
    });
}
void ToastWidget::buildUi(const QString &msg, ToastType type)
{
    m_msgLabel->setText(msg);

    QString iconChar, severity;
    switch (type) {
    case Success: iconChar = QStringLiteral("✓"); severity = "success"; break;
    case Failure: iconChar = QStringLiteral("✕"); severity = "error";   break;
    case Warning: iconChar = QStringLiteral("⚠"); severity = "warning"; break;
    case Info:    iconChar = QStringLiteral("ℹ"); severity = "info";    break;
    }
    m_iconLabel->setText(iconChar);
    m_iconLabel->setProperty("severity", severity);

    setProperty("severity", severity);
    setProperty("toastType", type);
    style()->unpolish(this);
    style()->polish(this);
}
void ToastWidget::setOpacity(qreal o)
{
    m_opacity = o;
    m_opacityEffect->setOpacity(o);
}
void ToastWidget::slideIn()
{

    QPoint start = m_targetPos - QPoint(0, kSlideOffset);
    m_posAnim->setDuration(kSlideInMs);
    m_posAnim->setStartValue(start);
    m_posAnim->setEndValue(m_targetPos);
    m_posAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_opaAnim->setDuration(kSlideInMs);
    m_opaAnim->setStartValue(0.0);
    m_opaAnim->setEndValue(1.0);

    move(start);
    m_posAnim->start();
    m_opaAnim->start();
}
void ToastWidget::slideOut(const std::function<void()> &onFinished)
{
    s_active.removeOne(this);

    m_opaAnim->setDuration(kFadeMs);
    m_opaAnim->setStartValue(1.0);
    m_opaAnim->setEndValue(0.0);

    connect(m_opaAnim, &QPropertyAnimation::finished, this, [this, onFinished]() {
        onFinished();
        restackAll();
    });
    m_opaAnim->start();
}
void ToastWidget::restackAll()
{

    const int total = s_active.size();
    for (int i = 0; i < total; ++i) {
        ToastWidget *t = s_active.at(i);
        QPoint end = targetPosition(i);
        if (t->pos() == end) continue;
        QPropertyAnimation *slide = new QPropertyAnimation(t, "pos", t);
        slide->setDuration(kPushMs);
        slide->setStartValue(t->pos());
        slide->setEndValue(end);
        slide->setEasingCurve(QEasingCurve::OutCubic);
        slide->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
bool ToastWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Destroy) {
        s_active.removeOne(this);
        deleteLater();
    }
    return QWidget::eventFilter(obj, event);
}
void ToastWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor accent;
    switch (property("toastType").toInt()) {
    case Success: accent.setNamedColor("#3498db"); break;
    case Failure: accent.setNamedColor("#e74c3c"); break;
    case Warning: accent.setNamedColor("#f39c12"); break;
    case Info:    accent.setNamedColor("#3498db"); break;
    }

    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawRoundedRect(rect().adjusted(4, 0, 0, 0), 10, 10);

    p.setBrush(accent);
    p.drawRoundedRect(0, 8, 4, height() - 16, 2, 2);

}