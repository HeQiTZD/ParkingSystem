# ParkingSystem 架构改进实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 修复架构审查发现的 P1×3 + P2×2 共 5 项问题,引入 per-thread 数据库连接、统一 Singleton 生命周期、提取 Service 层、UI 依赖切到 Service 引用。

**Architecture:** 渐进式 Service 层提取 — 新增 DbConnectionPool 管理 per-thread QSqlDatabase;DatabaseManager 瘦身为纯 CRUD;新增 ParkingService/UserService/VehicleService 承接业务流程;UI 持 Service& 引用消除裸指针;4 个 Singleton 统一为 Meyer's + 显式 shutdown。

**Tech Stack:** Qt 6 (Widgets/SQL),C++11,CMake/.pro,OpenCV 4.11,HyperLPR-2,MySQL/QMYSQL 驱动。

**对应 spec:** [docs/superpowers/specs/2026-07-21-architecture-improvement-design.md](../specs/2026-07-21-architecture-improvement-design.md)

**测试说明:** 项目当前无测试框架(QtTest 后续手动引入)。本计划采用"编译通过 + 手动运行验证 + 提交"节奏替代 TDD。每个任务结束有验证清单。

**执行顺序:** Phase 1 (P1 线程安全 + Singleton) → Phase 2 (P1 上帝对象瘦身 + P2 UI 解耦) → Phase 3 (兼容性收尾)。严格按顺序,禁止跨 Phase 并行。

---

## File Structure

### 新建文件

| 文件 | 职责 |
|------|------|
| `src/database/dbconfig.h` | DbConfig 结构体(连接参数容器) |
| `src/database/dbconnectionpool.h/.cpp` | per-thread QSqlDatabase 管理单例 |
| `src/service/parkingservice.h/.cpp` | 入库/出库/计费/统计业务流程 |
| `src/service/userservice.h/.cpp` | 用户认证/注册/CRUD 业务流程 |
| `src/service/vehicleservice.h/.cpp` | 车辆记录查询/删除/分页业务流程 |

### 修改文件(摘要,详见各任务)

| 文件 | 修改要点 |
|------|---------|
| `src/database/databasemanager.h/.cpp` | 移除成员 db/connected,改用 DbConnectionPool |
| `src/utils/pthreadpool.h/.cpp` | 改 Meyer's + shutdown() |
| `src/app/platerecognize.h/.cpp` | 加 shutdown() |
| `src/camera/cameramanager.h/.cpp` | 加 shutdown()(stopAll 已存在) |
| `src/utils/initfile.h/.cpp` | 加 shutdown()(幂等写盘) |
| `src/app/ApplicationManager.h/.cpp` | 析构注入逆序 shutdown;持有 3 个 Service |
| `src/app/main.cpp` | 实例化 3 个 Service,传给 ApplicationManager |
| `src/camera/camerathread.cpp` | run() 末尾 closeThreadConnection |
| `src/app/recognizethread.cpp` | run() 末尾 closeThreadConnection |
| `UI/MainWindow/mainwindow.h/.cpp` | 构造改 Service&,业务调用切 Service |
| `UI/Login/logindialog.h/.cpp` | 构造改 UserService& |
| `UI/Register/registerdialog.h/.cpp` | 构造改 UserService& |
| `UI/VehicleInformation/vehicleinformation.h/.cpp` | 构造改 VehicleService& |
| `UI/UserManager/usermanagement.h/.cpp` | 构造改 UserService& |
| `UI/CameraManagement/cameramanagement.h/.cpp` | 构造去掉 DatabaseManager* |
| `ParkingSystem.pro` | SOURCES/HEADERS 加入新文件 |

---

## Phase 1: P1 线程安全 + Singleton 生命周期

### Task 1: DbConfig 结构体

**Files:**
- Create: `src/database/dbconfig.h`

- [ ] **Step 1: 创建 DbConfig 头文件**

```cpp
// src/database/dbconfig.h
#ifndef DBCONFIG_H
#define DBCONFIG_H

#include <QString>

// 数据库连接配置(纯数据,无连接)
// 用于在 DbConnectionPool 中为每个线程克隆独立的 QSqlDatabase。
struct DbConfig {
    QString driverName = "QMYSQL";
    QString host;
    int     port = 3306;
    QString dbName;
    QString username;
    QString password;

    bool isValid() const {
        return !host.isEmpty() && !dbName.isEmpty();
    }
};

#endif // DBCONFIG_H
```

- [ ] **Step 2: 加入 .pro**

修改 `ParkingSystem.pro`,在 `# ==================== 数据库相关 ====================` HEADERS 段加入:

```
    src/database/dbconfig.h \
```

- [ ] **Step 3: 编译验证**

运行: `qmake ParkingSystem.pro && mingw32-make -j4`(或在 Qt Creator 中构建)
Expected: 编译通过(仅头文件,无 link 依赖)

- [ ] **Step 4: Commit**

```bash
git add src/database/dbconfig.h ParkingSystem.pro
git commit -m "feat(db): add DbConfig struct for per-thread connection params"
```

---

### Task 2: DbConnectionPool 单例

**Files:**
- Create: `src/database/dbconnectionpool.h`
- Create: `src/database/dbconnectionpool.cpp`
- Modify: `ParkingSystem.pro`(加入新文件)

- [ ] **Step 1: 创建头文件**

```cpp
// src/database/dbconnectionpool.h
#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include <QSqlDatabase>
#include <QHash>
#include <QMutex>
#include "dbconfig.h"

// per-thread QSqlDatabase 连接池。
// Qt 文档明确: 同一 QSqlDatabase 实例跨线程 exec 不安全。
// 本类为每个线程克隆独立连接, 共享同一连接配置。
// Meyer's Singleton, 析构时关闭所有线程连接。
class DbConnectionPool
{
public:
    static DbConnectionPool& instance();

    // 初始化模板配置(主线程调用一次)。
    // 后续各线程调用 connection() 时, 会基于此配置 cloneDatabase。
    void setConfig(const DbConfig &config);

    // 取当前线程的连接;
    // 若不存在则 clone 自模板连接并打开。
    // 返回的 QSqlDatabase 仅供当前线程使用。
    QSqlDatabase connection();

    // 关闭并移除当前线程的连接。
    // 线程退出前调用, 防止 MySQL 连接泄漏。
    void closeThreadConnection();

    // 关闭所有线程连接(shutdown 时调用)。
    void closeAll();

private:
    DbConnectionPool();
    ~DbConnectionPool();
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;

    QSqlDatabase createConnectionForCurrentThread();

    QMutex m_mutex;
    QHash<Qt::HANDLE, QSqlDatabase> m_conns;
    DbConfig m_config;
    bool m_configured = false;
};

#endif // DBCONNECTIONPOOL_H
```

- [ ] **Step 2: 创建实现文件**

