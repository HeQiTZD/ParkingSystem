#include "camerathread.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>
#include "framequeue.h"

CameraThread::CameraThread(int cameraIndex, QObject *parent)
    : QThread(parent)
    , m_cameraIndex(cameraIndex)
    , m_targetFps(30)
    , m_currentFps(0)
    , m_running(false)
    , m_paused(false)
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
    // 停止线程
    m_running = false;//通知工作线程的主循环（run() 函数中的 while(m_running)）退出循环
    m_paused = false;//避免线程处于”暂停”状态无法响应退出信号；强制取消暂停
    m_pauseCond.wakeAll(); // 唤醒暂停的线程，唤醒所有可能在条件变量 m_pauseCond 上等待的线程（例如因暂停而等待）。确保线程从等待中苏醒，立即检查 m_running 标志并退出

    // 等待线程结束
    if(QThread::isRunning()){//检查线程是否仍在运行
        quit();//让线程的事件循环退出（如果线程内有 exec() 或 QEventLoop）。对于自定义的非事件循环线程，通常不需要 quit()，但保留它也不会造成问题
        wait(3000);// 最多等待3秒
    }
}

//等待改进：摄像头索引不连续，性能开销，最大索引硬编码为 10，跨平台行为差异
int CameraThread::getCameraCount()
{
    // 使用OpenCV枚举摄像头
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
    m_pauseCond.wakeAll(); // 唤醒暂停的线程
    qDebug() << QStringLiteral("摄像头线程已恢复");
}

cv::Mat CameraThread::getLatestFrame()
{
    QMutexLocker locker(&m_mutex);
    return m_latestFrame.clone();// 返回副本，保证线程安全
}

void CameraThread::run()
{
    qDebug() << QStringLiteral("摄像头线程启动，线程ID:") << QThread::currentThreadId();

    // 初始化摄像头
    if(!initCamera()){
        return;
    }

    m_running = true;
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();

    // ===== 新增：采样计数器 =====
    // 每 sampleEveryN 帧向 FrameQueue 投递一帧供识别线程消费。
    // 30fps 下每 15 帧 ≈ 0.5 秒投递一次。
    // 投递频率高于识别频率（~1.5秒/帧），中间帧被覆盖，消费者始终拿最新帧。
    int sampleCounter = 0;
    const int sampleEveryN = 15;

    // 计算每帧的延迟时间（毫秒）
    int frameDelay = 1000 / m_targetFps;

    while(m_running){
        //是否暂停
        {
            QMutexLocker locker(&m_mutex);
            while(m_paused && m_running){
                m_pauseCond.wait(&m_mutex);// 等待恢复信号
            }
        }

        if(!m_running){
            break;
        }

        // 捕获一帧
        cv::Mat frame;
        m_capture >> frame;

        if(frame.empty()){
            qDebug() << QStringLiteral("捕获空帧，跳过");
            QThread::msleep(10);
            continue;
        }

        // 更新最新帧
        {
            QMutexLocker locker(&m_mutex);
            m_latestFrame = frame.clone();//调用 clone() 强制创建独立深拷贝，保证存储的帧数据不会因外部修改而意外变化
        }

        // 发射新帧信号
        emit newFrameCaptured(frame.clone());

        // ===== 新增：定期向识别队列投递 =====
        if(m_frameQueue && ++sampleCounter >= sampleEveryN){
            m_frameQueue->push(frame.clone());
            sampleCounter = 0;
        }
        
        // 计算帧率
        calculateFps();
        // 帧率控制：休眠以达到目标帧率
        QThread::msleep(frameDelay);
    }

    //清理资源
    releaseCamera();
    qDebug() << QStringLiteral("摄像头线程已结束");
}

bool CameraThread::initCamera()
{
    // 打开摄像头
    m_capture.open(m_cameraIndex);

    if(!m_capture.isOpened()){
        emit cameraStatusChanged(false, QStringLiteral("无法打开的摄像头 %1").arg(m_cameraIndex));
        return false;
    }

    // 设置摄像头参数（优先使用 setResolution 配置的值）
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

    // 每秒计算一次帧率
    if(currentTime - m_lastFpsTime >= 1000){
        m_currentFps = m_frameCount;
        m_frameCount = 0;
        m_lastFpsTime = currentTime;
        emit fpsUpdated(m_currentFps);
    }
}

cv::Mat CameraThread::QImageToMat(const QImage &image)
{
    // QImage转cv::Mat
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
    // cv::Mat转QImage
    if(mat.empty()){
        return QImage();
    }

    switch (mat.type()) {
        case CV_8UC1:
            {
                // 灰度图
                return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
            }
        case CV_8UC3:
            {
                // BGR -> RGB
                cv::Mat rgbMat;
                cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
                return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, static_cast<int>(rgbMat.step), QImage::Format_RGB888);
            }
        case CV_8UC4:
        {
            // BRG -> RGB
            cv::Mat rgbaMat;
            cv::cvtColor(mat, rgbaMat, cv::COLOR_BGRA2RGBA);
            return QImage(rgbaMat.data, rgbaMat.cols, rgbaMat.rows, static_cast<int>(rgbaMat.step), QImage::Format_RGBA8888);
        }
        default:
        qDebug() << QStringLiteral("不支持的cv::Mat格式:") << mat.type();
        return QImage();
    }
}








