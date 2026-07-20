# 架构改进设计 — ParkingSystem

**日期:** 2026-07-21
**对应审查:** [docs/architecture-review-2026-07-21.md](../../architecture-review-2026-07-21.md)
**范围:** P1×3 (线程安全、Singleton 生命周期、上帝对象) + P2×2 (裸指针、UI 耦合) 共 5 项
**已排除:** FrameQueue 单帧缓冲(业务设计)、零测试覆盖(QtTest 后续手动引入)

## 1. 背景

架构审查发现 11 项问题,本计划覆盖其中 5 项核心改进,采用渐进式 Service 层提取方案。核心目标:

1. 修复 `DatabaseManager` 跨线程不安全问题(4 个线程并发访问)
2. 统一 4 个 Singleton 的生命周期管理,消除析构顺序未定义风险
3. 提取 Service 层缓解上帝对象,同时解决 UI 与数据访问耦合
4. UI 层依赖从裸指针 `DatabaseManager*` 改为 Service 引用,消除空指针风险

## 2. 改进后架构

```
┌─────────────────────────────────────────────────────────────┐
│                          UI Layer                            │
│  LoginDialog  RegisterDialog  MainWindow                     │
│  VehicleInformation  UserManagement  CameraManagement        │
│       └─ 持 Service& 引用,不再持 DatabaseManager* ─────────┘ │
├──────────────────────────▼───────────────────────────────────┤
│                    Service Layer (新)                         │
│  ParkingService  UserService  VehicleService                 │
│   (业务流程编排,发业务信号,统一错误出口)                       │
├──────────────────────────▼───────────────────────────────────┤
│                  Data Access Layer                           │
│  DatabaseManager (瘦身后纯 CRUD,无业务流程)                   │
│   └─ DbConnectionPool (新): per-thread QSqlDatabase 管理     │
└─────────────────────────────────────────────────────────────┘

Singletons (统一 Meyer's,显式 shutdown)
  InitFile  PlateRecognize  CameraManager  ThreadPoolManager
```

### 2.1 Singleton shutdown 顺序(逆序依赖)

ApplicationManager 析构时固定调用:

```
ThreadPoolManager::shutdown()   →  等待识别任务完成
CameraManager::shutdown()       →  停止所有摄像头线程
PlateRecognize::shutdown()      →  释放模型资源
InitFile::shutdown()            →  配置写盘
```

## 3. 详细设计

### 3.1 DbConnectionPool(新,P1 线程安全)

**文件:** `src/database/dbconnectionpool.h/.cpp`

```cpp
class DbConnectionPool {
public:
    static DbConnectionPool& instance();  // Meyer's
    // 取当前线程连接,不存在则 clone 自模板
    QSqlDatabase connection(const DbConfig& cfg);
    // 关闭并移除当前线程连接(线程退出前调用)
    void closeThreadConnection();
private:
    QMutex m_mutex;
    QHash<Qt::HANDLE, QSqlDatabase> m_conns;
};
```

**设计要点:**

- 每个线程首次调用时,用 `QSqlDatabase::cloneDatabase(templateConn, "conn_<ThreadId>")` 创建独立连接
- 连接名唯一:`QString("conn_%1").arg(reinterpret_cast<quptr>(QThread::currentThreadId()))`
- `CameraThread::run()` 与 `RecognizeThread::run()` 末尾调用 `closeThreadConnection()` 防泄漏
- 线程池 worker 任务首格从 pool 取连接(worker 也是独立线程)

**选型理由:** Qt 官方明确同一 `QSqlDatabase` 跨线程 exec 不安全。mutex 串行化会拖累 4 并发线程,per-thread 连接让各线程无锁并行。

### 3.2 DatabaseManager 瘦身(P1 + P2)

**移除:**

- 成员 `QSqlDatabase db;`、`bool connected;`
- 业务流程方法(上移到 Service):`checkIn`, `checkOut`, `searchCars`, `searchUsers`, `validateUser`, `registerUser`, `addUser` 的业务包装

