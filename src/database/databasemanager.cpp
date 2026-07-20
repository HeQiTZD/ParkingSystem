#include "databasemanager.h"
#include "dbconnectionpool.h"
#include "src/utils/utils.h"
#include "src/utils/initfile.h"
#include <QSqlError>
#include <QDebug>
#include <QTimeZone>

DatabaseManager::DatabaseManager(QObject *parent): QObject(parent), m_connected(false){}

DatabaseManager::~DatabaseManager()
{
    disconnectDatabase();
}

bool DatabaseManager::connectDatabase(const QString &host, int port, const QString &dbName, const QString &username, const QString &password)
{
    if(m_connected){
        disconnectDatabase();
    }

    DbConfig cfg;
    cfg.host = host;
    cfg.port = port;
    cfg.dbName = dbName;
    cfg.username = username;
    cfg.password = password;

    DbConnectionPool::instance().setConfig(cfg);
    QSqlDatabase testConn = DbConnectionPool::instance().connection();
    if(!testConn.isOpen()){
        qDebug() << "数据库连接失败" << testConn.lastError().text();
        m_connected = false;
        emit connectionStatusChanged(false);
        return false;
    }

    m_config = cfg;
    m_connected = true;
    emit connectionStatusChanged(true);
    qDebug() << "数据库连接成功！";
    return true;
}

void DatabaseManager::disconnectDatabase()
{
    if(m_connected){
        DbConnectionPool::instance().closeAll();
        m_connected = false;
        emit connectionStatusChanged(false);
        qDebug() << "数据库连接已关闭";
    }
}

bool DatabaseManager::isConnected() const
{
    return m_connected;
}

QSqlDatabase DatabaseManager::threadConnection()
{
    return DbConnectionPool::instance().connection();
}

bool DatabaseManager::validateUser(const QString &username, const QString &password, QString &userRole)
{
    if(!isConnected()){
        qDebug() << "数据库未连接";
        return false;
    }

    //使用预处理语句防止sql注入
    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare("SELECT role FROM User WHERE username = :username AND password = :password");
    query.bindValue(":username",username);
    query.bindValue(":password",password);

    if(!query.exec()){
        qDebug() << "验证用户操作失败" << query.lastError().text();
        return false;
    }

    if(query.next()){
        userRole = query.value("role").toString();
        return true;
    }else{
        emit messageBox(MessageType::Type::Info, "用户名或密码错误!");
        return false;
    }
}

bool DatabaseManager::isUsernameExists(const QString &username)
{
    if(!m_connected){
        qDebug() << "数据库未连接！";
        return false;
    }
    QSqlDatabase dbc = threadConnection();
    QSqlQuery checkQuery(dbc);
    checkQuery.prepare("SELECT COUNT(*) FROM User WHERE username = :username");
    checkQuery.bindValue(":username",username);

    if(!checkQuery.exec() ||!checkQuery.next()){
        qDebug()<< "查询用户失败" <<checkQuery.lastError().text();
        return false;
    }
    return checkQuery.value(0).toInt() > 0;
}

