#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <atomic>
class FrameQueue;
class CameraThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraThread(int cameraIndex = 0, QObject *parent = nullptr);

    void setResolution(int width, int height) { m_cameraWidth = width; m_cameraHeight = height; }

    ~CameraThread();

    static int getCameraCount();

    bool isCapturing() const {return m_running;}

    void setTargetFps(int fps) {m_targetFps = fps;}

    int getCurrentFps() const {return m_currentFps;}

    void pause();

     void resume();

    void stop();

    void setFrameQueue(FrameQueue *queue) {m_frameQueue = queue;}

    cv::Mat getLatestFrame();
signals:
    void newFrameCaptured(cv::Mat frame);

    void cameraStatusChanged(bool connected, const QString &message);

    void fpsUpdated(int fps);
protected:
    void run() ;
private:
    bool initCamera();

    void releaseCamera();

    void calculateFps();

    static cv::Mat QImageToMat(const QImage &image);

    static QImage MatToQImage(const cv::Mat &mat);

    int m_cameraIndex;
    int m_cameraWidth  = 640;
    int m_cameraHeight = 480;
    int m_targetFps;
    int m_currentFps;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_paused{false};
    cv::VideoCapture m_capture;
    cv::Mat m_latestFrame;
    QMutex m_mutex;
    QWaitCondition m_pauseCond;

    int m_frameCount;
    qint64 m_lastFpsTime;
    FrameQueue *m_frameQueue = nullptr;
};
#endif