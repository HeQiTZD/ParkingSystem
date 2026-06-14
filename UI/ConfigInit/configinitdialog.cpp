#include "configinitdialog.h"
#include "ui_configinitdialog.h"
#include <QMessageBox>

ConfigInitDialog::ConfigInitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigInitDialog)
{
    ui->setupUi(this);
    setFixedSize(500, 600);
    setWindowTitle("停车场数据初始化");
}

ConfigInitDialog::~ConfigInitDialog()
{
    delete ui;
}

bool ConfigInitDialog::validateInputs()
{
    if (ui->txtIP->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入IP地址");
        return false;
    }
    if (ui->txtPort->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入端口号");
        return false;
    }
    if (ui->txtDBName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入数据库名");
        return false;
    }
    if (ui->txtUsername->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入账户");
        return false;
    }
    if (ui->txtPassword->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入密码");
        return false;
    }
    if (ui->txtParkingName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入停车场名");
        return false;
    }
    if (ui->txtPrice->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入价格");
        return false;
    }
    if (ui->txtCapacity->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入容量");
        return false;
    }
    return true;
}

void ConfigInitDialog::saveConfig()
{
    // TODO: 保存配置到文件或数据库
}

void ConfigInitDialog::on_btnSubmit_clicked()
{
    if (validateInputs()) {
        saveConfig();
        QMessageBox::information(this, "成功", "配置初始化完成");
        accept();
    }
}

void ConfigInitDialog::on_btnCancel_clicked()
{
    reject();
}