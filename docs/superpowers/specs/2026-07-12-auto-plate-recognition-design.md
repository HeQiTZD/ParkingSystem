# 车牌自动识别与出入库功能设计

## 概述

将现有的手动输入车牌出入库流程升级为**全自动模式**：摄像头实时采集画面，系统自动识别车牌并触发入库/出库操作，无需人工干预。同时保留手动输入作为备用。

### 设计决策

| 决策项 | 选择 | 理由 |
|--------|------|------|
| 触发方式 | 全自动 | 无需人工干预，效率最高 |
| 摄像头数量 | 单摄像头 | 根据车辆是否已在库中自动判断入库/出库方向 |
| 识别频率 | 定时采样 | 平衡响应速度和性能开销 |
| 防重复机制 | 双重保护（多次确认 + 冷却时间） | 最稳定可靠 |
| UI 模式 | 自动 + 手动兼容 | 自动识别失败时可手动干预 |
| 线程模型 | 生产者-消费者队列 | 采集与识别完全解耦 |

---

## 架构

### 线程模型

系统分为三个线程：

```
┌─────────────────┐     ┌──────────────────────┐     ┌─────────────────┐
│  CameraThread   │     │  RecognizeThread      │     │   UI Thread     │
│  (生产者)        │     │  (消费者)              │     │   (主线程)       │
│                 │     │                       │     │                 │
│ 每N帧写入队列    │────▶│ 循环取帧 → EasyPR识别  │────▶│ 防重复校验       │
│ (队列容量=1,    │     │ 识别成功发信号          │     │ 数据库操作       │
│  新帧覆盖旧帧)   │     │                       │     │ UI更新          │
└─────────────────┘     └──────────────────────┘     └─────────────────┘
```

### 模块依赖关系

```
MainWindow
  ├── CameraThread        (已有，微量改动)
  ├── FrameQueue           (新建)
  ├── RecognizeThread      (新建)
  ├── PlateRecognize       (已有，不改动)
  ├── PlateConfirmTracker  (新建)
  ├── DatabaseManager      (已有，不改动)
  └── VehicleEntryExitWidget (已有，不改动)
```

---

## 新建模块

### 1. FrameQueue — 线程安全帧队列

容量为 1 的帧队列，新帧覆盖旧帧，生产者永不阻塞。

```cpp
class FrameQueue {
public:
    // 生产者调用：覆盖当前帧，唤醒消费者。不阻塞。
    void push(cv::Mat frame);

    // 消费者调用：等待帧可用（带超时），取走帧。超时返回 false。
    bool take(cv::Mat &out, int timeoutMs);

    // 清空队列
    void clear();

private:
    cv::Mat m_frame;
    bool m_hasFrame = false;
    QMutex m_mutex;
    QWaitCondition m_cond;
};
```

**设计要点：**
- 容量为 1：停车场场景车辆通行速度慢，只需最新帧
- 新帧覆盖旧帧：避免帧堆积导致识别延迟
- 生产者永不阻塞：摄像头帧率不受识别速度影响

### 2. RecognizeThread — 识别消费者线程

独立线程，循环从 FrameQueue 取帧并调用 EasyPR 识别。

```cpp
class RecognizeThread : public QThread {
    Q_OBJECT
public:
    void setFrameQueue(FrameQueue *queue);
    void setSamplingInterval(int ms);  // 默认 1500ms
    void stop();

signals:
    void plateRecognized(QString plate, cv::Mat plateImg);
    void recognizeError(QString errorMsg);

protected:
    void run() override;

private:
    FrameQueue *m_queue = nullptr;
    int m_samplingInterval = 1500;
    bool m_running = false;
};
```

**run() 核心逻辑：**
- 循环调用 `m_queue->take(frame, m_samplingInterval)`
- 超时则 continue（无新帧可识别）
- 取到帧后调用 `PlateRecognize::instance()->recognizePlate(frame)`
- 识别成功 emit `plateRecognized` 信号（通过 QueuedConnection 跨线程到主线程）
- PlateRecognize 的识别方法是无状态纯计算，本设计中只有一个消费者，天然线程安全

### 3. PlateConfirmTracker — 防重复校验器

主线程中执行，接收识别结果后进行双重保护校验。

```cpp
struct ConfirmState {
    int count;              // 连续识别次数
    QDateTime firstSeen;    // 首次识别时间
    QDateTime lastSeen;     // 最近识别时间
    QDateTime lastAction;   // 上次出入库操作时间
};

class PlateConfirmTracker {
public:
    // 报告一次识别结果，返回是否通过确认
    bool report(const QString &plate);

    // 记录操作完成（更新冷却时间起点）
    void markActioned(const QString &plate);

    // 配置参数
    void setCooldownSecs(int secs);      // 默认 30 秒
    void setConfirmCount(int count);     // 默认 3 次
    void setExpirySecs(int secs);        // 连续性超时，默认 5 秒

private:
    QHash<QString, ConfirmState> m_tracker;
    int m_cooldownSecs = 30;
    int m_confirmCount = 3;
    int m_expirySecs = 5;
};
```

**校验流程：**