bool DatabaseManager::registerUser(const QString &username, const QString &password, const QString &name,const QString &phone)
{
    if(!m_connected){
        qDebug() << "数据库未连接！";
        return false;
    }

    QSqlDatabase dbc = threadConnection();
    QSqlQuery insertQuery(dbc);
    insertQuery.prepare("INSERT INTO User (username, password, telephone, truename, role)"
                        "VALUES (:username, :password, :telephone, :name, 'user')");
    insertQuery.bindValue(":username",username);
    insertQuery.bindValue(":password",password);
    insertQuery.bindValue(":telephone",phone);
    insertQuery.bindValue(":name",name);

    if(!insertQuery.exec()){
        qDebug() << "注册用户失败" << insertQuery.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateParkingConfig(const QString &name, double price, int capacity)
{
    if(!m_connected){
        qDebug() << QStringLiteral("数据库未连接");
        return false;
    }

    // 先检查停车场是否存在
    QSqlDatabase dbc = threadConnection();
    QSqlQuery checkQuery(dbc);
    checkQuery.prepare("SELECT COUNT(*) FROM PARKING WHERE P_name = :name");
    checkQuery.bindValue(":name",name);

    if(!checkQuery.exec() || !checkQuery.next()){
        qDebug() << QStringLiteral("查询停车场失败:") << checkQuery.lastError().text();
        return false;
    }

    if(checkQuery.value(0).toInt() > 0){
        QSqlQuery updateQuery(dbc);
        updateQuery.prepare(R"(
                UPDATE PARKING
                SET P_fee = :P_fee, P_all_count = :capacity
                WHERE P_name = :P_name;
            )");
        
        updateQuery.bindValue(":P_fee", price);
        updateQuery.bindValue(":capacity", capacity);
        updateQuery.bindValue(":P_name", name);

        if(!updateQuery.exec()){
            qDebug() << QStringLiteral("更新停车场配置失败:") << updateQuery.lastError().text();
            return false;
        }
    }else{
        QSqlQuery insertQuery(dbc);
        insertQuery.prepare(R"(
                INSERT INTO PARKING (P_name, P_all_count, P_fee)
                VALUES (:name, :capacity, :fee)
            )");

        insertQuery.bindValue(":name", name);
        insertQuery.bindValue(":capacity", capacity);
        insertQuery.bindValue(":fee", price);

        if(!insertQuery.exec()){
            qDebug() << QStringLiteral("插入停车场配置失败") << insertQuery.lastError().text();
            return false;
        }
    }

    qDebug() << QStringLiteral("数据库停车场配置已更新:") << name << price << capacity;
    return true;
}

bool DatabaseManager::isVehicleInPark(const QString &licensePlate)
{
    if (!m_connected) {
        qDebug() << "数据库未连接！";
        return false;
    }

    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare("SELECT COUNT(*) FROM CAR "
                  "WHERE license_plate = :plate AND check_out_time IS NULL");
    query.bindValue(":plate", licensePlate);

    if (!query.exec() || !query.next()) {
        qDebug() << "查询车辆状态失败:" << query.lastError().text();
        return false;
    }

    return query.value(0).toInt() > 0;
}

bool DatabaseManager::checkIn(const QString &licensePlate, const QString &parkingName)
{
    if (!m_connected) {
        qDebug() << "数据库未连接！";
        return false;
    }

    QSqlDatabase dbc = threadConnection();

    if (!dbc.transaction()) {
        qDebug() << "开启事务失败:" << dbc.lastError().text();
        return false;
    }

    QSqlQuery insertQuery(dbc);
    insertQuery.prepare("INSERT INTO CAR (license_plate, check_in_time) "
                        "VALUES (:plate, NOW())");
    insertQuery.bindValue(":plate", licensePlate);

    if (!insertQuery.exec()) {
        qDebug() << "入库失败，回滚事务:" << insertQuery.lastError().text();
        dbc.rollback();
        return false;
    }

    // 同步 PARKING 表计数：P_now_count + 1
    if (!parkingName.isEmpty()) {
        QSqlQuery countQuery(dbc);
        countQuery.prepare("UPDATE PARKING SET P_now_count = P_now_count + 1 "
                          "WHERE P_name = :name");
        countQuery.bindValue(":name", parkingName);
        if (!countQuery.exec()) {
            qDebug() << "更新车位计数失败:" << countQuery.lastError().text();
            dbc.rollback();
            return false;
        }
    }

    // 提交事务
    if (!dbc.commit()) {
        qDebug() << "提交事务失败:" << dbc.lastError().text();
        dbc.rollback();
        return false;
    }

    qDebug() << "车辆入库成功:" << licensePlate;
    emit parkingDataChanged();
    return true;
}

bool DatabaseManager::checkOut(const QString &licensePlate, const QString &parkingName, double fee)
{
    if (!m_connected) {
        qDebug() << "数据库未连接！";
        return false;
    }

    QSqlDatabase dbc = threadConnection();

    if (!dbc.transaction()) {
        qDebug() << "开启事务失败:" << dbc.lastError().text();
        return false;
    }

    // 1. 查询未出库记录
    QSqlQuery findQuery(dbc);
    findQuery.prepare("SELECT id FROM CAR "
                      "WHERE license_plate = :plate AND check_out_time IS NULL "
                      "ORDER BY check_in_time DESC LIMIT 1");
    findQuery.bindValue(":plate", licensePlate);

    if (!findQuery.exec() || !findQuery.next()) {
        qDebug() << "未找到该车辆的入库记录:" << licensePlate;
        dbc.rollback();
        return false;
    }

    int recordId = findQuery.value(0).toInt();

    // 2. 更新出库时间及费用
    QSqlQuery updateQuery(dbc);
    updateQuery.prepare("UPDATE CAR SET check_out_time = NOW(), fee = :fee WHERE id = :id");
    updateQuery.bindValue(":fee", fee);
    updateQuery.bindValue(":id", recordId);

    if (!updateQuery.exec()) {
        qDebug() << "出库更新失败:" << updateQuery.lastError().text();
        dbc.rollback();
        return false;
    }

    // 同步 PARKING 表计数：P_now_count - 1
    if (!parkingName.isEmpty()) {
        QSqlQuery countQuery(dbc);
        countQuery.prepare("UPDATE PARKING SET P_now_count = "
                          "CASE WHEN P_now_count > 0 THEN P_now_count - 1 ELSE 0 END "
                          "WHERE P_name = :name");
        countQuery.bindValue(":name", parkingName);
        if (!countQuery.exec()) {
            qDebug() << "更新车位计数失败:" << countQuery.lastError().text();
            dbc.rollback();
            return false;
        }
    }

    // 提交事务
    if (!dbc.commit()) {
        qDebug() << "提交事务失败:" << dbc.lastError().text();
        dbc.rollback();
        return false;
    }

    qDebug() << "车辆出库成功:" << licensePlate;
    emit parkingDataChanged();
    return true;
}

QSqlQuery DatabaseManager::queryVehicle(const QString &licensePlate, bool onlyInPark)
{
    QSqlQuery query(dbc);

    if (onlyInPark) {
        query.prepare("SELECT id, license_plate, check_in_time, check_out_time, fee "
                      "FROM CAR WHERE license_plate = :plate AND check_out_time IS NULL");
    } else {
        query.prepare("SELECT id, license_plate, check_in_time, check_out_time, fee "
                      "FROM CAR WHERE license_plate = :plate "
                      "ORDER BY check_in_time DESC");
    }
    query.bindValue(":plate", licensePlate);

    if (!query.exec()) {
        qDebug() << "查询失败:" << query.lastError().text();
    }

    return query;
}

ParkingStats DatabaseManager::getParkingStats(const QString &parkingName)
{
    ParkingStats stats = {0, 0, 0};

    if (!m_connected) {
        qDebug() << "数据库未连接！";
        return stats;
    }

    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare("SELECT P_all_count, P_now_count "
                  "FROM PARKING WHERE P_name = :name");
    query.bindValue(":name", parkingName);

    if (!query.exec()) {
        qDebug() << "查询停车场统计失败:" << query.lastError().text();
        return stats;
    }

    if (query.next()) {
        stats.totalSpaces = query.value("P_all_count").toInt();
        stats.usedSpaces = query.value("P_now_count").toInt();
        stats.freeSpaces = stats.totalSpaces - stats.usedSpaces;
    } else {
        qDebug() << "未找到停车场:" << parkingName;
    }

    return stats;
}

QList<QVariantList> DatabaseManager::searchCars(const QString &plate, const QDateTime &startTime, const QDateTime &endTime, int status)
{
    QList<QVariantList> resultList;
    if(!m_connected) {
        return resultList;
    }

   QString sql = R"(
        SELECT
            id,
            license_plate,
            check_in_time,
            check_out_time,
            fee,
            TIMESTAMPDIFF(MINUTE, check_in_time,
                          IFNULL(check_out_time, NOW())) AS parking_minutes,
            CASE WHEN check_out_time IS NULL THEN '在场'
                 ELSE '已离场' END AS status_text
        FROM CAR
        WHERE 1=1
    )";

    QStringList conditions;
    QVariantMap bindValues;

    // 1.车牌模糊匹配
    if(!plate.isEmpty()){
        conditions << "license_plate LIKE :plate";
        bindValues[":plate"] = "%" + plate + "%";
    }

    // 2.时间筛选
    if(startTime.isValid()){
        conditions << "check_in_time >= :startTime";
        bindValues[":startTime"] = startTime;
    }

    // 3.结束时间筛选
    if(endTime.isValid()){
        conditions << "check_out_time <= :endTime";
        bindValues[":endTime"] = endTime;
    }

    // 4.条件筛选
    if(status == 1){
        conditions << "check_out_time IS NULL";
    }else if(status == 2){
        conditions << "check_out_time IS NOT NULL";
    }

    //把所有条件用 " AND " 拼到 WHERE 子句后面
    if(!conditions.isEmpty()) {
        sql += " AND " + conditions.join(" AND ");
    }

    sql += " ORDER BY check_out_time DESC";

    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare(sql);

    for(auto it = bindValues.constBegin(); it != bindValues.constEnd() ; ++it){
        query.bindValue(it.key(), it.value());
    }

    if(!query.exec()){
        return resultList;
    }

    while(query.next()){
        QVariantList row;
        row << query.value("id")
            << query.value("license_plate")
            << query.value("check_in_time")
            << query.value("check_out_time")
            << query.value("fee")
            << query.value("parking_minutes")
            << query.value("status_text");
        resultList << row;
    }
    return resultList;
}

QList<QVariantList> DatabaseManager::getRecentRecords(int count)
{
    QList<QVariantList> resultList;
    if (!m_connected || count <= 0) return resultList;

    QString sql = R"(
        SELECT id, license_plate, check_in_time, check_out_time, fee,
               TIMESTAMPDIFF(MINUTE, check_in_time,
                             IFNULL(check_out_time, NOW())) AS parking_minutes,
               CASE WHEN check_out_time IS NULL THEN '在场'
                    ELSE '已离场' END AS status_text
        FROM CAR
        ORDER BY check_in_time DESC
        LIMIT :count
    )";

    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare(sql);
    query.bindValue(":count", count);

    if (!query.exec()) return resultList;

    while (query.next()) {
        QVariantList row;
        row << query.value("id")
            << query.value("license_plate")
            << query.value("check_in_time")
            << query.value("check_out_time")
            << query.value("fee")
            << query.value("parking_minutes")
            << query.value("status_text");
        resultList << row;
    }
    return resultList;
}

