#include "camerathread.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>
#include "framequeue.h"
#include "src/database/dbconnectionpool.h"
CameraThread::CameraThread(int cameraIndex, QObject *parent)
    : QThread(parent)
    , m_cameraIndex(cameraIndex)
    , m_targetFps(30)
    , m_currentFps(0)
    , m_frameCount(0)
    , m_lastFpsTime(0)
{}
CameraThread::~CameraThread()
{
    stop();
    releaseCamera();
}
void CameraThread::stop()
{
    m_running = false;
    m_paused = false;
    m_pauseCond.wakeAll();

    if(QThread::isRunning()){
        quit();
        if(!wait(3000)){
            qWarning() << "CameraThread::stop() timeout";
        }
    }
}
int CameraThread::getCameraCount()
{
    int count = 0;
    for(int i=0; i < 10; ++i){
        cv::VideoCapture cap(i);
        if(cap.isOpened()){
            count++;
            cap.release();
        }else{
            break;
        }
    }
    return count;
}
void CameraThread::pause()
{
    QMutexLocker locker(&m_mutex);
    m_paused = true;
    qDebug() << QStringLiteral("摄像头线程已暂停");
}
void CameraThread::resume()
{
    QMutexLocker locker(&m_mutex);
    m_paused = false;
    m_pauseCond.wakeAll();
    qDebug() << QStringLiteral("摄像头线程已恢复");
}
cv::Mat CameraThread::getLatestFrame()
{
    QMutexLocker locker(&m_mutex);
    return m_latestFrame.clone();
}
void CameraThread::run()
{
    qDebug() << QStringLiteral("摄像头线程启动，线程ID:") << QThread::currentThreadId();

    if(!initCamera()){
        return;
    }
    m_running = true;
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();

    int sampleCounter = 0;
    const int sampleEveryN = 15;

    int frameDelay = 1000 / m_targetFps;
    while(m_running){
        {
            QMutexLocker locker(&m_mutex);
            while(m_paused && m_running){
                m_pauseCond.wait(&m_mutex);
            }
        }
        if(!m_running){
            break;
        }

        cv::Mat frame;
        m_capture >> frame;
        if(frame.empty()){
            qDebug() << QStringLiteral("捕获空帧，跳过");
            QThread::msleep(10);
            continue;
        }

        {
            QMutexLocker locker(&m_mutex);
            m_latestFrame = frame.clone();
        }

        emit newFrameCaptured(frame.clone());

        if(m_frameQueue && ++sampleCounter >= sampleEveryN){
            m_frameQueue->push(frame.clone());
            sampleCounter = 0;
        }

        calculateFps();
        QThread::msleep(frameDelay);
    }

    releaseCamera();
    DbConnectionPool::instance().closeThreadConnection();
    qDebug() << QStringLiteral("摄像头线程已结束");
}
bool CameraThread::initCamera()
{
    m_capture.open(m_cameraIndex);
    if(!m_capture.isOpened()){
        emit cameraStatusChanged(false, QStringLiteral("无法打开的摄像头 %1").arg(m_cameraIndex));
        return false;
    }

    m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_cameraWidth);
    m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_cameraHeight);
    m_capture.set(cv::CAP_PROP_FPS, m_targetFps);
    emit cameraStatusChanged(true, QStringLiteral("摄像头 %1 已连接").arg(m_cameraIndex));
    return true;
}
void CameraThread::releaseCamera()
{
    if(m_capture.isOpened()){
        m_capture.release();
        qDebug() << QStringLiteral("摄像头资源已释放");
    }
}
void CameraThread::calculateFps()
{
    m_frameCount++;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    if(currentTime - m_lastFpsTime >= 1000){
        m_currentFps = m_frameCount;
        m_frameCount = 0;
        m_lastFpsTime = currentTime;
        emit fpsUpdated(m_currentFps);
    }
}
cv::Mat CameraThread::QImageToMat(const QImage &image)
{
    cv::Mat mat;
    switch (image.format()) {
        case QImage::Format_RGB888:
            {
            cv::Mat tmp(image.height(), image.width(), CV_8UC3,
                        const_cast<uchar*>(image.bits()), image.bytesPerLine());
            cv::cvtColor(tmp, mat, cv::COLOR_RGB2BGR);
            break;
            }
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
            {
            cv::Mat tmp(image.height(), image.width(), CV_8UC4,
                        const_cast<uchar*>(image.bits()), image.bytesPerLine());
            cv::cvtColor(tmp, mat, cv::COLOR_BGRA2BGR);
            break;
            }
        default:
                qDebug() << QStringLiteral("不支持的QImage格式");
            break;
    }
    return mat;
}
QImage CameraThread::MatToQImage(const cv::Mat &mat)
{
    if(mat.empty()){
        return QImage();
    }
    switch (mat.type()) {
        case CV_8UC1:
            {
                return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
            }
        case CV_8UC3:
            {
                cv::Mat rgbMat;
                cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
                return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, static_cast<int>(rgbMat.step), QImage::Format_RGB888);
            }
        case CV_8UC4:
        {
            cv::Mat rgbaMat;
            cv::cvtColor(mat, rgbaMat, cv::COLOR_BGRA2RGBA);
            return QImage(rgbaMat.data, rgbaMat.cols, rgbaMat.rows, static_cast<int>(rgbaMat.step), QImage::Format_RGBA8888);
        }
        default:
        qDebug() << QStringLiteral("不支持的cv::Mat格式:") << mat.type();
        return QImage();
    }
}