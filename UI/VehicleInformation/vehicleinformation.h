#ifndef VEHICLEINFORMATION_H
#define VEHICLEINFORMATION_H

#include <QWidget>
class QTableWidget;
class QComboBox;
class VehicleService;
class PaginationWidget;

namespace Ui {
class VehicleInformation;
}

class VehicleInformation : public QWidget
{
    Q_OBJECT

private:
    void setupTable();
    void populateTable();

public:
    explicit VehicleInformation(QWidget *parent = nullptr, VehicleService *vehicleSvc = nullptr);
    ~VehicleInformation();

private slots:
    void onQueryClicked();
    void onDeleteClicked();
    void onPageChanged(int page);

private:
    Ui::VehicleInformation *ui;
    QTableWidget *m_tableWidget;
    PaginationWidget *m_pagination;
    QComboBox *m_pageSizeCombo;
    VehicleService *m_vehicleSvc = nullptr;
    QList<QVariantList> m_allData;   // 缓存最近一次查询的全量结果
};

#endif // VEHICLEINFORMATION_H
