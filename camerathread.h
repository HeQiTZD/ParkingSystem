#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QQueue>
#include <opencv2/opencv.hpp>

/**
 * @brief 摄像头线程类
 *
 * 在独立线程中捕获摄像头视频帧，避免阻塞UI线程。
 * 使用OpenCV的VideoCapture进行图像捕获。
 *
 * 主要功能：
 * 1. 摄像头设备枚举
 * 2. 视频流捕获
 * 3. 帧率控制
 * 4. 图像格式转换（BGR -> RGB）
 * 5. 线程安全的帧队列
 *
 * 设计模式：
 * - 生产者-消费者模式
 * - 观察者模式（信号槽）
 */
class CameraThread : public QThread
{
public:
    /**
     * @brief 构造函数
     * @param cameraIndex 摄像头索引（默认为0，即第一个摄像头）
     * @param parent 父对象
     */
    explicit CameraThread(int cameraIndex = 0, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CameraThread();

    /**
     * @brief 获取可用摄像头数量
     * @return 摄像头数量
     */
    static int getCameraCount();

    /**
     * @brief 获取摄像头是否正在运行
     */
    bool isCapturing() const {return m_running;}

    /**
     * @brief 设置目标帧率
     * @param fps 帧率（默认30）
     */
    void setTargetFps(int fps) {m_targetFps = fps;}

    /**
     * @brief 获取当前帧率
     */
    int getCurrentFps() const {return m_currentFps;}

    /**
     * @brief 暂停捕获
     */
    void pause();

    /**
     * @brief 恢复捕获
     */
     void resume();

    /**
     * @brief 获取最新一帧（线程安全）
     * @return 最新的图像帧
     */
    cv::Mat getLatestFrame();

signals:
    /**
     * @brief 新帧可用信号
     * @param frame 捕获的图像帧（BGR格式）
     *
     * 当有新的视频帧被捕获时发射此信号。
     * 连接到此信号的槽函数将在接收线程中执行。
     */
    void newFrameCaptured(cv::Mat frame);

    /**
     * @brief 摄像头状态变化信号
     * @param connected 是否连接
     * @param message 状态信息
     */
    void cameraStatusChanged(bool connected, const QString &message);

    /**
     * @brief 帧率更新信号
     * @param fps 当前帧率
     */
    void fpsUpdated(int fps);

protected:
    /**
     * @brief 线程主函数
     *
     * 重写QThread的run()函数，在此实现摄像头捕获逻辑。
     */
    void run() ;

private:
    /**
     * @brief 初始化摄像头
     * @return 是否成功
     */
    bool initCamera();

    /**
     * @brief 释放摄像头资源
     */
    void releaseCamera();

    /**
     * @brief 计算帧率
     */
    void calculateFps();

    /**
     * @brief QImage转cv::Mat
     * @param image QImage图像
     * @return cv::Mat图像
     */
    static cv::Mat QImageToMat(const QImage &image);

    /**
     * @brief cv::Mat转QImage
     * @param mat cv::Mat图像
     * @return QImage图像
     */
    static QImage MatToQImage(const cv::Mat &mat);

    // 成员变量
    int m_cameraIndex; //摄像头索引
    int m_targetFps; //目标帧率
    int m_currentFps; //当前帧率
    bool m_running; //运行标志
    bool m_paused; //暂停标志

    cv::VideoCapture m_capture; //OpenCV摄像头捕获对象
    cv::Mat m_latestFrame; //最新帧（线程安全）
    QMutex m_mutex; //互斥锁
    QWaitCondition m_pauseCond; //暂停条件变量

    // 帧率计算相关
    int m_frameCount; //帧计数器
    qint64 m_lastFpsTime; //上次帧率计算时间
};

#endif // CAMERATHREAD_H
