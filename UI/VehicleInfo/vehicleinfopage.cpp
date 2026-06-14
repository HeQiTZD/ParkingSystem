#include "vehicleinfopage.h"
#include "ui_vehicleinfopage.h"

VehicleInfoPage::VehicleInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VehicleInfoPage)
{
    ui->setupUi(this);
    loadVehicleData();
}

VehicleInfoPage::~VehicleInfoPage()
{
    delete ui;
}

void VehicleInfoPage::loadVehicleData()
{
    // TODO: 从数据库加载车辆数据
    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setHorizontalHeaderLabels(
        {"车牌号", "车辆类型", "入库时间", "出库时间", "停车时长", "费用", "状态"});
    ui->tableWidget->setRowCount(10);
}

void VehicleInfoPage::on_btnSearch_clicked()
{
    // TODO: 实现查询功能
}

void VehicleInfoPage::on_btnDelete_clicked()
{
    // TODO: 实现删除功能
}
