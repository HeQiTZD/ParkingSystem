# 车牌自动识别与出入库 — 开发指南

> 本文档包含完整的代码和设计思路，供开发者按顺序手动实现。
> 建议按 Task 1 → 6 的顺序开发，每完成一个 Task 编译验证后再进入下一个。

**对应设计文档：** `docs/superpowers/specs/2026-07-12-auto-plate-recognition-design.md`

---

## 文件总览

### 新建文件（6 个）

| 文件 | 职责 |
|------|------|
| `src/camera/framequeue.h` | FrameQueue 头文件 |
| `src/camera/framequeue.cpp` | FrameQueue 实现 |
| `src/app/recognizethread.h` | RecognizeThread 头文件 |
| `src/app/recognizethread.cpp` | RecognizeThread 实现 |
| `src/app/plateconfirmtracker.h` | PlateConfirmTracker 头文件 |
| `src/app/plateconfirmtracker.cpp` | PlateConfirmTracker 实现 |

### 修改文件（5 个）

| 文件 | 改动内容 |
|------|----------|
| `src/camera/camerathread.h` | 新增 FrameQueue 指针和 setter |
| `src/camera/camerathread.cpp` | run() 中新增采样投递逻辑 |
| `UI/MainWindow/mainwindow.h` | 新增成员和 onPlateRecognized 槽 |
| `UI/MainWindow/mainwindow.cpp` | 初始化、自动出入库逻辑、析构清理 |
| `ParkingSystem.pro` | 添加新文件到 SOURCES/HEADERS |

---

## Task 1：FrameQueue — 线程安全帧队列

> **设计思路：** 这是生产者-消费者架构的核心通道。CameraThread（生产者）将视频帧
> 写入队列，RecognizeThread（消费者）从队列取帧进行识别。
>
> 为什么容量只有 1？停车场场景下车辆通行速度慢，识别只需要最新帧。如果用普通队列，
> 帧会不断堆积，消费者处理到的帧已经是几秒前的画面了。容量为 1 + 新帧覆盖旧帧的设计
> 保证消费者始终拿到最新画面，且生产者永远不阻塞（不影响摄像头帧率）。

### 创建 `src/camera/framequeue.h`

```cpp
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
    cv::Mat m_frame;            // 存储的单帧
    bool m_hasFrame = false;    // 是否有待消费的帧
    QMutex m_mutex;             // 保护 m_frame 和 m_hasFrame
    QWaitCondition m_cond;      // push() 唤醒 take() 的通知机制
};

#endif // FRAMEQUEUE_H
```

### 创建 `src/camera/framequeue.cpp`

```cpp
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
    if (!m_hasFrame) {
        // wait() 内部：释放 mutex → 挂起线程 → 被唤醒后重新获取 mutex
        if (!m_cond.wait(&m_mutex, timeoutMs)) {
            return false;   // 超时，仍然没有帧
        }
    }

    // 再次检查（防止虚假唤醒 spurious wakeup）
    if (!m_hasFrame) {
        return false;
    }

    out = m_frame.clone();
    m_hasFrame = false;
    return true;
}

void FrameQueue::clear()
{
    QMutexLocker locker(&m_mutex);
    m_frame.release();      // 释放 Mat 内存
    m_hasFrame = false;
}
```

### 编译验证

此时新增的两个文件还没加入 .pro，暂时不需要编译。继续下一个 Task。

---

## Task 2：RecognizeThread — 识别消费者线程

> **设计思路：** RecognizeThread 是生产者-消费者架构中的消费者。它在独立线程中
> 循环运行，从 FrameQueue 取帧并调用 EasyPR 进行车牌识别。
>
> 为什么用独立线程？EasyPR 单次识别耗时约 200-500ms，如果在 CameraThread 中做识别，
> 摄像头帧率会从 30fps 降到 2fps。如果在主线程做识别，UI 会卡死半秒。独立线程保证
> 三方（采集、识别、UI）互不干扰。
>
> 为什么不用 moveToThread？PlateRecognize 是单例，moveToThread 会改变其线程亲和性，
> 影响其他使用者。这里直接在 RecognizeThread 中调用 PlateRecognize 的识别方法——
> 该方法是无状态的纯计算（输入一张图，输出识别结果），本设计只有一个消费者线程，
> 天然不存在并发问题。

### 创建 `src/app/recognizethread.h`

