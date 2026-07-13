#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H

#include <QMutex>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>

/**
 * @brief 线程安全的单帧队列（容量 = 1）
 *
 * 用于 CameraThread（生产者）和 RecognizeThread（消费者）之间的帧传递。
 *
 * 核心设计：
 * - 容量固定为 1，新帧覆盖旧帧 → 消费者始终拿到最新画面
 * - push() 永不阻塞 → 摄像头帧率不受识别速度影响
 * - take() 带超时等待 → 消费者空闲时挂起，不浪费 CPU
 *
 * 线程安全保障：
 * - QMutex 互斥锁保护共享数据（m_frame / m_hasFrame）
 * - QWaitCondition 实现生产者唤醒消费者的通知机制
 * - cv::Mat 使用 clone() 深拷贝，避免跨线程共享内存
 */

 class FrameQueue
 {
public:
    FrameQueue() = default;

    /**
     * @brief 生产者写入一帧
     *
     * 覆盖当前存储的帧（无论消费者是否已取走旧帧），
     * 然后通过条件变量唤醒正在 take() 中等待的消费者。
     *
     * 调用方：CameraThread::run() 中每隔 N 帧调用一次
     * 线程：CameraThread 线程
     *
     * @param frame 要写入的帧（内部会 clone，调用方可安全释放原帧）
     */
    void push(const cv::Mat &frame);

    /**
     * @brief 消费者取出一帧（带超时）
     *
     * 如果当前有待消费的帧，立即取走并返回 true。
     * 如果当前没有帧，阻塞等待最多 timeoutMs 毫秒：
     *   - 在等待期间被 push() 唤醒 → 取走帧，返回 true
     *   - 超时仍无帧 → 返回 false
     *
     * 调用方：RecognizeThread::run() 的主循环
     * 线程：RecognizeThread 线程
     *
     * @param[out] out 取出的帧（深拷贝）
     * @param timeoutMs 最大等待时间（毫秒）
     * @return 是否成功取到帧
     */
    bool take(cv::Mat &out, int timeoutMs);

    /**
     * @brief 清空队列
     *
     * 将 m_hasFrame 置 false，释放 m_frame 内存。
     * 用于停止时清理残留帧。
     */
    void clear();

private:
    cv::Mat m_frame;// 存储的单帧
    bool m_hasFrame = false;// 是否有待消费的帧
    QMutex m_mutex;// 保护 m_frame 和 m_hasFrame
    QWaitCondition m_cond;// push() 唤醒 take() 的通知机制
 };

#endif // FRAMEQUEUE_H