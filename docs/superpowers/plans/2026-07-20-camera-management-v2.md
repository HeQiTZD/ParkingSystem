# 摄像头管理页面 v2 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把空壳的摄像头管理页面实现为多宫格监控视图,通过 CameraManager 单例与主页共享摄像头线程,融入 SXT.html 视觉语言。

**Architecture:** 自底向上分层——CameraInfo 数据结构 → InitFile 配置扩展 → CameraThread::stop() → CameraManager 单例 → CameraWindow(QFrame) → CameraManagement 页面 → 主窗口集成。每层独立编译可提交。

**Tech Stack:** Qt 6.10.2 (Widgets/Core/Sql/Svg)、OpenCV 4.11 (cv::Mat)、MinGW 13.1.0、qmake。配置文件 JSON(config.json)。

**参考文档:** `docs/superpowers/specs/2026-07-20-camera-management-design-v2.md`

**测试约定:** 本项目**无测试框架**(无 tests/ 目录、无 QTest 子工程)。本计划采用"实现 → 编译验证 → 运行程序手动验证 → 提交"模式,不写自动化测试。编译首选在 Qt Creator 中构建(Ctrl+B),命令行可用 `C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe` 在已 qmake 的构建目录执行。运行程序手动观察每个验证步骤描述的行为。

---

## 文件结构

| 文件 | 操作 | 职责 |
|---|---|---|
| `src/camera/camerainfo.h` | 新建 | CameraInfo 纯数据结构(无循环依赖) |
| `src/utils/initfile.h/.cpp` | 修改 | 加 `getCameras()/setCameras()` + 默认 cameras 数组 |
| `src/camera/camerathread.h/.cpp` | 修改 | 加 `public stop()`(封装停止逻辑) |
| `src/camera/cameramanager.h/.cpp` | 新建 | 单例:线程池管理 |
| `UI/imageQrc/videocam_off.svg` | 新建 | 空状态图标 |
| `UI/imageQrc/power_settings_new.svg` | 新建 | 停止运行按钮图标 |
| `UI/imageQrc/image.qrc` | 修改 | 注册两个新图标 |
| `UI/CameraManagement/camerawindow.h/.cpp` | 新建 | QFrame 子类:单摄像头显示 |
| `styles/cameramanagement.qss` | 新建 | 页面样式 |
| `styles/styles.qrc` | 修改 | 注册 qss |
| `UI/CameraManagement/cameramanagement.h/.cpp` | 修改 | 填充页面:UI+布局+绑定+独占+错误处理 |
| `UI/MainWindow/mainwindow.h/.cpp` | 修改 | scanCameras+getThread(0)+导航接通+析构改动 |
| `ParkingSystem.pro` | 修改 | 追加新文件到 SOURCES/HEADERS/RESOURCES |

<!-- PLAN-CONTINUE -->

---

## Task 1: CameraInfo 结构 + InitFile 配置扩展

**Files:**
- Create: `src/camera/camerainfo.h`
- Modify: `src/utils/initfile.h`
- Modify: `src/utils/initfile.cpp`
- Modify: `ParkingSystem.pro`

- [ ] **Step 1: 新建 `src/camera/camerainfo.h`**

```cpp
#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include <QString>

// 单个摄像头的描述信息
// - role="entry" 的摄像头喂识别队列(默认且仅一个,索引 0)
// - role="monitor" 的摄像头纯显示
struct CameraInfo {
    int index = 0;          // 摄像头索引(对应 CameraThread 的 cameraIndex)
    QString name;           // 显示名,如 "主入口"
    QString location;       // 位置描述,如 "大门"
    QString role;           // "entry" | "monitor"
    int width  = 1920;      // 采集宽度
    int height = 1080;      // 采集高度
    int fps    = 30;        // 帧率
};

#endif // CAMERAINFO_H
```

- [ ] **Step 2: 修改 `src/utils/initfile.h`,加 QJsonArray include 与两个方法声明**

在 `#include <QJsonObject>` 之后加一行:

```cpp
#include <QJsonArray>
```

在 `//摄像头设置` 区块的 `void setCameraConfig(...)` 声明之后,加入:

```cpp
    //摄像头列表(多摄像头管理)
    QJsonArray getCameras() const;              // 返回 cameras 数组
    void setCameras(const QJsonArray &cameras); // 写入 cameras 数组
```

- [ ] **Step 3: 修改 `src/utils/initfile.cpp`,实现两个方法**

在 `setCameraConfig` 实现之后(`//获取识别配置` 注释之前)加入:

```cpp
//获取摄像头列表配置
QJsonArray InitFile::getCameras() const
{
    return configData["cameras"].toArray();
}

void InitFile::setCameras(const QJsonArray &cameras)
{
    configData["cameras"] = cameras;
    qDebug() << QStringLiteral("摄像头列表配置已更新");
}
```

- [ ] **Step 4: 修改 `src/utils/initfile.cpp` 的 `getDefaultConfig()`,加默认 cameras 数组**

在 `getDefaultConfig()` 内、`defaultConfig["camera"] = cameraConfig;` 之后加入:

```cpp
    //摄像头列表默认配置(索引 0 为入口,负责识别)
    QJsonArray camerasArray;
    QJsonObject cam0;
    cam0["index"] = 0;
    cam0["name"] = QStringLiteral("主入口");
    cam0["location"] = QStringLiteral("大门");
    cam0["role"] = "entry";
    camerasArray.append(cam0);
    defaultConfig["cameras"] = camerasArray;
```

注意:`validateConfig()` 的 `requiredSections` **不加** `cameras`——cameras 是可选字段,旧 config.json 无此字段时 `getCameras()` 返回空数组,由 CameraManager 回退默认。

- [ ] **Step 5: 修改 `ParkingSystem.pro`,把 `camerainfo.h` 加入 HEADERS**

在 HEADERS 的 `src/camera/camerathread.h \` 行之后加入:

```
    src/camera/camerainfo.h \
