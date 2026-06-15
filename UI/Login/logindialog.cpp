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
#include <QTimer>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(nullptr)
    , m_dragging(false)
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

void LoginDialog::on_btnMinimize_clicked()
{
    showMinimized();
}

void LoginDialog::on_btnClose_clicked()
{
    close();
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    updateLayout();
    update();
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

    // 5. 绘制输入框
    drawInputField(painter, m_usernameRect, m_usernameFocused, m_usernameHovered, false,
                   m_usernameEdit->text(), m_personIcon);
    drawInputField(painter, m_passwordRect, m_passwordFocused, m_passwordHovered, true,
                   m_passwordVisible ? m_passwordEdit->text() : "",
                   m_lockIcon);

    // 6. 绘制密码可见性切换按钮
    QIcon toggleIcon = m_passwordVisible ? m_visibilityIcon : m_visibilityOffIcon;
    if (!toggleIcon.isNull()) {
        toggleIcon.paint(&painter, m_toggleBtnRect);
    }

    // 7. 绘制记住我复选框
    drawRememberMe(painter, m_rememberMeRect, m_rememberMeCheckBox->isChecked());

    // 8. 绘制忘记密码链接
    drawForgotPassword(painter, m_forgotPasswordRect);

    // 9. 绘制登录按钮
    drawButton(painter, m_loginBtnRect, "登 录", true, m_loginButtonHovered);

    // 10. 绘制注册按钮
    drawButton(painter, m_registerBtnRect, "新用户注册", false, m_registerButtonHovered);

    // 11. 绘制加载指示器
    if (m_isLoading) {
        drawLoadingIndicator(painter, m_loginBtnRect);
    }
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();

        // 检查是否点击在输入框区域
        if (isPointInRect(pos, m_usernameRect)) {
            m_usernameEdit->setFocus();
            update();
        } else if (isPointInRect(pos, m_passwordRect)) {
            m_passwordEdit->setFocus();
            update();
        }
        // 检查是否点击在记住我复选框
        else if (isPointInRect(pos, m_rememberMeRect)) {
            m_rememberMeCheckBox->setChecked(!m_rememberMeCheckBox->isChecked());
            update();
        }
        // 检查是否点击在忘记密码链接
        else if (isPointInRect(pos, m_forgotPasswordRect)) {
            // TODO: 实现忘记密码逻辑
        }
        // 检查是否点击在登录按钮
        else if (isPointInRect(pos, m_loginBtnRect)) {
            handleLogin();
        }
        // 检查是否点击在注册按钮
        else if (isPointInRect(pos, m_registerBtnRect)) {
            // TODO: 实现注册逻辑
        }
        // 检查是否点击在密码可见性切换按钮
        else if (isPointInRect(pos, m_toggleBtnRect)) {
            togglePasswordVisibility();
        }
        // 点击在品牌区或其他空白区域，启动窗口拖拽
        else {
            m_dragging = true;
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        }
    }

    QDialog::mousePressEvent(event);
}