```cpp
#ifndef RECOGNIZETHREAD_H
#define RECOGNIZETHREAD_H

#include <QThread>
#include <opencv2/opencv.hpp>

class FrameQueue;

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
     * @param plate 识别出的车牌号（格式："颜色:号码"，如 "蓝:苏A12345"）
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
    int m_samplingInterval = 1500;  // take() 超时（ms）
    bool m_running = false;
};

#endif // RECOGNIZETHREAD_H
```

### 创建 `src/app/recognizethread.cpp`

```cpp
#include "recognizethread.h"
#include "src/camera/framequeue.h"
#include "src/app/platerecognize.h"

#include <QDebug>

RecognizeThread::RecognizeThread(QObject *parent)
    : QThread(parent)
{
    // 注册 cv::Mat 类型，保证信号槽跨线程传递时能正确序列化
    qRegisterMetaType<cv::Mat>("cv::Mat");
}

RecognizeThread::~RecognizeThread()
{
    stop();
    wait(3000);
}

void RecognizeThread::setFrameQueue(FrameQueue *queue)
{
    m_queue = queue;
}

void RecognizeThread::setSamplingInterval(int ms)
{
    m_samplingInterval = ms;
}

void RecognizeThread::stop()
{
    m_running = false;
}

/*
 * run() 核心循环的执行流程：
 *
 *   ┌───────────────────────────────────────────────┐
 *   │                while (m_running)               │
 *   │                                               │
 *   │  1. m_queue->take(frame, 1500ms)              │
 *   │     ├── 超时(无新帧) → continue 重新等         │
 *   │     └── 取到帧 → 进入步骤 2                    │
 *   │                                               │
 *   │  2. PlateRecognize::instance()->recognizePlate │
 *   │     ├── 返回 0（未识别到车牌）→ continue       │
 *   │     ├── 返回 >0（识别成功）                    │
 *   │     │   → recognizeFinished 信号已被            │
 *   │     │     PlateRecognize 内部发射               │
 *   │     │   → 这里不重复发射，由连接关系处理        │
 *   │     └── 返回 -1（出错）→ continue              │
 *   └───────────────────────────────────────────────┘
 *
 * 关于 PlateRecognize 的信号：
 *   PlateRecognize::recognizePlate() 内部识别成功时会 emit recognizeFinished。
 *   但 PlateRecognize 是单例，在主线程中创建。
 *   这里在工作线程中调用它的方法，信号会在调用线程（即本线程）中发射。
 *   所以我们不依赖 PlateRecognize 的信号，而是自己发射 plateRecognized。
 *
 * 关于 EasyPR 识别结果格式：
 *   PlateRecognize::recognizeFinished 信号携带 plateStr 格式为 "颜色:号码"，
 *   如 "蓝:苏A12345"。我们需要从中解析出纯车牌号用于数据库查询。
 */
void RecognizeThread::run()
{
    qDebug() << QStringLiteral("识别线程启动，线程ID:") << QThread::currentThreadId();

    if (!m_queue) {
        qWarning() << QStringLiteral("识别线程启动失败：FrameQueue 未设置");
        return;
    }

    PlateRecognize *recognizer = PlateRecognize::instance();
    if (!recognizer->isModelsLoaded()) {
        emit recognizeError(QStringLiteral("EasyPR 模型未加载"));
        return;
    }

    m_running = true;

    while (m_running) {
        // 步骤 1：从队列取帧（带超时）
        cv::Mat frame;
        if (!m_queue->take(frame, m_samplingInterval)) {
            // 超时，没有新帧可识别，继续等待
            continue;
        }

        if (frame.empty()) {
            continue;
        }

        // 步骤 2：调用 EasyPR 识别
        // recognizePlate() 是 CPU 密集型操作（~200-500ms），
        // 在本独立线程中执行，不影响摄像头采集和 UI 响应。
        try {
            // 存储识别结果
            std::vector<easypr::CPlate> plates;
            int result = recognizer->plateRecognizeAccessor(frame, plates);

            if (result == 0 && !plates.empty()) {
                for (size_t i = 0; i < plates.size(); ++i) {
                    easypr::CPlate &plate = plates[i];
                    cv::Mat plateImg = plate.getPlateMat();
                    std::string plateStr = plate.getPlateStr();

                    // 解析车牌字符串："蓝:苏A12345" → 取 "苏A12345"
                    QString qPlateStr = QString::fromLocal8Bit(plateStr.c_str());
                    QStringList parts = qPlateStr.split(":");
                    QString plateNumber = (parts.size() >= 2) ? parts[1].trimmed() : qPlateStr;

                    if (!plateNumber.isEmpty()) {
                        emit plateRecognized(plateNumber, plateImg.clone());
                        qDebug() << QStringLiteral("识别线程发射结果:") << plateNumber;
                    }
                }
            }
        } catch (const std::exception &e) {
            qDebug() << QStringLiteral("识别异常:") << e.what();
        }
    }

    qDebug() << QStringLiteral("识别线程已结束");
}
```

