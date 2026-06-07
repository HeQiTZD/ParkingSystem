#ifndef CAR_H
#define CAR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariant>


/**
 * @brief 车辆信息数据模型
 *
 * 封装车辆进出记录的数据结构，提供数据验证和序列化功能。
 * 用于在数据库和UI之间传递车辆信息。
 *
 * 设计模式：
 * - 数据传输对象（DTO）模式
 * - 值对象（Value Object）模式
 */
class Car
{
public:
    /**
     * @brief 默认构造函数
     */
    Car();

    /**
     * @brief 参数化构造函数
     * @param licensePlate 车牌号
     * @param checkInTime 入库时间
     * @param location 停车位置
     */
    Car(const QString &licensePlate, const QDateTime &checkInTime, const QString &location);

    /**
     * @brief 拷贝构造函数
     */
    Car(const Car &other);

    /**
     * @brief 赋值操作符
     */
    Car& operator=(const Car &other);

    /**
     * @brief 析构函数
     */
    ~Car();

    // ========== Getter方法 ==========
    int getId() const {return m_id;};
    QString getLicensePlate() const {return m_licensePlate;}
    QDateTime getCheckInTime() const {return m_checkInTime;}
    QDateTime getCheckOutTime() const {return m_checkOutTime;}
    double getFee() const {return m_fee;}
    QString getLocation() const;

    // ========== Setter方法 ==========
    void setLicensePlate(const QString &plate) {m_licensePlate = plate;}
    void setCheckInTime(const QDateTime &time) {m_checkInTime = time;}
    void setCheckOutTime(const QDateTime &time) {m_checkOutTime = time;}
    void setFee(double fee) {m_fee = fee;}
    void setLocation(const QString &location) {m_location = location;}

    // ========== 业务方法 ==========
    /**
     * @brief 检查车辆是否在场
     * @return 是否在场（已入库未出库）
     */
    bool isParked();

    /**
     * @brief 计算停车时长（小时）
     * @return 停车小时数
     */
    double getParkingDuration() const;

    /**
     * @brief 验证车牌号格式
     * @param plate 车牌号
     * @return 是否有效
     *
     * 规则：
     * 1. 长度为7或8位
     * 2. 第一位为省份简称
     * 3. 第二位为字母
     * 4. 后续为字母和数字组合
     */
    static bool isValidLicensePlate(const QString &plate);

    /**
     * @brief 计算停车费用
     * @param hourlyRate 每小时费率
     * @return 费用
     */
    double calculateFee(double houlyRate) const;

    /**
     * @brief 转换为QVariantMap（用于数据库操作）
     */
    QVariantMap toMap() const;

    /**
     * @brief 从QVariantMap创建Car对象
     */
    static Car fromMap(const QVariantMap &map);

    /**
     * @brief 转换为可读字符串
     */
    QString toString() const;

private:
    int m_id;// 记录ID
    QString m_licensePlate;// 车牌号
    QDateTime m_checkInTime;// 入库时间
    QDateTime m_checkOutTime;// 出库时间
    double m_fee;// 停车费用
    QString m_location;// 停车位置

    /**
     * @brief 省份简称列表（用于车牌验证）
     */
    static const QString PLATE_LETTERS;
    static const QString PROVINCE_CHARS;
};
#endif // CAR_H

