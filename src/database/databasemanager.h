#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include "dbconfig.h"
#include "src/utils/messageType.h"
struct ParkingStats {
    int totalSpaces;
    int usedSpaces;
    int freeSpaces;
};
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool connectDatabase(const QString &host, int port,
                         const QString &dbName,
                         const QString &username,
                         const QString &password);

    void disconnectDatabase();

    bool isConnected() const;

    QSqlDatabase threadConnection();

    bool validateUser(const QString &username, const QString &password, QString &userRole);
    bool isUsernameExists(const QString &username);
    bool registerUser(const QString &username, const QString &password, const QString &name, const QString &phone);

    bool updateParkingConfig(const QString &name, double price, int capacity);

    bool isVehicleInPark(const QString &licensePlate);
    bool checkIn(const QString &licensePlate, const QString &parkingName);
    bool checkOut(const QString &licensePlate, const QString &parkingName, double fee);
    QSqlQuery queryVehicle(const QString &licensePlate, bool onlyInPark = true);
    ParkingStats getParkingStats(const QString &parkingName);
    QList<QVariantList> searchCars(const QString &plate, const QDateTime &startTime,
                                    const QDateTime &endTime, int status);
    QList<QVariantList> getRecentRecords(int count);
    QDateTime getVehicleCheckInTime(const QString &licensePlate);
    bool deleteCarRecord(int id);
    bool deleteCarRecords(const QList<int> &ids);

    bool addUser(const QString &username, const QString &password, const QString &telephone, const QString &truename, const QString &role);
    bool updateUser(int id, const QString &username, const QString &telephone, const QString &truename, const QString &role);
    bool deleteUser(int id);
    QList<QVariantList> searchUsers(const QString &keyword = "");
signals:
    void connectionStatusChanged(bool connected);
    void messageBox(MessageType::Type, const QString &msg, const QString &title = QString());

    void parkingDataChanged();
private:
    DbConfig m_config;
    bool m_connected = false;
};
#endif