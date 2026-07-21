#include "framequeue.h"
void FrameQueue::push(const cv::Mat &frame)
{
    QMutexLocker locker(&m_mutex);
    m_frame = frame.clone();
    m_hasFrame = true;
    m_cond.wakeOne();
}
bool FrameQueue::take(cv::Mat &out, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);

    if(!m_hasFrame){
        if(!m_cond.wait(&m_mutex, timeoutMs)){
            return false;
        }
    }

    if(!m_hasFrame){
        return false;
    }
    out = m_frame.clone();
    m_hasFrame = false;
    return true;
}
void FrameQueue::clear()
{
    QMutexLocker locker(&m_mutex);
    m_frame.release();
    m_hasFrame = false;
}