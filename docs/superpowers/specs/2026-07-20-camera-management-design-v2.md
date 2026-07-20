# 摄像头管理页面设计 v2(改进版)

**日期:** 2026-07-20
**状态:** 待批准
**优先级:** 中
**取代:** `2026-07-20-camera-management-design.md`(v1)
**视觉参考:** `E:\Parking-HTML\SXT.html`

---

## 1. 概述

### 1.1 目标

改进摄像头管理页面:多宫格同屏监控 1-8 个摄像头,融入 SXT.html 的视觉语言(深色科技感视频区、`Signal Lost` 空状态、半透明信息面板、底部控制栏按钮风格),并通过 `CameraManager` 单例与主页共享摄像头线程,解决索引 0 抢占冲突。

### 1.2 范围

- 新增 `CameraManager` 单例,接管所有 `CameraThread` 生命周期
- 填充 `CameraManagement` 页面(当前为空壳)为多宫格监控视图
- `CameraWindow` 改为 `QFrame` 子类,封装单摄像头显示
- `InitFile` 配置扩展:摄像头名称/位置/角色映射(JSON)
- 主窗口导航接通摄像头页面 + 摄像头线程所有权迁移到 `CameraManager`
- 新增 `styles/cameramanagement.qss`
- 更新 `ParkingSystem.pro`(SOURCES/HEADERS/RESOURCES)

### 1.3 与 v1 的差异

| 项 | v1 | v2 |
|---|---|---|
| 线程归属 | 页面直接持有 | `CameraManager` 单例共享(解决与主页抢占) |
| `CameraWindow` | `struct`(数据聚合) | `QFrame` 子类(封装显示逻辑) |
| 窗口控件 | 播放/暂停/全屏 + 时间戳 | 精简:信息面板 + 状态圆点,无按钮 |
| 名称来源 | 未明确 | `InitFile` JSON 配置映射 |
| 识别策略 | 未明确 | 只入口(`role=entry`)识别,链路不动 |
| 放大交互 | 无 | 双击窗口切独占模式 |
| 视觉 | 未明确 | 借鉴 SXT.html |

---

## 2. 决策摘要

1. **布局**:多宫格 1-8 动态自适应(沿用 v1 布局表)
2. **架构**:`CameraManager` 单例,只管线程池,不碰识别
3. **数据源**:本地枚举(`getCameraCount`)+ `InitFile` JSON 映射名称
4. **窗口**:精简(左上半透明信息面板 + 右上状态圆点,无每窗口按钮)
5. **放大**:双击窗口切独占模式(线程不断,只重排)
6. **视觉**:借鉴 SXT.html(`#1E293B` 视频区 + 网格叠加、`Signal Lost` 空状态、底部控制栏按钮风格)
7. **识别**:只入口摄像头(`role=entry`,默认索引 0)识别,识别链路仍挂主页,基于 `isVehicleInPark` 推断方向
8. **入库/出库**:不纳入本次范围,沿用现状

---

## 3. 架构设计

### 3.1 组件结构

```
src/camera/
├── camerathread.h/.cpp          # 已有,不动
├── cameramanager.h/.cpp          # 新增,单例
└── camerainfo.h                  # 新增,数据结构(或并入 cameramanager.h)

UI/CameraManagement/
├── cameramanagement.h/.cpp/.ui   # 已有空壳,填充
├── camerawindow.h/.cpp           # 新增,QFrame 子类
└── styles/
    └── cameramanagement.qss      # 新增

src/utils/initfile.h/.cpp         # 已有,扩展 getCameras()/setCameras()
UI/MainWindow/mainwindow.h/.cpp   # 已有,3 处改动
ParkingSystem.pro                 # 已有,追加新文件
```

### 3.2 CameraManager 单例

**职责:** 枚举本地摄像头、创建并持有 `CameraThread` 池(上限 8)、加载配置映射、统一 start/stop。**不碰 `FrameQueue`/识别链路**(决策 7)。

```cpp
class CameraManager : public QObject {
    Q_OBJECT
public:
    static CameraManager& instance();

    void scanCameras();           // getCameraCount() 枚举 + 创建 CameraThread(不 start) + 载配置
    int  count() const;           // 摄像头数量(≤8)
    CameraThread* getThread(int index);
    CameraInfo info(int index) const;

    void start(int index);
    void stop(int index);
    void startAll();
    void stopAll();

private:
    QList<CameraThread*> m_threads;   // parent = this
    QList<CameraInfo>    m_infos;
};
```

**生命周期:** 单例,进程退出释放。`CameraThread` 的 parent 为 `CameraManager`,生命周期由单例统一管理。

**初始化时机:** 需在 `MainWindow` 构造前完成扫描——在 `MainWindow` 构造函数首行调用 `CameraManager::instance().scanCameras()`。`scanCameras()` 内对所有线程统一 `setResolution/setTargetFps`(从 `InitFile` 读全局配置),替代原主页对索引 0 的单独设置。

### 3.3 CameraInfo 结构

