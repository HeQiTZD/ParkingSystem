#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H
#include <QObject>
#include <QList>
#include "camerainfo.h"
class CameraThread;
class CameraManager : public QObject
{
    Q_OBJECT
public:
    static CameraManager& instance();
    void scanCameras();
    int  count() const;
    CameraThread* getThread(int index);
    CameraInfo    info(int index) const;
    void updateInfo(int index, const CameraInfo &info);
    void start(int index);
    void stop(int index);
    void startAll();
    void stopAll();

    void shutdown();
private:
    CameraManager(QObject *parent = nullptr);
    ~CameraManager();
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    QList<CameraThread*> m_threads;
    QList<CameraInfo>    m_infos;
    static const int kMaxCameras = 8;
};
#endif