void LoginDialog::drawBrandPanel(QPainter &painter, const QRect &rect)
{
    painter.save();

    // 设置裁剪区域为圆角矩形
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect, 12, 12);
    painter.setClipPath(clipPath);

    // 1. 绘制背景图片
    if (!m_brandPixmap.isNull()) {
        // 按比例缩放图片以填充区域
        QPixmap scaledPixmap = m_brandPixmap.scaled(rect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 居中绘制
        int x = (rect.width() - scaledPixmap.width()) / 2;
        int y = (rect.height() - scaledPixmap.height()) / 2;
        painter.drawPixmap(rect.left() + x, rect.top() + y, scaledPixmap);
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

    // 3. 绘制品牌文字
    QRect textArea = rect.adjusted(20, 0, -20, -20);

    // 绘制Security图标
    if (!m_personIcon.isNull()) {
        m_personIcon.paint(&painter, QRect(rect.left() + 20, rect.bottom() - 200, 24, 24));
    }

    // 绘制Sentinel LPR标题
    painter.setPen(QColor("#FFFFFF"));
    painter.setFont(QFont("Microsoft YaHei", 24, QFont::Bold));
    QRect titleRect = textArea.adjusted(0, 0, 0, -160);
    painter.drawText(titleRect, Qt::AlignBottom | Qt::AlignLeft, "Sentinel LPR");

    // 绘制智能车牌识别系统标题
    painter.setFont(QFont("Microsoft YaHei", 16, QFont::Bold));
    QRect subtitleRect = textArea.adjusted(0, 0, 0, -120);
    painter.drawText(subtitleRect, Qt::AlignBottom | Qt::AlignLeft, "智能车牌识别系统");

    // 绘制描述文字
    painter.setFont(QFont("Microsoft YaHei", 10));
    painter.setPen(QColor(255, 255, 255, 204));  // 80%透明度
    QRect descRect = textArea.adjusted(0, 0, 0, -80);
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

    painter.restore();
}

void LoginDialog::drawLoginPanel(QPainter &painter, const QRect &rect)
{
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

    // 6. 绘制分隔线和"或"文字
    QRect separatorArea = rect.adjusted(40, 460, -40, -rect.height() + 480);
    painter.setPen(QColor("#E2E8F0"));
    painter.drawLine(separatorArea.left(), separatorArea.center().y(),
                     separatorArea.center().x() - 20, separatorArea.center().y());
    painter.drawLine(separatorArea.center().x() + 20, separatorArea.center().y(),
                     separatorArea.right(), separatorArea.center().y());

    painter.setPen(QColor("#737686"));
    painter.setFont(QFont("Microsoft YaHei", 10));
    painter.drawText(separatorArea, Qt::AlignCenter, "或");

    // 7. 绘制底部版权信息
    QRect copyrightArea = rect.adjusted(40, 0, -40, -20);
    painter.setPen(QColor("#737686"));
    painter.setFont(QFont("Microsoft YaHei", 8));
    painter.drawText(copyrightArea, Qt::AlignBottom | Qt::AlignLeft, "© 2024 Sentinel Technology. All rights reserved.");

    // 绘制服务条款和隐私政策
    QRect termsArea = rect.adjusted(0, 0, -40, -20);
    painter.drawText(termsArea, Qt::AlignBottom | Qt::AlignRight, "服务条款  隐私政策");

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
    QPoint pos = event->pos();

    // 更新悬停状态
    bool newUsernameHovered = isPointInRect(pos, m_usernameRect);
    bool newPasswordHovered = isPointInRect(pos, m_passwordRect);
    bool newLoginButtonHovered = isPointInRect(pos, m_loginBtnRect);
    bool newRegisterButtonHovered = isPointInRect(pos, m_registerBtnRect);

    // 检查是否需要更新
    if (newUsernameHovered != m_usernameHovered ||
        newPasswordHovered != m_passwordHovered ||
        newLoginButtonHovered != m_loginButtonHovered ||
        newRegisterButtonHovered != m_registerButtonHovered) {

        m_usernameHovered = newUsernameHovered;
        m_passwordHovered = newPasswordHovered;
        m_loginButtonHovered = newLoginButtonHovered;
        m_registerButtonHovered = newRegisterButtonHovered;

        // 更新鼠标光标
        if (m_loginButtonHovered || m_registerButtonHovered) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }

        update();
    }

    QDialog::mouseMoveEvent(event);
}

void LoginDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab) {
        // 在输入框之间切换焦点
        if (m_usernameEdit->hasFocus()) {
            m_passwordEdit->setFocus();
        } else if (m_passwordEdit->hasFocus()) {
            m_usernameEdit->setFocus();
        }
        update();
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        // 回车键触发登录
        handleLogin();
    } else if (event->key() == Qt::Key_Escape) {
        // ESC键关闭对话框
        close();
    }

    QDialog::keyPressEvent(event);
}

// ==================== 交互处理 ====================