### 关于 PlateRecognize 的改动说明

上面的代码中调用了 `recognizer->plateRecognizeAccessor(frame, plates)`。
当前 PlateRecognize 的 `recognizePlate()` 方法内部直接调用
`m_plateRecognize.plateRecognize()` 并通过信号发射结果。
但在 RecognizeThread 中我们需要直接获得 `vector<CPlate>` 结果。

**你需要在 `PlateRecognize` 中新增一个方法**，让 RecognizeThread 可以直接获取原始识别结果：

在 `src/app/platerecognize.h` 的 public 区域新增：

```cpp
    /**
     * @brief 直接访问 EasyPR 识别结果（供 RecognizeThread 使用）
     *
     * 与 recognizePlate() 不同，本方法不发射信号，
     * 而是直接返回识别到的 CPlate 向量，由调用方自行处理。
     *
     * @param image 输入图像
     * @param[out] plates 输出的车牌向量
     * @return 0=成功, -1=失败
     */
    int plateRecognizeAccessor(const cv::Mat &image, std::vector<easypr::CPlate> &plates);
```

在 `src/app/platerecognize.cpp` 新增实现：

```cpp
int PlateRecognize::plateRecognizeAccessor(const cv::Mat &image, std::vector<easypr::CPlate> &plates)
{
    if (!m_modelsLoaded || image.empty()) {
        return -1;
    }

    try {
        cv::Mat localImg = image.clone();
        int result = m_plateRecognize.plateRecognize(localImg, plates, 0);
        return result;
    } catch (const std::exception &e) {
        qDebug() << QStringLiteral("plateRecognizeAccessor 异常:") << e.what();
        return -1;
    }
}
```

---

## Task 3：PlateConfirmTracker — 防重复校验器

> **设计思路：** 全自动识别最大的风险是重复触发——车辆在摄像头前停留时，同一车牌
> 会被反复识别到。防重复校验器实现两层保护：
>
> **第 1 层：冷却时间（Cooldown）**
> 同一车牌上次成功出入库后 30 秒内，忽略所有对该车牌的识别结果。
> 场景：车辆入库后在闸口停留等待通行，不应被再次触发入库。
>
> **第 2 层：多次确认（Multi-confirm）**
> 需要连续 3 次识别到同一车牌才确认操作。防止偶然的误识别触发出入库。
> 如果两次识别间隔超过 5 秒，认为连续性中断（车辆可能已离开），重置计数。
>
> 这是一个纯逻辑组件，不依赖 Qt 特性（不继承 QObject），便于单独测试。

### 创建 `src/app/plateconfirmtracker.h`