QDateTime DatabaseManager::getVehicleCheckInTime(const QString &licensePlate)
{
    if(!m_connected) {
        return QDateTime();
    }

    QString sql = R"(SELECT check_in_time FROM CAR WHERE license_plate = :plate AND check_out_time IS NULL)";
    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare(sql);
    query.bindValue(":plate", licensePlate);

    if(!query.exec()){
        qDebug() << "执行失败" << query.lastError().text();
        return QDateTime();
    }

    if(query.next()){
        QDateTime dt = query.value(0).toDateTime();
        // 数据库存的是本地时间（无时区字符串），标记为系统时区，避免 Qt 6 误解析为 UTC
        dt.setTimeZone(QTimeZone::systemTimeZone());
        return dt;
    }else{
        return QDateTime();
    }
}

bool DatabaseManager::deleteCarRecord(int id)
{
    if(!isConnected()){
        qDebug() << "数据库连接失败";
        return false;
    }

    QSqlQuery searchCarId(dbc);
    QString searchCar = R"(SELECT check_out_time FROM Car WHERE id = :id)";
    searchCarId.prepare(searchCar);
    searchCarId.bindValue(":id", id);

    if(!searchCarId.exec()){
        qDebug() << "车辆记录出库信息查询失败" << searchCarId.lastError().text();
        return false;
    }

    QDateTime checkOutTime;
    if(searchCarId.next()){
        checkOutTime = searchCarId.value(0).toDateTime();
    }

    // 获取停车场名称（通过 InitFile 单例，避免绕过封装直接读 JSON）
    QString parkingName = InitFile::instance().getParkingName();

    if(!dbc.transaction()){
            qDebug() << "数据库事务开启失败";
            return false;
        }

    if(checkOutTime.isNull()){
        QSqlQuery deleteQuery(dbc);
        QString deleteParkingP_now_count = R"(UPDATE PARKING SET P_now_count = GREATEST(P_now_count-1,0) WHERE P_name = :P_name)";
        deleteQuery.prepare(deleteParkingP_now_count);
        deleteQuery.bindValue(":P_name", parkingName);
        if(!deleteQuery.exec()){
            qDebug() << "停车场车辆数量删除失败" << deleteQuery.lastError().text();
            dbc.rollback();
            return false;
        }
    }

    QSqlQuery deleteCarRecord(dbc);
    QString deleteRecord = R"(DELETE FROM Car WHERE id = :id)";
    deleteCarRecord.prepare(deleteRecord);
    deleteCarRecord.bindValue(":id", id);
    if(!deleteCarRecord.exec()){
        qDebug() << "车辆记录删除失败" << deleteCarRecord.lastError().text();
        dbc.rollback();
        return false;
    }

    if(!dbc.commit()){
        dbc.rollback();
        return false;
    }

    emit parkingDataChanged();
    return true;
}

