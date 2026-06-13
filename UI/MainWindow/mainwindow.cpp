#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_sidebarExpanded(true)
{
    ui->setupUi(this);
    setWindowTitle("车牌识别系统");
    resize(1200, 800);
    updateSidebar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnMenu_clicked()
{
    m_sidebarExpanded = !m_sidebarExpanded;
    updateSidebar();
}

void MainWindow::updateSidebar()
{
    int width = m_sidebarExpanded ? 200 : 60;
    ui->sidebar->setFixedWidth(width);

    ui->btnHome->setText(m_sidebarExpanded ? "主页" : "");
    ui->btnVehicleInfo->setText(m_sidebarExpanded ? "车辆信息" : "");
    ui->btnManage->setText(m_sidebarExpanded ? "管理" : "");
    ui->btnCamera->setText(m_sidebarExpanded ? "摄像头" : "");
}

void MainWindow::switchPage(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
}

void MainWindow::on_btnHome_clicked()
{
    switchPage(0);
}

void MainWindow::on_btnVehicleInfo_clicked()
{
    switchPage(1);
}

void MainWindow::on_btnManage_clicked()
{
    switchPage(2);
}

void MainWindow::on_btnCamera_clicked()
{
    switchPage(3);
}