```cpp
#ifndef PLATECONFIRMTRACKER_H
#define PLATECONFIRMTRACKER_H

#include <QString>
#include <QHash>
#include <QDateTime>

/**
 * @brief 单个车牌的确认状态
 */
struct ConfirmState {
    int count = 0;                  // 连续识别次数
    QDateTime firstSeen;            // 本轮首次识别时间
    QDateTime lastSeen;             // 最近一次识别时间
    QDateTime lastAction;           // 上次出入库操作完成时间
};

/**
 * @brief 车牌防重复校验器
 *
 * 在主线程中使用，接收来自 RecognizeThread 的识别结果，
 * 通过双重保护机制（冷却时间 + 多次确认）决定是否触发出入库操作。
 *
 * 使用方式：
 *   1. 每次收到识别结果 → 调用 report(plate)
 *   2. report 返回 true → 触发出入库操作
 *   3. 操作完成后 → 调用 markActioned(plate) 记录冷却起点
 *
 * 校验流程图：
 *
 *   report(plate)
 *     │
 *     ├─ 第1层：冷却时间
 *     │  lastAction 距今 < cooldownSecs？
 *     │  ├── 是 → return false（静默忽略）
 *     │  └── 否 → 进入第2层
 *     │
 *     └─ 第2层：多次确认
 *        lastSeen 距今 > expirySecs？
 *        ├── 是 → count = 1（连续性中断，重新开始）
 *        └── 否 → count++
 *        count >= confirmCount？
 *        ├── 否 → return false（等待更多确认）
 *        └── 是 → count = 0, return true（确认通过！）
 */
class PlateConfirmTracker
{
public:
    PlateConfirmTracker() = default;

    /**
     * @brief 报告一次识别结果
     *
     * 每次 RecognizeThread 识别到车牌后调用此方法。
     * 内部进行双重校验，只在两层都通过时返回 true。
     *
     * @param plate 识别到的车牌号
     * @return true = 确认通过，应触发出入库；false = 忽略
     */
    bool report(const QString &plate);

    /**
     * @brief 标记操作已完成
     *
     * 在 checkIn/checkOut 成功后调用，记录 lastAction 时间。
     * 后续 report() 会检查冷却时间，在冷却期内忽略该车牌。
     *
     * @param plate 完成操作的车牌号
     */
    void markActioned(const QString &plate);

    // ===== 配置方法 =====

    /** 设置冷却时间（秒），默认 30 */
    void setCooldownSecs(int secs) { m_cooldownSecs = secs; }

    /** 设置确认所需的连续识别次数，默认 3 */
    void setConfirmCount(int count) { m_confirmCount = count; }

    /** 设置连续性超时（秒），超过此时间未再次识别则重置计数，默认 5 */
    void setExpirySecs(int secs) { m_expirySecs = secs; }

private:
    /**
     * QHash<车牌号, 确认状态>
     *
     * 每个车牌独立追踪。不同车牌互不影响——
     * 摄像头交替识别到 A 和 B 时，A 的计数不会被 B 重置。
     */
    QHash<QString, ConfirmState> m_tracker;

    int m_cooldownSecs = 30;    // 冷却时间（秒）
    int m_confirmCount = 3;     // 确认所需连续次数
    int m_expirySecs = 5;       // 连续性超时（秒）
};

#endif // PLATECONFIRMTRACKER_H
```

### 创建 `src/app/plateconfirmtracker.cpp`

```cpp
#include "plateconfirmtracker.h"

/*
 * report() 完整执行示例：
 *
 * 假设 confirmCount=3, cooldownSecs=30, expirySecs=5
 *
 * 时间轴：
 *   T=0s    识别到 苏A12345 → count=1, return false (需要3次)
 *   T=1.5s  识别到 苏A12345 → count=2, return false (需要3次)
 *   T=3.0s  识别到 苏A12345 → count=3, return true  (确认通过！触发入库)
 *           → 入库成功后调用 markActioned("苏A12345"), lastAction=T=3.0s
 *   T=4.5s  识别到 苏A12345 → 冷却中(距lastAction仅1.5s < 30s), return false
 *   T=35.0s 识别到 苏A12345 → 冷却已过, count=1, return false (重新计数)
 *
 * 连续性中断示例：
 *   T=0s    识别到 苏A12345 → count=1
 *   T=1.5s  识别到 苏A12345 → count=2
 *   T=8.0s  识别到 苏A12345 → 距上次 6.5s > 5s, 连续性中断, count=1 (重新开始)
 */
bool PlateConfirmTracker::report(const QString &plate)
{
    QDateTime now = QDateTime::currentDateTime();

    // 获取或创建该车牌的追踪状态
    ConfirmState &state = m_tracker[plate];

    // ===== 第1层：冷却时间检查 =====
    // 如果该车牌最近刚完成过出入库操作，在冷却期内不再触发
    if (state.lastAction.isValid()) {
        int secsSinceAction = state.lastAction.secsTo(now);
        if (secsSinceAction < m_cooldownSecs) {
            return false;
        }
    }

    // ===== 第2层：多次确认 =====
    // 检查连续性：如果距离上次识别超过 expirySecs，说明车辆可能已离开又回来，
    // 需要重新开始计数
    if (state.lastSeen.isValid()) {
        int secsSinceLast = state.lastSeen.secsTo(now);
        if (secsSinceLast > m_expirySecs) {
            // 连续性中断 → 重置为第 1 次
            state.count = 0;
        }
    }

    // 更新时间戳
    if (state.count == 0) {
        state.firstSeen = now;  // 记录本轮首次识别时间
    }
    state.lastSeen = now;
    state.count++;

    // 检查是否达到确认阈值
    if (state.count >= m_confirmCount) {
        // 确认通过！重置计数（下次需要重新累计）
        state.count = 0;
        return true;
    }

    return false;
}

void PlateConfirmTracker::markActioned(const QString &plate)
{
    // 记录操作完成时间 → 后续 report() 会进入冷却期
    m_tracker[plate].lastAction = QDateTime::currentDateTime();
}
```

---

## Task 4：修改 CameraThread — 接入生产者

