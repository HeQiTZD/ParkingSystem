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
    static InitFile &instance();
    ~InitFile();

    bool loadConfig();
    bool saveConfig();
    void shutdown();
    bool createDefaultConfig();
    bool isConfigExists();

    QString getDbHost() const;
    int getDbPort() const;
    QString getDbName() const;
    QString getDbUsername() const;
    QString getDbPassword() const;
    void setDbConfig(const QString &host, const int port, const QString &name,
                     const QString &username, const QString &password);

    int getCameraIndex() const;
    int getCameraWidth() const;
    int getCameraHeight() const;
    int getCameraFps() const;
    void setCameraConfig(int index, int width, int height, int fps);

    QJsonArray getCameras() const;
    void setCameras(const QJsonArray &cameras);

    QString getParkingName() const;
    double getParkingPrice() const;
    int getParkingCapacity() const;
    int getFreeMinutes() const;
    void setParkingConfig(const QString &name, double price, int capacity, int freeMinutes = 15);

    QString getModelPath() const;
    double getConfidenceThreshold() const;
    void setRecognitionConfig(const QString &modelPath, double threshold);

    QString getAppName() const;
    QString getVersion() const;
    void setSystemConfig(const QString &appname, const QString &version);
signals:
    void configLoaded();
    void configSaved();
    void configError(const QString &errorMsg);
    void parkingConfigChanged(const QString &name, double price, int capacity, int freeMinutes);
private:
    QJsonObject configData;
    QString configFilePath;

    QString getConfigFilePath() const;
    bool ensureConfigDirExists();
    QJsonObject getDefaultConfig() const;
    bool validateConfig(const QJsonObject &config) const;
private:
    explicit InitFile(QObject *parent = nullptr);
    InitFile(const InitFile &) = delete;
    InitFile &operator=(const InitFile &) = delete;
    InitFile(InitFile &&) = delete;
    InitFile &operator=(InitFile &&) = delete;
};
#endif