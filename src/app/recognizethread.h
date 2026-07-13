#ifndef RECOGNIZETHREAD_H
#define RECOGNIZETHREAD_H

#include <QThread>
#include <opencv2/opencv.hpp>
#include "src/camera/framequeue.h"

/**
 * @brief 车牌识别消费者线程
 *
 * 从 FrameQueue 循环取帧，调用 PlateRecognize（EasyPR）进行识别，
 * 识别成功通过 plateRecognized 信号通知主线程。
 *
 * 线程模型：
 *   FrameQueue --take()--> RecognizeThread --signal--> MainWindow (UI线程)
 *
 * 生命周期：
 *   1. MainWindow 构造时创建并 start()
 *   2. 运行期间循环取帧识别
 *   3. MainWindow 析构时调用 stop() + wait()
 */
class RecognizeThread : public QThread
{
    Q_OBJECT

public:
    explicit RecognizeThread(QObject *parent = nullptr);
    ~RecognizeThread();

    /**
     * @brief 设置帧队列指针
     *
     * 必须在 start() 之前调用。FrameQueue 的生命周期由调用方管理。
     *
     * @param queue 与 CameraThread 共享的帧队列
     */
    void setFrameQueue(FrameQueue *queue);

    /**
     * @brief 设置取帧超时（也是采样间隔的下限）
     *
     * take() 的超时时间决定了消费者的最大识别频率。
     * 例如设为 1500ms，则每次 take() 最多等 1.5 秒。
     * 加上识别本身的耗时（~200-500ms），实际间隔约 1.7-2 秒。
     *
     * @param ms 超时毫秒数，默认 1500
     */
    void setSamplingInterval(int ms);

    /**
     * @brief 请求停止线程
     *
     * 设置 m_running = false，线程将在当前 take() 超时后退出循环。
     * 调用后应接 wait() 等待线程实际结束。
     */
    void stop();

signals:
    /**
     * @brief 车牌识别成功信号
     *
     * 在 RecognizeThread 线程中发射，通过 Qt 的 QueuedConnection
     * 自动投递到主线程的事件循环中执行对应槽函数。
     *
     * @param plate 识别出的车牌号（已解析为纯号码，如 "苏A12345"）
     * @param plateImg 车牌区域的裁剪图像
     */
    void plateRecognized(const QString &plate, const cv::Mat &plateImg);

    /**
     * @brief 识别出错信号
     * @param errorMsg 错误信息
     */
    void recognizeError(const QString &errorMsg);

protected:
    /**
     * @brief 线程主函数
     *
     * 核心循环：take() 取帧 → recognizePlate() 识别 → emit 结果
     */
    void run() override;

private:
    FrameQueue *m_queue = nullptr;
    int m_samplingInterval = 1500; //  take() 超时（ms）
    bool m_running = false;
};

#endif //RECOGNIZETHREAD_H