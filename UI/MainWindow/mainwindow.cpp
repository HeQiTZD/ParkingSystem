#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_homePage(nullptr)
    , m_sidebarExpanded(true)
{
    ui->setupUi(this);
    setWindowTitle("车牌识别系统");
    resize(1200, 800);

    // 移除占位页面并添加主页
    QWidget *placeholder = ui->stackedWidget->widget(0);
    ui->stackedWidget->removeWidget(placeholder);
    delete placeholder;

    m_homePage = new HomePage(this);
    ui->stackedWidget->insertWidget(0, m_homePage);

    updateSidebar();
    switchPage(0);
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
    // 取消所有导航按钮选中
    ui->btnHome->setChecked(false);
    ui->btnVehicleInfo->setChecked(false);
    ui->btnManage->setChecked(false);
    ui->btnCamera->setChecked(false);

    // 选中对应按钮
    switch(index) {
        case 0: ui->btnHome->setChecked(true); break;
        case 1: ui->btnVehicleInfo->setChecked(true); break;
        case 2: ui->btnManage->setChecked(true); break;
        case 3: ui->btnCamera->setChecked(true); break;
    }

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
