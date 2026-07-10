#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "src/utils/utils.h"
#include "src/utils/notification_global.h"
#include <QMessageBox>
#include <QRegularExpression>

RegisterDialog::RegisterDialog(QWidget *parent, DatabaseManager *db) :
    QDialog(parent),
    ui(new Ui::RegisterDialog),
    m_db(db)
{
    ui->setupUi(this);
    setFixedSize(800, 500);
    setWindowTitle("用户注册");

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

    if(m_db->isUsernameExists(ui->txtUsername->text())){
        notifyInfo(this, QStringLiteral("用户名已被占用"));
        return;
    }

    QString password = encryptPassword(ui->txtPassword->text());
    if(m_db->registerUser(ui->txtUsername->text(), password, ui->txtName->text(), ui->txtPhone->text())){
        notifySuccess(this, QStringLiteral("注册成功"));
        accept();
    }else{
        notifyFailure(this, QStringLiteral("注册失败"));
    }
}

void RegisterDialog::on_btnBack_clicked()
{
    reject();
}
