#include "databasemanager.h"


DatabaseManager::DatabaseManager(QObject *parent): QObject(parent), connected(false){}

DatabaseManager::~DatabaseManager()
{
    disconnectDatabase();
}

bool DatabaseManager::connectDatabase(const QString &host, int port, const QString &dbName, const QString &username, const QString &password)
{
    //检查是否已连接
    if(connected){
        disconnectDatabase();
    }

    //添加数据库驱动
    db = QSqlDatabase::addDatabase("QMYSQL");

    //设置连接参数
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(username);
    db.setPassword(password);

    //尝试连接
    if(!db.open()){
        qDebug() << "数据库连接失败" << db.lastError().text();
        connected = false;
        emit connectionStatusChanged(false);
        return false;
    }

    connected = true;
    emit connectionStatusChanged(true);
    qDebug() << "数据库连接成功！";
    return true;
}

void DatabaseManager::disconnectDatabase()
{
    if(connected && db.isOpen()){
        db.close();
        connected = false;
        emit connectionStatusChanged(false);
        qDebug() << "数据库连接已关闭";
    }
}

bool DatabaseManager::isConnected() const
{
    return connected;
}

QSqlQuery DatabaseManager::executeQuery(const QString &sql)
{
    QSqlQuery query(db);
    if(!query.exec(sql)){
        qDebug() << "sql执行失败"<<query.lastError().text();
    }
    return query;
}

bool DatabaseManager::validateUser(const QString &username, const QString &password, QString &userRole)
{
    if(!connected){
        qDebug() << "数据库未连接！";
        return false;
    }

    //使用预处理语句防止sql注入
    QSqlQuery query(db);
    query.prepare("SELECT role FROM user WHERE username = :username AND password = :password");
    query.bindValue(":username",username);
    query.bindValue(":password",password);

    if(!query.exec()){
        qDebug() << "用户验证查询失败" << query.lastError().text();
        return false;
    }

    if(query.next()){
        userRole = query.value("role").toString();
        return true;
    }
    return false;
}

bool DatabaseManager::isUsernameExists(const QString &username)
{
    if(!connected){
        qDebug() << "数据库未连接！";
        return false;
    }
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM user WHERE username = :username");
    checkQuery.bindValue(":username",username);

    if(!checkQuery.exec() ||!checkQuery.next()){
        qDebug()<< "查询用户失败" <<checkQuery.lastError().text();
        return false;
    }
    return checkQuery.value(0).toInt() > 0;
}

bool DatabaseManager::registerUser(const QString &username, const QString &password, const QString &phone)
{
    if(!connected){
        qDebug() << "数据库未连接！";
        return false;
    }

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO users (username, password, phone, role)"
                        "VALUES (:username, :password, :phone, 'user')");
    insertQuery.bindValue(":username",username);
    insertQuery.bindValue(":password",password);
    insertQuery.bindValue(":phone",phone);

    if(!insertQuery.exec()){
        qDebug() << "注册用户失败" << insertQuery.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateParkingConfig(const QString &name, double price, int capacity)
{
    if(!connected){
        qDebug() << QStringLiteral("数据库未连接");
        return false;
    }

    // 先检查停车场是否存在
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM PARKING WHERE P_name = :name");
    checkQuery.bindValue(":name",name);

    if(!checkQuery.exec() || !checkQuery.next()){
        qDebug() << QStringLiteral("查询停车场失败:") << checkQuery.lastError().text();
        return false;
    }

    if(checkQuery.value(0).toInt() > 0){
        QSqlQuery updateQuery;
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
        QSqlQuery insertQuery;
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
    return false;
}