```
report(plate) 被调用
  │
  ├── 第1层：冷却时间检查
  │   lastAction 距今 < 30 秒？
  │   ├── 是 → return false（忽略）
  │   └── 否 → 进入第2层
  │
  └── 第2层：多次确认
      lastSeen 距今 > 5 秒？
      ├── 是 → 重置 count = 1（连续性中断）
      └── 否 → count++
      count >= 3？
      ├── 否 → return false（等待更多确认）
      └── 是 → 重置 count = 0，return true（确认通过）
```

---

## 现有模块改动

### CameraThread — 微量改动

仅新增两处：

1. **新增成员**：`FrameQueue *m_frameQueue = nullptr;` 和 `setFrameQueue()` 方法
2. **run() 中新增采样逻辑**（4 行代码）：

```cpp
int sampleCounter = 0;
const int sampleEveryN = 15;  // 每15帧采样一次（30fps下约0.5秒）

// 在现有 while(m_running) 循环内，emit newFrameCaptured 之后：
if (m_frameQueue && ++sampleCounter >= sampleEveryN) {
    m_frameQueue->push(frame.clone());
    sampleCounter = 0;
}
```

不改动现有的 `newFrameCaptured` 信号、`getLatestFrame()` 接口、帧率控制等逻辑。

### MainWindow — 整合协调

**新增成员：**
- `FrameQueue *m_frameQueue`
- `RecognizeThread *m_recognizeThread`
- `PlateConfirmTracker *m_confirmTracker`
- 自动识别开关状态（bool 或绑定到 UI 控件）

**初始化流程（构造函数中追加）：**

```
1. 创建 FrameQueue 实例
2. 为 CameraThread 注入 FrameQueue 指针
3. 创建 RecognizeThread，注入同一个 FrameQueue
4. 确保 PlateRecognize 模型已加载
5. 创建 PlateConfirmTracker 实例
6. 连接信号：RecognizeThread::plateRecognized → MainWindow::onPlateRecognized
7. 启动 RecognizeThread
```

**新增槽函数 onPlateRecognized(QString plate, cv::Mat plateImg)：**

```
1. PlateConfirmTracker::report(plate)
   未通过 → return
2. 自动判断方向：
   isVehicleInPark(plate) ?
     是 → 出库流程（计算费用 + checkOut）
     否 → 入库流程（checkIn）
3. PlateConfirmTracker::markActioned(plate)
4. 更新 UI：
   - vehicleEntryExitWidget->prependEntry(...)
   - onUpdateParkingCount()
   - Toast 提示操作结果（含费用信息）
```

**自动出库费用处理：**
- 手动模式下现有流程会弹出 `notifyConfirm` 确认费用，需要操作员点击确认
- 自动模式下**不弹窗**，直接计算费用并完成出库，避免阻塞自动流程
- Toast 提示中显示车牌、停车时长和费用信息，供操作员事后查看

**自动/手动兼容：**
- 现有的手动输入框 + 按钮保持不变，走现有 `onEntrySearchButton` / `onExitSearchButton` 逻辑
- 新增自动识别开关控件，控制 RecognizeThread 启停
- 自动模式开启时，摄像头画面区域显示状态指示（如"自动识别中..."）
- 两个模式互不干扰，自动识别结果不影响输入框内容

**析构函数追加：**
- 停止 RecognizeThread（`stop()` + `wait()`）
- 释放 FrameQueue

---

## 采样频率设计

```
CameraThread (30fps)
  │ 每15帧 ≈ 0.5秒
  ▼
FrameQueue (容量1, 新帧覆盖旧帧)
  │ take() 超时 = 1500ms
  ▼
RecognizeThread
  │ EasyPR识别耗时 ~200-500ms
  │ 实际识别间隔 ≈ 1.5~2秒
  ▼
plateRecognized 信号 → 主线程
  │ 多次确认需3次 → 确认耗时 ≈ 4.5~6秒
  ▼
触发出入库
```

- 生产者投递频率（~0.5秒/帧）高于消费者消费频率（~1.5秒/帧），中间帧被覆盖
- 从车辆进入摄像头视野到触发操作，预期延迟约 5~6 秒（3 次确认 × ~1.5~2 秒间隔）
- 冷却时间 30 秒保证同一车辆不会短时间内被重复操作

---

## 新建文件清单

| 文件 | 说明 |
|------|------|
| `src/camera/framequeue.h` | FrameQueue 线程安全帧队列 |
| `src/camera/framequeue.cpp` | FrameQueue 实现 |
| `src/app/recognizethread.h` | RecognizeThread 识别消费者线程 |
| `src/app/recognizethread.cpp` | RecognizeThread 实现 |
| `src/app/plateconfirmtracker.h` | PlateConfirmTracker 防重复校验器 |
| `src/app/plateconfirmtracker.cpp` | PlateConfirmTracker 实现 |

## 修改文件清单

| 文件 | 改动范围 |
|------|----------|
| `src/camera/camerathread.h` | 新增 m_frameQueue 成员和 setFrameQueue() |
| `src/camera/camerathread.cpp` | run() 中新增 4 行采样逻辑 |
| `UI/MainWindow/mainwindow.h` | 新增成员和 onPlateRecognized 槽 |
| `UI/MainWindow/mainwindow.cpp` | 初始化流程、onPlateRecognized 实现、析构清理 |
| `UI/MainWindow/mainwindow.ui` | 新增自动识别开关控件 |
| `ParkingSystem.pro` | 添加新文件到 SOURCES/HEADERS |