bool DatabaseManager::deleteCarRecords(const QList<int> &ids)
{
    if (!isConnected() || ids.isEmpty()) return false;

    // 统计待删记录中仍在场的数量（check_out_time IS NULL）
    QStringList cntPl;
    for (int i = 0; i < ids.size(); ++i)
        cntPl << QString(":c%1").arg(i);

    QSqlQuery countQ(dbc);
    countQ.prepare(QString("SELECT COUNT(*) FROM CAR WHERE id IN (%1) AND check_out_time IS NULL")
                   .arg(cntPl.join(", ")));
    for (int i = 0; i < ids.size(); ++i)
        countQ.bindValue(QString(":c%1").arg(i), ids[i]);

    if (!countQ.exec() || !countQ.next()) return false;
    int inParkCount = countQ.value(0).toInt();

    // 构建 DELETE 占位符
    QStringList delPl;
    for (int i = 0; i < ids.size(); ++i)
        delPl << QString(":d%1").arg(i);

    QSqlDatabase dbc = threadConnection();

    if (!dbc.transaction()) return false;

    // 在场车辆 → 同步车位计数
    if (inParkCount > 0) {
        QSqlQuery upQ(dbc);
        upQ.prepare("UPDATE PARKING SET P_now_count = GREATEST(P_now_count - :cnt, 0) "
                    "WHERE P_name = :name");
        upQ.bindValue(":cnt", inParkCount);
        upQ.bindValue(":name", InitFile::instance().getParkingName());
        if (!upQ.exec()) { dbc.rollback(); return false; }
    }

    // 批量删除
    QSqlQuery delQ(dbc);
    delQ.prepare(QString("DELETE FROM CAR WHERE id IN (%1)").arg(delPl.join(", ")));
    for (int i = 0; i < ids.size(); ++i)
        delQ.bindValue(QString(":d%1").arg(i), ids[i]);

    if (!delQ.exec()) { dbc.rollback(); return false; }
    if (!dbc.commit()) { dbc.rollback(); return false; }

    emit parkingDataChanged();
    return true;
}