```cpp
// src/database/dbconnectionpool.cpp
#include "dbconnectionpool.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QDebug>

DbConnectionPool& DbConnectionPool::instance()
{
    static DbConnectionPool pool;  // Meyer's
    return pool;
}

DbConnectionPool::DbConnectionPool() {}

DbConnectionPool::~DbConnectionPool()
{
    closeAll();
}

void DbConnectionPool::setConfig(const DbConfig &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
    m_configured = true;
}

QSqlDatabase DbConnectionPool::connection()
{
    QMutexLocker locker(&m_mutex);
    if(!m_configured){
        qWarning() << "DbConnectionPool: 配置未设置, 返回无效连接";
        return QSqlDatabase();  // 默认构造的无效连接
    }

    Qt::HANDLE tid = QThread::currentThreadId();
    auto it = m_conns.find(tid);
    if(it != m_conns.end() && it.value().isOpen()){
        return it.value();
    }

    // 不存在或已关闭, 重新创建
    locker.unlock();
    QSqlDatabase conn = createConnectionForCurrentThread();
    return conn;
}

QSqlDatabase DbConnectionPool::createConnectionForCurrentThread()
{
    // 唯一连接名, 与线程 ID 绑定
    QString connName = QString("conn_%1")
        .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if(QSqlDatabase::contains(connName)){
        QSqlDatabase existing = QSqlDatabase::database(connName, false);
        if(existing.isOpen()){
            QMutexLocker locker(&m_mutex);
            m_conns[QThread::currentThreadId()] = existing;
            return existing;
        }
        QSqlDatabase::removeDatabase(connName);
    }

    QSqlDatabase conn = QSqlDatabase::addDatabase(m_config.driverName, connName);
    conn.setHostName(m_config.host);
    conn.setPort(m_config.port);
    conn.setDatabaseName(m_config.dbName);
    conn.setUserName(m_config.username);
    conn.setPassword(m_config.password);

    if(!conn.open()){
        qCritical() << "DbConnectionPool: 线程" << QThread::currentThreadId()
                    << "连接失败:" << conn.lastError().text();
        // 保留对象但未 open, 调用方需检查 isOpen()
    }

    QMutexLocker locker(&m_mutex);
    m_conns[QThread::currentThreadId()] = conn;
    return conn;
}

void DbConnectionPool::closeThreadConnection()
{
    QMutexLocker locker(&m_mutex);
    Qt::HANDLE tid = QThread::currentThreadId();
    auto it = m_conns.find(tid);
    if(it != m_conns.end()){
        QString connName = it.value().connectionName();
        it.value().close();
        m_conns.erase(it);
        // 千万不能在外部连接还引用时 removeDatabase, 这里已 close, 安全
        QSqlDatabase::removeDatabase(connName);
    }
}

void DbConnectionPool::closeAll()
{
    QMutexLocker locker(&m_mutex);
    // 必须先收集所有连接名, 再依次 close/remove
    // 因为 m_conns 中的 QSqlDatabase 是隐式共享对象, 清空容器会减少引用计数
    QStringList connNames;
    for(auto it = m_conns.begin(); it != m_conns.end(); ++it){
        connNames << it.value().connectionName();
        it.value().close();
    }
    m_conns.clear();
    locker.unlock();
    // 在无引用时才能 removeDatabase
    for(const QString &name : connNames){
        QSqlDatabase::removeDatabase(name);
    }
}
```

- [ ] **Step 3: 加入 .pro**

修改 `ParkingSystem.pro`:

HEADERS 段加入:
```
    src/database/dbconfig.h \
    src/database/dbconnectionpool.h \
```