> **设计思路：** CameraThread 目前只做两件事：
> 1. 捕获帧 → emit newFrameCaptured（给主线程显示画面）
> 2. 保存到 m_latestFrame（给 getLatestFrame() 用）
>
> 现在要加第三件事：每隔 N 帧，把帧投递到 FrameQueue 供识别线程消费。
> 改动极小：头文件加一个指针 + setter，cpp 加 4 行采样逻辑。

### 修改 `src/camera/camerathread.h`

在 `public:` 区域（`resume()` 之后）新增：

```cpp
    /**
     * @brief 设置帧队列（用于向识别线程投递采样帧）
     *
     * 如果不设置（保持 nullptr），CameraThread 的行为与原来完全一致，
     * 不会向任何队列投递帧。这保证了向后兼容。
     *
     * @param queue 帧队列指针，生命周期由调用方管理
     */
    void setFrameQueue(FrameQueue *queue) { m_frameQueue = queue; }
```

在 `private:` 成员区域最后新增：

```cpp
    FrameQueue *m_frameQueue = nullptr;   // 识别队列（可选）
```

在文件头部新增前向声明：

```cpp
class FrameQueue;   // 前向声明，避免引入头文件
```

### 修改 `src/camera/camerathread.cpp`

在 `run()` 函数中，找到 `while(m_running)` 循环，在 `emit newFrameCaptured(frame.clone());` 这一行**之后**，添加采样投递逻辑。

修改前的 `run()` 结构：

```cpp
void CameraThread::run()
{
    // ... 初始化 ...
    m_running = true;
    // ...
    while(m_running){
        // ... 暂停检查 ...
        // ... 捕获 frame ...
        // ... 更新 m_latestFrame ...

        emit newFrameCaptured(frame.clone());   // ← 现有代码

        calculateFps();
        QThread::msleep(frameDelay);
    }
    // ...
}
```

修改后：

```cpp
void CameraThread::run()
{
    // ... 初始化（保持不变）...
    m_running = true;
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();

    int frameDelay = 1000 / m_targetFps;

    // ===== 新增：采样计数器 =====
    // 每 sampleEveryN 帧向 FrameQueue 投递一帧供识别线程消费。
    // 30fps 下每 15 帧 ≈ 0.5 秒投递一次。
    // 投递频率高于识别频率（~1.5秒/帧），中间帧被覆盖，消费者始终拿最新帧。
    int sampleCounter = 0;
    const int sampleEveryN = 15;

    while(m_running){
        // ... 暂停检查（保持不变）...
        // ... 捕获 frame（保持不变）...
        // ... 更新 m_latestFrame（保持不变）...

        emit newFrameCaptured(frame.clone());

        // ===== 新增：定期向识别队列投递 =====
        if (m_frameQueue && ++sampleCounter >= sampleEveryN) {
            m_frameQueue->push(frame.clone());
            sampleCounter = 0;
        }

        calculateFps();
        QThread::msleep(frameDelay);
    }

    releaseCamera();
    qDebug() << QStringLiteral("摄像头线程已结束");
}
```

在文件头部新增 include：

```cpp
#include "framequeue.h"
```

### 改动总结

| 位置 | 改动 |
|------|------|
| `camerathread.h` | +1 前向声明, +1 setter 方法, +1 成员变量 |
| `camerathread.cpp` | +1 include, +4 行采样逻辑 (变量声明 + if 判断) |

现有的 `newFrameCaptured` 信号、`getLatestFrame()` 接口、帧率控制等逻辑**完全不变**。

---

## Task 5：修改 MainWindow — 整合所有模块

> **设计思路：** MainWindow 是所有模块的协调者。它负责：
> 1. 初始化阶段：创建 FrameQueue、RecognizeThread、PlateConfirmTracker，建立信号连接
> 2. 运行阶段：接收 RecognizeThread 的识别结果，通过 PlateConfirmTracker 防重复校验，
>    校验通过后自动执行 checkIn/checkOut
> 3. 清理阶段：析构时停止 RecognizeThread 并释放 FrameQueue
>
> 自动/手动兼容设计：
> - 现有的手动输入 + 按钮流程完全不变
> - 新增一个 QCheckBox 控制自动识别的开关
> - 两个模式互不干扰

### 修改 `UI/MainWindow/mainwindow.h`

新增 include（在文件头部现有 include 之后）：

```cpp
#include "src/camera/framequeue.h"
#include "src/app/recognizethread.h"
#include "src/app/plateconfirmtracker.h"
```

在 `private slots:` 区域新增：