QList<QVariantList> DatabaseManager::searchUsers(const QString &keyword)
{
    QList<QVariantList> resultList;
    if (!m_connected) return resultList;

    QString sql = R"(
        SELECT id, username, truename, telephone, role, create_at
        FROM User
        WHERE 1=1
    )";

    if (!keyword.isEmpty()) {
        sql += " AND (username LIKE :keyword OR truename LIKE :keyword OR telephone LIKE :keyword)";
    }

    sql += " ORDER BY create_at DESC";

    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare(sql);

    if (!keyword.isEmpty()) {
        query.bindValue(":keyword", "%" + keyword + "%");
    }

    if (!query.exec()) return resultList;

    while (query.next()) {
        QVariantList row;
        row << query.value("id")
            << query.value("username")
            << query.value("truename")
            << query.value("telephone")
            << query.value("role")
            << query.value("create_at");
        resultList << row;
    }
    return resultList;
}

bool DatabaseManager::addUser(const QString &username, const QString &password, const QString &telephone, const QString &truename, const QString &role)
{
    if(!isConnected()){
        qDebug() << "数据库未连接";
        return false;
    }

    if(isUsernameExists(username)){
        qDebug() << "用户名已被占用";
        return false;
    }

    QString addUser = R"(INSERT INTO `User` (username, password, telephone, truename, role) VALUES (:username, :password, :telephone, :truename, :role))";
    QSqlDatabase dbc = threadConnection();
    QSqlQuery add(dbc);
    add.prepare(addUser);
    add.bindValue(":username", username);
    add.bindValue(":password", encryptPassword(password));
    add.bindValue(":telephone", telephone);
    add.bindValue(":truename", truename);
    add.bindValue(":role", role);

    if(!add.exec()){
        qDebug() << "添加用户失败" << add.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateUser(int id, const QString &username, const QString &telephone, const QString &truename, const QString &role)
{
    if(!isConnected()){
        qDebug() << "数据库未连接";
        return false;
    }

    QStringList setClauses;
    QVariantMap bindValues;

    if(!username.isEmpty()){
        QSqlDatabase dbc = threadConnection();
        QSqlQuery check(dbc);
        check.prepare("SELECT COUNT(*) FROM User WHERE username = :u AND id != :id");
        check.bindValue(":u",username);
        check.bindValue(":id", id);
        if(check.exec() && check.next() && check.value(0).toInt() >0){
            qDebug() << username << "已被占用";
            return false;
        }

        setClauses << "username = :username";
        bindValues[":username"] = username;
    }

    if(!telephone.isEmpty()){
        setClauses << "telephone = :telephone";
        bindValues[":telephone"] = telephone;
    }

    if(!truename.isEmpty()){
        setClauses << "truename = :truename";
        bindValues[":truename"] = truename;
    }

    if(!role.isEmpty()){
        setClauses << "role = :role";
        bindValues[":role"] = role;
    }

    QString sql = "UPDATE User SET " + setClauses.join(", ") + " WHERE id = :id";
    bindValues[":id"] = id;

    QSqlQuery query(dbc);
    query.prepare(sql);

    for(auto it = bindValues.begin(); it != bindValues.end(); ++it){
        query.bindValue(it.key(), it.value());
    }

    if(!query.exec()){
        qDebug() << "信息修改失败" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteUser(int id)
{
    if(!isConnected()){
        qDebug() << "数据库未连接";
        return false;
    }

    QString sql = R"(DELETE FROM User WHERE id = :id)";
    QSqlDatabase dbc = threadConnection();
    QSqlQuery query(dbc);
    query.prepare(sql);
    query.bindValue(":id", id);

    if(!query.exec()){
        qDebug() << "删除用户失败" << query.lastError().text();
        return false;
    }

    return true;
}
