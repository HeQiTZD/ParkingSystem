#ifndef INITFILE_H
#define INITFILE_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class InitFile : public QObject
{
    Q_OBJECT
public:
    // 获取单例实例（首次调用时构造，进程退出时自动析构）
    static InitFile &instance();

    ~InitFile();

    //配置文件操作
    bool loadConfig();// 加载配置文件
    bool saveConfig();// 保存配置文件
    void shutdown();  // 幂等写盘(shutdown 序列调用)
    bool createDefaultConfig();// 创建默认配置文件
    bool isConfigExists();//检查配置文件是否存在

    //数据库配置
    QString getDbHost() const;
    int getDbPort() const;
    QString getDbName() const;
    QString getDbUsername() const;
    QString getDbPassword() const;
    void setDbConfig(const QString &host, const int port, const QString &name,
                     const QString &username, const QString &password);

    //摄像头设置
    int getCameraIndex() const;//获取当前使用的摄像头设备索引号
    int getCameraWidth() const;//获取摄像头采集的画面宽度（像素）
    int getCameraHeight() const;//获取摄像头采集的画面高度（像素）
    int getCameraFps() const;//获取摄像头的帧率（Frames Per Second，每秒帧数）
    void setCameraConfig(int index, int width, int height, int fps);

    //摄像头列表(多摄像头管理)
    QJsonArray getCameras() const;
    void setCameras(const QJsonArray &cameras);

    //停车场配置
    QString getParkingName() const;//获取停车场名称
    double getParkingPrice() const;//获取停车价格（每小时）
    int getParkingCapacity() const;//获取停车场容量
    int getFreeMinutes() const;//获取免费停车时长(分钟)
    void setParkingConfig(const QString &name, double price, int capacity, int freeMinutes = 15);

    //识别配置
    QString getModelPath() const;//获取当前使用的识别模型文件路径。
    double getConfidenceThreshold() const;//获取识别结果的置信度阈值（confidence threshold）
    void setRecognitionConfig(const QString &modelPath, double threshold);

    //系统配置
    QString getAppName() const;//获取当前应用程序的名称
    QString getVersion() const;//获取当前应用程序的版本号
    void setSystemConfig(const QString &appname, const QString &version);

signals:
    void configLoaded();//配置加载完成信号
    void configSaved();//配置保持完成信号
    void configError(const QString &errorMsg);//配置错误信号
    void parkingConfigChanged(const QString &name, double price, int capacity, int freeMinutes);// 配置改变信号

private:
    QJsonObject configData;//存储配置数据
    QString configFilePath;//配置文件路径

    //内部辅助函数
    QString getConfigFilePath() const;//获取配置文件路径
    bool ensureConfigDirExists();//
    QJsonObject getDefaultConfig() const;//
    bool validateConfig(const QJsonObject &config) const;//

private:
    // 单例：构造/拷贝/移动均私有，禁止外部实例化
    explicit InitFile(QObject *parent = nullptr);
    InitFile(const InitFile &) = delete;
    InitFile &operator=(const InitFile &) = delete;
    InitFile(InitFile &&) = delete;
    InitFile &operator=(InitFile &&) = delete;
};

#endif // INITFILE_H


/*
代码设计原理和作用解释
1. 类设计原理
单例模式：配置文件管理类已采用单例模式（InitFile::instance()），确保整个应用程序中只有一个配置实例，避免多实例内存副本冲突与无意义的重复磁盘读写。
封装性：将配置相关的所有操作封装在一个类中，提供统一的接口，便于维护和扩展。
信号槽机制：使用Qt的信号槽机制来通知配置状态变化，实现松耦合的设计。

2. JSON配置文件格式
配置文件使用JSON格式，具有以下优点：
可读性强：JSON格式易于人类阅读和编辑
结构清晰：支持嵌套结构，便于组织配置项
跨平台：JSON是通用的数据交换格式
Qt原生支持：Qt提供了完善的JSON处理类

3. 配置验证机制
在加载配置时进行验证，确保：
必要的配置节存在
配置项的数据类型正确
配置值在合理范围内

4. 错误处理
通过信号槽机制处理错误：
文件不存在时自动创建默认配置
JSON解析错误时提供错误信息
配置验证失败时使用默认配置

5. 内存管理
使用Qt的父子对象机制进行内存管理，避免内存泄漏。
*/