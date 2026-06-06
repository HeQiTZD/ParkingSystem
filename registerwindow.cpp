#include "registerwindow.h"
#include "ui_registerwindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include "utils.h"

RegisterWindow::RegisterWindow(QWidget *parent, DatabaseManager *dbManager)
    : QWidget(parent),
    ui(new Ui::RegisterWindow),
    dbManager(dbManager)
{
    ui->setupUi(this);

    //设置窗口标题
    setWindowTitle(QStringLiteral("用户注册"));

    //设置窗口大小
    resize(400,300);

    //设置密码输入框为密码模式
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordEdit->setEchoMode((QLineEdit::Password));
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

void RegisterWindow::on_registerButton_clicked()
{
    //验证输入
    if(!validateInput()) return;

    //获取输入的值
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();
    QString phone = ui->phoneEdit->text();

    if(dbManager->isUsernameExists(username)){
        QMessageBox::warning(this,"警告",QStringLiteral("用户已存在"));
        return;
    }

    //对密码进行SHA-256加密后在存储
    QString encryptedPassword = encryptPassword(password);

    if(!dbManager->registerUser(username,encryptedPassword,phone)){
        QMessageBox::warning(this,"警告",QStringLiteral("注册失败"));
        return;
    }
    QMessageBox::information(this,"成功",QStringLiteral("注册成功"));
    clearInput();
    emit registerSuccess();
}

void RegisterWindow::on_returnToLogin_clicked()
{
    emit returnToLogin();
}

//验证输入
bool RegisterWindow::validateInput()
{
    QString username = ui->usernameEdit->text();
    QString password = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();
    QString phone = ui->phoneEdit->text();

    //验证用户名
    if(username.isEmpty()){
        QMessageBox::warning(this,"警告",QStringLiteral("用户名不能为空！"));
        return false;
    }
    if(username.length() < 3 || username.length() > 20){
        QMessageBox::warning(this,"警告",QStringLiteral("用户名长度必须在3-20个字符之间！"));
        ui->usernameEdit->clear();
        return false;
    }
    //验证密码
    if(password.isEmpty()){
        QMessageBox::warning(this,"警告",QStringLiteral("密码不能为空！"));
        return false;
    }
    if(password.length() < 6){
        QMessageBox::warning(this,"警告",QStringLiteral("密码长度不能少于6个字符！"));
        ui->passwordEdit->clear();
        return false;
    }
    if(password != confirmPassword){
        QMessageBox::warning(this,"警告",QStringLiteral("两次输入的密码不一致！"));
        ui->passwordEdit->clear();
        ui->confirmPasswordEdit->clear();
        return false;
    }
    //验证手机号
    if(!phone.isEmpty() && !QRegularExpression("^1[3-9]\\d{9}$").match(phone).hasMatch()){
        QMessageBox::warning(this,"警告",QStringLiteral("请输入正确的手机号"));
        ui->phoneEdit->clear();
        return false;
    }
    return true;
}

void RegisterWindow::clearInput()
{
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    ui->confirmPasswordEdit->clear();
    ui->phoneEdit->clear();
}

