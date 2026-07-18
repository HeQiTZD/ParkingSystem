#ifndef PTHREADPOOL_H
#define PTHREADPOOL_H

#include <QObject>
#include <QThreadPool>//管理并复用一组 QThread 的线程池，可以避免频繁创建销毁线程的开销。通常与 QRunnable 配合使用，将任务交给线程池自动调度执行
#include <QRunnable>//表示一个可在线程中执行的任务（即 run() 函数）。它本身不是线程，需要提交给 QThreadPool 来运行。开发者继承 QRunnable 并实现 run() 方法。
#include <QMutex>
#include <opencv2/opencv.hpp>

/**
 * @brief 识别任务类
 *
 * 封装单个车牌识别任务，用于在线程池中执行。
 * 继承自QRunnable，可被QThreadPool管理。
 *
 * 设计模式：
 * - 命令模式（Command Pattern）
 * - 生产者-消费者模式
 */
class RecognitionTask : public QRunnable
{
public:
    /**
     * @brief 构造函数
     * @param taskId 任务ID
     * @param frame 待识别的图像帧
     */
    explicit RecognitionTask(int taskId, const cv::Mat &frame);

    /**
     * @brief 析构函数
     */
    ~RecognitionTask() override;

    /**
     * @brief 获取任务ID
     */
    int getTaskId() const {return m_taskId;}

protected:
    /**
     * @brief 任务执行函数
     *
     * 重写QRunnable的run()函数，在线程池中执行识别任务。
     */
    void run() override;

private:
    int m_taskId;// 任务ID
    cv::Mat m_frame;// 待识别图像
};

/**
 * @brief 线程池管理类
 *
 * 管理线程池，提供异步任务执行能力。
 * 使用Qt内置的QThreadPool，自动管理任务队列和调度。
 *
 * 主要功能：
 * 1. 线程池创建与销毁
 * 2. 任务提交与执行
 * 3. 负载均衡（Qt自动处理）
 *
 * 设计模式：
 * - 单例模式（Singleton）
 * - 门面模式（Facade）
 */
class ThreadPoolManager : public QObject{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static ThreadPoolManager* instance();

    /**
     * @brief 初始化线程池
     * @param maxThreadCount 最大线程数（默认为CPU核心数）
     */
    void init(int maxThreadCount = QThread::idealThreadCount());

    /**
     * @brief 提交识别任务
     * @param frame 待识别的图像
     * @return 任务ID，-1表示提交失败（参数无效或线程池未初始化）
     */
    int submitTask(const cv::Mat &frame);

    /**
     * @brief 获取线程池状态
     * @return 活动线程数
     */
    int activeThreadCount() const;

    /**
     * @brief 等待所有任务完成
     * @param timeout 超时时间（毫秒），-1表示无限等待
     */
    void waitForDone(int timeout = -1);

    /**
     * @brief 获取线程池是否已初始化
     */
    bool isInitialized() const {return m_initialized;}

signals:
    /**
     * @brief 任务完成信号
     * @param taskId 任务ID
     * @param success 是否成功
     * @param errorMsg 错误信息（成功时为空）
     */
    void taskFinished(int taskId, bool success, const QString &errorMsg = "");

    /**
     * @brief 线程池状态变化信号
     * @param activeCount 活动线程数
     */
    void statusChanged(int activeCount);

private:
    /**
     * @brief 构造函数（私有，单例模式）
     */
    explicit ThreadPoolManager(QObject* parent = nullptr);

    // 禁用拷贝
    ThreadPoolManager(const ThreadPoolManager&) = delete;
    ThreadPoolManager operator = (const ThreadPoolManager&) = delete;

    /**
     * @brief 析构函数
     */
    ~ThreadPoolManager();

     // 成员变量
    QThreadPool *m_threadPool;// Qt线程池
    QMutex m_mutex;// 互斥锁，保护任务计数器
    int m_taskCounter;// 任务计数器
    bool m_initialized; //初始化标志

    // 单例实例
    static ThreadPoolManager* s_instance;
};
#endif // PTHREADPOOL_H

