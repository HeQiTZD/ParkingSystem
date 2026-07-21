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

    bool checkIn(const QString& rawPlate);
    bool prepareCheckOut(const QString& rawPlate,
                         QDateTime& outTime, double& cost,
                         qint64& totalMinutes, QString& errMsg);
    bool confirmCheckOut(const QString& plate, double cost);
    ParkingStats getStats();
signals:
    void parkingDataChanged();
    void error(MessageType::Type type, const QString& msg);
private:
    DatabaseManager& m_db;
};
#endif