#include "managepage.h"
#include "ui_managepage.h"

ManagePage::ManagePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManagePage)
{
    ui->setupUi(this);
    loadUserData();
}

ManagePage::~ManagePage()
{
    delete ui;
}

void ManagePage::loadUserData()
{
    // TODO: 从数据库加载用户数据
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(
        {"用户名", "姓名", "手机号", "角色", "状态"});
    ui->tableWidget->setRowCount(6);
}

void ManagePage::on_btnAdd_clicked()
{
    // TODO: 实现添加用户功能
}

void ManagePage::on_btnDelete_clicked()
{
    // TODO: 实现删除用户功能
}

void ManagePage::on_btnModify_clicked()
{
    // TODO: 实现修改用户功能
}
