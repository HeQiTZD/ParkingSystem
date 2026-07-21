#include "userservice.h"
#include "src/database/databasemanager.h"
UserService::UserService(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {}
bool UserService::authenticate(const QString& username, const QString& password,
                               QString& userRole, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(username.isEmpty() || password.isEmpty()){
        errMsg = "用户名或密码不能为空";
        return false;
    }
    if(!m_db.validateUser(username, password, userRole)){
        errMsg = "用户名或密码错误";
        return false;
    }
    return true;
}
bool UserService::registerUser(const QString& username, const QString& password,
                               const QString& name, const QString& phone,
                               QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(username.isEmpty() || password.isEmpty()){
        errMsg = "用户名和密码不能为空";
        return false;
    }
    if(m_db.isUsernameExists(username)){
        errMsg = "用户名已存在";
        return false;
    }
    if(!m_db.registerUser(username, password, name, phone)){
        errMsg = "注册写入失败";
        return false;
    }
    emit userDataChanged();
    return true;
}
bool UserService::addUser(const QString& username, const QString& password,
                          const QString& telephone, const QString& truename,
                          const QString& role, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(username.isEmpty() || password.isEmpty()){
        errMsg = "用户名和密码不能为空";
        return false;
    }
    if(m_db.isUsernameExists(username)){
        errMsg = "用户名已存在";
        return false;
    }
    if(!m_db.addUser(username, password, telephone, truename, role)){
        errMsg = "添加用户写入失败";
        return false;
    }
    emit userDataChanged();
    return true;
}
bool UserService::updateUser(int id, const QString& username, const QString& telephone,
                              const QString& truename, const QString& role, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(!m_db.updateUser(id, username, telephone, truename, role)){
        errMsg = "修改用户失败";
        return false;
    }
    emit userDataChanged();
    return true;
}
bool UserService::deleteUser(int id, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(!m_db.deleteUser(id)){
        errMsg = "删除用户失败";
        return false;
    }
    emit userDataChanged();
    return true;
}
QList<QVariantList> UserService::listUsers(const QString& keyword)
{
    return m_db.searchUsers(keyword);
}