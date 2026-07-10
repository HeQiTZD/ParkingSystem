#include "car.h"
#include "src/utils/initfile.h"
#include <QRegularExpression>
#include <QDebug>

// 静态常量定义
const QString Car::PROVINCE_CHARS = "京津沪渝冀豫云辽黑湘皖鲁新苏浙赣鄂桂甘晋蒙陕吉闽贵粤青藏川宁琼";
const QString Car::PLATE_LETTERS = "ABCDEFGHJKLMNPQRSTUVWXYZ";

Car::Car() : m_id(0), m_fee(0.0) {}

Car::Car(const QString &licensePlate, const QDateTime &checkInTime)
    : m_id(0)
    , m_licensePlate(licensePlate)
    , m_checkInTime(checkInTime)
    , m_fee(0.0)
{}

Car::Car(const Car &other)
    : m_id(other.m_id)
    , m_licensePlate(other.m_licensePlate)
    , m_checkInTime(other.m_checkInTime)
    , m_checkOutTime(other.m_checkOutTime)
    , m_fee(other.m_fee)
{}

Car &Car::operator=(const Car &other)
{
    if(this != &other){
        m_id = other.m_id;
        m_licensePlate = other.m_licensePlate;
        m_checkInTime = other.m_checkInTime;
        m_checkOutTime = other.m_checkOutTime;
        m_fee = other.m_fee;
    }
    return *this;
}

Car::~Car() {}

bool Car::isParked() const
{
    // 入库时间有效且出库时间无效表示车辆在场
    return m_checkInTime.isValid() && !m_checkOutTime.isValid();
}

double Car::getParkingDuration() const
{
    if(!m_checkInTime.isValid()){
        return 0.0;
    }

    QDateTime endTime = m_checkOutTime.isValid() ? m_checkOutTime : QDateTime::currentDateTime();
    qint64 seconds = m_checkInTime.secsTo(endTime);
    return static_cast<double>(seconds) /3600.0; // 转换为小时
}

bool Car::isValidLicensePlate(const QString &plate)
{
    // 规则1：长度必须为7或8
    if(plate.length() != 7 && plate.length() !=8){
        return false;
    }

    // 规则2：第一位必须是省份简称
    if(!PROVINCE_CHARS.contains(plate.left(1))){
        return false;
    }

    if(!PLATE_LETTERS.contains(plate.mid(1,1))){
        return false;
    }

    // 规则4：后续字符必须是字母或数字
    QRegularExpression regex("^[京津沪渝冀豫云辽黑湘皖鲁新苏浙赣鄂桂甘晋蒙陕吉闽贵粤青藏川宁琼][A-Z][A-Z0-9]{5,6}$");
    return regex.match(plate).hasMatch();
}

// 计算停车费用：从配置文件读取费率与免费时长，按半小时递进计费
double Car::calculateFee(const QDateTime &checkInTime, const QDateTime &checkOutTime)
{
    InitFile cfg;
    cfg.loadConfig();

    double hourlyRate = cfg.getParkingPrice();
    int freeMinutes = cfg.getFreeMinutes();

    // 总停车分钟数（出库时间无效时按当前时间计）
    qint64 totalMinutes = checkInTime.secsTo(checkOutTime.isValid() ? checkOutTime : QDateTime::currentDateTime()) / 60;

    // 扣除免费时长后的计费分钟数，负值钳位到 0
    double billableMinutes = qMax(0LL, totalMinutes - freeMinutes);
    if(billableMinutes <= 0.0) return 0.0;

    // 按半小时阶梯向上取整 × 半小时费率（每小时费率的一半）
    int halfHours = static_cast<int>(std::ceil(billableMinutes / 30.0));
    return halfHours * hourlyRate / 2.0;
}

QVariantMap Car::toMap() const
{
    QVariantMap map;
    map["id"] = m_id;
    map["license_plate"] = m_licensePlate;
    map["check_in_time"] = m_checkInTime;
    map["check_out_time"] = m_checkOutTime;
    map["fee"] = m_fee;
    return map;
}

Car Car::fromMap(const QVariantMap &map)
{
    Car car;
    car.m_id = map["id"].toInt();
    car.m_licensePlate = map["license_plate"].toString();
    car.m_checkInTime = map["check_in_time"].toDateTime();
    car.m_checkOutTime = map["check_out_time"].toDateTime();
    car.m_fee = map["fee"].toDouble();
    return car;
}

QString Car::toString() const
{
    return QString("Car[id=%1, plate=%2, in=%3, out=%4, fee=%5]")
        .arg(m_id)
        .arg(m_licensePlate)
        .arg(m_checkInTime.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(m_checkOutTime.isValid() ? m_checkOutTime.toString("yyyy-MM-dd hh:mm:ss") : "未出库")
        .arg(m_fee, 0, 'f', 2);
}