```

- [ ] **Step 6: 编译验证**

在 Qt Creator 中 Ctrl+B 构建。预期:编译通过(`getCameras/setCameras` 暂无调用方,但不影响编译)。

- [ ] **Step 7: 提交**

```bash
git add src/camera/camerainfo.h src/utils/initfile.h src/utils/initfile.cpp ParkingSystem.pro
git commit -m "feat(config): 新增 CameraInfo 结构与 InitFile cameras 配置"
```

---

## Task 2: CameraThread::stop() 公开方法

**Files:**
- Modify: `src/camera/camerathread.h`
- Modify: `src/camera/camerathread.cpp`

**背景:** 当前停止逻辑封装在 `~CameraThread()`(camerathread.cpp:19-33),靠 `m_running=false`+`m_pauseCond.wakeAll()`+`wait(3000)`。`CameraManager::stop(int)` 需要能停止线程而不析构它(线程由单例长期持有)。把停止逻辑抽成 public `stop()`,析构函数复用它。

- [ ] **Step 1: 修改 `src/camera/camerathread.h`,在 public 区加 stop() 声明**

在 `void resume();` 声明之后加入:

```cpp
    /**
     * @brief 停止捕获并等待线程结束(不析构对象)
     *
     * 复用于析构函数与 CameraManager。线程停止后可通过 start() 再次启动
     * (QThread 规则:run() 返回后可重新 start())。
     */
    void stop();
```

- [ ] **Step 2: 修改 `src/camera/camerathread.cpp`,实现 stop() 并让析构复用它**

把现有析构函数(camathread.cpp:19-33)整体替换为:

```cpp
CameraThread::~CameraThread()
{
    stop();
    releaseCamera();
}

void CameraThread::stop()
{
    m_running = false;
    m_paused = false;
    m_pauseCond.wakeAll();

    if(QThread::isRunning()){
        quit();   // 退出事件循环(本线程用 while 循环无事件循环,保留以兼容)
        wait(3000);
    }
}
```

- [ ] **Step 3: 编译验证**

Qt Creator Ctrl+B。预期:编译通过,行为与原析构一致(停止逻辑只是搬到 stop())。

- [ ] **Step 4: 运行验证**

启动程序(Qt Creator Ctrl+R),登录进主页,摄像头画面正常显示(证明 stop() 重构未破坏现有主页摄像头)。关闭程序正常退出。

- [ ] **Step 5: 提交**

```bash
git add src/camera/camerathread.h src/camera/camerathread.cpp
git commit -m "refactor(camera): 抽取 CameraThread::stop() 公开方法"
```

---

## Task 3: CameraManager 单例

**Files:**
- Create: `src/camera/cameramanager.h`
- Create: `src/camera/cameramanager.cpp`
- Modify: `ParkingSystem.pro`

- [ ] **Step 1: 新建 `src/camera/cameramanager.h`**

```cpp
#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QList>
#include "camerainfo.h"

class CameraThread;

/**
 * @brief 摄像头管理单例
 *
 * 职责单一:枚举本地摄像头、创建并持有 CameraThread 池(上限 8)、
 * 加载 InitFile 映射名称、统一 start/stop。
 *
 * 不碰 FrameQueue/识别链路(识别由主页在入口摄像头 0 上自行挂接)。
 *
 * 生命周期:单例,进程退出释放。CameraThread 的 parent 为本单例,
 * 生命周期由单例统一管理。CameraManager 无法热插拔摄像头(枚举在 scanCameras 一次性完成)。
 */
class CameraManager : public QObject
{
    Q_OBJECT
public:
    static CameraManager& instance();

    /// 枚举本地摄像头、创建 CameraThread(不 start)、载配置映射名称。
    /// 需在 MainWindow 构造前调用一次。
    void scanCameras();

    int  count() const;                 // 摄像头数量(≤8)
    CameraThread* getThread(int index); // nullptr 表示 index 越界
    CameraInfo    info(int index) const; // 越界返回默认 CameraInfo

    void start(int index);              // 启动指定摄像头
    void stop(int index);               // 停止指定摄像头
    void startAll();                    // 启动所有
    void stopAll();                     // 停止所有

private:
    CameraManager(QObject *parent = nullptr);
    ~CameraManager();
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    QList<CameraThread*> m_threads;
    QList<CameraInfo>    m_infos;

    static const int kMaxCameras = 8;
};

#endif // CAMERAMANAGER_H
```

- [ ] **Step 2: 新建 `src/camera/cameramanager.cpp`**

```cpp
#include "cameramanager.h"
#include "camerathread.h"
#include "initfile.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

CameraManager& CameraManager::instance()
{
    static CameraManager instance;
    return instance;
}

CameraManager::CameraManager(QObject *parent) : QObject(parent) {}

CameraManager::~CameraManager()
{
    stopAll();
    qDeleteAll(m_threads);
    m_threads.clear();
}

void CameraManager::scanCameras()
{
    if(!m_threads.isEmpty()){
        return;  // 已扫描,避免重复
    }

    int available = CameraThread::getCameraCount();
    int n = qMin(available, kMaxCameras);

    // 从 InitFile 读全局分辨率/帧率
    int w = InitFile::instance().getCameraWidth();
    int h = InitFile::instance().getCameraHeight();
    int fps = InitFile::instance().getCameraFps();

    // 从 InitFile 读 cameras 映射(index → name/location/role)
    QJsonArray camerasCfg = InitFile::instance().getCameras();

    for(int i = 0; i < n; ++i){
        CameraThread* t = new CameraThread(i, this);
        t->setResolution(w, h);
        t->setTargetFps(fps);

        CameraInfo info;
        info.index = i;
        info.width = w;
        info.height = h;
        info.fps = fps;

        // 默认值
        info.name = QStringLiteral("摄像头 %1").arg(i + 1);
        info.location = QStringLiteral("位置 %1").arg(i + 1);
        info.role = (i == 0) ? "entry" : "monitor";

        // 用配置覆盖默认(按 index 匹配)
        for(const QJsonValue& v : camerasCfg){
            QJsonObject o = v.toObject();
            if(o["index"].toInt() == i){
                info.name     = o["name"].toString();
                info.location = o["location"].toString();
                info.role     = o["role"].toString();
                break;
            }
        }

        m_threads.append(t);
        m_infos.append(info);
    }

    qDebug() << QStringLiteral("CameraManager: 扫描到 %1 个摄像头").arg(m_threads.size());
}

int CameraManager::count() const
{
    return m_threads.size();
}

CameraThread* CameraManager::getThread(int index)
{
    if(index < 0 || index >= m_threads.size()){
        return nullptr;
    }
    return m_threads[index];
}

