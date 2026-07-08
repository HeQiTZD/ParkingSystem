#include "logindialog.h"
#include "ui_logindialog.h"
#include "usernameedit.h"
#include "passwordedit.h"
#include "UI/Register/registerdialog.h"
#include "src/utils/utils.h"

#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QMessageBox>
LoginDialog::LoginDialog(QWidget *parent, DatabaseManager *m_db)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , m_dbManager(m_db)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    m_dragArea = QRect(0, 0, width(), 50);

    ui->minimizeButton->setIconSize(QSize(16, 16));
    ui->closeButton->setIconSize(QSize(16, 16));
    ui->termsLabel->setCursor(Qt::PointingHandCursor);
    ui->privacyLabel->setCursor(Qt::PointingHandCursor);

    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }

    connect(ui->loginButton,&QPushButton::clicked,this,&LoginDialog::onLoginButton);
    connect(ui->registerButton,&QPushButton::clicked,this,&LoginDialog::onRegisterButton);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_closeButton_clicked()
{
    close();
}

void LoginDialog::on_miniButton_clicked()
{
    showMinimized();
}

void LoginDialog::onLoginButton()
{
    // 检查数据库管理器是否有效
    if (!m_dbManager) {
        QMessageBox::critical(this, "错误", "数据库未初始化！");
        return;
    }

    QString username = ui->usernameEdit->text().trimmed();
    QString password = encryptPassword(ui->passwordEdit->text());

    // 检查输入是否为空
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名和密码！");
        return;
    }

    // 验证用户
    if (m_dbManager->validateUser(username, password, userRole)) {
        qDebug() << "登录成功，角色:" << userRole;
        accept(); // 关闭对话框，返回 QDialog::Accepted
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误，请重试！");
        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
    }
}

void LoginDialog::onRegisterButton()
{
    RegisterDialog registerDialog(nullptr,m_dbManager);
    registerDialog.exec();
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