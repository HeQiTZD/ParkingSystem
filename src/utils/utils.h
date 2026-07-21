#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QCryptographicHash>
inline QString encryptPassword(const QString &password){

    QByteArray passwordBytes = password.toUtf8();

    QByteArray hashBytes = QCryptographicHash::hash(passwordBytes,QCryptographicHash::Sha256);

    return QString::fromLatin1(hashBytes.toHex());
}
#endif