```cpp
struct CameraInfo {
    int index;
    QString name;       // "主入口"   ← 配置映射
    QString location;   // "大门"
    QString role;       // "entry" | "monitor"   ← 仅 entry 喂识别队列
    int width, height, fps;  // 来自 InitFile 全局摄像头配置
};
```

`role=entry` 的摄像头喂识别队列(默认且仅一个:索引 0)。其余为 `monitor`,纯显示。

### 3.4 CameraWindow(QFrame 子类)

**封装单个摄像头的显示与状态。** 页面只管布局,不关心帧细节。

```cpp
class CameraWindow : public QFrame {
    Q_OBJECT
public:
    void bind(CameraThread* thread, const CameraInfo& info);
signals:
    void doubleClicked();
private slots:
    void onFrame(cv::Mat frame);
    void onStatus(bool connected, const QString& msg);
    void onFps(int fps);
protected:
    void mouseDoubleClickEvent(QMouseEvent*) override;  // emit doubleClicked
private:
    QLabel* m_videoLabel;     // 画面
    QLabel* m_infoLabel;      // 半透明,名称/分辨率/帧率
    QLabel* m_statusDot;      // 在线绿(脉冲)/离线红
    CameraThread* m_thread = nullptr;
};
```

无播放/暂停/全屏按钮(精简,统一由底部控制)。双击发 `doubleClicked()`。

### 3.5 CameraManagement 页面

```cpp
class CameraManagement : public QWidget {
    Q_OBJECT
public:
    explicit CameraManagement(QWidget* parent = nullptr, DatabaseManager* db = nullptr);
private:
    void setupUI();
    void updateGridLayout();     // 按 count + 当前模式重排
    int  getGridCols(int count); // 列数(见布局规则)

    QList<CameraWindow*> m_windows;
    bool m_soloMode = false;     // 独占模式
    int  m_soloIndex = -1;

    QFrame*      m_videoContainer;   // #1E293B
    QGridLayout* m_gridLayout;
    QLabel*      m_countLabel;       // "检测到 N 个摄像头 · WxH · FPS"
    QPushButton* m_startAllBtn;
    QPushButton* m_stopAllBtn;
    QPushButton* m_settingsBtn;
};
```

### 3.6 配置扩展(InitFile)

`InitFile` 使用 JSON(`QJsonObject configData`)。新增 `cameras` 数组:

```json
{
  "cameras": [
    {"index": 0, "name": "主入口",    "location": "大门",     "role": "entry"},
    {"index": 1, "name": "地下车库A", "location": "地下A区",  "role": "monitor"},
    {"index": 2, "name": "装卸区",    "location": "装卸区",   "role": "monitor"},
    {"index": 3, "name": "外围东侧",  "location": "外围东侧", "role": "monitor"}
  ]
}
```

`InitFile` 新增接口:
- `QList<CameraInfo> getCameras() const;`
- `void setCameras(const QList<CameraInfo>& cameras);`

`createDefaultConfig()` 默认含索引 0 `entry` "主入口"。`CameraManager::scanCameras()` 对未配置的摄像头回退 `name="摄像头 N"`、`role="monitor"`。分辨率/帧率沿用现有 `getCameraWidth/Height/Fps`(全局,所有摄像头共用)。

---

## 4. 数据流

```
启动:
  MainWindow 构造首行 → CameraManager::instance().scanCameras()
      └─ getCameraCount() 枚举 → 创建 N 个 CameraThread(不 start)
         → setResolution/setTargetFps(从 InitFile)→ getCameras() 映射名称

  MainWindow:
    m_cameraThread = CameraManager::instance().getThread(0)   // 入口
    connect(newFrameCaptured → updateFrame)                  // 主页显示(不变)
    setFrameQueue(m_frameQueue)                              // 喂识别(不变)
    CameraManager::instance().start(0)                       // 启动入口
    [FrameQueue / RecognizeThread / PlateConfirmTracker 完全不动]

  CameraManagement 页面(用户点导航进入):
    为每个 i: CameraWindow::bind(getThread(i), info(i))
    底部"开启所有" → startAll()
    底部"停止所有" → stopAll()
    双击窗口 i    → m_soloMode=true, m_soloIndex=i, updateGridLayout()(线程不断)
    再双击        → m_soloMode=false, updateGridLayout()
```

**帧显示:** 信号槽 `QueuedConnection`,主线程执行。索引 0 的 `newFrameCaptured` 被多路订阅:主页 `updateFrame`(显示)+ `setFrameQueue`(识别投递,在 `CameraThread::run` 内部)+ 摄像头页 `CameraWindow::onFrame`(显示)。识别只发生一次。

---

## 5. 布局规则

多宫格(沿用 v1):

| 数量 | 布局 | 数量 | 布局 |
|---|---|---|---|
| 1 | 1×1 | 5 | 上 3 下 2 居中 |
| 2 | 1×2 | 6 | 2×3 |
| 3 | 上 2 下 1(底部跨 2 列) | 7 | 3×3(末行 1 居中) |
| 4 | 2×2 | 8 | 3×3(末行 2 居中) |