CameraInfo CameraManager::info(int index) const
{
    if(index < 0 || index >= m_infos.size()){
        return CameraInfo();
    }
    return m_infos[index];
}

void CameraManager::start(int index)
{
    if(CameraThread* t = getThread(index)){
        if(!t->isRunning()){
            t->start();
        }
    }
}

void CameraManager::stop(int index)
{
    if(CameraThread* t = getThread(index)){
        t->stop();
    }
}

void CameraManager::startAll()
{
    for(int i = 0; i < m_threads.size(); ++i){
        start(i);
    }
}

void CameraManager::stopAll()
{
    for(int i = 0; i < m_threads.size(); ++i){
        stop(i);
    }
}
```

- [ ] **Step 3: 修改 `ParkingSystem.pro`,加 cameramanager 到 SOURCES/HEADERS**

SOURCES 中 `src/camera/framequeue.cpp \` 之后加:

```
    src/camera/cameramanager.cpp \
```

HEADERS 中 `src/camera/framequeue.h \` 之后加:

```
    src/camera/cameramanager.h \
```

- [ ] **Step 4: 编译验证**

Qt Creator Ctrl+B。预期:编译通过(CameraManager 暂无调用方)。

- [ ] **Step 5: 提交**

```bash
git add src/camera/cameramanager.h src/camera/cameramanager.cpp ParkingSystem.pro
git commit -m "feat(camera): 新增 CameraManager 单例管理摄像头线程池"
```

---

## Task 4: 新增图标资源

**Files:**
- Create: `UI/imageQrc/videocam_off.svg`
- Create: `UI/imageQrc/power_settings_new.svg`
- Modify: `UI/imageQrc/image.qrc`

图标用 Material Symbols 风格(与现有 `pause.svg` 一致:24px、viewBox 0 -960 960 960、fill #434343)。

- [ ] **Step 1: 新建 `UI/imageQrc/videocam_off.svg`**

```svg
<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#434343"><path d="m612-432-44-44v-204H364l-44-44h248v-80q0-17 11.5-28.5T608-844q17 0 28.5 11.5T648-804v372ZM790-72l-42-42H480q-17 0-28.5-11.5T440-154q0-17 11.5-28.5T480-194h184L264-594v330q0 17-11.5 28.5T224-224q-17 0-28.5-11.5T184-264v-90L72-790l42-42 718 718-42 42ZM612-432Z"/></svg>
```

- [ ] **Step 2: 新建 `UI/imageQrc/power_settings_new.svg`**

```svg
<svg xmlns="http://www.w3.org/2000/svg" height="24px" viewBox="0 -960 960 960" width="24px" fill="#434343"><path d="M480-440v-360h80v280l-80 80ZM440-80v-160h80v160h-80ZM260-240 200-300l56-56 60 60-56 56ZM124-380v-80h160v80H124Zm552 0-2-2 0-2 2 2 0 2ZM700-240l-56-56 60-60 56 56-60 60ZM440-480ZM124-580v-80h160v80H124Zm124-124 56-56 60 60-56 56-60-60ZM699-699l-56-56 60-60 56 56-60 60Z"/></svg>
```

> 说明:`power_settings_new.svg` 采用简化双圆弧+电源符号占位 path(因精确官方 path 较长)。执行时若需更精确图标,可从 Material Symbols 官方下载同名图标替换内容,文件名与 qrc 引用不变。

- [ ] **Step 3: 修改 `UI/imageQrc/image.qrc`,注册两个新图标**

在 `<file>videocam.svg</file>` 之后加入:

```xml
        <file>videocam_off.svg</file>
```

在 `<file>play_arrow.svg</file>` 之后加入:

```xml
        <file>power_settings_new.svg</file>
```

- [ ] **Step 4: 编译验证**

Qt Creator Ctrl+B(触发 rcc 重新编译资源)。预期:编译通过,新资源注册为 `:/icons/videocam_off.svg`、`:/icons/power_settings_new.svg`。

- [ ] **Step 5: 运行验证(快速核对资源可加载)**

在 main.cpp 临时加一行(验证后删除):`qDebug() << QIcon(":/icons/videocam_off.svg").isNull();`——应输出 `false`(非空)。或直接信任 rcc 编译成功。本步骤可选,资源加载在后续 Task 会被实际使用验证。

- [ ] **Step 6: 提交**

```bash
git add UI/imageQrc/videocam_off.svg UI/imageQrc/power_settings_new.svg UI/imageQrc/image.qrc
git commit -m "feat(icons): 新增 videocam_off 与 power_settings_new 图标"
```

---

## Task 5: CameraWindow(QFrame 子类)

**Files:**
- Create: `UI/CameraManagement/camerawindow.h`
- Create: `UI/CameraManagement/camerawindow.cpp`
- Modify: `ParkingSystem.pro`

**设计要点:**
- `videoLabel` 显示画面(`Qt::KeepAspectRatio`,黑底由于父容器已是 `#1E293B`)
- `infoPanel` 左上半透明黑底白字:`name · WxH · FPS`
- `statusDot` 右上圆点:在线绿/离线红(`offline` 时 `videoLabel` 显示 `videocam_off` + "Signal Lost / Awaiting Connection")
- `bind(thread, info)` 连接 `newFrameCaptured/cameraStatusChanged/fpsUpdated`
- `resizeEvent` 重排浮层(infoPanel/statusDot 用 absolute geometry)
- 双击发 `doubleClicked()`

- [ ] **Step 1: 新建 `UI/CameraManagement/camerawindow.h`**

```cpp
#ifndef CAMERAWINDOW_H
#define CAMERAWINDOW_H

#include <QFrame>
#include "src/camera/camerainfo.h"

class QLabel;
class CameraThread;
class cv::Mat;

class CameraWindow : public QFrame
{
    Q_OBJECT
public:
    explicit CameraWindow(QWidget *parent = nullptr);
    ~CameraWindow();

    void bind(CameraThread *thread, const CameraInfo &info);

signals:
    void doubleClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void onFrame(cv::Mat frame);
    void onStatus(bool connected, const QString &msg);
    void onFps(int fps);

private:
    void showEmptyState(const QString &text);

    QLabel      *m_videoLabel = nullptr;
    QLabel      *m_infoLabel  = nullptr;   // 半透明信息面板
    QLabel      *m_statusDot  = nullptr;   // 状态圆点
    CameraThread *m_thread    = nullptr;
    CameraInfo   m_info;
    bool         m_connected = false;
};

#endif // CAMERAWINDOW_H
```

