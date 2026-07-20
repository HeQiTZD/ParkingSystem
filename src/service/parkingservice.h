#ifndef PARKINGSERVICE_H
#define PARKINGSERVICE_H

#include <QObject>
#include <QDateTime>
#include "src/utils/messageType.h"

struct ParkingStats;
class DatabaseManager;

class ParkingService : public QObject
{
    Q_OBJECT
public:
    explicit ParkingService(DatabaseManager& db, QObject* parent = nullptr);

    // 入库业务流程(验证→查重→写库→发信号)
    bool checkIn(const QString& rawPlate);
    // 出库准备(验证→查入场时间→计费), 返回 false=前置失败
    bool prepareCheckOut(const QString& rawPlate,
                         QDateTime& outTime, double& cost,
                         qint64& totalMinutes, QString& errMsg);
    // 出库确认后的写入
    bool confirmCheckOut(const QString& plate, double cost);
    // 停车场统计
    ParkingStats getStats();

signals:
    void parkingDataChanged();
    void error(MessageType::Type type, const QString& msg);

private:
    DatabaseManager& m_db;
};

#endif // PARKINGSERVICE_H
