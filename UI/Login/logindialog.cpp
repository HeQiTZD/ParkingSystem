#include "logindialog.h"
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QFile>
#include <QDebug>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QKeyEvent>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_passwordVisible(false)
    , m_usernameFocused(false)
    , m_passwordFocused(false)
    , m_usernameHovered(false)
    , m_passwordHovered(false)
    , m_loginButtonHovered(false)
    , m_registerButtonHovered(false)
    , m_isLoading(false)
    , m_focusProgress(0.0)
{
    // 设置窗口标志
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置固定大小
    setFixedSize(800, 500);

    // 加载资源
    loadResources();

    // 初始化UI
    setupUI();

    // 初始化动画
    m_focusAnimation = new QPropertyAnimation(this, "focusProgress", this);
    m_focusAnimation->setDuration(200);
    m_focusAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    update();  // 触发重绘
}

void LoginDialog::setupUI()
{
    // 创建透明的输入框
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    m_usernameEdit->setStyleSheet("background: transparent; border: none; color: transparent; selection-background-color: transparent;");
    m_usernameEdit->setFocusPolicy(Qt::ClickFocus);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet("background: transparent; border: none; color: transparent; selection-background-color: transparent;");
    m_passwordEdit->setFocusPolicy(Qt::ClickFocus);

    // 创建密码可见性切换按钮
    m_togglePasswordBtn = new QPushButton(this);
    m_togglePasswordBtn->setIcon(m_visibilityOffIcon);
    m_togglePasswordBtn->setStyleSheet("background: transparent; border: none;");
    m_togglePasswordBtn->setCursor(Qt::PointingHandCursor);
    connect(m_togglePasswordBtn, &QPushButton::clicked, this, &LoginDialog::togglePasswordVisibility);

    // 创建登录按钮
    m_loginButton = new QPushButton("登 录", this);
    m_loginButton->setStyleSheet("background: transparent; border: none; color: white; font-weight: bold;");
    m_loginButton->setCursor(Qt::PointingHandCursor);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::handleLogin);

    // 创建注册按钮
    m_registerButton = new QPushButton("新用户注册", this);
    m_registerButton->setStyleSheet("background: transparent; border: none; color: #434654;");
    m_registerButton->setCursor(Qt::PointingHandCursor);

    // 创建记住我复选框
    m_rememberMeCheckBox = new QCheckBox("记住我", this);
    m_rememberMeCheckBox->setStyleSheet("background: transparent; border: none; color: #434654;");

    // 创建忘记密码标签
    m_forgotPasswordLabel = new QLabel("忘记密码？", this);
    m_forgotPasswordLabel->setStyleSheet("background: transparent; color: #737686;");
    m_forgotPasswordLabel->setCursor(Qt::PointingHandCursor);

    // 安装事件过滤器以监听焦点变化
    m_usernameEdit->installEventFilter(this);
    m_passwordEdit->installEventFilter(this);
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

// ==================== 事件处理 ====================

bool LoginDialog::eventFilter(QObject *obj, QEvent *event)
{
    // 处理输入框的焦点变化
    if (obj == m_usernameEdit) {
        if (event->type() == QEvent::FocusIn) {
            m_usernameFocused = true;
            update();
        } else if (event->type() == QEvent::FocusOut) {
            m_usernameFocused = false;
            update();
        }
    } else if (obj == m_passwordEdit) {
        if (event->type() == QEvent::FocusIn) {
            m_passwordFocused = true;
            update();
        } else if (event->type() == QEvent::FocusOut) {
            m_passwordFocused = false;
            update();
        }
    } else if (obj == this) {
        // 处理窗口拖拽
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

void LoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void LoginDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        handleLogin();
    } else {
        QDialog::keyPressEvent(event);
    }
}

// ==================== 交互处理 ====================

void LoginDialog::handleLogin()
{
    // TODO: 实现登录逻辑
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        return;
    }

    // 暂时只打印日志
    qInfo() << "登录尝试:" << username;
}

void LoginDialog::togglePasswordVisibility()
{
    m_passwordVisible = !m_passwordVisible;

    if (m_passwordVisible) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_togglePasswordBtn->setIcon(m_visibilityIcon);
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_togglePasswordBtn->setIcon(m_visibilityOffIcon);
    }

    update();
}

bool LoginDialog::isPointInRect(const QPoint &pos, const QRect &rect)
{
    return rect.contains(pos);
}

// ==================== 动画属性 ====================

double LoginDialog::focusProgress() const
{
    return m_focusProgress;
}

void LoginDialog::setFocusProgress(double progress)
{
    m_focusProgress = progress;
    update();
}

// ==================== 布局更新 ====================