```cpp
    /**
     * @brief 接收自动识别结果
     *
     * 由 RecognizeThread::plateRecognized 信号触发（跨线程 QueuedConnection）。
     * 在主线程中执行：防重复校验 → 自动出入库 → 更新 UI。
     *
     * @param plate 识别到的车牌号
     * @param plateImg 车牌区域图像（预留，暂未使用）
     */
    void onPlateRecognized(const QString &plate, const cv::Mat &plateImg);

    /** @brief 自动识别开关切换 */
    void onAutoRecognizeToggled(bool checked);
```

在 `private:` 成员区域新增（现有成员之后）：

```cpp
    // ===== 自动识别相关 =====
    FrameQueue *m_frameQueue = nullptr;
    RecognizeThread *m_recognizeThread = nullptr;
    PlateConfirmTracker *m_confirmTracker = nullptr;
    bool m_autoRecognizeEnabled = false;
```

### 修改 `UI/MainWindow/mainwindow.cpp`

#### 5.1 新增 include（文件头部）

```cpp
#include "src/camera/framequeue.h"
#include "src/app/recognizethread.h"
#include "src/app/plateconfirmtracker.h"
#include "src/app/platerecognize.h"
```

#### 5.2 构造函数末尾追加初始化（在最后一个 connect 之后）

```cpp
    // ========== 自动识别模块初始化 ==========
    //
    // 初始化顺序：
    // 1. FrameQueue（通道）→ 2. 注入 CameraThread（生产者）
    // → 3. RecognizeThread（消费者）→ 4. PlateConfirmTracker（校验器）
    // → 5. 信号连接 → 6. 加载模型 → 7. 启动识别线程

    // 1. 创建帧队列
    m_frameQueue = new FrameQueue();

    // 2. 注入 CameraThread（使其成为生产者）
    m_cameraThread->setFrameQueue(m_frameQueue);

    // 3. 创建识别消费者线程
    m_recognizeThread = new RecognizeThread(this);
    m_recognizeThread->setFrameQueue(m_frameQueue);
    m_recognizeThread->setSamplingInterval(1500);   // 1.5秒采样间隔

    // 4. 创建防重复校验器
    m_confirmTracker = new PlateConfirmTracker();

    // 5. 连接识别结果信号（跨线程，Qt 自动使用 QueuedConnection）
    connect(m_recognizeThread, &RecognizeThread::plateRecognized,
            this, &MainWindow::onPlateRecognized);

    // 6. 加载 EasyPR 模型（如果尚未加载）
    PlateRecognize *recognizer = PlateRecognize::instance();
    if (!recognizer->isModelsLoaded()) {
        // 模型路径：项目根目录下的 resources/model（按实际路径修改）
        QString modelPath = QCoreApplication::applicationDirPath() + "/resources/model";
        recognizer->loadModels(modelPath);
    }

    // 7. 连接自动识别开关
    // 注意：你需要在 mainwindow.ui 中添加一个名为 autoRecognizeCheckBox 的 QCheckBox
    // 放置在摄像头画面区域附近，文字为"自动识别"
    connect(ui->autoRecognizeCheckBox, &QCheckBox::toggled,
            this, &MainWindow::onAutoRecognizeToggled);
```

#### 5.3 析构函数修改

```cpp
MainWindow::~MainWindow()
{
    // 先停止识别线程（依赖 FrameQueue，必须在 FrameQueue 释放前停止）
    if (m_recognizeThread) {
        m_recognizeThread->stop();
        m_recognizeThread->wait(3000);
    }

    // 再停止摄像头线程
    if (m_cameraThread) {
        m_cameraThread->quit();
        m_cameraThread->wait();
    }

    // 释放帧队列（两个线程都已停止，安全释放）
    delete m_frameQueue;
    m_frameQueue = nullptr;

    // 释放校验器
    delete m_confirmTracker;
    m_confirmTracker = nullptr;

    delete ui;
}
```

#### 5.4 新增 onAutoRecognizeToggled 槽函数

```cpp
/*
 * 自动识别开关逻辑：
 *
 *   checked = true  → 启动 RecognizeThread，开始自动识别
 *   checked = false → 停止 RecognizeThread，切回纯手动模式
 *
 * 停止后重新开启时，RecognizeThread 需要重新 start()。
 * QThread 的规则：run() 返回后线程结束，可以再次 start()。
 */
void MainWindow::onAutoRecognizeToggled(bool checked)
{
    m_autoRecognizeEnabled = checked;

    if (checked) {
        // 启动识别线程
        if (m_recognizeThread && !m_recognizeThread->isRunning()) {
            m_recognizeThread->start();
        }
        notifyInfo(this, QStringLiteral("自动识别已开启"));
    } else {
        // 停止识别线程
        if (m_recognizeThread && m_recognizeThread->isRunning()) {
            m_recognizeThread->stop();
            m_recognizeThread->wait(3000);
        }
        notifyInfo(this, QStringLiteral("自动识别已关闭"));
    }
}
```