/*
 多线程编程基础
1.1 为什么需要多线程？

单线程执行：
┌─────────────────────────────────────────────────────┐
│  UI线程                                              │
│  ├─ 摄像头捕获 (100ms)                               │
│  ├─ 图像处理 (50ms)                                  │
│  ├─ 车牌识别 (200ms)                                 │
│  ├─ 更新UI (10ms)                                    │
│  └─ 总计: 360ms → UI卡顿！                          │
└─────────────────────────────────────────────────────┘

多线程执行：
┌──────────────────┐  ┌──────────────────┐
│  UI线程           │  │  工作线程         │
│  ├─ 更新UI        │  │  ├─ 摄像头捕获    │
│  ├─ 处理用户输入   │  │  ├─ 图像处理      │
│  └─ 响应事件      │  │  └─ 车牌识别      │
└──────────────────┘  └──────────────────┘
     UI响应流畅！         后台并行处理！
1.2 Qt多线程方式
方式	适用场景	复杂度
QThread	需要控制线程生命周期	中等
QRunnable + QThreadPool	短期任务，频繁创建销毁	简单
QtConcurrent	简单的并行计算	最简单
2. 线程同步机制
2.1 互斥锁（QMutex）

QMutex mutex;
int sharedData = 0;

// 线程1
mutex.lock();
sharedData++;  // 临界区
mutex.unlock();

// 线程2
mutex.lock();
sharedData++;  // 临界区
mutex.unlock();
2.2 QMutexLocker（自动锁）

QMutex mutex;

void threadSafeFunction()
{
    QMutexLocker locker(&m_mutex);  // 构造时自动加锁
    // ... 临界区代码 ...
}  // 析构时自动解锁，即使发生异常也能正确释放
2.3 条件变量（QWaitCondition）

QMutex mutex;
QWaitCondition condition;
bool paused = false;

// 暂停线程
void pauseThread() {
    QMutexLocker locker(&mutex);
    paused = true;
}

// 恢复线程
void resumeThread() {
    QMutexLocker locker(&mutex);
    paused = false;
    condition.wakeAll();  // 唤醒等待的线程
}

// 线程主循环
void threadFunction() {
    while (running) {
        QMutexLocker locker(&mutex);
        while (paused) {
            condition.wait(&mutex);  // 等待唤醒
        }
        // ... 执行任务 ...
    }
}
3. 生产者-消费者模式

┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│  生产者      │      │  队列        │      │  消费者      │
│  (摄像头)    │ ───> │  (帧缓冲)   │ ───> │  (识别线程)  │
└─────────────┘      └─────────────┘      └─────────────┘
     生产数据            缓冲数据            消费数据
优点：

解耦生产者和消费者
平衡生产和消费速度差异
提高系统吞吐量
4. 线程池原理

任务提交
    ↓
┌─────────────────────────────────────┐
│  任务队列                            │
│  ┌───┐ ┌───┐ ┌───┐ ┌───┐ ┌───┐    │
│  │ 1 │ │ 2 │ │ 3 │ │ 4 │ │ 5 │    │
│  └───┘ └───┘ └───┘ └───┘ └───┘    │
└─────────────────────────────────────┘
    ↓           ↓           ↓
┌───────┐  ┌───────┐  ┌───────┐
│线程1   │  │线程2   │  │线程3   │
│执行任务1│  │执行任务2│  │执行任务3│
└───────┘  └───────┘  └───────┘
    ↓           ↓           ↓
   完成        完成         完成
    ↓           ↓           ↓
  取任务4     取任务5      等待...
线程池优势：

避免频繁创建/销毁线程的开销
控制并发线程数量，防止资源耗尽
任务排队，有序执行
5. 帧率控制

// 目标帧率：30fps
int targetFps = 30;
int frameDelay = 1000 / targetFps;  // 约33ms

while (running) {
    // 捕获帧
    capture >> frame;

    // 处理帧
    processFrame(frame);

    // 帧率控制：休眠以达到目标帧率
    QThread::msleep(frameDelay);
}
6. 图像格式转换
格式	通道顺序	用途
BGR	蓝-绿-红	OpenCV默认格式
RGB	红-绿-蓝	Qt显示格式
BGRA	蓝-绿-红-透明	OpenCV带透明通道
RGBA	红-绿-蓝-透明	Qt带透明通道
Grayscale	单通道	灰度图

// BGR -> RGB
cv::cvtColor(bgrMat, rgbMat, cv::COLOR_BGR2RGB);

// RGB -> BGR
cv::cvtColor(rgbMat, bgrMat, cv::COLOR_RGB2BGR);
代码工作原理
摄像头捕获流程

启动摄像头线程
    ↓
初始化VideoCapture
    ↓
┌─────────────────────────────┐
│  while (running) {          │
│    检查暂停状态              │
│    捕获一帧                  │
│    更新最新帧（加锁）        │
│    发射newFrame信号          │
│    计算帧率                  │
│    休眠（帧率控制）          │
│  }                          │
└─────────────────────────────┘
    ↓
释放摄像头资源
识别任务执行流程

submitTask(frame)
    ↓
生成任务ID（原子递增）
    ↓
创建 RecognitionTask(taskId, frame)
    ↓
m_threadPool->start(task)  // Qt自动管理队列和调度
    ↓
线程池分配空闲线程执行 task->run()
    ↓
PlateRecognize::recognizePlate(m_frame)
    ↓
发射 taskFinished(taskId, success, errorMsg) 信号
    ↓
任务完成，setAutoDelete(true) 自动释放内存

简化设计说明：
- 去掉手动任务队列，Qt的QThreadPool内置队列管理
- 去掉信号量控制，Qt自动调度任务执行
- 去掉processNextTask()，Qt自动从队列取任务
- 合并taskCompleted/taskError为统一的taskFinished信号
*/