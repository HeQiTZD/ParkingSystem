#include "framequeue.h"

/*
 * push() 的执行流程：
 *
 *   CameraThread 线程                FrameQueue               RecognizeThread 线程
 *        │                              │                           │
 *        │── push(frame) ──────────────▶│                           │
 *        │                              │ lock mutex                │
 *        │                              │ m_frame = frame.clone()   │
 *        │                              │ m_hasFrame = true         │
 *        │                              │ m_cond.wakeOne() ────────▶│ (从 wait 中醒来)
 *        │                              │ unlock mutex              │
 *        │◀──────────── return ─────────│                           │
 *
 * 关键点：
 * 1. clone() 深拷贝 — OpenCV 的 Mat 默认是浅拷贝（共享像素数据），
 *    跨线程必须 clone() 出独立副本，否则生产者下一帧覆盖时消费者读到的数据会损坏。
 * 2. wakeOne() 而非 wakeAll() — 本设计只有一个消费者，唤醒一个就够了。
 * 3. 覆盖语义 — 直接赋值 m_frame，不检查旧帧是否被消费。
 *    这意味着如果消费者太慢，中间帧会被跳过——这正是我们想要的行为。
 */
void FrameQueue::push(const cv::Mat &frame)
{
    QMutexLocker locker(&m_mutex);
    m_frame = frame.clone();
    m_hasFrame = true;
    m_cond.wakeOne();
}

/*
 * take() 的执行流程：
 *
 *   RecognizeThread 线程              FrameQueue
 *        │                              │
 *        │── take(out, 1500) ──────────▶│
 *        │                              │ lock mutex
 *        │                              │ m_hasFrame == false?
 *        │                              │   → m_cond.wait(mutex, 1500ms)
 *        │                              │      (mutex 被临时释放，允许 push() 写入)
 *        │                              │      (被 push() 的 wakeOne() 唤醒，或超时)
 *        │                              │ 醒来后重新持有 mutex
 *        │                              │ m_hasFrame == true?
 *        │                              │   → out = m_frame.clone()
 *        │                              │   → m_hasFrame = false
 *        │                              │   → return true
 *        │                              │ 否则 → return false (超时)
 *        │◀──────────── return ─────────│
 *
 * 关键点：
 * 1. wait() 会临时释放 mutex — 这样 push() 才能在 take() 等待期间获得锁并写入。
 *    wait() 返回时会重新获取 mutex。
 * 2. 取走后 clone() — 同样是为了线程安全的深拷贝。
 * 3. 取走后 m_hasFrame = false — 标记帧已被消费，避免同一帧被重复处理。
 */
bool FrameQueue::take(cv::Mat &out, int timeoutMs)
{
    QMutexLocker locker(&m_mutex);

    // 没有待消费的帧 → 等待生产者写入（带超时）
    if(!m_hasFrame){
        // wait() 内部：释放 mutex → 挂起线程 → 被唤醒后重新获取 mutex
        if(!m_cond.wait(&m_mutex, timeoutMs)){
            return false;// 超时，仍然没有帧
        }
    }

    // 再次检查（防止虚假唤醒 spurious wakeup）
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
    m_frame.release();//  释放 Mat 内存
    m_hasFrame = false;
}