#### 5.5 新增 onPlateRecognized 槽函数

```cpp
/*
 * 自动识别结果处理流程：
 *
 *   RecognizeThread 发射 plateRecognized 信号
 *     │
 *     ▼  (QueuedConnection 跨线程投递到主线程)
 *   onPlateRecognized(plate, plateImg)
 *     │
 *     ├── 自动模式关闭？ → return
 *     ├── 数据库未连接？ → return
 *     │
 *     ├── PlateConfirmTracker::report(plate)
 *     │   ├── 冷却中 → return
 *     │   ├── 连续次数不足 → return
 *     │   └── 确认通过 ↓
 *     │
 *     ├── 判断方向：isVehicleInPark(plate)?
 *     │   ├── 否 → 入库流程 (checkIn)
 *     │   └── 是 → 出库流程 (计算费用 + checkOut，不弹窗确认)
 *     │
 *     ├── markActioned(plate) — 记录冷却起点
 *     │
 *     └── 更新 UI：记录卡片 + 车位统计 + Toast
 */
void MainWindow::onPlateRecognized(const QString &plate, const cv::Mat &plateImg)
{
    Q_UNUSED(plateImg)

    // 自动模式关闭时忽略识别结果
    if (!m_autoRecognizeEnabled) {
        return;
    }

    if (!m_db) {
        qDebug() << QStringLiteral("自动识别：数据库未连接");
        return;
    }

    // ===== 防重复校验（双重保护） =====
    if (!m_confirmTracker->report(plate)) {
        return;     // 冷却中 或 连续次数不足，静默忽略
    }

    // ===== 确认通过，执行出入库 =====
    // 标准化车牌号（与手动输入保持一致的格式）
    QString normalizedPlate = Car::normalizePlate(plate);
    if (normalizedPlate.isEmpty() || !Car::isValidLicensePlate(normalizedPlate)) {
        qDebug() << QStringLiteral("自动识别：车牌格式无效") << plate;
        return;
    }

    QString parkingName = getParkingNameFromConfig();

    if (!m_db->isVehicleInPark(normalizedPlate)) {
        // ===== 入库流程 =====
        if (m_db->checkIn(normalizedPlate, parkingName)) {
            m_confirmTracker->markActioned(normalizedPlate);
            notifySuccess(this, QStringLiteral("[自动] %1 入库成功").arg(normalizedPlate));
            onUpdateParkingCount();
            if (ui->vehicleEntryExitWidget) {
                ui->vehicleEntryExitWidget->prependEntry(
                    {Car::displayPlate(normalizedPlate), QDateTime::currentDateTime(), VehicleEntryStatus::In});
            }
        } else {
            notifyFailure(this, QStringLiteral("[自动] %1 入库失败").arg(normalizedPlate));
        }
    } else {
        // ===== 出库流程（不弹窗确认，直接计算费用并出库） =====
        QDateTime inTime = m_db->getVehicleCheckInTime(normalizedPlate);
        QDateTime outTime = QDateTime::currentDateTime();

        double cost = Car::calculateFee(inTime, outTime,
                                        InitFile::instance().getParkingPrice(),
                                        InitFile::instance().getFreeMinutes());

        if (m_db->checkOut(normalizedPlate, parkingName, cost)) {
            m_confirmTracker->markActioned(normalizedPlate);

            // 计算时长用于 Toast 展示
            qint64 totalMinutes = inTime.secsTo(outTime) / 60;
            int hours = totalMinutes / 60;
            int minutes = totalMinutes % 60;

            notifySuccess(this, QStringLiteral("[自动] %1 出库成功 | %2时%3分 | %4元")
                          .arg(normalizedPlate)
                          .arg(hours).arg(minutes)
                          .arg(cost, 0, 'f', 2));
            onUpdateParkingCount();
            if (ui->vehicleEntryExitWidget) {
                ui->vehicleEntryExitWidget->prependEntry(
                    {Car::displayPlate(normalizedPlate), QDateTime::currentDateTime(), VehicleEntryStatus::Out});
            }
        } else {
            notifyFailure(this, QStringLiteral("[自动] %1 出库失败").arg(normalizedPlate));
        }
    }
}
```

