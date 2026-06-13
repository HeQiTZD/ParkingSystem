#include "homepage.h"
#include "ui_homepage.h"

HomePage::HomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);
}

HomePage::~HomePage()
{
    delete ui;
}

void HomePage::on_btnPlay_clicked()
{
    // TODO: 实现播放功能
}

void HomePage::on_btnPause_clicked()
{
    // TODO: 实现暂停功能
}

void HomePage::on_btnCapture_clicked()
{
    // TODO: 实现截取照片功能
}

void HomePage::on_btnVehicleIn_clicked()
{
    // TODO: 实现入库功能
}

void HomePage::on_btnVehicleOut_clicked()
{
    // TODO: 实现出库功能
}
