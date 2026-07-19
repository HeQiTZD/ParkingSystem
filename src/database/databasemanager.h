#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include "src/utils/messageType.h"

struct ParkingStats {
    int totalSpaces;      // 总车位数
    int usedSpaces;       // 已用车位（现有车辆数）
    int freeSpaces;       // 空闲车位
};

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    //连接数据库，地址，端口，数据库名称，用户名，密码
    bool connectDatabase(const QString &host, int port,
                         const QString &dbName,
                         const QString &username,
                         const QString &password);

    //断开数据库连接
    void disconnectDatabase();

    //检查连接状态
    bool isConnected() const;

    //执行查询
    QSqlQuery executeQuery(const QString &sql);

    //验证用户登录
    bool validateUser(const QString &username, const QString &password, QString &userRole);
    //查询用户是否已存在
    bool isUsernameExists(const QString &username);
    //用户注册
    bool registerUser(const QString &username, const QString &password, const QString &name, const QString &phone);

    //数据库操作
    bool updateParkingConfig(const QString &name, double price, int capacity);

    //车辆管理
    bool isVehicleInPark(const QString &licensePlate);
    bool checkIn(const QString &licensePlate, const QString &parkingName);
    bool checkOut(const QString &licensePlate, const QString &parkingName, double fee);
    QSqlQuery queryVehicle(const QString &licensePlate, bool onlyInPark = true);

    //车位统计
    ParkingStats getParkingStats(const QString &parkingName);

    //车辆信息搜索
    QList<QVariantList> searchCars(const QString &plate, const QDateTime &startTime,
                                    const QDateTime &endTime, int status);

    // 获取最近 N 条车辆记录（按入库时间降序）
    QList<QVariantList> getRecentRecords(int count);
    
    // 获取入库时间
    QDateTime getVehicleCheckInTime(const QString &licensePlate);

    //车辆信息删除
    bool deleteCarRecord(int id);
    bool deleteCarRecords(const QList<int> &ids);

    //---------- 用户管理 -----------
    // 添加新用户
    bool addUser(const QString &username, const QString &password, const QString &telephone, const QString &truename, const QString &role);
    // 修改用户信息
    bool updateUser(int id, const QString &username, const QString &telephone, const QString &truename, const QString &role);
    // 删除用户
    bool deleteUser(int id);
    // 搜索用户（支持用户名/姓名/手机号模糊匹配）
    QList<QVariantList> searchUsers(const QString &keyword = "");

signals:
    void connectionStatusChanged(bool connected);
    void messageBox(MessageType::Type, const QString &msg, const QString &title = QString());

    // 停车数据变化（入库/出库/删除），供 UI 订阅者刷新显示
    void parkingDataChanged();

private:
    QSqlDatabase db;
    bool connected;
};

#endif // DATABASEMANAGER_H
