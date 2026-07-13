#include "mysqlinit.h"
#include "src/utils/utils.h"<longcat_arg_value>

//构造函数
MySQLInit::MySQLInit(DatabaseManager *dbManager, InitFile *initFile, QObject *parent)
    : QObject(parent), m_dbManager(dbManager), m_initFile(initFile) {}

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

    // 1.1 初始化默认管理员（表为空时才插入，保证幂等）
    emit initProgress("初始化默认管理员", 20);
    if(!initAdminUser()){
        emit initFinished(false, "初始化默认管理员失败");
        return false;
    }

    // 2. 创建停车场表
    emit initProgress("创建停车场表", 40);
    if(!createParkingTable()){
        emit initFinished(false, "创建停车场表失败");
        return false;
    }

    // 3. 创建车辆记录表
    emit initProgress("创建车辆记录表", 55);
    if(!createCarTable()){
        emit initFinished(false, "创建车辆记录表失败");
        return false;
    }

    // 4. 初始化停车场数据 — 从配置文件中读取用户在初始化窗口填写的停车场信息
    emit initProgress("初始化停车场数据", 75);
    if (!m_initFile) {
        emit initFinished(false, "配置文件管理器未注入");
        return false;
    }
    if (!initParkingData(m_initFile->getParkingName(),
                         m_initFile->getParkingCapacity(),
                         m_initFile->getParkingPrice())) {
        emit initFinished(false, "初始化停车场数据失败");
        return false;
    }

    emit initProgress("初始化完成", 100);
    emit initFinished(true,"数据库初始化完成");
    return true;
}

bool MySQLInit::createUserTable()
{   //待改进
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS User(
            id INT PRIMARY KEY AUTO_INCREMENT,
            username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
            password VARCHAR(64) NOT NULL COMMENT '密码(SHA256)',
            telephone VARCHAR(11) DEFAULT '' COMMENT '手机号',
            truename VARCHAR(50) DEFAULT '' COMMENT '真实姓名',
            role VARCHAR(10) DEFAULT '' COMMENT '角色(admin/user)',
            create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
            INDEX idx_username(username)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表'
    )";

    return executeSql(sql,"创建用户表");
}

bool MySQLInit::createCarTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS CAR(
            id INT PRIMARY KEY AUTO_INCREMENT,
            license_plate VARCHAR(20) NOT NULL COMMENT '车牌号',
            check_in_time DATETIME NOT NULL COMMENT '入库时间',
            check_out_time DATETIME DEFAULT NULL COMMENT '出库时间',
            fee DECIMAL(10,2) DEFAULT NULL COMMENT '停车费用',
            create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
            INDEX idx_license_plate (license_plate),
            INDEX idx_check_in_time (check_in_time)
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='车辆记录表'
    )";

    return executeSql(sql,"创建车辆记录表");
}

bool MySQLInit::createParkingTable()
{
    QString sql = R"(
    CREATE TABLE IF NOT EXISTS PARKING(
    P_id INT PRIMARY KEY AUTO_INCREMENT,
    P_name VARCHAR(100) UNIQUE NOT NULL COMMENT '停车场',
    P_now_count INT DEFAULT 0 COMMENT '现有车辆数',
    P_all_count INT NOT NULL COMMENT '总车位数',
    P_fee DECIMAL(10,2) NOT NULL COMMENT '每小时费用',
    create_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间'
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='停车场记录表'
    )";

    return executeSql(sql,"创建停车场表");
}

bool MySQLInit::initParkingData(const QString &parkingName, int totalSpace, double fee)
{
    //检查是否已存在
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM PARKING WHERE P_name = :name");
    checkQuery.bindValue(":name", parkingName);

    if(!checkQuery.exec() || !checkQuery.next()){
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
        INSERT INTO PARKING (P_name, P_now_count, P_all_count, P_fee)
        VALUES (:name, 0, :all_count, :fee)
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

bool MySQLInit::initAdminUser()
{
    // 1. 判断 admin 是否已存在，存在则跳过（幂等，可重复调用）
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM User WHERE username = :username");
    checkQuery.bindValue(":username", "admin");

    if(!checkQuery.exec() || !checkQuery.next()){
        qDebug() << QStringLiteral("检查默认管理员失败") << checkQuery.lastError().text();
        return false;
    }

    if(checkQuery.value(0).toInt() > 0){
        qDebug() << QStringLiteral("默认管理员已存在，跳过初始化");
        return true;
    }

    // 2. 不存在则插入默认管理员（密码 SHA256 加密后存入）
    QSqlQuery insertQuery;
    insertQuery.prepare(R"(
        INSERT INTO User (username, password, telephone, truename, role)
        VALUES (:username, :password, :telephone, :truename, 'admin')
    )");

    insertQuery.bindValue(":username", "admin");
    insertQuery.bindValue(":password", encryptPassword("admin123"));   // 默认密码 admin123，生产环境请修改
    insertQuery.bindValue(":telephone", "13800000000");
    insertQuery.bindValue(":truename", "系统管理员");

    if(!insertQuery.exec()){
        qDebug() << QStringLiteral("插入默认管理员失败") << insertQuery.lastError().text();
        return false;
    }

    qDebug() << QStringLiteral("初始化默认管理员成功（用户名: admin, 密码: admin123）");
    return true;
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

bool MySQLInit::executeSql(const QString &sql, const QString &description)
{
    QSqlQuery query;
    if(!query.exec(sql)){
        qDebug() << description << QStringLiteral("失败") << query.lastError().text();
        return false;
    }
    qDebug() << description << QStringLiteral("成功");
    return true;
}
