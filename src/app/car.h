#ifndef CAR_H
#define CAR_H
#include <QString>
#include <QDateTime>
#include <QVariant>
class Car
{
public:
    Car();

    Car(const QString &licensePlate, const QDateTime &checkInTime);

    Car(const Car &other);

    Car& operator=(const Car &other);

    ~Car();

    int getId() const {return m_id;}
    QString getLicensePlate() const {return m_licensePlate;}
    QDateTime getCheckInTime() const {return m_checkInTime;}
    QDateTime getCheckOutTime() const {return m_checkOutTime;}
    double getFee() const {return m_fee;}

    void setLicensePlate(const QString &plate) {m_licensePlate = plate;}
    void setCheckInTime(const QDateTime &time) {m_checkInTime = time;}
    void setCheckOutTime(const QDateTime &time) {m_checkOutTime = time;}
    void setFee(double fee) {m_fee = fee;}

    bool isParked() const;

    double getParkingDuration() const;

    static bool isValidLicensePlate(const QString &plate);

    static double calculateFee(const QDateTime &checkInTime,
                              const QDateTime &checkOutTime,
                              double hourlyRate,
                              int freeMinutes);

    QVariantMap toMap() const;

    static Car fromMap(const QVariantMap &map);

    QString toString() const;
    static QString normalizePlate(const QString &input);

    static QString displayPlate(const QString &plate);
private:
    int m_id;
    QString m_licensePlate;
    QDateTime m_checkInTime;
    QDateTime m_checkOutTime;
    double m_fee;

    static const QString PLATE_LETTERS;
    static const QString PROVINCE_CHARS;
};
#endif