#ifndef VEHICLESERVICE_H
#define VEHICLESERVICE_H
#include <QObject>
#include <QList>
#include <QVariantList>
#include <QDateTime>
#include "src/utils/messageType.h"
class DatabaseManager;
class VehicleService : public QObject
{
    Q_OBJECT
public:
    explicit VehicleService(DatabaseManager& db, QObject* parent = nullptr);
    QList<QVariantList> search(const QString& plate,
                                const QDateTime& startTime,
                                const QDateTime& endTime,
                                int status);
    QList<QVariantList> recentRecords(int count);
    bool deleteRecord(int id, QString& errMsg);
    bool deleteRecords(const QList<int>& ids, QString& errMsg);
signals:
    void vehicleDataChanged();
    void error(MessageType::Type type, const QString& msg);
private:
    DatabaseManager& m_db;
};
#endif