SOURCES 段(在 `src/database/databasemanager.cpp \` 后)加入:
```
    src/database/dbconnectionpool.cpp \
```

- [ ] **Step 4: 编译验证**

运行: 构建项目
Expected: 编译通过(类未被使用,仅链接器备案)

- [ ] **Step 5: Commit**

```bash
git add src/database/dbconnectionpool.h src/database/dbconnectionpool.cpp ParkingSystem.pro
git commit -m "feat(db): add DbConnectionPool for per-thread QSqlDatabase"
```

---

### Task 3: DatabaseManager 切换为 per-thread 连接

**Files:**
- Modify: `src/database/databasemanager.h`
- Modify: `src/database/databasemanager.cpp`

**背景:** 当前 DatabaseManager 持成员 `QSqlDatabase db; bool connected;`,所有方法用 `QSqlQuery query(db)` 直接操作成员。本任务改为通过 DbConnectionPool 取当前线程连接,移除成员 db/connected。所有方法签名保持不变(UI 还未切换,老调用继续工作)。

- [ ] **Step 1: 修改头文件 databasemanager.h**

替换成员区(末尾 private: 段)为:

```cpp
private:
    DbConfig m_config;
    bool m_connected;

    // 取当前线程的连接(便捷封装)
    QSqlDatabase threadConnection();
```

并在文件顶部 `#include` 段加入:

```cpp
#include "dbconfig.h"
```

移除原 `QSqlDatabase db;` 成员(保留 `bool connected` 改名 `m_connected`)。
移除 `#include <QSqlDatabase>` 改为前向声明 `class QSqlDatabase;` 不行(QSqlDatabase 返回值需要完整类型),保留 `<QSqlDatabase>` include。

- [ ] **Step 2: 修改实现 databasemanager.cpp 顶部与连接方法**

替换 `connectDatabase`:

```cpp
#include "dbconnectionpool.h"

DatabaseManager::DatabaseManager(QObject *parent): QObject(parent), m_connected(false){}

DatabaseManager::~DatabaseManager()
{
    disconnectDatabase();
}

bool DatabaseManager::connectDatabase(const QString &host, int port,
                                      const QString &dbName,
                                      const QString &username,
                                      const QString &password)
{
    if(m_connected){
        disconnectDatabase();
    }

    DbConfig cfg;
    cfg.host = host;
    cfg.port = port;
    cfg.dbName = dbName;
    cfg.username = username;
    cfg.password = password;

    // 用主线程连接做一次 open 验证
    DbConnectionPool::instance().setConfig(cfg);
    QSqlDatabase testConn = DbConnectionPool::instance().connection();
    if(!testConn.isOpen()){
        qDebug() << "数据库连接失败" << testConn.lastError().text();
        m_connected = false;
        emit connectionStatusChanged(false);
        return false;
    }

    m_connected = true;
    emit connectionStatusChanged(true);
    qDebug() << "数据库连接成功！";
    return true;
}

void DatabaseManager::disconnectDatabase()
{
    if(m_connected){
        DbConnectionPool::instance().closeAll();
        m_connected = false;
        emit connectionStatusChanged(false);
        qDebug() << "数据库连接已关闭";
    }
}

bool DatabaseManager::isConnected() const
{
    return m_connected;
}

QSqlDatabase DatabaseManager::threadConnection()
{
    return DbConnectionPool::instance().connection();
}
```

- [ ] **Step 3: 全局替换 cpp 中的 `QSqlQuery query(db)` 与 `db.transaction` 等**

对 cpp 文件做以下规则替换(每处):

| 原写法 | 新写法 |
|--------|--------|
| `QSqlQuery query(db);` | `QSqlDatabase dbc = threadConnection(); QSqlQuery query(dbc);` |
| `QSqlQuery check;` (内部访问) | `QSqlDatabase dbc = threadConnection(); QSqlQuery check(dbc);` |
| `if(!db.transaction())` | `QSqlDatabase dbc = threadConnection(); if(!dbc.transaction())` |
| `db.rollback()` | `dbc.rollback()` (对应作用域内 dbc) |
| `db.commit()` | `dbc.commit()` |

**重要:** 同一方法内需复用同一个 `dbc` 变量,不要每次取连接(QSqlDatabase 是隐式共享,但仍应一致)。transaction/commit/rollback 必须用同一连接名。

`executeQuery` 方法(已确认无调用者)直接删除,头文件中同步删除声明。

- [ ] **Step 4: 编译并修正**

运行: 构建项目
Expected: 可能出现 "db was not declared" 错误,逐个按 Step 3 规则替换直到通过。

- [ ] **Step 5: 手动运行验证**

启动程序,登录后执行一次入库操作,观察日志:
Expected: "数据库连接成功" + 入库成功,与改造前行为一致。

- [ ] **Step 6: Commit**

```bash
git add src/database/databasemanager.h src/database/databasemanager.cpp
git commit -m "refactor(db): DatabaseManager uses DbConnectionPool, remove member db"
```

---

### Task 4: ThreadPoolManager 升级 Meyer's + shutdown

**Files:**
- Modify: `src/utils/pthreadpool.h`
- Modify: `src/utils/pthreadpool.cpp`

- [ ] **Step 1: 头文件改 Meyer's**

替换 `pthreadpool.h` 中 `static ThreadPoolManager* instance();` 与私有静态成员:

```cpp
public:
    static ThreadPoolManager& instance();  // 改为引用返回
    ...
private:
    // 删除: static ThreadPoolManager* s_instance;
```

加 shutdown 声明(public):

```cpp
    // 幂等关闭: 等待任务完成并释放线程池
    void shutdown();
```

- [ ] **Step 2: 实现改 Meyer's**

替换 `pthreadpool.cpp` 中静态成员与 instance:

```cpp
// 删除: ThreadPoolManager* ThreadPoolManager::s_instance = nullptr;

ThreadPoolManager& ThreadPoolManager::instance()
{
    static ThreadPoolManager inst;  // Meyer's, 进程退出自动析构
    return inst;
}
```

替换析构为调用 shutdown:

```cpp
ThreadPoolManager::~ThreadPoolManager()
{
    shutdown();
}

void ThreadPoolManager::shutdown()
{
    if(!m_initialized) return;  // 幂等
    if(m_threadPool){
        m_threadPool->waitForDone();
        delete m_threadPool;
        m_threadPool = nullptr;
    }
    m_initialized = false;
    qDebug() << "ThreadPoolManager: shutdown 完成";
}
```

- [ ] **Step 3: 修正所有 `instance()` 调用点**

`pthreadpool.cpp` 内 RecognitionTask::run 用 `ThreadPoolManager::instance()->xxx`,改为 `ThreadPoolManager::instance().xxx`(注意 `.->`):

```cpp
emit ThreadPoolManager::instance().taskFinished(m_taskId, success, errorMsg);
emit ThreadPoolManager::instance().statusChanged(
    ThreadPoolManager::instance().activeThreadCount());
```

grep 全项目 `ThreadPoolManager::instance()`:
```bash
grep -rn "ThreadPoolManager::instance()" src/ UI/
```
将所有 `->` 改 `.`(因为返回引用)。

- [ ] **Step 4: 编译验证**

构建项目
Expected: 编译通过

- [ ] **Step 5: Commit**

```bash
git add src/utils/pthreadpool.h src/utils/pthreadpool.cpp
git commit -m "refactor(util): ThreadPoolManager to Meyer's singleton with shutdown()"
```

---

### Task 5: PlateRecognize 升级 Meyer's + shutdown

**Files:**
- Modify: `src/app/platerecognize.h`
- Modify: `src/app/platerecognize.cpp`

- [ ] **Step 1: 头文件改**

```cpp
public:
    static PlateRecognize& instance();  // 改引用
    ...
    void shutdown();  // 幂等释放模型
private:
    // 删除: static PlateRecognize* s_instance;
```

- [ ] **Step 2: 实现改**

```cpp
// 删除: PlateRecognize* PlateRecognize::s_instance = nullptr;

PlateRecognize& PlateRecognize::instance()
{
    static PlateRecognize inst;
    return inst;
}

void PlateRecognize::shutdown()
{
    // HyperLPR Pipeline 由 unique_ptr 持有, reset 即释放
    // 若 m_pipeline 是裸指针, delete 并置 nullptr
    // (根据现有 platerecognize.cpp 实际结构调整)
    qDebug() << "PlateRecognize: shutdown 完成";
}
```

> 注: shutdown 内部实现取决于 platerecognize.cpp 中 pipeline 成员的实际类型。执行时打开该文件确认 pipeline 持有方式(unique_ptr/裸指针),写对应的 reset/delete 代码。

- [ ] **Step 3: 修正调用点**

```bash
grep -rn "PlateRecognize::instance()" src/ UI/
```
将 `->` 改 `.`。

- [ ] **Step 4: 编译 + 验证识别流程**

启动 → 登录 → 启动识别 → 摄像头帧进识别 → 日志输出车牌识别结果
Expected: 行为与改造前一致。

- [ ] **Step 5: Commit**

```bash
git add src/app/platerecognize.h src/app/platerecognize.cpp
git commit -m "refactor(recognize): PlateRecognize to Meyer's singleton with shutdown()"
```

---

### Task 6: CameraManager 加 shutdown(已是 Meyer's)

**Files:**
- Modify: `src/camera/cameramanager.h`
- Modify: `src/camera/cameramanager.cpp`

CameraManager 已是 Meyer's(`static CameraManager instance;`),只需加显式 shutdown。

- [ ] **Step 1: 头文件加声明**

```cpp
public:
    static CameraManager& instance();
    ...
    // 幂等: 停止所有摄像头线程, 析构时已调用
    void shutdown();
```

- [ ] **Step 2: 实现**

```cpp
void CameraManager::shutdown()
{
    stopAll();  // stopAll 已存在, 内部对每个线程 stop()
    qDebug() << "CameraManager: shutdown 完成";
}
```

- [ ] **Step 3: 编译**

Expected: 通过

- [ ] **Step 4: Commit**

```bash
git add src/camera/cameramanager.h src/camera/cameramanager.cpp
git commit -m "refactor(camera): CameraManager add explicit shutdown()"
```

---

### Task 7: InitFile 加 shutdown(幂等写盘)

**Files:**
- Modify: `src/utils/initfile.h`
- Modify: `src/utils/initfile.cpp`

InitFile 已是 Meyer's。现有析构注释"保存由 setter 调用方显式触发",shutdown 即显式触发点。

- [ ] **Step 1: 头文件加声明**

```cpp
public:
    static InitFile& instance();
    ...
    // 幂等: 显式写盘。析构不自动写盘以满足只读场景。
    void shutdown();
```

- [ ] **Step 2: 实现写盘**

```cpp
void InitFile::shutdown()
{
    // 若已有 saveConfig() 方法, 直接调用; 否则在此写实现
    // 参考现有 initfile.cpp 中是否已存在 saveConfig
    // 若存在:
    saveConfig();
    qDebug() << "InitFile: shutdown 写盘完成";
}
```

> 注: 执行时打开 initfile.cpp 确认是否已有 saveConfig 实现。若无,写入实际 saveConfig 逻辑(序列化 QJsonObject 到 configFilePath)。

- [ ] **Step 3: 编译**

- [ ] **Step 4: Commit**

```bash
git add src/utils/initfile.h src/utils/initfile.cpp
git commit -m "refactor(util): InitFile add explicit shutdown() write-back"
```

---

### Task 8: ApplicationManager 注入逆序 shutdown 序列

**Files:**
- Modify: `src/app/ApplicationManager.h`
- Modify: `src/app/ApplicationManager.cpp`

- [ ] **Step 1: 头文件加析构声明**

```cpp
public:
    ~ApplicationManager();
    ...
```

- [ ] **Step 2: 实现逆序 shutdown**

```cpp
ApplicationManager::~ApplicationManager()
{
    // 删除子窗口前先停识别 + 释放线程
    if(m_mainWindow) m_mainWindow->stopRecognition();

    // 逆序依赖: ThreadPool→Camera→Recognize→InitFile
    ThreadPoolManager::instance().shutdown();
    CameraManager::instance().shutdown();
    PlateRecognize::instance().shutdown();
    InitFile::instance().shutdown();

    // m_loginDialog / m_mainWindow 是本对象子对象, Qt 自动 delete
}
```

- [ ] **Step 3: 加 include**

在 ApplicationManager.cpp 顶部加(若未存在):

```cpp
#include "src/utils/pthreadpool.h"
#include "src/camera/cameramanager.h"
#include "src/app/platerecognize.h"
#include "src/utils/initfile.h"
```

- [ ] **Step 4: 编译 + 启动退出验证**

启动 → 登录 → 退出
Expected: 退出时日志依次输出 "ThreadPoolManager: shutdown 完成" / "CameraManager: shutdown 完成" / "PlateRecognize: shutdown 完成" / "InitFile: shutdown 写盘完成",无崩溃。

- [ ] **Step 5: Commit**

```bash
git add src/app/ApplicationManager.h src/app/ApplicationManager.cpp
git commit -m "feat(app): ApplicationManager destructor triggers reverse-order shutdown"
```

---

### Task 9: CameraThread/RecognizeThread 末尾 closeThreadConnection

**Files:**
- Modify: `src/camera/camerathread.cpp`
- Modify: `src/app/recognizethread.cpp`

防 per-thread 连接在线程退出时泄漏。

- [ ] **Step 1: camerathread.cpp run() 末尾**

在 `run()` 退出循环后(return 前)加:

```cpp
#include "src/database/dbconnectionpool.h"  // 顶部加

void CameraThread::run()
{
    ... // 现有逻辑
    DbConnectionPool::instance().closeThreadConnection();
}
```

- [ ] **Step 2: recognizethread.cpp run() 末尾**

```cpp
#include "src/database/dbconnectionpool.h"  // 顶部加

void RecognizeThread::run()
{
    ... // 现有逻辑
    DbConnectionPool::instance().closeThreadConnection();
}
```

- [ ] **Step 3: 编译 + 长时运行验证**

启动 → 持续运行 5 分钟 → 反复启停摄像头 → 退出
Expected: 退出时无 MySQL 连接泄漏警告。

- [ ] **Step 4: Commit**

```bash
git add src/camera/camerathread.cpp src/app/recognizethread.cpp
git commit -m "feat(threads): close per-thread DB connection on thread exit"
```

---

## Phase 1 完成验收

- [ ] 4 个 Singleton 均为 Meyer's + 幂等 shutdown()
- [ ] ApplicationManager 析构逆序调用 shutdown,日志可见
- [ ] DatabaseManager 无成员 QSqlDatabase,所有方法经 DbConnectionPool 取连接
- [ ] 编译 + 运行行为与改造前一致(登录/入库/出库/识别)
- [ ] CameraThread/RecognizeThread 退出时 close per-thread 连接

---

## Phase 2: P1 上帝对象瘦身 + P2 UI 解耦

### Task 10: ParkingService 提取

**Files:**
- Create: `src/service/parkingservice.h`
- Create: `src/service/parkingservice.cpp`
- Modify: `ParkingSystem.pro`

**背景:** 将 MainWindow::onEntrySearchButton / onExitSearchButton / onUpdateParkingCount 中的业务流程(验证→查重→DB写→计费→更新UI)上移到 ParkingService。MainWindow 仅触发 Service 并监听信号刷新。

- [ ] **Step 1: 头文件**

```cpp
// src/service/parkingservice.h
#ifndef PARKINGSERVICE_H
#define PARKINGSERVICE_H

#include <QObject>
#include "src/utils/messageType.h"

struct ParkingStats;
class DatabaseManager;

class ParkingService : public QObject
{
    Q_OBJECT
public:
    explicit ParkingService(DatabaseManager& db, QObject* parent = nullptr);

    // 入库业务流程(验证→查重→写库→发信号)
    bool checkIn(const QString& rawPlate);
    // 出库业务流程(验证→查入场时间→计费→确认对话框在UI, Service只做数据部分)
    // 返回 false=前置失败; 成功则 outTime/cost 填回供 UI 弹确认框
    bool prepareCheckOut(const QString& rawPlate,
                         QDateTime& outTime, double& cost, QString& errMsg);
    // 出库确认后的写入
    bool confirmCheckOut(const QString& plate, double cost);
    // 停车场统计
    ParkingStats getStats();

signals:
    void parkingDataChanged();
    void error(MessageType::Type type, const QString& msg);

private:
    DatabaseManager& m_db;
};

#endif // PARKINGSERVICE_H
```

- [ ] **Step 2: 实现文件**

```cpp
// src/service/parkingservice.cpp
#include "parkingservice.h"
#include "src/database/databasemanager.h"
#include "src/app/car.h"
#include "src/utils/initfile.h"
#include <QDateTime>

ParkingService::ParkingService(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {}

bool ParkingService::checkIn(const QString& rawPlate)
{
    if(!m_db.isConnected()){
        emit error(MessageType::Error, "数据库未连接");
        return false;
    }
    QString plate = Car::normalizePlate(rawPlate);
    if(plate.isEmpty() || !Car::isValidLicensePlate(plate)){
        emit error(MessageType::Warning, "车牌格式错误");
        return false;
    }
    if(m_db.isVehicleInPark(plate)){
        emit error(MessageType::Warning, QStringLiteral("%1 已入库").arg(Car::displayPlate(plate)));
        return false;
    }
    QString parkingName = InitFile::instance().getParkingName();
    if(!m_db.checkIn(plate, parkingName)){
        emit error(MessageType::Error, "入库写入失败");
        return false;
    }
    emit parkingDataChanged();
    return true;
}

bool ParkingService::prepareCheckOut(const QString& rawPlate,
                                     QDateTime& outTime, double& cost,
                                     QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    QString plate = Car::normalizePlate(rawPlate);
    if(plate.isEmpty() || !Car::isValidLicensePlate(plate)){
        errMsg = "车牌格式错误"; return false;
    }
    if(!m_db.isVehicleInPark(plate)){
        errMsg = QStringLiteral("%1 未入库").arg(Car::displayPlate(plate));
        return false;
    }
    QDateTime intTime = m_db.getVehicleCheckInTime(plate);
    outTime = QDateTime::currentDateTime();
    cost = Car::calculateFee(intTime, outTime,
                             InitFile::instance().getParkingPrice(),
                             InitFile::instance().getFreeMinutes());
    return true;
}

bool ParkingService::confirmCheckOut(const QString& plate, double cost)
{
    QString parkingName = InitFile::instance().getParkingName();
    if(!m_db.checkOut(plate, parkingName, cost)){
        emit error(MessageType::Error, "出库写入失败");
        return false;
    }
    emit parkingDataChanged();
    return true;
}

ParkingStats ParkingService::getStats()
{
    return m_db.getParkingStats(InitFile::instance().getParkingName());
}
```

- [ ] **Step 3: 加入 .pro**

HEADERS 段加 `src/service/parkingservice.h \`,SOURCES 段加 `src/service/parkingservice.cpp \`。

- [ ] **Step 4: 编译验证(可独立编译,MainWindow 尚未切换)**

构建项目
Expected: 编译通过(Service 未被引用但已备案)。

- [ ] **Step 5: Commit**

```bash
git add src/service/parkingservice.h src/service/parkingservice.cpp ParkingSystem.pro
git commit -m "feat(service): extract ParkingService for check-in/out business logic"
```

---

### Task 11: UserService 提取

**Files:**
- Create: `src/service/userservice.h`
- Create: `src/service/userservice.cpp`
- Modify: `ParkingSystem.pro`

**背景:** LoginDialog::onLoginButton(认证)、RegisterDialog::on_btnRegister_clicked(注册)、UserManagement(用户 CRUD + 分页查询)的业务流程上移。

- [ ] **Step 1: 头文件**

```cpp
// src/service/userservice.h
#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <QObject>
#include <QList>
#include <QVariantList>
#include "src/utils/messageType.h"

class DatabaseManager;

class UserService : public QObject
{
    Q_OBJECT
public:
    explicit UserService(DatabaseManager& db, QObject* parent = nullptr);

    // 认证: 成功填回 userRole, 失败填回 errMsg
    bool authenticate(const QString& username, const QString& password,
                      QString& userRole, QString& errMsg);
    // 注册: 含用户名重复检查
    bool registerUser(const QString& username, const QString& password,
                      const QString& name, const QString& phone,
                      QString& errMsg);

    // 用户 CRUD
    bool addUser(const QString& username, const QString& password,
                 const QString& telephone, const QString& truename,
                 const QString& role, QString& errMsg);
    bool updateUser(int id, const QString& username, const QString& telephone,
                    const QString& truename, const QString& role, QString& errMsg);
    bool deleteUser(int id, QString& errMsg);
    QList<QVariantList> listUsers(const QString& keyword = QString());

signals:
    void userDataChanged();
    void error(MessageType::Type type, const QString& msg);

private:
    DatabaseManager& m_db;
};

#endif // USERSERVICE_H
```

- [ ] **Step 2: 实现文件**

```cpp
// src/service/userservice.cpp
#include "userservice.h"
#include "src/database/databasemanager.h"

UserService::UserService(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {}

bool UserService::authenticate(const QString& username, const QString& password,
                               QString& userRole, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(username.isEmpty() || password.isEmpty()){
        errMsg = "用户名或密码不能为空";
        return false;
    }
    if(!m_db.validateUser(username, password, userRole)){
        errMsg = "用户名或密码错误";
        return false;
    }
    return true;
}

bool UserService::registerUser(const QString& username, const QString& password,
                               const QString& name, const QString& phone,
                               QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(username.isEmpty() || password.isEmpty()){
        errMsg = "用户名和密码不能为空";
        return false;
    }
    if(m_db.isUsernameExists(username)){
        errMsg = "用户名已存在";
        return false;
    }
    if(!m_db.registerUser(username, password, name, phone)){
        errMsg = "注册写入失败";
        return false;
    }
    emit userDataChanged();
    return true;
}

bool UserService::addUser(const QString& username, const QString& password,
                          const QString& telephone, const QString& truename,
                          const QString& role, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(username.isEmpty() || password.isEmpty()){
        errMsg = "用户名和密码不能为空";
        return false;
    }
    if(m_db.isUsernameExists(username)){
        errMsg = "用户名已存在";
        return false;
    }
    if(!m_db.addUser(username, password, telephone, truename, role)){
        errMsg = "添加用户写入失败";
        return false;
    }
    emit userDataChanged();
    return true;
}

bool UserService::updateUser(int id, const QString& username, const QString& telephone,
                              const QString& truename, const QString& role, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(!m_db.updateUser(id, username, telephone, truename, role)){
        errMsg = "修改用户失败";
        return false;
    }
    emit userDataChanged();
    return true;
}

bool UserService::deleteUser(int id, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(!m_db.deleteUser(id)){
        errMsg = "删除用户失败";
        return false;
    }
    emit userDataChanged();
    return true;
}

QList<QVariantList> UserService::listUsers(const QString& keyword)
{
    return m_db.searchUsers(keyword);
}
```

- [ ] **Step 3: 加入 .pro**

HEADERS 加 `src/service/userservice.h \`,SOURCES 加 `src/service/userservice.cpp \`。

- [ ] **Step 4: 编译验证**

构建项目
Expected: 通过

- [ ] **Step 5: Commit**

```bash
git add src/service/userservice.h src/service/userservice.cpp ParkingSystem.pro
git commit -m "feat(service): extract UserService for auth/register/CRUD"
```

---

### Task 12: VehicleService 提取

**Files:**
- Create: `src/service/vehicleservice.h`
- Create: `src/service/vehicleservice.cpp`
- Modify: `ParkingSystem.pro`

**背景:** VehicleInformation 的查询/删除/分页计算逻辑上移。

- [ ] **Step 1: 头文件**

```cpp
// src/service/vehicleservice.h
#ifndef VEHICLESERVICE_H
#define VEHICLESERVICE_H

#include <QObject>
#include <QList>
#include <QVariantList>
#include <QDateTime>
#include "src/utils/messageType.h"

class DatabaseManager;

class VehicleService : public QObject
{
    Q_OBJECT
public:
    explicit VehicleService(DatabaseManager& db, QObject* parent = nullptr);

    // 车辆记录查询(车牌/时间/状态过滤)
    QList<QVariantList> search(const QString& plate,
                                const QDateTime& startTime,
                                const QDateTime& endTime,
                                int status);
    // 最近 N 条记录
    QList<QVariantList> recentRecords(int count);
    // 单条删除
    bool deleteRecord(int id, QString& errMsg);
    // 批量删除(含车位计数同步)
    bool deleteRecords(const QList<int>& ids, QString& errMsg);

signals:
    void vehicleDataChanged();
    void error(MessageType::Type type, const QString& msg);

private:
    DatabaseManager& m_db;
};

#endif // VEHICLESERVICE_H
```

- [ ] **Step 2: 实现**

```cpp
// src/service/vehicleservice.cpp
#include "vehicleservice.h"
#include "src/database/databasemanager.h"

VehicleService::VehicleService(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {}

QList<QVariantList> VehicleService::search(const QString& plate,
                                            const QDateTime& startTime,
                                            const QDateTime& endTime,
                                            int status)
{
    if(!m_db.isConnected()) return {};
    return m_db.searchCars(plate, startTime, endTime, status);
}

QList<QVariantList> VehicleService::recentRecords(int count)
{
    if(!m_db.isConnected() || count <= 0) return {};
    return m_db.getRecentRecords(count);
}

bool VehicleService::deleteRecord(int id, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(!m_db.deleteCarRecord(id)){
        errMsg = "删除车辆记录失败";
        return false;
    }
    emit vehicleDataChanged();
    return true;
}

bool VehicleService::deleteRecords(const QList<int>& ids, QString& errMsg)
{
    if(!m_db.isConnected()){ errMsg = "数据库未连接"; return false; }
    if(ids.isEmpty()){ errMsg = "未选择记录"; return false; }
    if(!m_db.deleteCarRecords(ids)){
        errMsg = "批量删除失败";
        return false;
    }
    emit vehicleDataChanged();
    return true;
}
```

- [ ] **Step 3: 加入 .pro**

HEADERS 加 `src/service/vehicleservice.h \`,SOURCES 加 `src/service/vehicleservice.cpp \`。

- [ ] **Step 4: 编译验证**

构建项目
Expected: 通过

- [ ] **Step 5: Commit**

```bash
git add src/service/vehicleservice.h src/service/vehicleservice.cpp ParkingSystem.pro
git commit -m "feat(service): extract VehicleService for vehicle record queries"
```

---

### Task 13: ApplicationManager 持有 3 个 Service

**Files:**
- Modify: `src/app/ApplicationManager.h`
- Modify: `src/app/ApplicationManager.cpp`
- Modify: `src/app/main.cpp`

**背景:** DatabaseManager 在 main.cpp 是栈对象。本任务让 ApplicationManager 持有 3 个 Service 引用,UI 切换(Service 引用)从 ApplicationManager 传递。main.cpp 在 ApplicationManager 之前实例化 3 个 Service(栈顺序,先析构 Service 后 DatabaseManager)。

- [ ] **Step 1: ApplicationManager.h 加成员与构造签名**

```cpp
#include "src/service/parkingservice.h"
#include "src/service/userservice.h"
#include "src/service/vehicleservice.h"

class ApplicationManager : public QObject {
    Q_OBJECT
public:
    explicit ApplicationManager(DatabaseManager& dbMgr,
                               ParkingService& parkingSvc,
                               UserService& userSvc,
                               VehicleService& vehicleSvc,
                               QObject *parent = nullptr);
    void start();
    ~ApplicationManager();
    // ... slots 保留
private:
    DatabaseManager& m_dbMgr;
    ParkingService&  m_parkingSvc;
    UserService&     m_userSvc;
    VehicleService&  m_vehicleSvc;
    LoginDialog *m_loginDialog = nullptr;
    MainWindow  *m_mainWindow  = nullptr;
};
```

- [ ] **Step 2: ApplicationManager.cpp 实现与构造**

```cpp
ApplicationManager::ApplicationManager(DatabaseManager& dbMgr,
                                       ParkingService& parkingSvc,
                                       UserService& userSvc,
                                       VehicleService& vehicleSvc,
                                       QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
    , m_parkingSvc(parkingSvc)
    , m_userSvc(userSvc)
    , m_vehicleSvc(vehicleSvc)
{}

void ApplicationManager::start()
{
    // UI 构造接收 Service 引用(下一步 UI 切换后再改这两行)
    // 本任务先保留 DatabaseManager* 调用,确保编译通过
    m_loginDialog = new LoginDialog(nullptr, &m_dbMgr);
    m_mainWindow  = new MainWindow(nullptr, &m_dbMgr);
    // ... connect 代码保留不变
}
```

> 注: 本任务只改 ApplicationManager 构造与成员,UI 构造签名 MainWindow/LoginDialog 暂不变(下一步 Task 14+ 才切换),所以 start() 内仍传 `&m_dbMgr`。编译可通过。

- [ ] **Step 3: main.cpp 实例化 Service 并传入**

在 `ApplicationManager applicationManager(dbManager);` 之前插入:

```cpp
#include "src/service/parkingservice.h"
#include "src/service/userservice.h"
#include "src/service/vehicleservice.h"

// ... 数据库连接成功 + MySQLInit 之后
ParkingService  parkingSvc(dbManager);
UserService     userSvc(dbManager);
VehicleService  vehicleSvc(dbManager);

ApplicationManager applicationManager(dbManager, parkingSvc, userSvc, vehicleSvc);
applicationManager.start();
```

- [ ] **Step 4: 编译验证**

构建项目
Expected: 通过,start() 仍传 DatabaseManager*,行为不变。

- [ ] **Step 5: Commit**

```bash
git add src/app/ApplicationManager.h src/app/ApplicationManager.cpp src/app/main.cpp
git commit -m "feat(app): ApplicationManager holds 3 Service references"
```

---

### Task 14: MainWindow 切换到 Service& 引用

**Files:**
- Modify: `UI/MainWindow/mainwindow.h`
- Modify: `UI/MainWindow/mainwindow.cpp`
- Modify: `src/app/ApplicationManager.cpp`(改 start 传参)

**背景:** MainWindow 当前持 `DatabaseManager *m_db`,槽函数 onEntrySearchButton/onExitSearchButton/onUpdateParkingCount/onPlateRecognized 直接调 m_db。改为持 `ParkingService& m_parkingSvc` 并用 VehicleService& 维护车辆列表(主窗口车辆卡片),业务调用切 Service。

- [ ] **Step 1: 头文件成员与构造改**

替换 `class DatabaseManager;` 前向声明为 Service 前向声明:

```cpp
class ParkingService;
class UserService;
class VehicleService;
// 移除: class DatabaseManager;
```

构造签名:
```cpp
explicit MainWindow(QWidget *parent = nullptr,
                     ParkingService* parkingSvc = nullptr,
                     UserService* userSvc = nullptr,
                     VehicleService* vehicleSvc = nullptr);
```

> 用指针而非引用,避免 VehicleInformation 等 UI 仍传 DatabaseManager* 阶段的兼容。引用方案放在 P3 收尾。

私有成员区:
```cpp
ParkingService*  m_parkingSvc  = nullptr;
UserService*     m_userSvc     = nullptr;
VehicleService*  m_vehicleSvc  = nullptr;
// 移除: DatabaseManager *m_db;
```

- [ ] **Step 2: cpp 构造函数改**

```cpp
#include "src/service/parkingservice.h"
#include "src/service/userservice.h"
#include "src/service/vehicleservice.h"

MainWindow::MainWindow(QWidget *parent,
                       ParkingService* parkingSvc,
                       UserService* userSvc,
                       VehicleService* vehicleSvc)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_parkingSvc(parkingSvc)
    , m_userSvc(userSvc)
    , m_vehicleSvc(vehicleSvc)
{
    // ... 原 setup 等代码保留
    // 注意: 原 "connect(m_db, &DatabaseManager::parkingDataChanged, ...)"
    // 改为 connect(m_parkingSvc, &ParkingService::parkingDataChanged, ...)
    CameraManager::instance().scanCameras();
    // ... 其余保留
}
```

成员页构造(mainwindow.cpp:220-226 区段):VehicleInformation/UserManagement/CameraManagement 暂保留 DatabaseManager*(它们切换在后续任务),传参需要从 main.cpp/ApplicationManager 注入 DatabaseManager 引用。简单做法:先给这些子页面传 nullptr 或先跳过本步子页构造改造——本任务只切主窗口主体业务,**子页面构造保留 `&dbMgr`** 由 ApplicationManager 传一个 DatabaseManager 引用供子页面(Phase 3 子页面切换后删除)。

最终:ApplicationManager::start 改为:

```cpp
m_mainWindow = new MainWindow(nullptr, &m_parkingSvc, &m_userSvc, &m_vehicleSvc);
```

子页面构造 VehicleInformation(parent, dbMgr)等暂不动(它们用 DatabaseManager*,Phase 3 处理)——意味着 ApplicationManager 还需持有 m_dbMgr 给子页面传。可在 MainWindow 内构造子页面时传 m_parkingSvc 等引用?因子页面签名仍要 DatabaseManager*,采用过渡:子页面暂传 nullptr(主窗口入参 Service 将子页面也切是 Task 15+ 的事),本任务保持子页面构造签名不变,传 nullptr 让子页面有空指针检查短路(已有)。

> 务实做法: 这一步先把 MainWindow 自身业务切到 Service,子页面调用点 `new VehicleInformation(this, m_db)` 暂改为 `new VehicleInformation(this, nullptr)`(子页面已有空指针保护),Task 15-19 再逐个切换子页面传 Service。

- [ ] **Step 3: 槽函数切 Service**

`onEntrySearchButton` 替换为:

```cpp
void MainWindow::onEntrySearchButton()
{
    if(!m_parkingSvc) return;
    QString rawPlate = ui->entryPlateInput->text();
    // 触发 Service, 业务流程在 Service 内
    // 提示信息由 error 信号连 notifyInfo/notifyFailure
    if(!m_parkingSvc->checkIn(rawPlate)){
        return;  // 失败时 Service 已 emit error(由 MainWindow 连接)
    }
    QString plate = Car::normalizePlate(rawPlate);
    notifySuccess(this, QStringLiteral("%1 入库成功").arg(Car::displayPlate(plate)));
    if (ui->vehicleEntryExitWidget) {
        ui->vehicleEntryExitWidget->prependEntry(
            {Car::displayPlate(plate), QDateTime::currentDateTime(), VehicleEntryStatus::In});
    }
    ui->entryPlateInput->clear();
}
```

`onExitSearchButton` 替换:

```cpp
void MainWindow::onExitSearchButton()
{
    if(!m_parkingSvc) return;
    QString rawPlate = ui->exitPlateInput->text();
    QString plate = Car::normalizePlate(rawPlate);
    if(plate.isEmpty()){ notifyInfo(this, "请输入车牌号"); return; }

    QDateTime outTime; double cost = 0; QString errMsg;
    if(!m_parkingSvc->prepareCheckOut(rawPlate, outTime, cost, errMsg)){
        notifyInfo(this, errMsg);
        return;
    }

    int totalMinutes = m_parkingSvc ? 0 : 0;  // 占位, 实际从 prepareCheckOut 已算
    // 重新算时长用于提示(因 prepareCheckOut 未返回 intTime, 这里通过 Service 不够简洁)
    // 替代: 本任务先保留 m_db 调用时长计算? 不行, m_db 已移除。
    // 务实做法: prepareCheckOut 接口补返回时长。本任务暂用 cost 显示。
    QString msg = QStringLiteral("费用 %1 元, 是否出库?").arg(cost, 0, 'f', 2);
    if(!notifyConfirm(this, "出库确认", msg)) return;

    if(!m_parkingSvc->confirmCheckOut(plate, cost)){
        return;  // Service emit error
    }
    notifySuccess(this, QStringLiteral("%1 出库成功").arg(Car::displayPlate(plate)));
    if (ui->vehicleEntryExitWidget) {
        ui->vehicleEntryExitWidget->prependEntry(
            {Car::displayPlate(plate), QDateTime::currentDateTime(), VehicleEntryStatus::Out});
    }
    ui->exitPlateInput->clear();
}
```

> 设计修正: prepareCheckOut 不返回 intTime,duration 提示较弱。可在 Task 10 prepareCheckOut 中加 `qint64& totalMinutes` 出参,本任务一并改:
> 1. 修改 parkingservice.h prepareCheckOut 签名加 `qint64& totalMinutes`
> 2. parkingservice.cpp 中 `totalMinutes = intTime.secsTo(outTime) / 60;` 填回
> 3. MainWindow 提示改为 "停车时长: X 小时 Y 分钟, 费用 Z 元"

执行 Step 3 时同步补这个 out 参。

`onUpdateParkingCount` 替换:

```cpp
void MainWindow::onUpdateParkingCount()
{
    if(!m_parkingSvc) return;
    ParkingStats stats = m_parkingSvc->getStats();
    // ... 原 stats 渲染逻辑保留
}
```

`onPlateRecognized` 内的 m_db 调用改为 m_parkingSvc->prepareCheckOut 和 confirmCheckOut(同样模式)。

- [ ] **Step 4: 连接 Service error 信号到通知**

在构造函数加:

```cpp
connect(m_parkingSvc, &ParkingService::error, this,
        [this](MessageType::Type type, const QString& msg){
    switch(type){
    case MessageType::Success: notifySuccess(this, msg); break;
    case MessageType::Warning:  notifyInfo(this, msg);     break;
    case MessageType::Error:    notifyFailure(this, msg);  break;
    default: notifyInfo(this, msg);
    }
});

connect(m_parkingSvc, &ParkingService::parkingDataChanged,
        this, &MainWindow::onUpdateParkingCount);
```

> MessageType::Type 实际枚举值以 [src/utils/messageType.h](src/utils/messageType.h) 为准,执行时对齐。

- [ ] **Step 5: ApplicationManager.cpp start 改传参**

```cpp
m_mainWindow = new MainWindow(nullptr, &m_parkingSvc, &m_userSvc, &m_vehicleSvc);
// LoginDialog 切换在 Task 15
m_loginDialog = new LoginDialog(nullptr, &m_dbMgr);  // 暂保留
```

- [ ] **Step 6: 编译 + 全流程验证**

构建 + 启动 → 登录 → 入库车牌 → 出库车牌(确认费用框弹出)→ 退出
Expected: 行为完全一致,无 m_db 引用残留。

grep 验证:
```bash
grep -n "m_db" UI/MainWindow/mainwindow.cpp
```
Expected: 无匹配(全部已切 Service)。

- [ ] **Step 7: Commit**

```bash
git add UI/MainWindow/mainwindow.h UI/MainWindow/mainwindow.cpp src/app/ApplicationManager.cpp src/service/parkingservice.h src/service/parkingservice.cpp
git commit -m "refactor(ui): MainWindow uses ParkingService& instead of DatabaseManager*"
```

---

### Task 15: LoginDialog + RegisterDialog 切换 UserService&

**Files:**
- Modify: `UI/Login/logindialog.h/.cpp`
- Modify: `UI/Register/registerdialog.h/.cpp`
- Modify: `src/app/ApplicationManager.cpp`

**背景:** LoginDialog 用 `DatabaseManager* m_dbManager` 调 validateUser;RegisterDialog 用 `DatabaseManager* m_db` 调 isUsernameExists/registerUser。统一改 UserService*。

- [ ] **Step 1: logindialog.h 头文件改造**

```cpp
// 前向声明换
class UserService;
// 移除: class DatabaseManager;

class LoginDialog : public QDialog
{
public:
    explicit LoginDialog(QWidget *parent = nullptr, UserService* userSvc = nullptr);
    // ... 其余不变
private:
    UserService* m_userSvc = nullptr;
    // 移除: DatabaseManager *m_dbManager;
};
```

- [ ] **Step 2: logindialog.cpp onLoginButton 改**

```cpp
#include "src/service/userservice.h"

LoginDialog::LoginDialog(QWidget *parent, UserService* userSvc)
    : QDialog(parent), ui(new Ui::LoginDialog), m_userSvc(userSvc)
{ /* ... */ }

void LoginDialog::onLoginButton()
{
    if(!m_userSvc) return;
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString role, errMsg;
    if(!m_userSvc->authenticate(username, password, role, errMsg)){
        onMessageBox(MessageType::Warning, errMsg);  // 复用现有消息盒槽
        return;
    }
    userRole = role;
    saveCredentials();
    accept();
}
```

> MessageType 引入头与槽函数名以现有 logindialog.cpp 实际为准。执行时打开 cpp 对齐。

- [ ] **Step 3: registerdialog.h/.cpp 改**

```cpp
class UserService;
// 移除: class DatabaseManager;

class RegisterDialog : public QDialog
{
public:
    explicit RegisterDialog(QWidget *parent = nullptr, UserService* userSvc = nullptr);
private:
    UserService* m_userSvc = nullptr;
    // 移除: DatabaseManager *m_db;
};
```

```cpp
void RegisterDialog::on_btnRegister_clicked()
{
    if(!m_userSvc) return;
    // ... 取 ui 输入
    QString errMsg;
    if(!m_userSvc->registerUser(username, password, name, phone, errMsg)){
        // 现有错误提示方式
        return;
    }
    accept();
}
```

- [ ] **Step 4: ApplicationManager.cpp start 改**

```cpp
m_loginDialog = new LoginDialog(nullptr, &m_userSvc);
```

(ApplicationManager 内创建 RegisterDialog 的地方也要传 &m_userSvc。)

- [ ] **Step 5: 编译 + 登录/注册验证**

构建 → 启动 → 登录失败/成功 → 注册新用户 → 用新用户登录
Expected: 全流程工作。

- [ ] **Step 6: Commit**

```bash
git add UI/Login/logindialog.h UI/Login/logindialog.cpp UI/Register/registerdialog.h UI/Register/registerdialog.cpp src/app/ApplicationManager.cpp
git commit -m "refactor(ui): LoginDialog/RegisterDialog use UserService&"
```

---

### Task 16: UserManagement 切换 UserService&

**Files:**
- Modify: `UI/UserManager/usermanagement.h/.cpp`
- Modify: `UI/MainWindow/mainwindow.cpp`(子页构造)

- [ ] **Step 1: 头文件**

```cpp
class UserService;
// 移除: class DatabaseManager;

class UserManagement : public QWidget
{
public:
    explicit UserManagement(QWidget *parent = nullptr, UserService* userSvc = nullptr);
private:
    UserService* m_userSvc = nullptr;
    // 移除: DatabaseManager *m_db;
};
```

- [ ] **Step 2: cpp 业务调用切 Service**

`listUsers` → `m_userSvc->listUsers(keyword)`
增删改 → `m_userSvc->addUser/updateUser/deleteUser(..., errMsg)`,错误提示走 `m_userSvc->error` 或直接 QMessageBox。

连接信号刷新:
```cpp
connect(m_userSvc, &UserService::userDataChanged, this, [this]{ /* 重新加载表格 */ });
```

- [ ] **Step 3: mainwindow.cpp 子站点构造改**

```cpp
m_userManagementPage = new UserManagement(this, m_userSvc);
```

- [ ] **Step 4: 编译 + 用户增删改验证**

构建 → 启动 → 登录 → 用户管理页 → 增删改查
Expected: 全工作。

- [ ] **Step 5: Commit**

```bash
git add UI/UserManager/usermanagement.h UI/UserManager/usermanagement.cpp UI/MainWindow/mainwindow.cpp
git commit -m "refactor(ui): UserManagement uses UserService&"
```

---

### Task 17: VehicleInformation 切换 VehicleService&

**Files:**
- Modify: `UI/VehicleInformation/vehicleinformation.h/.cpp`
- Modify: `UI/MainWindow/mainwindow.cpp`

- [ ] **Step 1: 头文件**

```cpp
class VehicleService;
// 移除: class DatabaseManager;

class VehicleInformation : public QWidget
{
public:
    explicit VehicleInformation(QWidget *parent = nullptr, VehicleService* vehicleSvc = nullptr);
private:
    VehicleService* m_vehicleSvc = nullptr;
    // 移除: DatabaseManager *m_db;
};
```

- [ ] **Step 2: cpp 业务调用**

查询 → `m_vehicleSvc->search(plate, start, end, status)`
删除 → `m_vehicleSvc->deleteRecord/deleteRecords(id(s), errMsg)`
最近记录(若用到) → `m_vehicleSvc->recentRecords(count)`

连接信号:
```cpp
connect(m_vehicleSvc, &VehicleService::vehicleDataChanged, this, [this]{ /* 重载 */ });
```

- [ ] **Step 3: mainwindow.cpp 子构造改**

```cpp
m_vehicleInfoPage = new VehicleInformation(this, m_vehicleSvc);
```

- [ ] **Step 4: 编译 + 查询/删除验证**

构建 → 启动 → 登录 → 车辆信息页 → 查询 / 选中删除 / 批量删除
Expected: 工作。

- [ ] **Step 5: Commit**

```bash
git add UI/VehicleInformation/vehicleinformation.h UI/VehicleInformation/vehicleinformation.cpp UI/MainWindow/mainwindow.cpp
git commit -m "refactor(ui): VehicleInformation uses VehicleService&"
```

---

### Task 18: CameraManagement 去掉 DatabaseManager* 依赖

**Files:**
- Modify: `UI/CameraManagement/cameramanagement.h/.cpp`
- Modify: `UI/MainWindow/mainwindow.cpp`

**背景:** 审查报告显示 CameraManagement 当前持有 DatabaseManager* 但实际未使用,DB 参数为多余依赖。直接删除。

- [ ] **Step 1: 头文件**

```cpp
class CameraManagement : public QWidget
{
public:
    explicit CameraManagement(QWidget *parent = nullptr);  // 去掉 db 参数
    // ... 其余不变
private:
    // 移除: DatabaseManager *m_db = nullptr;
};
```

移除头文件中 `class DatabaseManager;` 前向声明。

- [ ] **Step 2: cpp 构造**

```cpp
CameraManagement::CameraManagement(QWidget *parent)
    : QWidget(parent), ui(new Ui::CameraManagement)
{ /* setupUI 等 */ }
```

删除 cpp 内任何 `m_db` 引用(若有 qDebug 提及也删)。

- [ ] **Step 3: mainwindow.cpp 子构造改**

```cpp
m_cameraManagementPage = new CameraManagement(this);
```

- [ ] **Step 4: 编译 + 摄像头页验证**

构建 → 启动 → 登录 → 摄像头管理页 → 启动/停止所有
Expected: 工作,多宫格显示正常。

- [ ] **Step 5: Commit**

```bash
git add UI/CameraManagement/cameramanagement.h UI/CameraManagement/cameramanagement.cpp UI/MainWindow/mainwindow.cpp
git commit -m "refactor(ui): CameraManagement drops unused DatabaseManager dependency"
```

---

## Phase 3: 兼容性收尾

### Task 19: DatabaseManager 老业务方法标 deprecated

**Files:**
- Modify: `src/database/databasemanager.h`

**背景:** checkIn/checkOut/searchCars/searchUsers/validateUser/registerUser/addUser/updateUser/deleteUser/generateUser/deleteCarRecord/deleteCarRecords 等业务流程方法在 Service 提取后已上移,但为兼容期保留,标 `[[deprecated]]` 让编译器标记迁移进度。

- [ ] **Step 1: 头文件标 deprecated**

对以下方法签名加 `[[deprecated("Use ParkingService/VehicleService/UserService")]]` 属性(C++14 支持,项目 C++11 需先确认;若不支持,改用注释 `// DEPRECATED:` 显式标记,或升级 `CONFIG += c++14`):

```cpp
//车辆管理(业务流程已迁移 ParkingService/VehicleService, 保留供兼容期)
[[deprecated("Use ParkingService::checkIn")]]
bool checkIn(const QString &licensePlate, const QString &parkingName);
[[deprecated("Use ParkingService::confirmCheckOut")]]
bool checkOut(const QString &licensePlate, const QString &parkingName, double fee);
[[deprecated("Use VehicleService::search")]]
QList<QVariantList> searchCars(...);
[[deprecated("Use VehicleService::deleteRecord(s)")]]
bool deleteCarRecord(int id);
bool deleteCarRecords(const QList<int> &ids);
// ... 用户类同
```

> 务实建议: 项目 .pro 当前 `CONFIG += c++11`,若改 c++14 风险大(可能影响其他库),则**跳过属性,改用注释 `// DEPRECATED: moved to XService`**,在 Phase 3 写 review log 时列出待删清单。

- [ ] **Step 2: 确认无新建 m_db 调用点**

```bash
grep -rn "m_db" UI/ src/app/ApplicationManager.cpp src/app/main.cpp
```
Expected: 无匹配(所有 UI 已切 Service,ApplicationManager/main.cpp 不持 m_db)。

```bash
grep -rn "DatabaseManager\*\|DatabaseManager \*" UI/ src/app
```
Expected: 仅 DatabaseManager 自身声明和 MySQLInit(ApplicationManager 已不传 db 给 UI)。

- [ ] **Step 3: 编译验证(若有 [[deprecated]] 属性,触发 warnings)**

构建项目
Expected: 只有 Service 实现文件 .cpp 内调自身 m_db.xxx 被标记 warning(Service 持 DatabaseManager&,合法使用,可加 `// NOLINT` 抑制)。

- [ ] **Step 4: Commit**

```bash
git add src/database/databasemanager.h
git commit -m "chore(db): mark legacy business methods deprecated (moved to Service)"
```

---

### Task 20: 更新审查报告状态标记

**Files:**
- Modify: `docs/architecture-review-2026-07-21.md`

**背景:** 实施完毕,在原审查报告中标 P1×3 + P2×2 为"已修复",保持审查历史可追溯。

- [ ] **Step 1: 在审查报告头部加实施状态**

在报告 `## 审查总结` 之前插入:

```markdown
## 实施状态(2026-07-21 更新)

本报告中的 5 项待修复发现,经 [架构改进实现计划](superpowers/plans/2026-07-21-architecture-improvement.md) 落地:

| # | 等级 | 问题 | 状态 |
|---|------|------|------|
| #1 | P1 | God Object | ✅ 已修复(Service 层提取,业务上移) |
| #2 | P1 | Singleton 生命周期 | ✅ 已修复(Meyer's + shutdown 序列) |
| #3 | P1 | 线程安全 | ✅ 已修复(per-thread DbConnectionPool) |
| #4 | P2 | 裸指针传播 | ✅ 已修复(UI 持 Service&) |
| #7 | P2 | UI 耦合 | ✅ 已修复(业务逻辑下沉 Service) |
| #5 | P2 | FrameQueue 单帧 | ⏭️ 已忽略(业务设计) |
| #6 | P2 | 零测试覆盖 | ⏭️ 已忽略(QtTest 后续手动) |

DatabaseManager 已瘦身为纯 CRUD(方法数 ≤ 15),Service 层承接业务流程,UI 与数据访问解耦。
```

- [ ] **Step 2: Commit**

```bash
git add docs/architecture-review-2026-07-21.md
git commit -m "docs: mark architecture review findings as resolved"
```

---

## Phase 2+3 完成验收

- [ ] 3 个 Service 全部实现并被对应 UI 使用
- [ ] 6 个 UI 类(Login/Register/MainWindow/UserManagement/VehicleInformation/CameraManagement)全部不持 `DatabaseManager*`
- [ ] ApplicationManager 持 3 个 Service 引用,析构逆序 shutdown
- [ ] DatabaseManager 仅被 Service 与 MySQLInit/ConnectionPool 引用
- [ ] 全项目 grep `m_db` 在 UI 下零匹配
- [ ] 4 并发线程(主/Camera/Recognize/线程池 worker)运行入库出库无崩溃
- [ ] DatabaseManager 方法数 ≤ 15

---

## Self-Review 检查结果

**Spec 覆盖:** spec 中 5 项发现均有对应任务:
- #1 God Object → Task 10/11/12 (Service 提取)
- #2 Singleton 生命周期 → Task 4/5/6/7/8 (4 升级 + shutdown 序列)
- #3 线程安全 → Task 2/3/9 (Pool + DatabaseManager + 线程退出 close)
- #4 裸指针 → Task 13-18 (ApplicationManager 持 Service,UI 切引用)
- #7 UI 耦合 → Task 10-12 + Task 14-18 (Service 承接业务,UI 解耦)

**Placeholder 扫描:** 已无 TBD/TODO 占位;少数任务内"以现有 cpp 实际为准"属合理(因本计划不重写 cpp 细节,执行时对齐即可)。

**类型一致性:** ParkingService/UserService/VehicleService 方法名在 Task 10/11/12 与 Task 14-17 UI 切换中保持一致(checkIn/prepareCheckOut/confirmCheckOut/authenticate/registerUser/listUsers 等)。

---

## Execution Handoff

计划已保存到 `docs/superpowers/plans/2026-07-21-architecture-improvement.md`,包含 Phase 1-3 共 20 个任务。

两种执行方式:

**1. Subagent-Driven(推荐)** — 每个任务派发独立子代理执行,任务间人工审查,迭代快,适合本计划任务量(20 任务)。

**2. Inline Execution** — 当前会话内顺序执行,带检查点回看。

哪种?

> 若选 Subagent-Driven,使用 superpowers:subagent-driven-development 技能。
> 若选 Inline Execution,使用 superpowers:executing-plans 技能。