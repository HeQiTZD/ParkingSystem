# 摄像头管理页面设计

**日期:** 2026-07-20
**状态:** 待批准
**优先级:** 中

## 1. 概述

### 1.1 目标

创建一个独立的摄像头管理页面，支持动态多宫格视图布局，同时显示最多8个摄像头的实时画面。

### 1.2 范围

- 新增 `UI/CameraManagement/` 目录和对应的 `CameraManagement` 类
- 复用现有的 `CameraThread` 进行视频捕获
- 实现1-8个摄像头的动态自适应布局
- 提供统一的控制栏和独立窗口控制

## 2. 架构设计

### 2.1 组件结构

```
UI/CameraManagement/
├── cameramanagement.h          # 主窗口头文件
├── cameramanagement.cpp        # 主窗口实现
└── styles/
    └── cameramanagement.qss    # 样式表
```

### 2.2 类设计

#### CameraManagement (主窗口)

**职责:** 摄像头管理页面的顶层容器，管理所有摄像头窗口和控制栏

**继承:** `QWidget`

**主要成员:**

```cpp
class CameraManagement : public QWidget
{
    Q_OBJECT

public:
    explicit CameraManagement(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~CameraManagement();

    void scanCameras();           // 扫描并初始化摄像头
    void startAllCameras();       // 启动所有摄像头
    void stopAllCameras();        // 停止所有摄像头

private:
    void setupUI();               // 初始化UI
    void updateGridLayout();      // 更新网格布局
    QString getGridClass(int count); // 根据摄像头数量返回布局类名

private slots:
    void onCameraFrameReady(int index, cv::Mat frame);  // 摄像头帧更新
    void onCameraStatusChanged(int index, bool connected);  // 摄像头状态变化
    void onStartAllClicked();      // 开启所有监控
    void onStopAllClicked();       // 停止所有运行
    void onSettingsClicked();      // 设置按钮

private:
    Ui::CameraManagement *ui;
    QGridLayout *m_gridLayout;     // 网格布局
    QFrame *m_videoContainer;      // 视频容器
    QLabel *m_cameraCountLabel;    // 摄像头数量显示
    QLabel *m_resolutionLabel;     // 分辨率显示
    QLabel *m_fpsLabel;            // 帧率显示
    QPushButton *m_startAllBtn;    // 开启监控按钮
    QPushButton *m_stopAllBtn;     // 停止运行按钮
    QPushButton *m_settingsBtn;    // 设置按钮

    QList<CameraWindow> m_cameras; // 摄像头窗口列表
    int m_cameraCount;             // 摄像头总数
};
```

#### CameraWindow (摄像头窗口)

**职责:** 单个摄像头视频显示窗口，包含视频显示、状态信息和独立控制

**继承:** `QFrame`

**主要成员:**

```cpp
struct CameraWindow {
    int cameraIndex;               // 摄像头索引
    QString name;                  // 摄像头名称
    QString location;              // 位置描述
    int width;                     // 分辨率宽度
    int height;                    // 分辨率高度
    int fps;                       // 帧率
    bool isConnected;              // 是否连接
    bool isPlaying;                // 是否播放中
    CameraThread *thread;          // 摄像头线程
    QLabel *videoLabel;            // 视频显示标签
    QLabel *nameLabel;             // 名称标签
    QLabel *statusLabel;           // 状态标签
    QLabel *resolutionLabel;       // 分辨率标签
    QLabel *fpsLabel;              // 帧率标签
    QLabel *timeLabel;             // 时间戳标签
    QPushButton *playBtn;          // 播放按钮
    QPushButton *pauseBtn;         // 暂停按钮
    QPushButton *fullscreenBtn;    // 全屏按钮
};
```

### 2.3 数据流

```
CameraThread (后台线程)
    ↓ newFrameCaptured(cv::Mat)
CameraManagement::onCameraFrameReady()
    ↓ 更新对应的CameraWindow
CameraWindow::videoLabel 显示图像
```

## 3. 动态布局规则

| 摄像头数量 | 布局方式 | QGridLayout规格 |
|-----------|---------|-----------------|
| 1 | 独占全屏 | 1×1 |
| 2 | 左右分屏 | 1×2 |
| 3 | 上二下一（底部大屏） | 2×2, 第三个占第二行两列 |
| 4 | 2×2四宫格 | 2×2 |
| 5 | 第一行3个，第二行2个居中 | 2×3, 第三个和第四个空 |
| 6 | 3×2六宫格 | 2×3 |
| 7 | 3×3，前两行各3个，第三行1个居中 | 3×3, 部分为空 |
| 8 | 3×3，前两行各3个，第三行2个居中 | 3×3 |

## 4. UI设计

### 4.1 视频区域

- 背景色: `#1E293B` (深蓝灰色)
- 视频窗口间距: 12px
- 圆角: 8px
- 支持自适应缩放

### 4.2 单个摄像头窗口

**左上角状态信息 (半透明黑底白字):**
- 摄像头名称（图标 + 文字）
- 分辨率（图标 + 文字）
- 帧率（图标 + 文字）

**右上角状态指示器:**
- 在线：绿色背景 + 脉冲动画圆点
- 离线：红色背景

**左下角控制按钮:**
- 播放按钮 (play_arrow)
- 暂停按钮 (pause)
- 全屏按钮 (fullscreen)

**右下角时间戳:**
- 格式: `YYYY-MM-DD HH:MM:SS`
- 等宽字体
- 每秒更新

### 4.3 底部控制栏

**左侧信息区:**
- 检测到的摄像头数量
- 统一分辨率显示
- 统一帧率显示

**右侧按钮区:**
- 开启监控 (蓝色背景)
- 停止运行 (白色背景 + 红色边框)
- 设置按钮 (灰色背景)

## 5. 错误处理

### 5.1 摄像头不可用

- 显示 "Signal Lost / Awaiting Connection" 空状态
- 状态指示器显示红色
- 播放/暂停按钮禁用

### 5.2 摄像头数量超限

- 限制最多8个摄像头
- 超过8个时显示警告对话框
- 仅使用前8个摄像头

### 5.3 分辨率不匹配

- 尝试使用默认分辨率 640×480
- 在状态标签中显示警告信息

## 6. 性能考虑

- 使用独立线程进行视频捕获 (复用 `CameraThread`)
- 图像显示在主线程 (Qt要求)
- 使用信号槽机制进行线程间通信
- 限制帧率避免CPU过载
- 使用 `QPixmapCache` 缓存图标资源

## 7. 测试策略

### 7.1 单元测试

- `CameraManagement::scanCameras()` - 测试摄像头检测
- `CameraManagement::updateGridLayout()` - 测试布局更新
- `CameraManagement::getGridClass()` - 测试布局类名生成

### 7.2 集成测试

- 测试1-8个摄像头的布局切换
- 测试播放/暂停功能
- 测试全屏功能
- 测试状态更新

### 7.3 UI测试

- 测试所有摄像头数量的显示效果
- 测试按钮交互
- 测试样式应用

## 8. 依赖项

- Qt 6.10.2 (Core, Widgets)
- OpenCV 4.x (用于视频处理)
- 现有 `CameraThread` 类
- 现有 `DatabaseManager` 类 (可选，用于持久化摄像头配置)

## 9. 未来扩展

- 支持摄像头拖拽排序
- 支持自定义布局
- 支持摄像头录制
- 支持画面截图
- 支持摄像头配置保存和加载
- 支持网络摄像头 (RTSP流)