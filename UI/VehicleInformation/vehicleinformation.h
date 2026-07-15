#ifndef VEHICLEINFORMATION_H
#define VEHICLEINFORMATION_H

#include <QWidget>

namespace Ui {
class VehicleInformation;
}

class VehicleInformation : public QWidget
{
    Q_OBJECT

public:
    explicit VehicleInformation(QWidget *parent = nullptr);
    ~VehicleInformation();

private:
    Ui::VehicleInformation *ui;
};

#endif // VEHICLEINFORMATION_H
