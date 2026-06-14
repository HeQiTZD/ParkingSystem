#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "UI/HomePage/homepage.h"
#include "UI/ManagePage/managepage.h"
#include "UI/VehicleInfo/vehicleinfopage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnMenu_clicked();
    void on_btnHome_clicked();
    void on_btnVehicleInfo_clicked();
    void on_btnManage_clicked();
    void on_btnCamera_clicked();

private:
    Ui::MainWindow *ui;
    HomePage *m_homePage;
    ManagePage *m_managePage;
    VehicleInfoPage *m_vehicleInfoPage;
    bool m_sidebarExpanded;
    void updateSidebar();
    void switchPage(int index);
};

#endif // MAINWINDOW_H