**保留:** 纯数据 CRUD(无业务流程):

```cpp
class DatabaseManager {
public:
    bool connect(const DbConfig& cfg);
    bool isConnected();

    // 纯 CRUD(参数化,线程安全)
    bool insertCarCheckIn(...);
    bool updateCarCheckOut(...);
    bool isVehicleInPark(...);
    QDateTime queryCheckInTime(...);
    QList<QVariantList> queryCars(...);
    ParkingStats queryParkingStats(...);
    bool deleteCarRecord(int id);

    bool insertUser(...), updateUser(...), deleteUser(...);
    QList<QVariantList> queryUsers(...);
    bool validateCredentials(...);  // 认证原语

private:
    DbConfig m_config;
};
```

方法数从 25+ 降到 ~12。所有方法首行:`QSqlDatabase db = DbConnectionPool::instance().connection(m_config);`,`QSqlQuery query(db)`。

### 3.3 Service 层(P1 上帝对象 + P2 UI 解耦)

**ParkingService** (`src/service/parkingservice.h/.cpp`):

```cpp
class ParkingService : public QObject {
    Q_OBJECT
public:
    explicit ParkingService(DatabaseManager& db, QObject* parent = nullptr);
    bool checkIn(const QString& plate);
    bool checkOut(const QString& plate);
    ParkingStats getStats();
signals:
    void parkingDataChanged();
    void error(MessageType::Type type, const QString& msg);
private:
    DatabaseManager& m_db;
};
```

`checkOut` 内部编排:验证车牌 → 查入场时间 → `Car::calculateFee` 计费 → DB 更新 → emit 信号。

**UserService:** `authenticate`, `register`, `list(keyword)`, `add/update/delete`,信号 `userDataChanged`, `error`。

**VehicleService:** `search(plate, start, end, status)`, `deleteRecord(int id)`, `recentRecords(int)`,信号 `vehicleDataChanged`, `error`。

### 3.4 UI 依赖切换(P2 裸指针)

```cpp
// before
MainWindow(QWidget* parent, DatabaseManager* db);

// after
MainWindow(QWidget* parent,
           ParkingService& parkingSvc,
           VehicleService& vehicleSvc,
           UserService& userSvc);
```

