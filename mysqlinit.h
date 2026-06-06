#ifndef MYSQLINIT_H
#define MYSQLINIT_H

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "databasemanager.h"

/**
 * @brief 数据库初始化类
 *
 * 负责创建数据库表结构、初始化数据、建立索引和触发器。
 * 在系统首次运行或数据库重置时调用。
 *
 * 主要功能：
 * 1. 创建用户表（USER）
 * 2. 创建车辆记录表（CAR）
 * 3. 创建停车场信息表（PARKING）
 * 4. 创建预约表（reservations）
 * 5. 初始化停车场数据
 * 6. 创建触发器和定时任务
 */
class MySQLInit :public QObject
{
    Q_OBJECT
public:
    explicit MySQLInit(DatabaseManager* dbManager, QObject* parent = nullptr);
    ~MySQLInit();

    /**
     * @brief 执行所有初始化操作
     * @return 是否全部成功
     */
    bool initAll();

    /**
     * @brief 创建用户表
     * @return 是否成功
     *
     * 表结构：
     * - id: 主键，自增
     * - username: 用户名，唯一
     * - password: 密码（SHA256加密）
     * - telephone: 手机号
     * - truename: 真实姓名
     * - role: 角色（admin/user）
     */
    bool createUserTable();

    /**
     * @brief 创建车辆记录表
     * @return 是否成功
     *
     * 表结构：
     * - id: 主键，自增
     * - license_plate: 车牌号，索引
     * - check_in_time: 入库时间
     * - check_out_time: 出库时间
     * - fee: 停车费用
     * - location: 停车位置
     */
    bool createCarTable();

    /**
     * @brief 创建停车场信息表
     * @return 是否成功
     *
     * 表结构：
     * - P_id: 主键，自增
     * - P_name: 停车场名称，唯一
     * - P_now_count: 现有车辆数
     * - P_reserve_count: 预约车辆数
     * - P_all_count: 总车位数
     * - P_fee: 每小时费用
     */
    bool createParkingTable();

    /**
     * @brief 创建预约表
     * @return 是否成功
     *
     * 表结构：
     * - id: 主键，自增
     * - license_plate: 车牌号，唯一
     * - P_name: 停车场名称
     * - created_at: 创建时间
     */
    bool createReservationTable();

    /**
     * @brief 初始化停车场数据
     * @param parkingName 停车场名称
     * @param totalSpaces 总车位数
     * @param fee 每小时费用
     * @return 是否成功
     */
    bool initParkingData(const QString &parkingName, int totalSpace, double fee);

    /**
     * @brief 创建触发器和定时任务
     * @return 是否成功
     *
     * 功能：
     * 1. 预约数量自动增减触发器
     * 2. 过期预约自动清理任务（30分钟）
     */
    bool createTriggers();

signals:
    /**
     * @brief 初始化进度信号
     * @param message 进度信息
     * @param progress 进度百分比（0-100）
     */
    void initProgress(const QString &message, int progress);

    /**
     * @brief 初始化完成信号
     * @param success 是否成功
     * @param message 结果信息
     */
    void initFinished(bool success,const QString &message);

private:
    DatabaseManager *m_dbManager;// 数据库管理器

    /**
     * @brief 检查表是否存在
     * @param tableName 表名
     * @return 是否存在
     */
    bool isTableExists(const QString &tableName);

    /**
     * @brief 执行SQL语句
     * @param sql SQL语句
     * @param description 描述（用于日志）
     * @return 是否成功
     */
    bool execteSql(const QString &sql, const QString &description);
};

#endif // MYSQLINIT_H