- [ ] **Step 2: 新建 `UI/CameraManagement/camerawindow.cpp`**

```cpp
#include "camerawindow.h"
#include "src/camera/camerathread.h"
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QImage>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

CameraWindow::CameraWindow(QWidget *parent) : QFrame(parent)
{
    // 边框/圆角由 qss(#cameraWindow) 控制
    setObjectName("cameraWindow");
    setAttribute(Qt::WA_StyledBackground, true);

    // 视频显示
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setScaledContents(false);
    m_videoLabel->setStyleSheet("background: transparent;");

    // 左上信息面板(半透明黑底白字)
    m_infoLabel = new QLabel(this);
    m_infoLabel->setObjectName("cameraInfoPanel");
    m_infoLabel->raise();

    // 右上状态圆点
    m_statusDot = new QLabel(this);
    m_statusDot->setObjectName("cameraStatusDot");
    m_statusDot->setFixedSize(12, 12);
    m_statusDot->raise();

    showEmptyState(QStringLiteral("Signal Lost / Awaiting Connection"));
}

CameraWindow::~CameraWindow()
{
    if(m_thread){
        disconnect(m_thread, &CameraThread::newFrameCaptured, this, &CameraWindow::onFrame);
        disconnect(m_thread, &CameraThread::cameraStatusChanged, this, &CameraWindow::onStatus);
        disconnect(m_thread, &CameraThread::fpsUpdated, this, &CameraWindow::onFps);
    }
}

void CameraWindow::bind(CameraThread *thread, const CameraInfo &info)
{
    if(m_thread){
        disconnect(m_thread, &CameraThread::newFrameCaptured, this, &CameraWindow::onFrame);
        disconnect(m_thread, &CameraThread::cameraStatusChanged, this, &CameraWindow::onStatus);
        disconnect(m_thread, &CameraThread::fpsUpdated, this, &CameraWindow::onFps);
    }
    m_thread = thread;
    m_info = info;

    if(m_thread){
        connect(m_thread, &CameraThread::newFrameCaptured, this, &CameraWindow::onFrame);
        connect(m_thread, &CameraThread::cameraStatusChanged, this, &CameraWindow::onStatus);
        connect(m_thread, &CameraThread::fpsUpdated, this, &CameraWindow::onFps);

        // 首次显示信息
        m_infoLabel->setText(QStringLiteral("%1 · %2x%3 · %4 FPS")
                                 .arg(m_info.name)
                                 .arg(m_info.width).arg(m_info.height)
                                 .arg(m_info.fps));
        m_connected = false;
        m_statusDot->setProperty("status", "offline");
        m_statusDot->style()->unpolish(m_statusDot);
        m_statusDot->style()->polish(m_statusDot);
    }
}

void CameraWindow::onFrame(cv::Mat frame)
{
    if(frame.empty()) return;
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    QPixmap pm = QPixmap::fromImage(img.copy());
    m_videoLabel->setPixmap(pm.scaled(m_videoLabel->size(),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation));
}

void CameraWindow::onStatus(bool connected, const QString &msg)
{
    Q_UNUSED(msg);
    m_connected = connected;
    m_statusDot->setProperty("status", connected ? "online" : "offline");
    m_statusDot->style()->unpolish(m_statusDot);
    m_statusDot->style()->polish(m_statusDot);

    if(!connected){
        showEmptyState(QStringLiteral("Signal Lost / Awaiting Connection"));
    }
}

void CameraWindow::onFps(int fps)
{
    m_info.fps = fps;
    m_infoLabel->setText(QStringLiteral("%1 · %2x%3 · %4 FPS")
                             .arg(m_info.name)
                             .arg(m_info.width).arg(m_info.height)
                             .arg(fps));
}

void CameraWindow::showEmptyState(const QString &text)
{
    m_videoLabel->setText(
        QStringLiteral("<div style='text-align:center;'>"
                       "<img src=':/icons/videocam_off.svg' width='48' height='48'/><br>"
                       "<span style='color:#94A3B8; font-size:13px; letter-spacing:2px;'>%1</span>"
                       "</div>").arg(text));
}

void CameraWindow::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    QSize s = size();
    m_videoLabel->setGeometry(rect());
    // 左上浮层
    m_infoLabel->adjustSize();
    int panelW = m_infoLabel->width() + 24;
    int panelH = m_infoLabel->height() + 16;
    m_infoLabel->setGeometry(8, 8, panelW, panelH);
    // 右上圆点
    m_statusDot->setGeometry(s.width() - 24, 12, 12, 12);
}

void CameraWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit doubleClicked();
}
```

- [ ] **Step 3: 修改 `ParkingSystem.pro`,加 camerawindow 到 SOURCES/HEADERS**

SOURCES 中 `UI/CameraManagement/cameramanagement.cpp \` 之后加:

```
    UI/CameraManagement/camerawindow.cpp \
```

HEADERS 中 `UI/CameraManagement/cameramanagement.h \` 之后加:

```
    UI/CameraManagement/camerawindow.h \
```

- [ ] **Step 4: 编译验证**

Qt Creator Ctrl+B。预期:编译通过(CameraWindow 暂无 UI 调用方,bind/onFrame 逻辑静态可查)。

- [ ] **Step 5: 提交**

```bash
git add UI/CameraManagement/camerawindow.h UI/CameraManagement/camerawindow.cpp ParkingSystem.pro
git commit -m "feat(camera): 新增 CameraWindow 单摄像头显示组件"
```

---

## Task 6: 摄像头管理页面样式表

**Files:**
- Create: `styles/cameramanagement.qss`
- Modify: `styles/styles.qrc`

- [ ] **Step 1: 新建 `styles/cameramanagement.qss`**

```css
/* ── 视频容器(深色科技感) ── */
#videoContainer {
    background: #1E293B;
    border: 1px solid #E2E8F0;
    border-radius: 12px;
}

/* ── 底部控制栏 ── */
#controlBar {
    background: #ffffff;
    border: 1px solid #E2E8F0;
    border-radius: 12px;
}