- UI 持 Service 引用,引用天然非空 → 消除空指针检查分布不一致(P3-#8 顺手解决)
- UI 不再调 `m_db->checkIn(...)`,改 `m_parkingSvc.checkIn(...)`
- DatabaseManager 不再 emit `messageBox`,改为 Service emit `error`,UI 连 NotificationDialog/ToastWidget

### 3.5 错误处理统一

```cpp
bool ParkingService::checkIn(const QString& plate) {
    if(!m_db.isConnected()) { emit error(Error, "数据库未连接"); return false; }
    if(!Car::validatePlate(plate)) { emit error(Warning, "车牌格式错误"); return false; }
    if(m_db.isVehicleInPark(plate)) { emit error(Warning, "车辆已在场"); return false; }
    if(!m_db.insertCarCheckIn(plate, parkingName)) {
        emit error(Error, "入库写入失败: " + m_db.lastError()); return false;
    }
    emit parkingDataChanged();
    return true;
}
```

- Service 层为唯一错误出口(`emit error`)
- DatabaseManager 数据层只返回 bool + `lastError()` 字符串
- 所有 UI 的 error slot 连接到集中的 `MessageBus`(转发到 ToastWidget/NotificationDialog)

## 4. 实施阶段

```
Phase 1 — P1 主线(线程安全优先)
  P1.1  DbConnectionPool 提取与实现
  P1.2  DatabaseManager 切换为 per-thread 连接模型
  P1.3  ThreadPoolManager 升级为 Meyer's Singleton + shutdown()
  P1.4  ApplicationManager 注入 shutdown 逆序调用

Phase 2 — P1 上帝对象瘦身 + P2 UI 解耦
  P2.1  提取 ParkingService
  P2.2  提取 UserService
  P2.3  提取 VehicleService
  P2.4  UI 依赖切换:DatabaseManager* → Service& (分页面切换)

Phase 3 — 兼容性与收尾
  P3.1  DatabaseManager 老业务方法标 [[deprecated]]
  P3.2  清理全部旧调用点,删除 deprecated 方法
  P3.3  文档与 ASCII 图更新
```

## 5. 迁移兼容性

| 阶段 | 数据库老业务方法 | 调用点状态 |
|------|------------------|-----------|
| P2.x 进行中 | 保留,加 `[[deprecated]]` | 老新并存 |
| UI 切换完 | 仍保留 | 老调用归零 |
| P3 收尾 | 删除 | 不可调 |

- Service 构造只接受 `DatabaseManager&`,不改 DatabaseManager 老方法签名 → 老代码可并行编译运行
- UI 构造新增 Service 重载,而非改原签名 → 分页面渐进迁移

## 6. 失败场景与缓解

| 风险 | 影响 | 缓解 |
|------|------|------|
| per-thread 连接未 close → MySQL 连接泄漏 | 长期运行 | CameraThread/RecognizeThread::run 末尾强制 close |
| Service 与 DB 都 emit parkingDataChanged → UI 刷新两次 | 短期 bug | 收尾阶段删除 DB 的信号 |
| 线程池 worker 线程无 per-thread 连接 | 识别失败 | worker 任务首格从 pool 取连接 |
| shutdown 顺序错乱 → CameraThread 访问已析构 InitFile | 崩溃 | ApplicationManager 析构逆序固定 |

## 7. 非范围(NOT in scope)

- DatabaseManager 完全拆解为 3 个 Repository(留待后续,本次只提取 Service 不破坏 DB 类)
- 单元测试集成(QtTest 后续手动引入)
- FrameQueue 单帧→环形(业务设计保持)
- 收费规则每日封顶逻辑(设计文档 Phase 6 范畴)
- P3 低优先级项(死代码、轮询、格式转换)留待后续

## 8. 验收标准

- 4 个 Singleton 全部 Meyer's + 显式 shutdown() 幂等
- DatabaseManager 无成员 QSqlDatabase,所有方法通过 DbConnectionPool 取连接
- UI 类构造函数参数全部为 Service& 引用,无 DatabaseManager*
- 4 并发线程同时调用入库/出库操作,无崩溃、无 MySQL 连接泄漏
- DatabaseManager 方法数 ≤ 15,无业务流程方法
- 老业务调用点全部迁移到 Service,deprecated 方法删除

## 9. ASCII 架构图(改造后)

```
                    ApplicationManager (持有所有 Service&)
                              │ application exec loop
              ┌───────────────┼──────────────────┐
              ▼               ▼                   ▼
       ParkingService   UserService       VehicleService
              │               │                   │
              └───────────────┼───────────────────┘
                              ▼
                     DatabaseManager (持有 DbConfig)
                              │
                              ▼
                    DbConnectionPool (per-thread QSqlDatabase)
                              │
              ┌───────────────┼───────────────────┐
              ▼               ▼                   ▼
        UI Thread       CameraThread       RecognizeThread
        (conn_0)       (conn_threads)     (conn_threads)

Singletons (逆序 shutdown):
  ThreadPoolManager → CameraManager → PlateRecognize → InitFile
```

## 10. 未解决问题

| # | 问题 | 决策时机 |
|---|------|---------|
| 1 | 3 个 Service 是否共享 error 信号出口或分别 emit | 实现时按 UI 现有 NotificationDialog 决定 |
| 2 | DbConnectionPool 是否需最大连接数上限 | 视 MySQL 服务器 max_connections 调整,默认无限 |
| 3 | DatabaseManager 完全拆解为 Repository 的可行性 | 本次完成后评估是否进一步重构 |