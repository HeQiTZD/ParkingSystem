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

    // 加载自定义样式表
    QFile customStyleFile(":/styles/login-custom.qss");
    if (customStyleFile.open(QFile::ReadOnly)) {
        QString customStyleSheet = QLatin1String(customStyleFile.readAll());
        this->setStyleSheet(this->styleSheet() + customStyleSheet);
        customStyleFile.close();
    } else {
        qWarning() << "无法加载自定义样式表:" << customStyleFile.fileName();
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
    update();  // 触发重绘
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
    // 主容器阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 5);
    ui->mainContainer->setGraphicsEffect(shadow);
}

void LoginDialog::setupTitleBar()
{
    this->installEventFilter(this);
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        switch (event->type()) {
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

void LoginDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 1. 绘制底层白色圆角矩形背景
    QRect mainRect = rect().adjusted(10, 10, -10, -10);  // 留出阴影空间
    painter.setBrush(QColor("#FFFFFF"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(mainRect, 12, 12);

    // 2. 计算左侧品牌区和右侧登录面板的区域
    int brandWidth = static_cast<int>(mainRect.width() * BRAND_PANEL_RATIO);
    QRect brandRect(mainRect.left(), mainRect.top(), brandWidth, mainRect.height());
    QRect loginRect(mainRect.left() + brandWidth, mainRect.top(),
                    mainRect.width() - brandWidth, mainRect.height());

    // 3. 绘制左侧品牌区
    drawBrandPanel(painter, brandRect);

    // 4. 绘制右侧登录面板
    drawLoginPanel(painter, loginRect);
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 检查是否点击在标题栏区域
        QRect titleBarRect(10, 10, width() - 20, 40);
        if (titleBarRect.contains(event->pos())) {
            m_dragging = true;
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
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
    // 保存画家状态
    painter.save();

    // 1. 绘制白色背景
    painter.setBrush(QColor("#FFFFFF"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 0, 12);

    // 2. 绘制标题区域
    QRect titleArea = rect.adjusted(40, 40, -40, -rect.height() + 120);

    // 绘制"用户登录"标题
    painter.setPen(QColor("#003FB1"));
    QFont titleFont("STLiti", 32, QFont::Normal);
    titleFont.setStyleName("华文隶书");
    painter.setFont(titleFont);
    painter.drawText(titleArea, Qt::AlignLeft | Qt::AlignBottom, "用户登录");

    // 3. 绘制副标题
    QRect subtitleArea = rect.adjusted(40, 100, -40, -rect.height() + 160);
    painter.setPen(QColor("#434654"));
    painter.setFont(QFont("Microsoft YaHei", 14));
    painter.drawText(subtitleArea, Qt::AlignLeft | Qt::AlignBottom, "请输入您的账户进行登录");

    // 4. 绘制用户名标签
    QRect usernameLabelArea = rect.adjusted(40, 180, -40, -rect.height() + 210);
    painter.setPen(QColor("#434654"));
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(usernameLabelArea, Qt::AlignLeft | Qt::AlignBottom, "用户名 USERNAME");

    // 5. 绘制密码标签
    QRect passwordLabelArea = rect.adjusted(40, 280, -40, -rect.height() + 310);
    painter.setPen(QColor("#434654"));
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(passwordLabelArea, Qt::AlignLeft | Qt::AlignBottom, "密码 PASSWORD");

    // 6. 绘制输入框背景（装饰性，实际输入框由UI控件实现）
    QRect usernameEditBg = rect.adjusted(40, 220, -40, -rect.height() + 265);
    QRect passwordEditBg = rect.adjusted(40, 320, -40, -rect.height() + 365);

    painter.setPen(QColor("#E2E8F0"));
    painter.setBrush(QColor("#F3F3FE"));
    painter.drawRoundedRect(usernameEditBg, 8, 8);
    painter.drawRoundedRect(passwordEditBg, 8, 8);

    // 7. 绘制输入框图标
    painter.setPen(QColor("#737686"));
    if (!m_personIcon.isNull()) {
        m_personIcon.paint(&painter, QRect(usernameEditBg.left() + 12, usernameEditBg.top() + 10, 20, 20));
    }
    if (!m_lockIcon.isNull()) {
        m_lockIcon.paint(&painter, QRect(passwordEditBg.left() + 12, passwordEditBg.top() + 10, 20, 20));
    }

    // 8. 绘制登录按钮背景
    QRect loginBtnBg = rect.adjusted(40, 400, -40, -rect.height() + 448);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#003FB1"));
    painter.drawRoundedRect(loginBtnBg, 8, 8);

    // 绘制登录按钮文字
    painter.setPen(QColor("#FFFFFF"));
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    painter.drawText(loginBtnBg, Qt::AlignCenter, "登 录");

    // 9. 绘制分隔线和"或"文字
    QRect separatorArea = rect.adjusted(40, 460, -40, -rect.height() + 480);
    painter.setPen(QColor("#E2E8F0"));
    painter.drawLine(separatorArea.left(), separatorArea.center().y(),
                     separatorArea.center().x() - 20, separatorArea.center().y());
    painter.drawLine(separatorArea.center().x() + 20, separatorArea.center().y(),
                     separatorArea.right(), separatorArea.center().y());

    painter.setPen(QColor("#737686"));
    painter.setFont(QFont("Microsoft YaHei", 10));
    painter.drawText(separatorArea, Qt::AlignCenter, "或");

    // 10. 绘制注册按钮背景
    QRect registerBtnBg = rect.adjusted(40, 490, -40, -rect.height() + 534);
    painter.setPen(QColor("#E2E8F0"));
    painter.setBrush(QColor("#FFFFFF"));
    painter.drawRoundedRect(registerBtnBg, 8, 8);

    // 绘制注册按钮文字
    painter.setPen(QColor("#434654"));
    painter.setFont(QFont("Microsoft YaHei", 14));
    painter.drawText(registerBtnBg, Qt::AlignCenter, "新用户注册");

    // 恢复画家状态
    painter.restore();
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