/*
1. 数据库设计原则
1.1 表结构设计

CREATE TABLE IF NOT EXISTS CAR (
    id INT PRIMARY KEY AUTO_INCREMENT,    -- 主键自增
    license_plate VARCHAR(20) NOT NULL,   -- 车牌号
    check_in_time DATETIME NOT NULL,      -- 入库时间
    check_out_time DATETIME DEFAULT NULL, -- 出库时间（可为空）
    fee DECIMAL(10, 2) DEFAULT NULL,      -- 费用（精确到分）
    INDEX idx_license_plate (license_plate) -- 索引
);
知识点：

主键（PRIMARY KEY）：唯一标识每条记录
自增（AUTO_INCREMENT）：自动分配ID
NOT NULL：字段不允许为空
DEFAULT：设置默认值
INDEX：索引加速查询
1.2 数据类型选择
字段类型	MySQL类型	Qt类型	用途
整数	INT	int	ID、数量
字符串	VARCHAR	QString	车牌号、名称
时间	DATETIME	QDateTime	入库/出库时间
小数	DECIMAL	double	费用
布尔	TINYINT	bool	状态标志
2. SQL语句类型
2.1 DDL（数据定义语言）

-- 创建表
CREATE TABLE IF NOT EXISTS table_name (...);

-- 修改表
ALTER TABLE table_name ADD COLUMN column_name type;

-- 删除表
DROP TABLE IF EXISTS table_name;
2.2 DML（数据操作语言）

-- 插入数据
INSERT INTO table_name (col1, col2) VALUES (:val1, :val2);

-- 查询数据
SELECT * FROM table_name WHERE condition;

-- 更新数据
UPDATE table_name SET col1 = :val1 WHERE condition;

-- 删除数据
DELETE FROM table_name WHERE condition;
3. Qt数据库编程
3.1 预处理语句（防止SQL注入）

// 错误方式（容易被SQL注入攻击）
QString sql = QString("SELECT * FROM user WHERE name='%1'").arg(username);

// 正确方式（使用预处理语句）
QSqlQuery query;
query.prepare("SELECT * FROM user WHERE name = :name");
query.bindValue(":name", username);
query.exec();
3.2 事务处理

QSqlDatabase::database().transaction();  // 开始事务

// 执行多个SQL操作
QSqlQuery query1;
query1.exec("INSERT INTO ...");

QSqlQuery query2;
query2.exec("UPDATE ...");

if (allSuccess) {
    QSqlDatabase::database().commit();   // 提交事务
} else {
    QSqlDatabase::database().rollback(); // 回滚事务
}
4. 数据模型设计模式
4.1 DTO（数据传输对象）模式

class Car {
public:
    // Getter/Setter方法
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    // 序列化方法
    QVariantMap toMap() const;
    static Car fromMap(const QVariantMap &map);

private:
    int m_id;
    QString m_licensePlate;
    // ...
};
作用：在不同层之间传输数据，降低耦合度。

4.2 数据验证

bool Car::isValidLicensePlate(const QString &plate)
{
    // 1. 长度验证
    if (plate.length() != 7 && plate.length() != 8) {
        return false;
    }

    // 2. 格式验证（正则表达式）
    QRegularExpression regex("^[京津沪渝...][A-Z][A-Z0-9]{5,6}$");
    return regex.match(plate).hasMatch();
}
5. 触发器和事件
5.1 触发器（Trigger）

CREATE TRIGGER trg_reservation_insert
AFTER INSERT ON reservations
FOR EACH ROW
BEGIN
    UPDATE PARKING SET P_reserve_count = P_reserve_count + 1
    WHERE P_name = NEW.P_name;
END
作用：当表数据变化时，自动执行相关操作。

5.2 定时事件（Event）

CREATE EVENT clean_reservations
ON SCHEDULE EVERY 1 MINUTE
DO DELETE FROM reservations WHERE TIMESTAMPDIFF(MINUTE, created_at, NOW()) > 30;
作用：定期执行清理任务。

6. 索引优化

-- 单列索引
INDEX idx_license_plate (license_plate)

-- 复合索引
INDEX idx_time_location (check_in_time, location)
索引使用原则：

在WHERE条件中频繁使用的列
JOIN连接的列
ORDER BY排序的列
高选择性的列（不同值多）
代码工作原理
数据库初始化流程

系统启动
    ↓
检查数据库连接
    ↓
创建用户表 (USER)
    ↓
创建停车场表 (PARKING)
    ↓
创建车辆记录表 (CAR)
    ↓
创建预约表 (reservations)
    ↓
初始化停车场数据
    ↓
创建触发器和事件
    ↓
初始化完成
车辆入库流程

用户输入/识别车牌号
    ↓
验证车牌号格式
    ↓
检查车辆是否在场
    ↓
记录入库时间
    ↓
插入CAR表
    ↓
更新PARKING表（现有车辆+1）
    ↓
显示成功消息
车辆出库流程

用户输入/识别车牌号
    ↓
查询车辆入库记录
    ↓
计算停车时长
    ↓
计算停车费用
    ↓
更新CAR表（出库时间、费用）
    ↓
更新PARKING表（现有车辆-1）
    ↓
显示费用信息
*/