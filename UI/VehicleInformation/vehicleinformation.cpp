#include "vehicleinformation.h"
#include "ui_vehicleinformation.h"
#include "src/utils/iconlineedit.h" 
#include <QFile>
#include <QGridLayout>                 

VehicleInformation::VehicleInformation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VehicleInformation)
{
    ui->setupUi(this);

    auto replaceWithIconEdit = [this](QGridLayout *layout, QLineEdit *oldEdit, const QString &iconPath)->IconLineEdit * 
    {
        int idx = layout->indexOf(oldEdit);
        int row, col, rowSpan, colSpan;
        layout->getItemPosition(idx, &row, &col, &rowSpan, &colSpan);

        layout->removeWidget(oldEdit);
        oldEdit->hide();

        IconLineEdit *newEdit = new IconLineEdit(this);
        newEdit->setObjectName(oldEdit->objectName());
        newEdit->setIconPath(iconPath);
        layout->addWidget(newEdit, row, col, rowSpan, colSpan);

        return newEdit;
    };

    replaceWithIconEdit(ui->startTimeGridLayout, ui->startTimeLineEdit, ":/icons/calendar_today.svg");

    replaceWithIconEdit(ui->plateGridLayout, ui->plateLineEdit, ":/icons/directions.svg");

    replaceWithIconEdit(ui->endTimeGridLayout, ui->endTimeLineEdit, ":/icons/calendar_today.svg");


    QFile styleFile(":/styles/vehicleInformation.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }
}

VehicleInformation::~VehicleInformation()
{
    delete ui;
}