**独占模式:** `m_soloMode` 时,视频区只显示 `m_soloIndex` 对应的 `CameraWindow`(1×1 占满),其余隐藏。线程不断,只重排布局,无重连开销。

---

## 6. 视觉设计(借鉴 SXT.html)

- **视频容器:** `#1E293B` 深色 + 径向渐变(`slate-800 → #1E293B`)+ 40px 网格叠加(opacity 0.03)
- **空状态:** `videocam_off` 图标 + "Signal Lost / Awaiting Connection"(摄像头未连接或停止时)
- **信息面板:** 左上,半透明黑底白字,名称/分辨率/帧率
- **状态圆点:** 右上,在线绿(脉冲动画)/离线红
- **底部控制栏:** 左侧"检测到 N 个摄像头 · WxH · FPS",右侧"开启所有"(`#003FB1` 主色实心)/ "停止所有"(白底 `#DC2626` 红边)/ "设置"(灰)
- **配色:** Material Design 3,与 [styles/userManagement.qss](../../../styles/userManagement.qss) 一致
- **图标:** 复用 `:/icons/`(`videocam`/`play_arrow`/`power_settings_new`/`videocam_off` 等);缺失则新增 SVG 到 `UI/imageQrc/`
- **样式文件:** `styles/cameramanagement.qss`

---

## 7. 与主页集成改动

**mainwindow.cpp:**

1. **构造首行:** 新增 `CameraManager::instance().scanCameras();`
2. **L152:** `m_cameraThread = new CameraThread(0, this)` → `m_cameraThread = CameraManager::instance().getThread(0);`
3. **L154-157:** `setResolution/setTargetFps` 删除(已在 `scanCameras` 统一设置)
4. **L159:** `m_cameraThread->start()` → `CameraManager::instance().start(0);`
5. **L218-235:** 导航接通——`addWidget(new CameraManagement(this, m_db))` 索引 3,`cameraManagementButton` → `setCurrentIndex(3)`
6. **析构 L247-250:** 删除 `m_cameraThread->quit()/wait()`(归 `CameraManager`);`RecognizeThread` stop 保留;`FrameQueue` 释放保留。**停止顺序:** 先 `RecognizeThread` → 再由 `CameraManager` 单例退出时释放线程

**mainwindow.h:** 新增 `CameraManagement* m_cameraManagementPage = nullptr;` + 前向声明

**ParkingSystem.pro:** 追加 `cameramanager.h/.cpp`、`camerawindow.h/.cpp`、`styles/cameramanagement.qss`(RESOURCES)

---

## 8. 错误处理

- **摄像头不可用:** 该 `CameraWindow` 显示 `Signal Lost` + 红圆点,不影响其他窗口
- **超过 8 个:** 警告对话框,仅使用前 8 个
- **无摄像头:** 整个视频区显示 `Signal Lost`
- **入口(索引 0)不可用:** 识别暂停,主页提示"入口摄像头离线"
- **配置缺 `cameras`:** 用默认(索引 0 `entry` "主入口"),其余 `monitor`

---

## 9. 性能考虑

- 每摄像头独立 `CameraThread`(已有)
- 帧显示在主线程(`QueuedConnection`)
- 独占模式只重排布局,线程不断,无重连开销
- 限制帧率(`setTargetFps`,沿用)
- `QPixmap` 缓存图标资源

---

## 10. 测试策略

**单元测试:**
- `CameraManager::scanCameras()`(mock `getCameraCount`)
- `CameraManagement::getGridCols()`(1-8 各数量)
- `InitFile::getCameras()/setCameras()` 往返

**集成测试:**
- 1-8 布局切换
- 双击独占 / 恢复
- 空状态(摄像头不可用)
- `startAll/stopAll`
- 主页与摄像头页共享索引 0(无抢占)

**UI 测试:**
- 样式应用
- 按钮交互

---

## 11. 范围外(本次不做)

**入库/出库逻辑:** 沿用主页 `onPlateRecognized` 基于 `isVehicleInPark` 推断方向,完全不动。其机制为:入口摄像头既看进也看出——车进时识别(不在库→入库),车出时同一摄像头再识别(在库→出库)。其风险(漏检导致误入库、单入口语义假设)为现有设计固有特性,不在本次范围。摄像头管理页面不参与出入库,只显示画面。

---

## 12. 未来扩展

- 双摄像头方向识别(入口/出口分别 `role=in/out`,解决单入口推断风险)
- 漏检恢复(车进未识别→出时识别的状态机校正)
- RTSP 网络摄像头
- 摄像头拖拽排序 / 自定义布局
- 录制 / 截图
- 配置 UI(可视化管理 `cameras` 列表)
- 多摄像头并行识别(策略 2/3)

---

## 13. 依赖项

- Qt 6.10.2(Core, Widgets)
- OpenCV 4.x
- 现有 `CameraThread` / `DatabaseManager` / `InitFile` / `RecognizeThread` / `PlateConfirmTracker`
- `SXT.html`(视觉参考)