#countLabel {
    color: #5C647A;
    font-size: 13px;
    font-weight: 500;
}

#startAllBtn {
    background-color: #003FB1;
    color: white;
    border: none;
    border-radius: 6px;
    padding: 8px 24px;
    font-size: 14px;
    font-weight: 500;
}
#startAllBtn:hover { background-color: #003DAB; }

#stopAllBtn {
    background-color: #ffffff;
    color: #DC2626;
    border: 1px solid #FECACA;
    border-radius: 6px;
    padding: 8px 20px;
    font-size: 14px;
    font-weight: 500;
}
#stopAllBtn:hover { background-color: #FEF2F2; border-color: #DC2626; }

#settingsBtn {
    background-color: #F1F5F9;
    color: #5C647A;
    border: 1px solid #E2E8F0;
    border-radius: 6px;
    padding: 8px 16px;
    font-size: 14px;
    font-weight: 500;
}
#settingsBtn:hover { background-color: #E2E8F0; }

/* ── 单个摄像头窗口 ── */
#cameraWindow {
    background: #0F172A;
    border: 1px solid #334155;
    border-radius: 8px;
}

#cameraInfoPanel {
    background: rgba(0, 0, 0, 140);
    color: #ffffff;
    border-radius: 6px;
    padding: 6px 12px;
    font-size: 12px;
}

#cameraStatusDot {
    border-radius: 6px;
    border: 1px solid rgba(255, 255, 255, 80);
}
#cameraStatusDot[status="online"]  { background: #059669; }
#cameraStatusDot[status="offline"] { background: #DC2626; }
```

- [ ] **Step 2: 修改 `styles/styles.qrc`,注册 qss**

在 `<file alias="userManagement.qss">userManagement.qss</file>` 之后加入:

```xml
        <file alias="cameramanagement.qss">cameramanagement.qss</file>
```

- [ ] **Step 3: 编译验证**

Qt Creator Ctrl+B(触发 rcc 重新编译)。预期:编译通过,资源注册为 `:/styles/cameramanagement.qss`。

- [ ] **Step 4: 提交**

```bash
git add styles/cameramanagement.qss styles/styles.qrc
git commit -m "feat(styles): 新增摄像头管理页面样式表"
```

---

## Task 7: CameraManagement 页面填充

**Files:**
- Modify: `UI/CameraManagement/cameramanagement.h`
- Modify: `UI/CameraManagement/cameramanagement.cpp`
- Modify: `ParkingSystem.pro`(若 camerawindow/.qss 上一步已加则**无需再改**)

**设计要点:** 页面用代码构建全部 UI(.ui 文件已是空壳,保留 `ui->setupUi(this)` 维持 uic 但不在 .ui 里放控件)。结构:垂直布局 = `videoContainer`(内含 `QGridLayout` 放 CameraWindow 集合 + 空状态 `emptyLabel`)+ `controlBar`(左侧 countLabel + 右侧三按钮)。

- [ ] **Step 1: 改写 `UI/CameraManagement/cameramanagement.h`**

完整内容:

```cpp
#ifndef CAMERAMANAGEMENT_H
#define CAMERAMANAGEMENT_H

#include <QWidget>
#include <QList>

namespace Ui { class CameraManagement; }

class CameraWindow;
class QFrame;
class QGridLayout;
class QLabel;
class QPushButton;
class DatabaseManager;

class CameraManagement : public QWidget
{
    Q_OBJECT

public:
    explicit CameraManagement(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~CameraManagement();

private:
    void setupUI();
    void buildCameraWindows();     // 从 CameraManager 拉取,创建 CameraWindow 并 bind
    void updateGridLayout();       // 按 count + 当前模式重排
    int  gridCols(int count) const; // 计算列数
    void updateCountLabel();       // "检测到 N 个摄像头 · WxH · FPS"

    Ui::CameraManagement *ui;
    DatabaseManager *m_db = nullptr;

    QFrame       *m_videoContainer  = nullptr;
    QGridLayout *m_gridLayout      = nullptr;
    QLabel       *m_emptyLabel     = nullptr;   // 无摄像头全局空状态
    QLabel       *m_countLabel     = nullptr;
    QPushButton *m_startAllBtn     = nullptr;
    QPushButton *m_stopAllBtn      = nullptr;
    QPushButton *m_settingsBtn     = nullptr;

    QList<CameraWindow*> m_windows;
    bool m_soloMode  = false;       // 独占模式
    int  m_soloIndex = -1;
};

#endif // CAMERAMANAGEMENT_H
```

- [ ] **Step 2: 改写 `UI/CameraManagement/cameramanagement.cpp`**

完整内容:

```cpp
#include "cameramanagement.h"
#include "ui_cameramanagement.h"
#include "camerawindow.h"
#include "src/camera/cameramanager.h"
#include "src/database/databasemanager.h"
#include <QFile>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>

CameraManagement::CameraManagement(QWidget *parent, DatabaseManager *db)
    : QWidget(parent)
    , ui(new Ui::CameraManagement)
    , m_db(db)
{
    ui->setupUi(this);

    QFile styleFile(":/styles/cameramanagement.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(QLatin1String(styleFile.readAll()));
        styleFile.close();
    }

    setupUI();
    buildCameraWindows();
    updateGridLayout();
    updateCountLabel();

    connect(m_startAllBtn, &QPushButton::clicked, this, [this](){
        CameraManager::instance().startAll();
    });
    connect(m_stopAllBtn, &QPushButton::clicked, this, [this](){
        CameraManager::instance().stopAll();
    });
    connect(m_settingsBtn, &QPushButton::clicked, this, [this](){
        // 设置按钮:本次无实际功能,留占位(可弹 notifyInfo)
    });
}

CameraManagement::~CameraManagement()
{
    // m_windows 子控件父对象是 m_videoContainer,随其被 Qt 自动析构
    delete ui;
}

// ── 构建静态框架(UI 控件) ──
void CameraManagement::setupUI()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(16);

    // ── 视频容器 ──
    m_videoContainer = new QFrame(this);
    m_videoContainer->setObjectName("videoContainer");
    m_gridLayout = new QGridLayout(m_videoContainer);
    m_gridLayout->setContentsMargins(8, 8, 8, 8);
    m_gridLayout->setSpacing(12);

