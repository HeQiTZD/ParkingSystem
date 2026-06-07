#include "mysqlinit.h"
#include <QStringLiteral>
//构造函数
MySQLInit::MySQLInit(DatabaseManager *dbManager, QObject *parent)
    :QObject(parent), m_dbManager(dbManager) {}

//析构函数
MySQLInit::~MySQLInit() {}

bool MySQLInit::initAll()
{
    if(!m_dbManager || !m_dbManager->isConnected()){
        emit initFinished(false,"数据库未连接");
        return false;
    }

    emit initProgress("开始初始化数据库...", 0);

    // 1. 创建用户表
    emit initProgress("创建用户表", 10);
    if(!createUserTable()){
        emit initFinished(false, "创建用户表失败");
        return false;
    }

    // 2. 创建停车场表
    emit initProgress("创建停车场表", 30);
    if(!createParkingTable()){
        emit initFinished(false, "创建停车场表失败");
        return false;
    }

    // 3. 创建车辆记录表
    emit initProgress("创建车辆记录表", 50);
    if(!createCarTable()){
        emit initFinished(false, "创建车辆记录表失败");
        return false;
    }

    // 4. 创建预约表
    emit initProgress("创建预约表", 70);
    if(!createReservationTable()){
        emit initFinished(false, "创建预约表失败");
        return false;
    }

    // 5. 初始化停车场数据
    emit initProgress("初始化停车场数据", 80);
    if(!initParkingData("默认停车场", 100, 5.0)){
        emit initFinished(false, "初始化停车场数据失败");
        return false;
    }

    // 6. 创建触发器
    emit initProgress("创建触发器...", 90);
    if(!createTriggers()){
        qDebug() << QStringLiteral("创建触发器失败,但不影响主要功能");
    }

    emit initProgress("初始化完成", 100);
    emit initFinished(true,"数据库初始化完成");
    return true;
}

bool MySQLInit::createUserTable()
{   //待改进
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS USER(
            id INT PRIMARY KEY AUTO_INCREMENT,
            username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
            password VARCHAR(64) NOT NULL COMMENT '密码(SHA256)',
            telephone VARCHAR(11) DEFAULT '' COMMENT '手机号',
            truename VARCHAR(50) DEFAULT '' COMMENT '真实姓名',
            role VARCHAR(10) DEFAULT '' COMMENT '角色(admin/user)'，
            create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
            update_at DATETIME DEFAULT CURRENT_TIMESTAMP NO UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
            INDEX idx_username(username)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表'
    )";

    return execteSql(sql,"创建用户表");
}

bool MySQLInit::createCarTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS CAR(
            id INT PRIMARY KEY AUTO_INCREMENT,
            license_plate CARCHAR(20) NOT NULL COMMENT '车牌号',
            check_in_time DATATIME NOT NULL COMMENT '入库时间',
            chekc_out_time DATATIME NOT NULL COMMENT '出库时间'，
            fee DECIMAL(10,2) DEFAULT NOT NULL COMMENT '停车费用',
            location VARCHAR(50) NOT NULL COMMENT '停车位置',
            create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
            INDEX idx_license_plate (license_plate),
            INDEX idx_check_in_time (check_in_time),
            INDEX idx_location (location)
        ) ENGINE=InnoDB DEFAULT CAHRSET=utf8mb4 COMMENT='车辆记录表'
    )";

    return execteSql(sql,"创建车辆记录表");
}

bool MySQLInit::createParkingTable()
{
    QString sql = R"(
    CREATE TABLE IF NOT EXISTS PARKING(
    P_id INT PRIMARY KEY AUTO_INCREMENT,
    P_name VARCHAR(100) UNIQUE NOT NULL COMMENT '停车场',
    P_now_count INT DEFAULT 0 COMMENT '现有车辆数',
    P_reserve_count INT DEFAULT 0 COMMENT '预约车辆数',
    P_all_count INT NOT NULL COMMENT '总车位数',
    P_fee DECIMAL(10,,2) NOT NULL COMMENT '每小时费用',
    create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_p_name (P_name)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb6 COMMENT='停车场记录表'
    )";

    return execteSql(sql,"创建停车场表");
}

