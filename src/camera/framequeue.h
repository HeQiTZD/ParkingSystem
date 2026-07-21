#ifndef FRAMEQUEUE_H
#define FRAMEQUEUE_H
#include <QMutex>
#include <QWaitCondition>
#include <opencv2/opencv.hpp>
 class FrameQueue
 {
public:
    FrameQueue() = default;

    void push(const cv::Mat &frame);

    bool take(cv::Mat &out, int timeoutMs);

    void clear();
private:
    cv::Mat m_frame;
    bool m_hasFrame = false;
    QMutex m_mutex;
    QWaitCondition m_cond;
 };
#endif
 FRAMEQUEUE_H