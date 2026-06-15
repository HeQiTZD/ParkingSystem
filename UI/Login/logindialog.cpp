#include "logindialog.h"
#include "ui_logindialog.h"
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QFile>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QLinearGradient>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_dragging(false)
    , m_passwordVisible(false)
{
    ui->setupUi(this);

    // 加载资源
    loadResources();

    // 设置密码输入框模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    updatePasswordVisibilityIcon();

    // 设置无边框窗口
    setupWindowFlags();
    setupShadow();
    setupTitleBar();

    // 加载登录界面样式表
    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "无法加载样式表:" << styleFile.fileName();
    }

    // 连接密码可见性切换按钮信号
    connect(ui->togglePasswordVisibility, &QPushButton::clicked,
            this, &LoginDialog::on_togglePasswordVisibility_clicked);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    updateBrandPanelBackground();
}

void LoginDialog::updateBrandPanelBackground()
{
    // 品牌区背景将由drawBrandPanel方法通过QPainter绘制
    // 此方法保留为空，以便在需要时进行其他更新
}

void LoginDialog::setupWindowFlags()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void LoginDialog::setupShadow()
{
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    ui->mainContainer->setGraphicsEffect(shadow);
}

void LoginDialog::setupTitleBar()
{
    ui->titleBar->installEventFilter(this);
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->titleBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
                return true;
            }
            break;

        case QEvent::MouseMove:
            if (m_dragging && (mouseEvent->buttons() & Qt::LeftButton)) {
                move(mouseEvent->globalPos() - m_dragPosition);
                return true;
            }
            break;

        case QEvent::MouseButtonRelease:
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = false;
                return true;
            }
            break;

        default:
            break;
        }
    }

    return QDialog::eventFilter(obj, event);
}

void LoginDialog::on_btnMinimize_clicked()
{
    showMinimized();
}

void LoginDialog::on_btnClose_clicked()
{
    close();
}

// 空的存根实现 - 将在后续任务中实现
void LoginDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // TODO: 在Task 6中实现自定义绘制
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // TODO: 在Task 6中实现鼠标按下事件处理
}

void LoginDialog::drawBrandPanel(QPainter &painter, const QRect &rect)
{
    // 保存画家状态
    painter.save();

    // 设置裁剪区域为圆角矩形
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect, 12, 12);
    painter.setClipPath(clipPath);

    // 1. 绘制背景图片
    if (!m_brandPixmap.isNull()) {
        painter.drawPixmap(rect, m_brandPixmap);
    } else {
        // 如果图片加载失败，使用纯色背景
        painter.fillRect(rect, QColor("#1A56DB"));
    }

    // 2. 绘制渐变叠加层
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, QColor(0, 63, 177, 102));   // 40%透明度
    gradient.setColorAt(0.5, QColor(0, 63, 177, 153)); // 60%透明度
    gradient.setColorAt(1, QColor(0, 63, 177, 230));   // 90%透明度
    painter.fillRect(rect, gradient);

    // 3. 绘制品牌文字区域
    QRect textArea = rect.adjusted(20, 0, -20, -20);

    // 绘制Security图标和Sentinel LPR标题
    painter.setPen(QColor("#FFFFFF"));
    painter.setFont(QFont("Microsoft YaHei", 24, QFont::Bold));
    QRect titleRect = textArea.adjusted(0, 0, 0, -200);
    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, "Sentinel LPR");

    // 绘制智能车牌识别系统标题
    painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    QRect subtitleRect = textArea.adjusted(0, 0, 0, -160);
    painter.drawText(subtitleRect, Qt::AlignBottom | Qt::AlignLeft, "智能车牌识别系统");

    // 绘制描述文字
    painter.setFont(QFont("Microsoft YaHei", 10));
    painter.setPen(QColor(255, 255, 255, 204));  // 80%透明度
    QRect descRect = textArea.adjusted(0, 0, 0, -120);
    painter.drawText(descRect, Qt::AlignBottom | Qt::AlignLeft | Qt::TextWordWrap,
                     "高精度算法，实时全天候监控，为您的车辆安全保驾护航。");

    // 4. 绘制标签
    painter.setFont(QFont("Microsoft YaHei", 8));
    painter.setPen(QColor(255, 255, 255, 180));

    QRect tag1Rect(rect.left() + 20, rect.bottom() - 50, 80, 24);
    QRect tag2Rect(rect.left() + 110, rect.bottom() - 50, 80, 24);

    // 绘制标签背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 25));
    painter.drawRoundedRect(tag1Rect, 4, 4);
    painter.drawRoundedRect(tag2Rect, 4, 4);

    // 绘制标签文字
    painter.setPen(QColor(255, 255, 255, 200));
    painter.drawText(tag1Rect, Qt::AlignCenter, "99.9% 识别率");
    painter.drawText(tag2Rect, Qt::AlignCenter, "毫秒级响应");

    // 恢复画家状态
    painter.restore();
}

void LoginDialog::drawLoginPanel(QPainter &painter, const QRect &rect)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    // TODO: 在Task 5中实现登录面板绘制
}

void LoginDialog::loadResources()
{
    // 加载品牌背景图片
    m_brandPixmap.load(":/icons/login-brand-image");
    if (m_brandPixmap.isNull()) {
        qWarning() << "无法加载品牌背景图片";
    }

    // 加载图标
    m_personIcon = QIcon(":/icons/icon-person");
    m_lockIcon = QIcon(":/icons/icon-lock");
    m_visibilityIcon = QIcon(":/icons/icon-visibility");
    m_visibilityOffIcon = QIcon(":/icons/icon-visibility-off");
}

void LoginDialog::updatePasswordVisibilityIcon()
{
    if (m_passwordVisible) {
        ui->togglePasswordVisibility->setIcon(m_visibilityIcon);
    } else {
        ui->togglePasswordVisibility->setIcon(m_visibilityOffIcon);
    }
}

void LoginDialog::on_togglePasswordVisibility_clicked()
{
    m_passwordVisible = !m_passwordVisible;
    if (m_passwordVisible) {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
    } else {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
    }
    updatePasswordVisibilityIcon();
}