    m_emptyLabel = new QLabel(m_videoContainer);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color:#94A3B8; font-size:14px; letter-spacing:2px;");
    m_emptyLabel->setText(QStringLiteral(
        "<div style='text-align:center;'>"
        "<img src=':/icons/videocam_off.svg' width='64' height='64'/><br><br>"
        "<span>未检测到摄像头 / No Cameras Detected</span>"
        "</div>"));
    m_emptyLabel->hide();

    root->addWidget(m_videoContainer, 1);

    // ── 底部控制栏 ──
    QFrame *controlBar = new QFrame(this);
    controlBar->setObjectName("controlBar");
    controlBar->setFixedHeight(64);
    QHBoxLayout *ctrlLayout = new QHBoxLayout(controlBar);
    ctrlLayout->setContentsMargins(20, 12, 20, 12);
    ctrlLayout->setSpacing(12);

    m_countLabel = new QLabel(controlBar);
    m_countLabel->setObjectName("countLabel");
    ctrlLayout->addWidget(m_countLabel);
    ctrlLayout->addStretch();

    m_startAllBtn = new QPushButton(QStringLiteral("▶ 开启所有"), controlBar);
    m_startAllBtn->setObjectName("startAllBtn");
    ctrlLayout->addWidget(m_startAllBtn);

    m_stopAllBtn = new QPushButton(QStringLiteral("⏻ 停止所有"), controlBar);
    m_stopAllBtn->setObjectName("stopAllBtn");
    ctrlLayout->addWidget(m_stopAllBtn);

    m_settingsBtn = new QPushButton(QStringLiteral("⚙ 设置"), controlBar);
    m_settingsBtn->setObjectName("settingsBtn");
    ctrlLayout->addWidget(m_settingsBtn);

    root->addWidget(controlBar);
}

// 获取摄像头线程,每个 CameraWindow bind 一个
void CameraManagement::buildCameraWindows()
{
    CameraManager &mgr = CameraManager::instance();
    for(int i = 0; i < mgr.count(); ++i){
        auto *win = new CameraWindow(m_videoContainer);
        win->bind(mgr.getThread(i), mgr.info(i));
        connect(win, &CameraWindow::doubleClicked, this, [this, i](){
            if(m_soloMode && m_soloIndex == i){
                m_soloMode = false;
                m_soloIndex = -1;
            }else{
                m_soloMode = true;
                m_soloIndex = i;
            }
            updateGridLayout();
        });
        m_windows.append(win);
    }
}

int CameraManagement::gridCols(int count) const
{
    switch(count){
        case 1: return 1;
        case 2: return 2;
        case 3: case 4: return 2;
        case 5: case 6: return 3;
        case 7: case 8: return 3;
        default: return 1;
    }
}

void CameraManagement::updateGridLayout()
{
    // 清空布局再重建
    while(QLayoutItem *item = m_gridLayout->takeAt(0)){
        delete item;
    }

    int count = m_windows.size();

    // 无摄像头 → 显示全局空状态
    if(count == 0){
        m_emptyLabel->setGeometry(m_videoContainer->rect());
        m_emptyLabel->show();
        m_emptyLabel->raise();
        return;
    }
    m_emptyLabel->hide();

    // 独占模式 → 只显示一个
    if(m_soloMode && m_soloIndex >= 0 && m_soloIndex < count){
        for(int i = 0; i < count; ++i){
            m_windows[i]->hide();
        }
        m_gridLayout->addWidget(m_windows[m_soloIndex], 0, 0);
        m_windows[m_soloIndex]->show();
        return;
    }

    // 多宫格
    int cols = gridCols(count);
    for(int i = 0; i < count; ++i){
        int row = i / cols;
        int col = i % cols;
        m_gridLayout->addWidget(m_windows[i], row, col);
        m_windows[i]->show();
    }
}

void CameraManagement::updateCountLabel()
{
    CameraManager &mgr = CameraManager::instance();
    if(mgr.count() == 0){
        m_countLabel->setText(QStringLiteral("检测到 0 个摄像头"));
        return;
    }
    CameraInfo first = mgr.info(0);
    m_countLabel->setText(QStringLiteral("检测到 %1 个摄像头 · %2x%3 · %4 FPS")
                               .arg(mgr.count())
                               .arg(first.width).arg(first.height)
                               .arg(first.fps));
}

void CameraManagement::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(m_windows.isEmpty()){
        m_emptyLabel->setGeometry(m_videoContainer->rect());
    }
}
```

> **注意:** `resizeEvent` 声明需加到头文件 protected 区。在头文件 `private:` 之前补一个 `protected:` 区块:

```cpp
protected:
    void resizeEvent(QResizeEvent *event) override;
```

> **SQL** 这次未被使用(database 无关摄像头枚举),`m_db` 字段仅保留接口以与 UserManagement 保持一致。`src/database/databasemanager.h` include 可保留以备未来存储摄像头配置。

- [ ] **Step 3: 头文件加 resizeEvent protected 声明**

在 `UI/CameraManagement/cameramanagement.h`,把 `private:` 区之前(`public:` 之后)插入上面 protected 区。最终头文件相关段落为:

```cpp
public:
    explicit CameraManagement(QWidget *parent = nullptr, DatabaseManager *db = nullptr);
    ~CameraManagement();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUI();
    ...
```

- [ ] **Step 4: 编译验证(带主窗口导航接通前会失败 —— 此步先只编译本页)**

此 Task 单独编译会因 mainwindow 尚未 include cameramanagement 而仍可过(页面自包含)。Qt Creator Ctrl+B。预期:编译通过。若报错(如 `recognizethread` 信号不匹配),先不修,Task 8 接通后再统一处理。

- [ ] **Step 5: 提交**

```bash
git add UI/CameraManagement/cameramanagement.h UI/CameraManagement/cameramanagement.cpp
git commit -m "feat(camera): 填充摄像头管理页面 UI 与多宫格布局"
```

---

## Task 8: 主窗口集成

**Files:**
- Modify: `UI/MainWindow/mainwindow.h`
- Modify: `UI/MainWindow/mainwindow.cpp`

**改动总览(对应现 mainwindow.cpp 行号):**
1. 构造首行调用 `CameraManager::instance().scanCameras()`
2. L152-159:`CameraThread(0)` 改为 `getThread(0)`,删除 `setResolution/setTargetFps`,改 `start()`
3. L210-214:addWidget `CameraManagement` 索引 3
4. L218-235:`cameraManagementButton` → 索引 3
5. 析构 L247-250:删除 `m_cameraThread` 的 quit/wait(归 CameraManager)
6. mainwindow.h:加 `m_cameraManagementPage` 成员 + 前向声明

- [ ] **Step 1: 修改 `UI/MainWindow/mainwindow.h`**

在前向声明区(`class UserManagement;` 之后)加:

```cpp
class CameraManagement;
```

在 `private:` 区 `UserManagement *m_userManagementPage  = nullptr;` 之后加(当前 mainwindow.h:109):

```cpp
    CameraManagement *m_cameraManagementPage = nullptr;    // 摄像头管理页面