---

## Task 6：更新 ParkingSystem.pro 和 UI

### 6.1 修改 `ParkingSystem.pro`

在 `SOURCES +=` 的 `# ==================== 摄像头相关 ====================` 区域新增：

```
    src/camera/framequeue.cpp \
```

在 `SOURCES +=` 的 `# ==================== 应用程序核心代码 ====================` 区域新增：

```
    src/app/recognizethread.cpp \
    src/app/plateconfirmtracker.cpp \
```

在 `HEADERS +=` 的 `# ==================== 摄像头相关 ====================` 区域新增：

```
    src/camera/framequeue.h \
```

在 `HEADERS +=` 的 `# ==================== 应用程序核心代码 ====================` 区域新增：

```
    src/app/recognizethread.h \
    src/app/plateconfirmtracker.h \
```

### 6.2 修改 `UI/MainWindow/mainwindow.ui`

在 Qt Designer 中添加一个 `QCheckBox` 控件：

- **objectName：** `autoRecognizeCheckBox`
- **text：** `自动识别`
- **推荐位置：** 摄像头画面下方的控制按钮区域（与 `cameraCtrlBtn1` ~ `cameraCtrlBtn5` 同一行），或摄像头标题栏右侧
- **默认状态：** 不勾选（unchecked）

如果不想用 Designer，也可以在 `mainwindow.cpp` 构造函数中用代码创建：

```cpp
    // 在摄像头初始化之后添加：
    QCheckBox *autoRecognizeCheckBox = new QCheckBox(QStringLiteral("自动识别"), ui->cameraControlWidget);
    autoRecognizeCheckBox->setObjectName("autoRecognizeCheckBox");
    // 然后把后面 connect 中的 ui->autoRecognizeCheckBox 替换为 autoRecognizeCheckBox
```

---

## 编译与运行验证

### 编译

完成所有改动后，在 Qt Creator 中重新 qmake + 构建。预期编译通过，无错误。

### 运行验证清单

1. **启动应用**：登录后进入主窗口，摄像头画面正常显示，与改动前行为一致
2. **手动模式**：不勾选"自动识别"，手动输入车牌号点击入库/出库，与改动前行为一致
3. **开启自动识别**：勾选"自动识别"，看到 Toast 提示"自动识别已开启"
4. **车牌识别**：将车牌图片放在摄像头前，观察 qDebug 输出是否有"识别线程发射结果"
5. **多次确认**：同一车牌需要连续识别 3 次（约 5~6 秒）后才触发入库
6. **入库成功**：Toast 提示"[自动] 苏Axxxxx 入库成功"，车位统计更新，记录卡片新增一条
7. **冷却期**：入库后 30 秒内再次识别到同一车牌，不再触发操作
8. **出库**：冷却期过后再次识别到同一车牌（已在库中），自动计算费用并出库
9. **关闭自动识别**：取消勾选，Toast 提示"自动识别已关闭"，后续识别结果被忽略

---

## 数据流总结

```
                          ┌──────────────────────────────────────────────────────────────────┐
                          │                         MainWindow (主线程)                       │
                          │                                                                  │
  CameraThread            │   onPlateRecognized()                                            │
  (工作线程)               │     │                                                            │
       │                  │     ├── PlateConfirmTracker::report()                             │
       │ 每帧              │     │     ├── 冷却中? → return                                    │
       │ emit              │     │     ├── 次数不足? → return                                  │
       │ newFrameCaptured  │     │     └── 确认通过 ↓                                         │
       │─────────────────▶│     │                                                            │
       │                  │     ├── isVehicleInPark?                                          │
       │ 每15帧            │     │     ├── 否 → checkIn → Toast + 更新UI                       │
       │ push()            │     │     └── 是 → calculateFee + checkOut → Toast + 更新UI       │
       │                  │     │                                                            │
       ▼                  │     └── markActioned() — 开始冷却                                  │
  FrameQueue ◀────────────│                                                                  │
  (容量=1)                │                                                                  │
       │                  │   手动模式（不变）                                                  │
       │ take()           │     entryPlateInput → onEntrySearchButton → checkIn               │
       ▼                  │     exitPlateInput  → onExitSearchButton  → checkOut              │
  RecognizeThread         │                                                                  │
  (工作线程)               │                                                                  │
       │                  └──────────────────────────────────────────────────────────────────┘
       │ EasyPR 识别
       │ emit plateRecognized ──────▶ (QueuedConnection 跨线程投递)
       │
```
