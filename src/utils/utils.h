#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QCryptographicHash>//Qt 的加密哈希算法类，支持 MD4、MD5、SHA-1、SHA-256、SHA-512 等。

//使用SHA-256算法对密码进行加密
//密码加密函数（SHA256）
inline QString encryptPassword(const QString &password){

    //将输入的 QString 密码转换为 UTF-8 编码的字节数组 (QByteArray)。
    //哈希算法通常操作原始字节，而非 Unicode 字符。
    QByteArray passwordBytes = password.toUtf8();

    //调用 QCryptographicHash::hash 静态方法，对 passwordBytes 计算 SHA-256 哈希。
    //返回值是一个 QByteArray，包含 32 字节（256 位）的原始二进制哈希数据。
    QByteArray hashBytes = QCryptographicHash::hash(passwordBytes,QCryptographicHash::Sha256);

    //hashBytes.toHex()：将二进制哈希转换为十六进制字符串的 QByteArray（例如 "e3b0c442..."）。
    //QString::fromLatin1()：将该十六进制字节数组转换为 QString（假设每个字符都是 ASCII 字符，十六进制串恰好是 Latin1 编码）。
    return QString::fromLatin1(hashBytes.toHex());
}

#endif // UTILS_H
