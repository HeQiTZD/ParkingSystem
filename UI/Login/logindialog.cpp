#include "logindialog.h"
#include "ui_logindialog.h"
#include "usernameedit.h"
#include "passwordedit.h"
#include "UI/Register/registerdialog.h"
#include "src/service/userservice.h"
#include "src/utils/utils.h"
#include "src/utils/notification_global.h"

#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
LoginDialog::LoginDialog(QWidget *parent, UserService *userSvc)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_userSvc(userSvc)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    m_dragArea = QRect(0, 0, width(), 50);

    ui->minimizeButton->setIconSize(QSize(16, 16));
    ui->closeButton->setIconSize(QSize(16, 16));
    ui->termsLabel->setCursor(Qt::PointingHandCursor);
    ui->privacyLabel->setCursor(Qt::PointingHandCursor);
    ui->forgotPasswordLabel->setCursor(Qt::PointingHandCursor);

    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }

    connect(ui->loginButton,&QPushButton::clicked,this,&LoginDialog::onLoginButton);
    connect(ui->registerButton,&QPushButton::clicked,this,&LoginDialog::onRegisterButton);
    connect(m_userSvc, &UserService::error, this, [this](MessageType::Type type, const QString& msg) {
        onMessageBox(type, msg);
    });

    loadCredentials(); // 从配置文件加载用户名和密码
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::reset()
{
    // 复用登录框时清空上次输入，避免残留密码
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    ui->usernameEdit->setFocus();
}

void LoginDialog::on_closeButton_clicked()
{
    close();
}

void LoginDialog::on_minimizeButton_clicked()
{
    showMinimized();
}

void LoginDialog::onLoginButton()
{
    // 检查数据库管理器是否有效
    if (!m_userSvc) {
        qDebug() << "数据库未连接";
        return;
    }

    // 检查输入是否为空
    if (ui->usernameEdit->text().isEmpty() || ui->passwordEdit->text().isEmpty()) {
        notifyInfo(this, QStringLiteral("请输入用户名和密码!"));
        return;
    }

    QString username = ui->usernameEdit->text().trimmed();
    QString password = encryptPassword(ui->passwordEdit->text());

    // 验证用户
    QString errMsg;
if (m_userSvc->authenticate(username, password, userRole, errMsg)) {
        notifySuccess(this,"登录成功!");
        saveCredentials(); // 保存用户名和密码到配置文件
        accept(); // 关闭对话框，返回 QDialog::Accepted
    } else {
        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
    }
}

void LoginDialog::onRegisterButton()
{
    RegisterDialog registerDialog(nullptr,m_userSvc);
    if(registerDialog.exec() == QDialog::Accepted){
        ui->usernameEdit->setText(registerDialog.getUserName());
        ui->passwordEdit->setText(registerDialog.getPassword());
    }
}

void LoginDialog::onMessageBox(MessageType::Type type, const QString &msg, const QString &title)
{
    switch(type){
        case MessageType::Type::Success : notifySuccess(this, msg);
            break;
        case MessageType::Type::Failure : notifyFailure(this, msg);
            break;
        case MessageType::Type::Info : notifyInfo(this, msg);
            break;
        case MessageType::Type::ToastWarning : notifyToastWarning(this, msg);
            break;
       case MessageType::Type::Warning : notifyWarning(this, title, msg);
            break;
       case MessageType::Type::Error : notifyError(this, title, msg);
            break;
       case MessageType::Type::Confirm : notifyConfirm(this, title, msg);
            break;
       default:
            break;
    }
}

void LoginDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragArea.contains(event->pos())) {
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

void LoginDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    //初始化画家
    Q_UNUSED(event);
    QPainter painter(this);//创建画家对象，绑定到当前窗口
    painter.setRenderHint(QPainter::Antialiasing);//启用抗锯齿，让圆角边缘更平滑

    QPainterPath path;//QPainterPath 是 Qt 的路径类，用于定义复杂的图形形状
    path.addRoundedRect(rect(), 10, 10);//rect() - 获取窗口整个区域（0,0 到 width(),height()
    painter.setClipPath(path);//setClipPath(path) - 设置裁剪区域，后续所有绘制只在这个圆角区域内生效

    // 绘制左侧图片
    QPixmap leftPixmap(":/icons/login-brand-image-old");//
    if (!leftPixmap.isNull()) {
        int leftWidth = ui->brandImageWidget->width();
        painter.drawPixmap(0, 0, leftWidth, height(), leftPixmap);//drawPixmap(x, y, w, h, pixmap) - 将图片拉伸绘制到指定区域
    }

    // 绘制右侧白色背景 //fillRect - 用纯色填充指定矩形
    painter.fillRect(ui->formPanelWidget->geometry(), QColor(0xffffff));
}

void LoginDialog::saveCredentials()
{
    if(ui->rememberMeCheckBox->isChecked()){
        QSettings settings("Sentinel", "ParkingSystem");
        settings.setValue("login/username", ui->usernameEdit->text());
        settings.setValue("login/password", ui->passwordEdit->text());
        settings.setValue("login/remember", true);
    } else {
        QSettings settings("Sentinel", "ParkingSystem");
        settings.remove("login/username");
        settings.remove("login/password");
        settings.setValue("login/remember", false);
    }
}

void LoginDialog::loadCredentials()
{
    QSettings settings("Sentinel", "ParkingSystem");
    if(settings.value("login/remember", false).toBool()){
        ui->usernameEdit->setText(settings.value("login/username").toString());
        ui->passwordEdit->setText(settings.value("login/password").toString());
        ui->rememberMeCheckBox->setChecked(true);
    }
}
