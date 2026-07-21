#include "vehicleservice.h"
#include "src/database/databasemanager.h"
VehicleService::VehicleService(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {}
QList<QVariantList> VehicleService::search(const QString& plate,
                                            const QDateTime& startTime,
                                            const QDateTime& endTime,
                                            int status)
{
    if(!m_db.isConnected()) return {};
    return m_db.searchCars(plate, startTime, endTime, status);
}
QList<QVariantList> VehicleService::recentRecords(int count)
{
    if(!m_db.isConnected() || count <= 0) return {};
    return m_db.getRecentRecords(count);
}
bool VehicleService::deleteRecord(int id, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(!m_db.deleteCarRecord(id)){
        errMsg = "删除车辆记录失败";
        return false;
    }
    emit vehicleDataChanged();
    return true;
}
bool VehicleService::deleteRecords(const QList<int>& ids, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(ids.isEmpty()){ errMsg = "未选择记录"; return false; }
    if(!m_db.deleteCarRecords(ids)){
        errMsg = "批量删除失败";
        return false;
    }
    emit vehicleDataChanged();
    return true;
}