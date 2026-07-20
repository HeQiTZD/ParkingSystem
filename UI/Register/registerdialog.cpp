#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "src/service/userservice.h"
#include "src/utils/utils.h"
#include "src/utils/notification_global.h"
#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>

RegisterDialog::RegisterDialog(QWidget *parent, UserService *userSvc) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),
    m_userSvc(userSvc)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);        // 无边框
    setAttribute(Qt::WA_TranslucentBackground);    // 透明背景（圆角必需）
    setFixedSize(480, 420);                        // 固定大小
    m_dragArea = QRect(0, 0, width(), 50);         // 顶部 50px 区域可拖动
    setWindowTitle("用户注册");

    // 标题栏按钮图标（与登录窗口一致）
    ui->minimizeButton->setIcon(QIcon(":/icons/minied.svg"));
    ui->minimizeButton->setIconSize(QSize(16, 16));
    ui->closeButton->setIcon(QIcon(":/icons/close.svg"));
    ui->closeButton->setIconSize(QSize(16, 16));

    // 标题栏按钮信号连接
    connect(ui->minimizeButton, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);

    // 加载 QSS 样式表（与登录窗口一致）
    QFile styleFile(":/styles/register.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }

    // 设置密码输入框模式
    ui->txtPassword->setEchoMode(QLineEdit::Password);
    ui->txtConfirmPassword->setEchoMode(QLineEdit::Password);

    // 设置占位符文本
    ui->txtUsername->setPlaceholderText("请输入用户名");
    ui->txtPassword->setPlaceholderText("请输入密码");
    ui->txtConfirmPassword->setPlaceholderText("请再次输入密码");
    ui->txtName->setPlaceholderText("请输入姓名");
    ui->txtPhone->setPlaceholderText("请输入手机号");
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

QString RegisterDialog::getUserName() const
{
    return ui->txtUsername->text();
}

QString RegisterDialog::getPassword() const
{
    return ui->txtPassword->text();
}

bool RegisterDialog::validateInputs()
{
    if (ui->txtUsername->text().isEmpty()) {
        notifyInfo(this,"请输入用户名");
        return false;
    }
    if (ui->txtPassword->text().isEmpty()) {
        notifyInfo(this,"请输入密码");
        return false;
    }
    if (ui->txtPassword->text() != ui->txtConfirmPassword->text()) {
        notifyInfo(this,"两次输入的密码不一致");
        return false;
    }
    if (ui->txtName->text().isEmpty()) {
        notifyInfo(this,"请输入姓名");
        return false;
    }
    if (ui->txtPhone->text().isEmpty()) {
        notifyInfo(this,"请输入手机号");
        return false;
    }

    if(ui->txtPassword->text().length() < 6){
        notifyInfo(this,"密码长度不能小于6位");
        ui->txtPassword->clear();
        return false;
    }

    QRegularExpression phoneRegex("^1[3-9]\\d{9}$");
    if(!phoneRegex.match(ui->txtPhone->text()).hasMatch()){
        notifyInfo(this,"手机号格式错误,请输入正确的手机号");
        return false;
    }

    return true;
}

void RegisterDialog::on_btnRegister_clicked()
{
    if(!validateInputs()) return;

    QString password = encryptPassword(ui->txtPassword->text());
    QString errMsg;
    if(m_userSvc->registerUser(ui->txtUsername->text(), password,
                               ui->txtName->text(), ui->txtPhone->text(), errMsg)){
        notifySuccess(this, QStringLiteral("注册成功"));
        accept();
    }else{
        notifyFailure(this, errMsg.isEmpty() ? QStringLiteral("注册失败") : errMsg);
    }
}

void RegisterDialog::on_btnBack_clicked()
{
    reject();
}

// 绘制圆角白色背景（WA_TranslucentBackground 下 QSS background 不生效）
void RegisterDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);   // 与 QSS border-radius 一致
    painter.setClipPath(path);

    painter.fillPath(path, QColor(0xffffff));
}

// 无边框窗口手动拖动：鼠标按下
void RegisterDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragArea.contains(event->pos())) {
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

// 无边框窗口手动拖动：鼠标移动
void RegisterDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
    }
}

// 无边框窗口手动拖动：鼠标释放
void RegisterDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
}
