#include "notificationdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

// 内联 QSS
static const char *kDialogQss = R"(
NotificationDialog { background:transparent; }
QLabel#notifTitle { font-size:17px; font-weight:bold; color:#1a1a2e; }
QLabel#notifMsg   { font-size:13px; color:#555; line-height:1.6; padding:0 8px; }
QLabel[severity="success"], QLabel[severity="info"]    { font-size:22px; color:#3498db; background:#ebf3fd; border-radius:24px; padding:2px; }
QLabel[severity="warning"] { font-size:22px; color:#f39c12; background:#fef5e7; border-radius:24px; padding:2px; }
QLabel[severity="error"]   { font-size:22px; color:#e74c3c; background:#fdecea; border-radius:24px; padding:2px; }
QPushButton#notifPrimary   { background:#3498db; color:#fff; border:none; border-radius:6px; font-size:13px; font-weight:bold; }
QPushButton#notifPrimary:hover { background:#2980b9; }
QPushButton#notifSecondary { background:#fff; color:#555; border:1px solid #ddd; border-radius:6px; font-size:13px; }
QPushButton#notifSecondary:hover { background:#f1f3f4; border:#ccc; }
)";

// ─── 静态入口 ─────────────────────────────────────────────────
void NotificationDialog::info(QWidget *parent, const QString &title, const QString &msg)
{
    NotificationDialog *dlg = new NotificationDialog(parent, Info, title, msg);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

bool NotificationDialog::warning(QWidget *parent, const QString &title, const QString &msg)
{
    NotificationDialog *dlg = new NotificationDialog(parent, Warning, title, msg);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    return dlg->exec() == QDialog::Accepted;
}

void NotificationDialog::error(QWidget *parent, const QString &title, const QString &msg)
{
    NotificationDialog *dlg = new NotificationDialog(parent, Error, title, msg);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

bool NotificationDialog::question(QWidget *parent, const QString &title, const QString &msg)
{
    NotificationDialog *dlg = new NotificationDialog(parent, Question, title, msg);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    return dlg->exec() == QDialog::Accepted;
}

void NotificationDialog::errorWithDetail(QWidget *parent, const QString &title,
                                         const QString &brief, const QString &detail)
{
    NotificationDialog *dlg = new NotificationDialog(parent, Error, title, brief, detail);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

// ─── 构造 ──────────────────────────────────────────────────────
NotificationDialog::NotificationDialog(QWidget *parent, DialogType type,
                                       const QString &title, const QString &msg,
                                       const QString &detail)
    : QDialog(parent)
    , m_type(type)
    , m_detail(detail)
    , m_iconLabel(new QLabel(this))
    , m_titleLabel(new QLabel(title, this))
    , m_msgLabel(new QLabel(msg, this))
    , m_primaryButton(new QPushButton(this))
    , m_secondaryButton(nullptr)
    , m_mask(nullptr)
    , m_scaleAnim(new QPropertyAnimation(this, "geometry", this))
    , m_opaAnim(new QPropertyAnimation(this, "windowOpacity", this))
{
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    // 尺寸改为最小/最大约束，允许按消息长度自适应（修复长文本显示不全）
    setMinimumSize(kDialogWidth, kDialogHeight);
    setMaximumSize(kMaxDialogWidth, kMaxDialogHeight);

    setStyleSheet(QString::fromLatin1(kDialogQss));

    buildUi();
    adjustSize();   // 按内容重新计算尺寸

    // 尺寸变化后重新居中（playShowAnimation 内也居中，但基于原始 kDialogWidth/Height）
    QWidget *top = parentWidget() ? parentWidget()->window() : nullptr;
    if (top) {
        move(top->frameGeometry().center() - rect().center());
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        move(screen->availableGeometry().center() - rect().center());
    }

    playShowAnimation();
}

// ─── UI 构建 ──────────────────────────────────────────────────
void NotificationDialog::buildUi()
{
    // 标题
    m_titleLabel->setObjectName("notifTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    // 消息
    m_msgLabel->setObjectName("notifMsg");
    m_msgLabel->setAlignment(Qt::AlignCenter);
    m_msgLabel->setWordWrap(false);   // 单行显示，弹窗自动适应宽度

    // 图标
    m_iconLabel->setFixedSize(48, 48);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    // 颜色 + 按钮
    buildButtons();

    // 布局
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);
    rootLayout->setSpacing(14);
    rootLayout->setAlignment(Qt::AlignHCenter);

    rootLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    rootLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    rootLayout->addWidget(m_msgLabel, 1, Qt::AlignHCenter);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(12);
    btnLayout->addStretch();
    if (m_secondaryButton) {
        btnLayout->addWidget(m_secondaryButton);
    }
    btnLayout->addWidget(m_primaryButton);
    btnLayout->addStretch();
    rootLayout->addLayout(btnLayout);
}

void NotificationDialog::buildButtons()
{
    QString iconChar, severity, primaryText, secondaryText;
    bool hasSecondary = false;

    switch (m_type) {
    case Info:
        iconChar = QStringLiteral("ℹ");
        severity = "info";
        primaryText = QStringLiteral("我知道了");
        break;
    case Warning:
        iconChar = QStringLiteral("⚠");
        severity = "warning";
        primaryText = QStringLiteral("确定");
        secondaryText = QStringLiteral("取消");
        hasSecondary = true;
        break;
    case Error:
        iconChar = QStringLiteral("✕");
        severity = "error";
        primaryText = m_detail.isEmpty()
                          ? QStringLiteral("我知道了")
                          : QStringLiteral("我知道了");
        break;
    case Question:
        iconChar = QStringLiteral("?");
        severity = "info";
        primaryText = QStringLiteral("是(Y)");
        secondaryText = QStringLiteral("否(N)");
        hasSecondary = true;
        break;
    }

    m_iconLabel->setText(iconChar);
    m_iconLabel->setProperty("severity", severity);

    m_primaryButton->setText(primaryText);
    m_primaryButton->setObjectName("notifPrimary");
    m_primaryButton->setFixedSize(96, 36);
    connect(m_primaryButton, &QPushButton::clicked, this, [this]() {
        playCloseAnimation([this]() { accept(); });
    });

    if (hasSecondary) {
        m_secondaryButton = new QPushButton(secondaryText, this);
        m_secondaryButton->setObjectName("notifSecondary");
        m_secondaryButton->setFixedSize(96, 36);
        connect(m_secondaryButton, &QPushButton::clicked, this, [this]() {
            playCloseAnimation([this]() { reject(); });
        });
    }

    // 错误弹窗带详情 → 把复制详情作为 secondary
    if (m_type == Error && !m_detail.isEmpty()) {
        m_secondaryButton = new QPushButton(QStringLiteral("复制详情"), this);
        m_secondaryButton->setObjectName("notifSecondary");
        m_secondaryButton->setFixedSize(96, 36);
        connect(m_secondaryButton, &QPushButton::clicked, this, [this]() {
            QApplication::clipboard()->setText(m_titleLabel->text() + "\n"
                                              + m_msgLabel->text() + "\n" + m_detail);
            // 复制后短暂提示
            m_secondaryButton->setText(QStringLiteral("已复制"));
            QTimer::singleShot(1200, this, [this]() {
                m_secondaryButton->setText(QStringLiteral("复制详情"));
            });
        });
    }
}

// ─── 动画 ─────────────────────────────────────────────────────
void NotificationDialog::playShowAnimation()
{
    // 居中显示
    QWidget *top = parentWidget() ? parentWidget()->window() : nullptr;
    QPoint center;
    if (top) {
        center = top->frameGeometry().center() - rect().center();
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        center = screen->availableGeometry().center() - rect().center();
    }
    move(center);

    // 缩放动画
    QRect endRect(center.x(), center.y(), kDialogWidth, kDialogHeight);
    QRect startRect(center.x(), center.y(), kDialogWidth, kDialogHeight);

    m_scaleAnim->setDuration(180);
    m_scaleAnim->setStartValue(startRect);
    m_scaleAnim->setEndValue(endRect);
    m_scaleAnim->setEasingCurve(QEasingCurve::OutBack);

    m_opaAnim->setDuration(180);
    m_opaAnim->setStartValue(0.0);
    m_opaAnim->setEndValue(1.0);

    m_scaleAnim->start();
    m_opaAnim->start();
}

void NotificationDialog::playCloseAnimation(std::function<void()> onFinished)
{
    m_scaleAnim->setDuration(150);
    m_scaleAnim->setStartValue(geometry());
    m_scaleAnim->setEndValue(geometry());
    m_scaleAnim->setEasingCurve(QEasingCurve::InQuad);

    m_opaAnim->setDuration(150);
    m_opaAnim->setStartValue(1.0);
    m_opaAnim->setEndValue(0.0);

    connect(m_opaAnim, &QPropertyAnimation::finished, this, onFinished);
    m_opaAnim->start();
}

// ─── 事件 ─────────────────────────────────────────────────────
void NotificationDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        playCloseAnimation([this]() { reject(); });
    } else {
        QDialog::keyPressEvent(event);
    }
}

void NotificationDialog::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawRoundedRect(rect(), 14, 14);

    // 阴影
    p.setBrush(QColor(0, 0, 0, 40));
    p.drawRoundedRect(rect().adjusted(2, 4, -2, -2), 14, 14);
}

bool NotificationDialog::eventFilter(QObject *obj, QEvent *event)
{
    return QDialog::eventFilter(obj, event);
}
