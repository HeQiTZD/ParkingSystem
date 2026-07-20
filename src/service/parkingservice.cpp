#include "parkingservice.h"
#include "src/database/databasemanager.h"
#include "src/app/car.h"
#include "src/utils/initfile.h"
#include <QDateTime>

ParkingService::ParkingService(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {}

bool ParkingService::checkIn(const QString& rawPlate)
{
    if(!m_db.isConnected()){
        emit error(MessageType::Error, "数据库未连接");
        return false;
    }
    QString plate = Car::normalizePlate(rawPlate);
    if(plate.isEmpty() || !Car::isValidLicensePlate(plate)){
        emit error(MessageType::Warning, "车牌格式错误");
        return false;
    }
    if(m_db.isVehicleInPark(plate)){
        emit error(MessageType::Warning,
                   QStringLiteral("%1 已入库").arg(Car::displayPlate(plate)));
        return false;
    }
    QString parkingName = InitFile::instance().getParkingName();
    if(!m_db.checkIn(plate, parkingName)){
        emit error(MessageType::Error, "入库写入失败");
        return false;
    }
    emit parkingDataChanged();
    return true;
}

bool ParkingService::prepareCheckOut(const QString& rawPlate,
                                     QDateTime& outTime, double& cost,
                                     qint64& totalMinutes, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    QString plate = Car::normalizePlate(rawPlate);
    if(plate.isEmpty() || !Car::isValidLicensePlate(plate)){
        errMsg = "车牌格式错误"; return false;
    }
    if(!m_db.isVehicleInPark(plate)){
        errMsg = QStringLiteral("%1 未入库").arg(Car::displayPlate(plate));
        return false;
    }
    QDateTime intTime = m_db.getVehicleCheckInTime(plate);
    outTime = QDateTime::currentDateTime();
    totalMinutes = intTime.secsTo(outTime) / 60;
    cost = Car::calculateFee(intTime, outTime,
                             InitFile::instance().getParkingPrice(),
                             InitFile::instance().getFreeMinutes());
    return true;
}

bool ParkingService::confirmCheckOut(const QString& plate, double cost)
{
    QString parkingName = InitFile::instance().getParkingName();
    if(!m_db.checkOut(plate, parkingName, cost)){
        emit error(MessageType::Error, "出库写入失败");
        return false;
    }
    emit parkingDataChanged();
    return true;
}

ParkingStats ParkingService::getStats()
{
    return m_db.getParkingStats(InitFile::instance().getParkingName());
}