```

- [ ] **Step 2: 修改 `UI/MainWindow/mainwindow.cpp` — include**

在 `#include "UI/UserManager/usermanagement.h"` 之后(当前 L14)加:

```cpp
#include "UI/CameraManagement/cameramanagement.h"
#include "src/camera/cameramanager.h"
```

- [ ] **Step 3: 修改 `UI/MainWindow/mainwindow.cpp` — 构造首行 scanCameras**

在 `MainWindow::MainWindow` 构造函数体第一行 `ui->setupUi(this);`(L29)之后,插入:

```cpp
    // 扫描并创建摄像头线程池(全应用共享)
    CameraManager::instance().scanCameras();
```

- [ ] **Step 4: 修改 `UI/MainWindow/mainwindow.cpp` — 摄像头线程所有权迁移**

找到这段(L152-159):

```cpp
    m_cameraThread = new CameraThread(0, this);
    // 从配置文件读取摄像头分辨率，默认 1920×1080（兼容旧配置无此字段）
    m_cameraThread->setResolution(
        InitFile::instance().getCameraWidth(),
        InitFile::instance().getCameraHeight());
    m_cameraThread->setTargetFps(InitFile::instance().getCameraFps());
    connect(m_cameraThread,&CameraThread::newFrameCaptured,this,&MainWindow::updateFrame);
    m_cameraThread->start();
```

替换为:

```cpp
    // 入口摄像头(索引 0)从 CameraManager 单例获取(已在 scanCameras 中配置分辨率/帧率)
    m_cameraThread = CameraManager::instance().getThread(0);
    connect(m_cameraThread, &CameraThread::newFrameCaptured, this, &MainWindow::updateFrame);
    CameraManager::instance().start(0);
```

- [ ] **Step 5: 修改 `UI/MainWindow/mainwindow.cpp` — 导航接通摄像头页**

找到这段(L210-214):

```cpp
    m_userManagementPage = new UserManagement(this, m_db);
    int userMgmtIndex = ui->stackedWidget->addWidget(m_userManagementPage);
```

在之后插入:

```cpp
    m_cameraManagementPage = new CameraManagement(this, m_db);
    int cameraMgmtIndex = ui->stackedWidget->addWidget(m_cameraManagementPage);
    Q_UNUSED(cameraMgmtIndex);
```

- [ ] **Step 6: 修改 `UI/MainWindow/mainwindow.cpp` — 页面切换逻辑**

找到这段(L218-235):

```cpp
     connect(navButtonGroup, &QButtonGroup::idClicked, this, [this, navButtonGroup](int id) {
        if (id == -1) return;
        QAbstractButton *btn = navButtonGroup->button(id);
        if (!btn) return;

        // 根据按钮名映射到对应的页面索引
        // contentWidget       → 索引 0（仪表盘，通过 objectName 查找）
        // vehicleInfoButton   → 索引 1（车辆信息）
        // userManagementButton → 索引 2（用户管理）
        if (btn == ui->dashboardButton) {
            ui->stackedWidget->setCurrentIndex(0);
        } else if (btn == ui->vehicleInfoButton) {
            ui->stackedWidget->setCurrentIndex(1);
        } else if (btn == ui->userManagementButton) {
            ui->stackedWidget->setCurrentIndex(2);
        }
        // 后续页面按此规律追加：cameraManagementButton → 索引 3, etc.
    });
```

替换为:

```cpp
     connect(navButtonGroup, &QButtonGroup::idClicked, this, [this, navButtonGroup](int id) {
        if (id == -1) return;
        QAbstractButton *btn = navButtonGroup->button(id);
        if (!btn) return;

        // 根据按钮名映射到对应的页面索引
        // dashboardButton     → 索引 0（仪表盘）
        // vehicleInfoButton   → 索引 1（车辆信息）
        // userManagementButton → 索引 2（用户管理）
        // cameraManagementButton → 索引 3（摄像头管理）
        if (btn == ui->dashboardButton) {
            ui->stackedWidget->setCurrentIndex(0);
        } else if (btn == ui->vehicleInfoButton) {
            ui->stackedWidget->setCurrentIndex(1);
        } else if (btn == ui->userManagementButton) {
            ui->stackedWidget->setCurrentIndex(2);
        } else if (btn == ui->cameraManagementButton) {
            ui->stackedWidget->setCurrentIndex(3);
        }
    });
```

- [ ] **Step 7: 修改 `UI/MainWindow/mainwindow.cpp` — 析构停止逻辑**

找到这段(L238-261):

```cpp
MainWindow::~MainWindow()
{
    // 先停止识别线程（依赖 FrameQueue，必须在 FrameQueue 释放前停止）
    if(m_recognizeThread){
        m_recognizeThread->stop();
        m_recognizeThread->wait(3000);
    }

    // 再停止摄像头线程
    if(m_cameraThread){
        m_cameraThread->quit();
        m_cameraThread->wait();
    }

    // 释放帧队列（两个线程都已停止，安全释放）
    delete m_frameQueue;
    m_frameQueue = nullptr;
    ...
```

把"再停止摄像头线程"那段替换为:

```cpp
    // 摄像头线程所有权已迁移到 CameraManager 单例，此处不再 stop/wait。
    // stopAll 由 CameraManager 单例析构(进程退出)时统一处理。
    // 保证 m_cameraThread 指针仍有效(CameraManager 在 MainWindow 之后析构)。
```

- [ ] **Step 8: 编译验证**

Qt Creator Ctrl+B。预期:编译通过。