void LoginDialog::updateLayout()
{
    // 以800x500为基准计算布局区域
    QRect mainRect = rect().adjusted(10, 10, -10, -10);

    int brandWidth = static_cast<int>(mainRect.width() * BRAND_PANEL_RATIO);
    m_brandRect = QRect(mainRect.left(), mainRect.top(), brandWidth, mainRect.height());
    m_loginRect = QRect(mainRect.left() + brandWidth, mainRect.top(),
                        mainRect.width() - brandWidth, mainRect.height());

    // 登录面板内控件区域（相对于窗口）
    int loginLeft = m_loginRect.left();
    int loginTop = m_loginRect.top();

    m_usernameRect = QRect(loginLeft + 40, loginTop + 220, m_loginRect.width() - 80, 45);
    m_passwordRect = QRect(loginLeft + 40, loginTop + 320, m_loginRect.width() - 80, 45);
    m_toggleBtnRect = QRect(m_passwordRect.right() - 35, m_passwordRect.top() + 5, 30, 35);
    m_loginBtnRect = QRect(loginLeft + 40, loginTop + 400, m_loginRect.width() - 80, 48);
    m_registerBtnRect = QRect(loginLeft + 40, loginTop + 490, m_loginRect.width() - 80, 44);
    m_rememberMeRect = QRect(loginLeft + 40, loginTop + 460, 100, 24);
    m_forgotPasswordRect = QRect(loginLeft + m_loginRect.width() - 160, loginTop + 460, 120, 24);

    // 将控件定位到正确的位置（透明控件用于交互）
    m_usernameEdit->setGeometry(m_usernameRect);
    m_passwordEdit->setGeometry(m_passwordRect);
    m_togglePasswordBtn->setGeometry(m_toggleBtnRect);
    m_loginButton->setGeometry(m_loginBtnRect);
    m_registerButton->setGeometry(m_registerBtnRect);
    m_rememberMeCheckBox->setGeometry(m_rememberMeRect);
    m_forgotPasswordLabel->setGeometry(m_forgotPasswordRect);
}

// ==================== 绘制辅助方法 ====================

void LoginDialog::drawInputField(QPainter &painter, const QRect &rect, bool focused, bool hovered, bool isPassword, const QString &text, const QIcon &icon)
{
    Q_UNUSED(isPassword);

    painter.save();

    // 绘制输入框背景
    QColor bgColor = focused ? QColor("#FFFFFF") : QColor("#F3F3FE");
    QColor borderColor = focused ? QColor("#003FB1") : (hovered ? QColor("#C0C4D6") : QColor("#E2E8F0"));

    painter.setPen(QPen(borderColor, focused ? 2 : 1));
    painter.setBrush(bgColor);
    painter.drawRoundedRect(rect, 8, 8);

    // 绘制左侧图标
    if (!icon.isNull()) {
        icon.paint(&painter, QRect(rect.left() + 12, rect.top() + (rect.height() - 20) / 2, 20, 20));
    }

    // 绘制文本（如果可见）
    if (!text.isEmpty()) {
        painter.setPen(QColor("#1F2937"));
        painter.setFont(QFont("Microsoft YaHei", 11));
        QRect textRect = rect.adjusted(40, 0, -12, 0);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
    } else {
        // 绘制占位符文字
        painter.setPen(QColor("#9CA3AF"));
        painter.setFont(QFont("Microsoft YaHei", 11));
        QRect textRect = rect.adjusted(40, 0, -12, 0);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                         isPassword ? "请输入密码" : "请输入用户名");
    }

    painter.restore();
}

void LoginDialog::drawButton(QPainter &painter, const QRect &rect, const QString &text, bool primary, bool hovered)
{
    painter.save();

    if (primary) {
        QColor bgColor = hovered ? QColor("#1E40AF") : QColor("#003FB1");
        painter.setPen(Qt::NoPen);
        painter.setBrush(bgColor);
        painter.drawRoundedRect(rect, 8, 8);

        painter.setPen(QColor("#FFFFFF"));
        painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    } else {
        QColor bgColor = hovered ? QColor("#F9FAFB") : QColor("#FFFFFF");
        painter.setPen(QColor("#E2E8F0"));
        painter.setBrush(bgColor);
        painter.drawRoundedRect(rect, 8, 8);

        painter.setPen(QColor("#434654"));
        painter.setFont(QFont("Microsoft YaHei", 14));
    }

    painter.drawText(rect, Qt::AlignCenter, text);
    painter.restore();
}

void LoginDialog::drawRememberMe(QPainter &painter, const QRect &rect, bool checked)
{
    painter.save();

    // 绘制复选框
    QRect checkboxRect(rect.left(), rect.center().y() - 9, 18, 18);
    QColor borderColor = QColor("#9CA3AF");
    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(checkboxRect, 3, 3);

    if (checked) {
        painter.setPen(QPen(QColor("#003FB1"), 2));
        painter.drawLine(checkboxRect.left() + 4, checkboxRect.center().y(),
                         checkboxRect.center().x() - 1, checkboxRect.center().y() + 4);
        painter.drawLine(checkboxRect.center().x() - 1, checkboxRect.center().y() + 4,
                         checkboxRect.right() - 4, checkboxRect.top() + 4);
    }

    // 绘制文字
    painter.setPen(QColor("#434654"));
    painter.setFont(QFont("Microsoft YaHei", 10));
    QRect textRect = checkboxRect.adjusted(24, 0, 60, 0);
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, "记住我");

    painter.restore();
}

void LoginDialog::drawForgotPassword(QPainter &painter, const QRect &rect)
{
    painter.save();

    painter.setPen(QColor("#737686"));
    painter.setFont(QFont("Microsoft YaHei", 10));
    painter.drawText(rect, Qt::AlignRight | Qt::AlignVCenter, "忘记密码？");

    painter.restore();
}

void LoginDialog::drawLoadingIndicator(QPainter &painter, const QRect &rect)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    // TODO: 实现加载指示器动画
}