bool MySQLInit::createReservationTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS reservations(
        id INT PRIMARY KEY AUTO_INCREMENT,
        license_plate VARCHAR(20) NOT NULL COMMENT '车牌号',
        P_namae VARCHAR(100) NOT NULL COMMENT '停车场名称',
        create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '预约时间',
        UNIQUE INDEX idx_license_plate (license_plate)
        ) ENGINE=InnoDB DEFAULT=utf8mb6 COMMENT='预约表'
    )";

    return execteSql(sql, "创建预约表");
}

bool MySQLInit::initParkingData(const QString &parkingName, int totalSpace, double fee)
{
    //检查是否已存在
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM PARKING WHERE P_name = :name");
    checkQuery.bindValue(":name", parkingName);

    if(!checkQuery.exec() || checkQuery.next()){
        qDebug() << QStringLiteral("检查停车场数据失败") << checkQuery.lastError().text();
        return false;
    }

    if(checkQuery.value(0).toInt() > 0){
        qDebug() << QStringLiteral("停车场数据已存在，跳过初始化");
        return true;
    }

    //插入初始数据
    QSqlQuery inserQuery;
    inserQuery.prepare(R"(
        INSERT INTO PARKING (P_name, p_now_count, P_reserve_count, P_all_count, P_fee)
        VALUES (:name, 0, 0, :all_count, :fee)
    )");

    inserQuery.bindValue(":name", parkingName);
    inserQuery.bindValue(":all_count", totalSpace);
    inserQuery.bindValue(":fee", fee);

    if(!inserQuery.exec()){
        qDebug() << QStringLiteral("初始化停车场数据失败") << inserQuery.lastError().text();
        return false;
    }

    qDebug() << QStringLiteral("初始化停车场数据成功") << parkingName;
    return true;
}

bool MySQLInit::createTriggers()
{
    bool success = true;

    // 1.开启事件调度器
    success &= execteSql("SET GLOBAL event_scheduler = ON", "开启事件调度器");

    // 2. 创建定时清理过期预约的任务（每分钟执行，清理30分钟前的预约）
    success &= execteSql(R"(
        CREATE EVENT IF NOT EXISTS clean__reservations
        ON SCHEDULE EVERY 1 MINUTE
        DO DELETE FROM reservations WHERE TIMESTAMPDIFF(MINUTE, create_at, NOW()) >30
        )", "创建定时清理过期预约任务");

    // 3. 创建预约时插入触发器（预约数+1）
    success &= execteSql(R"(
        CREATE TRIGGER IF NOT EXISTS trg_reservation_insert
        AFTER INSERT ON reservations
        FOR EACH ROW
        BEGIN
            UPDATE PARKING SET P_reserve_count = P_reserve_count +1
            WHERE p_name = NEW.P_name;
            END
        )", "创建预约插入触发器");

    // 4. 创建预约删除触发器（预约数-1）
    success &= execteSql(R"(
        CREATE TRIGGER IF NOT EXISTS trg_reservation_delete
        AFTER DELETE ON reservations
        FOR EACH ROW
        BEGIN
            UPDATE PARKGING SET P_reserve_count = P_reserve_count -1
            WHERE P_name = OLD.P_name;
            END
        )", "创建预约删除触发器");

    return success;
}

bool MySQLInit::isTableExists(const QString &tableName)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = DATABASE() AND table_name = :name");
    query.bindValue(":name", tableName);

    if(query.exec() && query.next()){
        return query.value(0).toInt() >0;
    }
    return false;
}

bool MySQLInit::execteSql(const QString &sql, const QString &description)
{
    QSqlQuery query;
    if(!query.exec(sql)){
        qDebug() << description << QStringLiteral("失败") << query.lastError().text();
        return false;
    }
    qDebug() << description << QStringLiteral("成功");
    return true;
}