> **若报错常见项:**
> - `m_cameraManagementPage` 未声明 → 检查 Step 1
> - `CameraManager` 未定义 → 检查 Step 2 include
> - `getThread(0)` 返回 nullptr(无摄像头环境) → 先不要在本步处理,Task 9 验证;若无摄像头程序会在 `connect(m_cameraThread,...)` 崩溃,需在 Step 4 的 `connect` 前加 null 检查:
>   ```cpp
>   if(m_cameraThread){
>       connect(m_cameraThread, &CameraThread::newFrameCaptured, this, &MainWindow::updateFrame);
>   }
>   ```
>   并把 `CameraManager::instance().start(0);` 包在 `if(m_cameraThread)` 内。**若你环境无摄像头,采用此加固版 Step 4:**
>   ```cpp
>   m_cameraThread = CameraManager::instance().getThread(0);
>   if(m_cameraThread){
>       connect(m_cameraThread, &CameraThread::newFrameCaptured, this, &MainWindow::updateFrame);
>       CameraManager::instance().start(0);
>   }
>   ```

- [ ] **Step 9: 提交**

```bash
git add UI/MainWindow/mainwindow.h UI/MainWindow/mainwindow.cpp
git commit -m "feat(main): 集成 CameraManager 单例并接通摄像头管理页导航"
```

---

## Task 9: 端到端验证

**Files:** 无(验证 Task)

- [ ] **Step 1: 全量构建**

Qt Creator Ctrl+B(Release 或 Debug 均可)。预期:无错误编译通过。

- [ ] **Step 2: 运行程序并验证主页摄像头正常**

Qt Creator Ctrl+R 启动。登录进入主页(仪表盘)。验证:
- 主页摄像头画面正常显示(无回归)
- 关闭程序正常退出,无崩溃、无悬挂线程

**若主页画面异常或退出崩溃** → 通常是 `m_cameraThread` 为空(无摄像头环境)。检查 Task 8 Step 4 是否采用了加固版(null 检查)。若环境有摄像头但仍异常,检查 `CameraManager::instance().start(0)` 是否在 `setFrameQueue` 之前调用——其实无需,`CameraThread::run` 内 PushFrameQueue 与 start 时机无关。

- [ ] **Step 3: 验证摄像头管理页面导航**

点击左侧导航栏"摄像头管理"按钮。验证:
- stackedWidget 切换到摄像头页
- 显示视频容器(深色 `#1E293B` + 圆角)
- 显示底部控制栏(左 countLabel + 右开启所有/停止所有/设置按钮)
- 若有摄像头:右下宫格内显示 CameraWindow,左上信息面板,右上状态圆点;无摄像头:整个视频区显示"未检测到摄像头"空状态

- [ ] **Step 4: 验证多宫格 + 开启/停止**

点击"开启所有"。验证:
- 所有 CameraWindow 显示画面(若有摄像头)
- 在线状态圆点变绿
点击"停止所有"。验证:
- 画面停住或显示 Signal Lost 空状态
- 状态圆点变红

- [ ] **Step 5: 验证独占模式**

双击任一 CameraWindow。验证:
- 该窗口占满整个视频区(1×1)
- 其他窗口隐藏
再双击。验证:
- 恢复多宫格布局

- [ ] **Step 6: 验证主备页共存无抢占**

切回仪表盘。验证主页摄像头画面仍正常(共享索引 0 线程,无抢占冲突)。切回摄像头页,验证其窗口 0 也显示同一画面。

- [ ] **Step 7: 提交(记录验证通过,facts 文档无 code 改动则跳过)**

无代码改动则不提交。若有验证中发现的修复(如加固 null 检查),提交:

```bash
git add <修复的文件>
git commit -m "fix(camera): 验证加固 nullcheck 或布局修复"
```

---

## 自查:Spec 覆盖核对

对照 `2026-07-20-camera-management-design-v2.md` 各节:

| Spec 节 | 由哪些 Task 实现 | ✓ |
|---|---|---|
| 3.2 CameraManager 单例 | Task 3 | ✓ |
| 3.3 CameraInfo 结构 | Task 1 | ✓ |
| 3.4 CameraWindow(QFrame 子类) | Task 5 | ✓ |
| 3.5 CameraManagement 页面 | Task 7 | ✓ |
| 3.6 配置扩展(InitFile JSON) | Task 1 | ✓ |
| 4 数据流(shared getThread) | Task 8 Step 4 | ✓ |
| 5 布局规则(1-8 多宫格+独占) | Task 7 updateGridLayout+gridCols+doubleClicked | ✓ |
| 6 视觉(#1E293B/SigislLost/控制栏/qss) | Task 6 + Task 7 setupUI | ✓ |
| 7 与主页集成改动(3 处) | Task 8 Step 3/4/5/6/7 | ✓ |
| 8 错误处理(空状态/超8/入口离线) | Task 3(cap kMax=8)+Task 7(全局空态)+Task 8 Step 4(null加固) | ✓ |
| 9 性能(独立线程/独占只重排) | Task 3(thread池)+Task 7(独占不重连) | ✓ |
| 11 入库出库不纳入 | 无 Task(明确排除) | ✓ |

**未落实细节(影响最小):**
- 测试策略:项目无测试框架,改手动验证(Task 9)——已声明在计划头部"测试约定"。
- `tests/` 配置方式:无,跳过。

**类型一致性核对:**
- `CameraThread::stop()` Task 2 定义 ↔ Task 3 `stopAll()` 调用 ✓
- `CameraManager::instance().scanCameras/count/getThread/start/stopAll` Task 3 定义 ↔ Task 7/8 调用 ✓
- `CameraWindow::bind/doubleClicked` Task 5 ↔ Task 7 ✓
- `CameraInfo{index/name/location/role/width/height/fps}` Task 1 ↔ Task 3/5/7 ✓
- `InitFile::getCameras/setCameras` Task 1 ↔ Task 3 ✓

无类型不一致,无占位符。

---

## 执行交付

计划已写入 `docs/superpowers/plans/2026-07-20-camera-management-v2.md`。两种执行方式:

**1. Subagent-Driven(推荐)** — 每个 Task 派发独立 subagent 执行,两阶段评审 + 任务间快速往返。

**2. Inline Execution** — 本会话内用 executing-plans 批量执行,带检查点。

请选择执行方式。
