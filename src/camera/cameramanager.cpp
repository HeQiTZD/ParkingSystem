#include "cameramanager.h"
#include "camerathread.h"
#include "src/utils/initfile.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

CameraManager& CameraManager::instance()
{
    static CameraManager instance;
    return instance;
}

CameraManager::CameraManager(QObject *parent) : QObject(parent) {}

CameraManager::~CameraManager()
{
    stopAll();
    qDeleteAll(m_threads);
    m_threads.clear();
}

void CameraManager::scanCameras()
{
    if(!m_threads.isEmpty()){
        return;
    }

    int available = CameraThread::getCameraCount();
    int n = qMin(available, kMaxCameras);

    int w = InitFile::instance().getCameraWidth();
    int h = InitFile::instance().getCameraHeight();
    int fps = InitFile::instance().getCameraFps();

    QJsonArray camerasCfg = InitFile::instance().getCameras();

    for(int i = 0; i < n; ++i){
        CameraThread* t = new CameraThread(i, this);
        t->setResolution(w, h);
        t->setTargetFps(fps);

        CameraInfo info;
        info.index = i;
        info.width = w;
        info.height = h;
        info.fps = fps;

        info.name = QStringLiteral("摄像头 %1").arg(i + 1);
        info.location = QStringLiteral("位置 %1").arg(i + 1);
        info.role = (i == 0) ? "entry" : "monitor";

        for(const QJsonValue& v : camerasCfg){
            QJsonObject o = v.toObject();
            if(o["index"].toInt() == i){
                if(!o["name"].toString().isEmpty())
                    info.name = o["name"].toString();
                if(!o["location"].toString().isEmpty())
                    info.location = o["location"].toString();
                if(!o["role"].toString().isEmpty())
                    info.role = o["role"].toString();
                break;
            }
        }

        m_threads.append(t);
        m_infos.append(info);
    }

    qDebug() << QStringLiteral("CameraManager: 扫描到 %1 个摄像头").arg(m_threads.size());
}

int CameraManager::count() const { return m_threads.size(); }

CameraThread* CameraManager::getThread(int index)
{
    if(index < 0 || index >= m_threads.size()) return nullptr;
    return m_threads[index];
}

CameraInfo CameraManager::info(int index) const
{
    if(index < 0 || index >= m_infos.size()) return CameraInfo();
    return m_infos[index];
}

void CameraManager::start(int index)
{
    if(CameraThread* t = getThread(index)){
        if(!t->isRunning()){
            t->start();
        }
    }
}

void CameraManager::stop(int index)
{
    if(CameraThread* t = getThread(index)){
        t->stop();
    }
}

void CameraManager::startAll()
{
    for(int i = 0; i < m_threads.size(); ++i) start(i);
}

void CameraManager::stopAll()
{
    for(int i = 0; i < m_threads.size(); ++i) stop(i);
}
