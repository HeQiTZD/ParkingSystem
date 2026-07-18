#include "initfile.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

InitFile &InitFile::instance()
{
    // Meyers 单例：首次调用时构造，进程退出时自动析构，线程安全（C++11 保证）
    static InitFile instance;
    return instance;
}

InitFile::InitFile(QObject *parent) : QObject(parent) {
    //设置文件路径
    configFilePath = getConfigFilePath();
    qDebug() << QStringLiteral("配置文件路径：") << configFilePath;
}

InitFile::~InitFile()
{
    // 保存由 setter 调用方显式触发，析构不自动写盘。
    // 避免只读场景下局部/临时实例析构时把内存副本无意义地回写磁盘。
}

//加载配置文件
bool InitFile::loadConfig()
{
    //确保目录存在
    if(!ensureConfigDirExists()){
        return false;
    }

    // 检查配置文件是否存在
    if(!isConfigExists()){
        qDebug() << QStringLiteral("配置文件不存在，创建默认配置");
        if(!createDefaultConfig()){
            return false;
        }
    }

    // 读取配置文件
    QFile file(configFilePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << QStringLiteral("无法打开配置文件：") <<file.errorString();
        emit configError("无法打开配置文件:"+file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    //解析Json
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data,&parseError);
    if(parseError.error != QJsonParseError::NoError){
        qDebug() << QStringLiteral("配置文件JSON解析错误：") << parseError.errorString();
        emit configError(QStringLiteral("配置文件格式错误：")+parseError.errorString());
        return false;
    }

    configData = doc.object();

    //验证配置
    if(!validateConfig(configData)){
        qDebug() << QStringLiteral("配置文件验证失败，使用默认配置");
        configData = getDefaultConfig();
        saveConfig();
    }

    qDebug() << QStringLiteral("配置文件加载成功");
    emit configLoaded();
    return true;
}

//保存配置文件
bool InitFile::saveConfig()
{
    //确保目录存在
    if(!ensureConfigDirExists()){
        return false;
    }

    //创建JSON文件
    QJsonDocument doc(configData);

    //打开文件进行写入
    QFile file(configFilePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << QStringLiteral("无法创建配置文件：") << file.errorString();
        emit configError(QStringLiteral("无法创建配置文件：")+file.errorString());
        return false;
    }

    //写入JSON数据
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << QStringLiteral("配置文件保存成功");
    emit configSaved();
    return true;
}

//创建默认配置文件
bool InitFile::createDefaultConfig()
{
    configData = getDefaultConfig();
    return saveConfig();
}

//检查配置文件是否存在
bool InitFile::isConfigExists()
{
    return QFile::exists(configFilePath);
}

//获取数据库配置
QString InitFile::getDbHost() const
{
    return configData["database"].toObject()["host"].toString();
}

int InitFile::getDbPort() const
{
    return configData["database"].toObject()["port"].toInt();
}

QString InitFile::getDbName() const
{
    return configData["database"].toObject()["name"].toString();
}

QString InitFile::getDbUsername() const
{
    return configData["database"].toObject()["username"].toString();
}

QString InitFile::getDbPassword() const
{
    return configData["database"].toObject()["password"].toString();
}

void InitFile::setDbConfig(const QString &host, const int port, const QString &name, const QString &username, const QString &password)
{
    QJsonObject dbConfig;
    dbConfig["host"] = host;
    dbConfig["port"] = port;
    dbConfig["name"] = name;
    dbConfig["username"] = username;
    dbConfig["password"] = password;

    configData["database"] = dbConfig;
    qDebug() << QStringLiteral("数据库配置已更新");
}

//获取停车场配置
QString InitFile::getParkingName() const
{
    return configData["parking"].toObject()["name"].toString();
}

double InitFile::getParkingPrice() const
{
    return configData["parking"].toObject()["price"].toDouble();
}

int InitFile::getParkingCapacity() const
{
    return configData["parking"].toObject()["capacity"].toInt();
}

int InitFile::getFreeMinutes() const
{
    // 兜底默认15分钟，兼容旧版config.json缺少该字段的情况
    return configData["parking"].toObject()["freeMinutes"].toInt(15);
}

void InitFile::setParkingConfig(const QString &name, double price, int capacity, int freeMinutes)
{
    QJsonObject parkingConfig;
    parkingConfig["name"] = name;
    parkingConfig["price"] = price;
    parkingConfig["capacity"] = capacity;
    parkingConfig["freeMinutes"] = freeMinutes;

    configData["parking"] = parkingConfig;

    // 发射信号，通知停车场配置变更
    emit parkingConfigChanged(name, price, capacity, freeMinutes);

    qDebug() << QStringLiteral("停车场配置已更新");
}

//获取摄像头配置
int InitFile::getCameraIndex() const
{
    return configData["camera"].toObject()["index"].toInt();
}

int InitFile::getCameraWidth() const
{
    return configData["camera"].toObject()["width"].toInt();
}

int InitFile::getCameraHeight() const
{
    return configData["camera"].toObject()["height"].toInt();
}

int InitFile::getCameraFps() const
{
    return configData["camera"].toObject()["fps"].toInt();
}

void InitFile::setCameraConfig(int index, int width, int height, int fps)
{
    QJsonObject cameraConfig;
    cameraConfig["index"] = index;
    cameraConfig["width"] = width;
    cameraConfig["height"] = height;
    cameraConfig["fps"] = fps;

    configData["camera"] = cameraConfig;
     qDebug() << QStringLiteral("摄像头配置已更新");
}

//获取识别配置
QString InitFile::getModelPath() const
{
    return configData["recognition"].toObject()["modelpath"].toString();
}

double InitFile::getConfidenceThreshold() const
{
    return configData["recognition"].toObject()["confidenceThreshold"].toDouble();
}

void InitFile::setRecognitionConfig(const QString &modelPath, double threshold)
{
    QJsonObject recogonitionConfig;
    recogonitionConfig["modelpath"] = modelPath;
    recogonitionConfig["confidenceThreshold"] = threshold;

    configData["recognition"] = recogonitionConfig;
     qDebug() << QStringLiteral("识别配置已更新");
}

//获取系统配置
QString InitFile::getAppName() const
{
    return configData["system"].toObject()["appName"].toString();
}

QString InitFile::getVersion() const
{
    return configData["system"].toObject()["version"].toString();
}

void InitFile::setSystemConfig(const QString &appname, const QString &version)
{
    QJsonObject systemConfig;
    systemConfig["appName"] = appname;
    systemConfig["version"] = version;

    configData["system"] = systemConfig;
     qDebug() << QStringLiteral("系统配置已更新");
}

// 获取配置文件路径
QString InitFile::getConfigFilePath() const
{
    //使用应用程序目录下的config.json文件
    QString appPath = QCoreApplication::applicationDirPath();// 获取当前应用程序可执行文件所在的目录的绝对路径
    return appPath + "/config.json";
}

// 确保配置目录存在
bool InitFile::ensureConfigDirExists()
{
    QFileInfo fileInfo(configFilePath);
    QDir dir = fileInfo.absoluteDir();//返回配置文件所在目录的 QDir 对象（即不包含文件名部分）

    if(!dir.exists()){//检查目录是否存在
        if(!dir.mkpath(".")){//创建当前 dir 对象所代表的目录（参数 "." 表示当前目录，即 dir 本身）。
            qDebug() << QStringLiteral("创建配置目录失败:") << dir.absolutePath();
            emit configError(QStringLiteral("无法创建配置目录"));
            return false;
        }
    }
    return true;
}

//获取默认配置
QJsonObject InitFile::getDefaultConfig() const
{
    QJsonObject defaultConfig;
    //数据库默认配置
    QJsonObject dbConfig;
    dbConfig["host"] = "localhost";
    dbConfig["port"] = 3306;
    dbConfig["name"] = "parking_system";
    dbConfig["username"] = "root";
    dbConfig["password"] = "";
    defaultConfig["database"] = dbConfig;

    //摄像头默认配置
    QJsonObject cameraConfig;
    cameraConfig["index"] = 0;
    cameraConfig["width"] = 1920;
    cameraConfig["height"] = 1080;
    cameraConfig["fps"] = 30;
    defaultConfig["camera"] = cameraConfig;

    //识别默认配置
    QJsonObject recognitionConfig;
    recognitionConfig["modelpath"] = "./model/";
    recognitionConfig["confidenceThreshold"] = 0.7;
    defaultConfig["recognition"] = recognitionConfig;

    //停车场默认配置
    QJsonObject parkingConfig;
    parkingConfig["name"] = "";
    parkingConfig["price"] = 0.0;
    parkingConfig["capacity"] = 0;
    parkingConfig["freeMinutes"] = 15;
    defaultConfig["parking"] = parkingConfig;

    //系统默认配置
    QJsonObject systemConfig;
    systemConfig["appName"] = "停车场管理系统";
    systemConfig["version"] = "1.0.0";
    systemConfig["language"] = "zh_CN";
    defaultConfig["system"] = systemConfig;

    return defaultConfig;
}

//验证配置有效性
bool InitFile::validateConfig(const QJsonObject &config) const
{
    //检查必要的配置节是否存在
    QStringList requiredSections = {"database", "camera", "recognition", "parking", "system"};
    for(const QString &section : requiredSections){
        if(!config.contains(section)){
            qDebug() << QStringLiteral("配置缺少必要的配置节:") <<section;
            return false;
        }
    }

    //检查数据库配置
    QJsonObject dbConfig = config["database"].toObject();
    if(!dbConfig.contains("host") || !dbConfig.contains("port") ||
       !dbConfig.contains("name") || !dbConfig.contains("username") ||
       !dbConfig.contains("password")){
        qDebug() << QStringLiteral("数据库配置不完整");
        return false;
    }

    //检查摄像头配置
    QJsonObject cameraConfig = config["camera"].toObject();
    if(!cameraConfig.contains("index") || !cameraConfig.contains("width")||
        !cameraConfig.contains("height") || !cameraConfig.contains("fps")){
        qDebug() << QStringLiteral("摄像头配置不完整");
        return false;
    }

    //检查停车场配置
    QJsonObject parkingConfig = config["parking"].toObject();
    if(!parkingConfig.contains("name") || !parkingConfig.contains("price") ||
       !parkingConfig.contains("capacity") || !parkingConfig.contains("freeMinutes")){
        qDebug() << QStringLiteral("停车场配置不完整");
        return false;
    }
    return true;
}

