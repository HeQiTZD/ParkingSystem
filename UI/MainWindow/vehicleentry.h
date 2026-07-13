#ifndef VEHICLEENTRY_H
#define VEHICLEENTRY_H

#include <QString>
#include <QDateTime>

enum class VehicleEntryStatus { In, Out};

struct VehicleEntry {
    QString plate;
    QDateTime timestamp;
    VehicleEntryStatus status;

    bool isEntry() const { return status == VehicleEntryStatus::In;}
    QString timeStr() const { return timestamp.toString("HH:mm:ss");}
    QString displayText() const { return QStringLiteral("%1 · %2 · %3").arg(plate, timeStr(), isEntry() ? QStringLiteral("入库") : QStringLiteral("出库"));}
};

#endif