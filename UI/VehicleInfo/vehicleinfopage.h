#ifndef VEHICLEINFOPAGE_H
#define VEHICLEINFOPAGE_H

#include <QWidget>

namespace Ui {
class VehicleInfoPage;
}

class VehicleInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit VehicleInfoPage(QWidget *parent = nullptr);
    ~VehicleInfoPage();

private slots:
    void on_btnSearch_clicked();
    void on_btnDelete_clicked();

private:
    Ui::VehicleInfoPage *ui;
    void loadVehicleData();
};

#endif // VEHICLEINFOPAGE_H
