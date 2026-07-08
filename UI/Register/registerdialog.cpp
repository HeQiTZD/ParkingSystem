#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "src/utils/utils.h"
#include <QMessageBox>

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

bool RegisterDialog::validateInputs()
{
    if (ui->txtUsername->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入用户名");
        return false;
    }
    if (ui->txtPassword->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入密码");
        return false;
    }
    if (ui->txtPassword->text() != ui->txtConfirmPassword->text()) {
        QMessageBox::warning(this, "警告", "两次输入的密码不一致");
        return false;
    }
    if (ui->txtName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入姓名");
        return false;
    }
    if (ui->txtPhone->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入手机号");
        return false;
    }
    return true;
}

void RegisterDialog::on_btnRegister_clicked()
{
    if (validateInputs()) {
        QString password = encryptPassword(ui->txtPassword->text());
        if(m_db->registerUser(ui->txtUsername->text(),password,ui->txtName->text(),ui->txtPhone->text())){
            QMessageBox::information(this, "成功", "注册成功");
            accept();
        }
    }else{
        QMessageBox::critical(this,"失败","注册失败");
    }
}

void RegisterDialog::on_btnBack_clicked()
{
    reject();
}
