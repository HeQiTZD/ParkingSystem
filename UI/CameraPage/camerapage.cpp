#include "camerapage.h"
#include "ui_camerapage.h"

CameraPage::CameraPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraPage)
{
    ui->setupUi(this);
}

CameraPage::~CameraPage()
{
    delete ui;
}

void CameraPage::on_btnInit_clicked()
{
    // TODO: 实现摄像头初始化
}

void CameraPage::on_btnOpen_clicked()
{
    // TODO: 实现打开摄像头
}

void CameraPage::on_btnRelease_clicked()
{
    // TODO: 实现释放资源
}

void CameraPage::on_btnPause_clicked()
{
    // TODO: 实现暂停功能
}