void LoginDialog::handleLogin()
{
    // 获取输入内容
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    // 简单验证
    if (username.isEmpty()) {
        // TODO: 显示用户名不能为空的提示
        m_usernameEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        // TODO: 显示密码不能为空的提示
        m_passwordEdit->setFocus();
        return;
    }

    // TODO: 实现实际的登录逻辑
    qDebug() << "登录尝试:" << username;

    // 模拟登录过程
    m_isLoading = true;
    update();

    // 使用QTimer模拟异步登录
    QTimer::singleShot(1000, this, [this]() {
        m_isLoading = false;
        update();
        // TODO: 登录成功后关闭对话框
    });
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
    painter.save();

    // 绘制背景
    QColor bgColor = focused ? QColor("#FFFFFF") : QColor("#F3F3FE");
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 8, 8);

    // 绘制边框
    QColor borderColor;
    if (focused) {
        borderColor = QColor("#003FB1");
    } else if (hovered) {
        borderColor = QColor("#9CA3AF");
    } else {
        borderColor = QColor("#E2E8F0");
    }
    painter.setPen(QPen(borderColor, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect, 8, 8);

    // 绘制图标
    if (!icon.isNull()) {
        icon.paint(&painter, QRect(rect.left() + 12, rect.top() + 10, 20, 20));
    }

    // 绘制文本
    painter.setPen(QColor("#191B23"));
    painter.setFont(QFont("Microsoft YaHei", 14));
    QRect textRect = rect.adjusted(40, 0, -10, 0);
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    // 绘制占位符文本
    if (text.isEmpty() && !focused) {
        painter.setPen(QColor("#9CA3AF"));
        QString placeholder = isPassword ? "请输入密码" : "请输入用户名";
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, placeholder);
    }

    painter.restore();
}

void LoginDialog::drawButton(QPainter &painter, const QRect &rect, const QString &text, bool primary, bool hovered)
{
    painter.save();

    // 绘制背景
    QColor bgColor;
    if (primary) {
        bgColor = hovered ? QColor("#002D8A") : QColor("#003FB1");
    } else {
        bgColor = hovered ? QColor("#F5F5F5") : QColor("#FFFFFF");
    }
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 8, 8);

    // 绘制边框（非主按钮）
    if (!primary) {
        QColor borderColor = hovered ? QColor("#9CA3AF") : QColor("#E2E8F0");
        painter.setPen(QPen(borderColor, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect, 8, 8);
    }

    // 绘制文字
    QColor textColor = primary ? QColor("#FFFFFF") : QColor("#434654");
    painter.setPen(textColor);
    painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
    painter.drawText(rect, Qt::AlignCenter, text);

    painter.restore();
}

void LoginDialog::drawRememberMe(QPainter &painter, const QRect &rect, bool checked)
{
    painter.save();

    // 绘制复选框
    QRect checkboxRect(rect.left(), rect.top() + 3, 16, 16);
    QColor bgColor = checked ? QColor("#003FB1") : QColor("#FFFFFF");
    painter.setBrush(bgColor);
    painter.setPen(QPen(QColor("#9CA3AF"), 1));
    painter.drawRoundedRect(checkboxRect, 3, 3);

    // 如果选中，绘制勾选标记
    if (checked) {
        painter.setPen(QPen(QColor("#FFFFFF"), 2));
        painter.drawLine(checkboxRect.left() + 4, checkboxRect.center().y(),
                         checkboxRect.center().x(), checkboxRect.center().y() + 3);
        painter.drawLine(checkboxRect.center().x(), checkboxRect.center().y() + 3,
                         checkboxRect.right() - 3, checkboxRect.top() + 3);
    }

    // 绘制文字
    painter.setPen(QColor("#434654"));
    painter.setFont(QFont("Microsoft YaHei", 12));
    QRect textRect = rect.adjusted(22, 0, 0, 0);
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, "记住我");

    painter.restore();
}

void LoginDialog::drawForgotPassword(QPainter &painter, const QRect &rect)
{
    painter.save();

    painter.setPen(QColor("#737686"));
    painter.setFont(QFont("Microsoft YaHei", 12));
    painter.drawText(rect, Qt::AlignVCenter | Qt::AlignRight, "忘记密码？");

    painter.restore();
}

void LoginDialog::drawLoadingIndicator(QPainter &painter, const QRect &rect)
{
    if (!m_isLoading) return;

    painter.save();

    // 绘制半透明背景
    painter.setBrush(QColor(255, 255, 255, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect, 8, 8);

    // 绘制加载动画
    painter.setPen(QPen(QColor("#003FB1"), 3));
    painter.setBrush(Qt::NoBrush);

    int size = qMin(rect.width(), rect.height()) - 20;
    QRect indicatorRect(rect.center().x() - size/2, rect.center().y() - size/2, size, size);

    // 绘制旋转的弧线
    static int angle = 0;
    angle = (angle + 5) % 360;
    painter.drawArc(indicatorRect, angle * 16, 270 * 16);

    painter.restore();

    // 触发重绘以创建动画效果
    if (m_isLoading) {
        update();
    }
}

