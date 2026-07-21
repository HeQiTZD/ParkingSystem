#include "initfile.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
InitFile &InitFile::instance()
{
    static InitFile instance;
    return instance;
}
InitFile::InitFile(QObject *parent) : QObject(parent) {
    configFilePath = getConfigFilePath();
    qDebug() << QStringLiteral("配置文件路径：") << configFilePath;
}
InitFile::~InitFile()
{

}
void InitFile::shutdown()
{
    saveConfig();
    qDebug() << "InitFile: shutdown 写盘完成";
}
bool InitFile::loadConfig()
{
    if(!ensureConfigDirExists()){
        return false;
    }

    if(!isConfigExists()){
        qDebug() << QStringLiteral("配置文件不存在，创建默认配置");
        if(!createDefaultConfig()){
            return false;
        }
    }

    QFile file(configFilePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << QStringLiteral("无法打开配置文件：") <<file.errorString();
        emit configError("无法打开配置文件:"+file.errorString());
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data,&parseError);
    if(parseError.error != QJsonParseError::NoError){
        qDebug() << QStringLiteral("配置文件JSON解析错误：") << parseError.errorString();
        emit configError(QStringLiteral("配置文件格式错误：")+parseError.errorString());
        return false;
    }
    configData = doc.object();

    if(!validateConfig(configData)){
        qDebug() << QStringLiteral("配置文件验证失败，使用默认配置");
        configData = getDefaultConfig();
        saveConfig();
    }
    qDebug() << QStringLiteral("配置文件加载成功");
    emit configLoaded();
    return true;
}
bool InitFile::saveConfig()
{
    if(!ensureConfigDirExists()){
        return false;
    }

    QJsonDocument doc(configData);

    QFile file(configFilePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug() << QStringLiteral("无法创建配置文件：") << file.errorString();
        emit configError(QStringLiteral("无法创建配置文件：")+file.errorString());
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qDebug() << QStringLiteral("配置文件保存成功");
    emit configSaved();
    return true;
}
bool InitFile::createDefaultConfig()
{
    configData = getDefaultConfig();
    return saveConfig();
}
bool InitFile::isConfigExists()
{
    return QFile::exists(configFilePath);
}
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

    emit parkingConfigChanged(name, price, capacity, freeMinutes);
    qDebug() << QStringLiteral("停车场配置已更新");
}
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
QJsonArray InitFile::getCameras() const
{
    return configData["cameras"].toArray();
}
void InitFile::setCameras(const QJsonArray &cameras)
{
    configData["cameras"] = cameras;
    qDebug() << QStringLiteral("摄像头列表配置已更新");
}
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
QString InitFile::getConfigFilePath() const
{
    QString appPath = QCoreApplication::applicationDirPath();
    return appPath + "/config.json";
}
bool InitFile::ensureConfigDirExists()
{
    QFileInfo fileInfo(configFilePath);
    QDir dir = fileInfo.absoluteDir();
    if(!dir.exists()){
        if(!dir.mkpath(".")){
            qDebug() << QStringLiteral("创建配置目录失败:") << dir.absolutePath();
            emit configError(QStringLiteral("无法创建配置目录"));
            return false;
        }
    }
    return true;
}
QJsonObject InitFile::getDefaultConfig() const
{
    QJsonObject defaultConfig;
    QJsonObject dbConfig;
    dbConfig["host"] = "localhost";
    dbConfig["port"] = 3306;
    dbConfig["name"] = "parking_system";
    dbConfig["username"] = "root";
    dbConfig["password"] = "";
    defaultConfig["database"] = dbConfig;

    QJsonObject cameraConfig;
    cameraConfig["index"] = 0;
    cameraConfig["width"] = 1920;
    cameraConfig["height"] = 1080;
    cameraConfig["fps"] = 30;
    defaultConfig["camera"] = cameraConfig;
    QJsonArray camerasArray;
    QJsonObject cam0;
    cam0["index"] = 0;
    cam0["name"] = QStringLiteral("主入口");
    cam0["location"] = QStringLiteral("大门");
    cam0["role"] = "entry";
    camerasArray.append(cam0);
    defaultConfig["cameras"] = camerasArray;

    QJsonObject recognitionConfig;
    recognitionConfig["modelpath"] = "./model/";
    recognitionConfig["confidenceThreshold"] = 0.7;
    defaultConfig["recognition"] = recognitionConfig;

    QJsonObject parkingConfig;
    parkingConfig["name"] = "";
    parkingConfig["price"] = 0.0;
    parkingConfig["capacity"] = 0;
    parkingConfig["freeMinutes"] = 15;
    defaultConfig["parking"] = parkingConfig;

    QJsonObject systemConfig;
    systemConfig["appName"] = "停车场管理系统";
    systemConfig["version"] = "1.0.0";
    systemConfig["language"] = "zh_CN";
    defaultConfig["system"] = systemConfig;
    return defaultConfig;
}
bool InitFile::validateConfig(const QJsonObject &config) const
{
    QStringList requiredSections = {"database", "camera", "recognition", "parking", "system"};
    for(const QString &section : requiredSections){
        if(!config.contains(section)){
            qDebug() << QStringLiteral("配置缺少必要的配置节:") <<section;
            return false;
        }
    }

    QJsonObject dbConfig = config["database"].toObject();
    if(!dbConfig.contains("host") || !dbConfig.contains("port") ||
       !dbConfig.contains("name") || !dbConfig.contains("username") ||
       !dbConfig.contains("password")){
        qDebug() << QStringLiteral("数据库配置不完整");
        return false;
    }

    QJsonObject cameraConfig = config["camera"].toObject();
    if(!cameraConfig.contains("index") || !cameraConfig.contains("width")||
        !cameraConfig.contains("height") || !cameraConfig.contains("fps")){
        qDebug() << QStringLiteral("摄像头配置不完整");
        return false;
    }

    QJsonObject parkingConfig = config["parking"].toObject();
    if(!parkingConfig.contains("name") || !parkingConfig.contains("price") ||
       !parkingConfig.contains("capacity") || !parkingConfig.contains("freeMinutes")){
        qDebug() << QStringLiteral("停车场配置不完整");
        return false;
    }
    return true;
}