# ParkingSystem 架构审查报告

**审查日期:** 2026-07-21
**审查范围:** 当前分支 master diff（83 文件, 10,968+ / 725-）
**审查方式:** gstack /plan-eng-review 流程
**参考文档:** [设计文档](docs/LENOVO-master-design-20260623-214343.md)

---

## 项目架构总览

```
┌─────────────────────────────────────────────────────────────┐
│                        UI Layer                              │
│  LoginDialog  RegisterDialog  MainWindow                     │
│  VehicleInformation  UserManagement  CameraManagement        │
│  CameraWindow  ConfigInitDialog                              │
├─────────────────────────────────────────────────────────────┤
│                     Core Logic (src/app/)                    │
│  ApplicationManager  Car  PlateRecognize(s)                  │
│  RecognizeThread  PlateConfirmTracker                        │
├─────────────────────────────────────────────────────────────┤
│  Camera (src/camera/)     │  Database (src/database/)        │
│  CameraManager(s)         │  DatabaseManager                  │
│  CameraThread × N          │  MySQLInit                        │
│  FrameQueue                │                                  │
├─────────────────────────────────────────────────────────────┤
│                     Utils (src/utils/)                        │
│  ThreadPoolManager(s)  InitFile(s)  PaginationWidget         │
│  ToastWidget  NotificationDialog  CustomDateChooser          │
│  DateLineEdit  TimeSelector  CalendarDayBtn                  │
│  YearMonthPopup  IconLineEdit                               │
├─────────────────────────────────────────────────────────────┤
│                   Third-party                                 │
│  OpenCV 4.11  HyperLPR-2                                    │
└─────────────────────────────────────────────────────────────┘
  (s) = Singleton
```

### 设计模式分布

| 模式 | 使用位置 | 评估 |
|------|---------|------|
| Singleton | InitFile, PlateRecognize, CameraManager, ThreadPoolManager | ⚠️ 4 个，无生命周期管理 |
| God Object | DatabaseManager | ⚠️ 25+ 方法，所有模块依赖 |
| Producer-Consumer | CameraThread → FrameQueue → RecognizeThread | ✅ 模式正确，但单帧缓冲有局限 |
| Observer (Signal/Slot) | 全局 Qt 信号槽 | ✅ Qt 标准模式 |
| Raw Pointer DI | 所有 UI 类构造函数 | ⚠️ 无 DI 容器，空指针检查不一致 |

---

## Section 1: 架构审查

### 发现 #1 [P1] DatabaseManager 上帝对象

**文件:** [src/database/databasemanager.h](src/database/databasemanager.h):16
**置信度:** 9/10

单个类承载了用户认证、车辆管理、停车场配置、统计数据、用户 CRUD 等 25+ 公共方法。所有 UI 组件（LoginDialog, RegisterDialog, MainWindow, VehicleInformation, UserManagement, CameraManagement）都通过构造函数裸指针依赖这一个类。

**具体问题：**
- 修改 `searchCars` 可能影响 `validateUser` 的 SQL 连接状态
- 无法单独测试车辆管理逻辑而不涉及用户管理
- 类已 700+ 行，违反单一职责原则

**建议:** 拆分为 `UserRepository`, `VehicleRepository`, `ParkingRepository`，每个 ~50-100 行。

---

### 发现 #2 [P1] Singleton 泛滥且无生命周期管理

**文件:** [src/utils/initfile.cpp](src/utils/initfile.cpp):8, [src/app/platerecognize.cpp](src/app/platerecognize.cpp):19, [src/camera/cameramanager.cpp](src/camera/cameramanager.cpp):11, [src/utils/pthreadpool.cpp](src/utils/pthreadpool.cpp):68
**置信度:** 8/10

4 个单例散布在不同模块中，初始化顺序隐式依赖调用时机，析构顺序未定义：

```
InitFile → CameraManager (scanCameras 读取 InitFile 配置)
CameraManager → CameraThread (拥有线程，析构调用 stopAll)
PlateRecognize ← RecognizeThread (通过 ThreadPoolManager 间接使用)
ThreadPoolManager ← CameraThread (共用 Qt 线程池资源)
```

**具体风险：**
- `CameraManager::scanCameras()` 隐式调用 `InitFile::instance()` ——如果 InitFile 先于 CameraManager 析构，scanCameras 重新调用会读已析构对象
- `ThreadPoolManager` 用裸指针 `static ThreadPoolManager* s_instance`（其他三个用 Meyer's Singleton），不一致
- 无法在单元测试中替换单例

**建议:** 
1. 统一为 Meyer's Singleton（函数内静态变量）
2. 添加显式 `shutdown()` 方法，由 `ApplicationManager` 在析构时按逆序调用

---

### 发现 #3 [P1] DatabaseManager 无线程安全保护

**文件:** [src/database/databasemanager.h](src/database/databasemanager.h):88-90
**置信度:** 8/10

`DatabaseManager` 被以下线程同时访问，成员 `db` 和 `connected` 无任何 mutex 保护：

| 访问线程 | 触发路径 |
|---------|---------|
| UI 线程 | MainWindow::onUpdateParkingCount → getParkingStats() |
| UI 线程 | 用户点击入库/出库 → checkIn/checkOut() |
| CameraThread | 帧捕获 → emit newFrameCaptured → UI slot → DB 查询 |
| RecognizeThread | 识别完成 → emit plateRecognized → UI slot → isVehicleInPark/checkIn |

`QSqlDatabase::exec()` 本身不是线程安全的——Qt 文档明确要求同一连接只能在创建它的线程中使用。

**建议:** 
1. 每条数据库操作在调用线程创建临时 `QSqlDatabase::addDatabase("QMYSQL", uniqueName)` 连接
2. 或使用 `QSqlDatabase::cloneDatabase()` 为每个线程创建独立连接
3. 至少为 `connected` 加 `QMutex` 保护

---

### 发现 #4 [P2] 裸指针传播，无所有权追踪

**文件:** 8 个 UI 类头文件
**置信度:** 9/10

`DatabaseManager*` 通过构造函数注入 8 个 UI 类，全部为裸指针：

```
main.cpp: DatabaseManager dbManager (栈对象)
  → ApplicationManager(DatabaseManager&)
    → LoginDialog(DatabaseManager*)
    → MainWindow(DatabaseManager*)
      → VehicleInformation(DatabaseManager*)
      → UserManagement(DatabaseManager*)
      → CameraManagement(DatabaseManager*)
```

部分方法有空指针检查（`if(!m_db) return;`），部分没有。`CameraManagement` 接受 `DatabaseManager*` 但在当前实现中未使用它。

**建议:** 使用 `std::shared_ptr<DatabaseManager>` 或在 ApplicationManager 中集中管理生命周期、通过引用传递。

---

### ~~发现 #5 [P2] FrameQueue 单帧缓冲无法支持多消费者~~ → 已忽略

> **业务设计确认:** 当前只需要一个消费者（RecognizeThread），单帧缓冲是刻意设计。如果将来需要多消费者再扩展。不计入待修复项。

---

## Section 2: 代码质量审查

### ~~发现 #6 [P2] 零测试覆盖~~ → 已忽略

> **后续计划:** QtTest 将由开发者手动引入，当前阶段不纳入修复项。不计入待修复项。

---

### 发现 #7 [P2] UI 类混合展示逻辑与数据访问

**置信度:** 8/10

多个 UI 类在展示代码中直接构建 SQL 条件或调用 DB 方法：

- [VehicleInformation](UI/VehicleInformation/vehicleinformation.cpp) — `searchCars()`, `deleteCarRecords()`, 分页计算
- [MainWindow](UI/MainWindow/mainwindow.cpp):332-560 — 入库/出库流程（验证→DB 写入→统计更新）全部揉在 MainWindow 中
- [UserManagement](UI/UserManager/usermanagement.cpp) — `searchUsers()`, `deleteUser()`, 分页

单个 MainWindow::onUpdateParkingCount 方法混合了：数据库查询、UI 更新、错误处理。

**建议:** 引入简单的 Service 层（`ParkingService`, `UserService`），UI 只负责展示和用户交互。

---

### 发现 #8 [P3] 不一致的空指针防护

**置信度:** 6/10

部分方法检查 `m_db`：
```cpp
// mainwindow.cpp:332 — 有检查
if(!m_db) { qDebug() << "..."; return; }
```

部分方法直接使用：
```cpp
// 大部分 DatabaseManager 内部方法不检查 connected
```

没有统一的错误处理策略——有的返回默认值，有的静默失败，有的 qDebug。

**建议:** 在 DatabaseManager 层面统一错误处理：所有方法检查 `connected`，失败时 emit `messageBox(MessageType::Error, ...)` 并返回错误状态。

---

### 发现 #9 [P3] 死代码

**置信度:** 9/10

`DatabaseManager::executeQuery()` ([databasemanager.h](src/database/databasemanager.h):36) 是 public 方法但无任何调用者。如果被误用，可能引入 SQL 注入。

**建议:** 删除或改为 private。

---

## Section 3: 测试审查

| 指标 | 当前值 | 目标 |
|------|--------|------|
| 单元测试 | 0 | ≥80% 核心逻辑覆盖 |
| 集成测试 | 0 | 数据库 CRUD 全覆盖 |
| E2E 测试 | 0 | 至少入库→出库→计费完整流程 |
| CI/CD | 无 | GitHub Actions 或本地脚本 |

### 关键测试路径（按优先级）

```ascii
优先级 P0 — 计费正确性:
  checkIn(plate) → [等N分钟] → checkOut(plate)
  └─ 免费时段(0-15min) → fee=0
  └─ 12分钟 → fee=0 (免费时段内)
  └─ 45分钟 → fee = hourlyRate * 0.5 (扣免费15min后30min=1半小时)
  └─ 70分钟 → fee = hourlyRate * 1.0 (扣免费15min后55min=2半小时)

优先级 P1 — 线程安全:
  FrameQueue: push(线程A) / take(线程B, timeout) / clear(主线程)
  DatabaseManager: 两个线程同时调用 checkIn()

优先级 P2 — UI 交互:
  登录失败 → 错误提示 → 重试
  入库重复车牌 → 拒绝提示
  翻页 → 数据加载 → 控件状态更新
```

---

## Section 4: 性能审查

### 发现 #10 [P3] 定时轮询而非事件驱动

**置信度:** 7/10

停车位统计使用 QTimer 每 5 秒轮询数据库 ([mainwindow.cpp] 相关逻辑）。对于停车场管理系统，车位变化由入库/出库事件确定——应使用 `parkingDataChanged()` 信号驱动更新，而非固定间隔轮询。

当数据库在远程服务器时（非本地 MySQL），每次轮询都是网络往返。

---

### 发现 #11 [P3] 视频帧经过双重格式转换

**置信度:** 6/10

CameraThread 捕获 BGR → QImage → cv::Mat → QImage，每帧经历 2-3 次格式转换和内存拷贝。多路摄像头（最多 4 路）时 CPU 开销显著。

**建议:** 统一使用 cv::Mat，QImage 转换仅在实际显示时进行。

---

## 实施状态(2026-07-21 更新)

经架构改进实现计划 `docs/superpowers/plans/2026-07-21-architecture-improvement.md` 落地:

| # | 等级 | 问题 | 状态 |
|---|------|------|------|
| #1 | P1 | God Object | ✅ 已修复(Service 层提取,DatabasManager 瘦身) |
| #2 | P1 | Singleton 生命周期 | ✅ 已修复(Meyer's + shutdown 序列) |
| #3 | P1 | 线程安全 | ✅ 已修复(per-thread DbConnectionPool) |
| #4 | P2 | 裸指针传播 | ✅ 已修复(UI 持 Service 引用) |
| #7 | P2 | UI 耦合 | ✅ 已修复(业务逻辑下沉 Service) |
| #5 | P2 | FrameQueue 单帧 | ⏭️ 已忽略(业务设计) |
| #6 | P2 | 零测试覆盖 | ⏭️ 已忽略(QtTest 后续手动) |

---

## 审查总结

### 严重度统计

| 等级 | 数量 | 说明 |
|------|------|------|
| P0 (Critical) | 0 | 无即时崩溃/安全漏洞 |
| P1 (High) | 3 | 上帝对象、单例生命周期、线程安全 |
| P2 (Medium) | 2 | UI 耦合、裸指针 |
| P3 (Low) | 4 | 空指针不一致、死代码、轮询、格式转换 |

### 建议修复顺序

```ascii
Phase A (短期, 1-2天)
  ├── A1: 删除 executeQuery 死代码
  ├── A2: 统一 ThreadPoolManager 为 Meyer's Singleton
  ├── A3: 添加 DatabaseManager 线程安全保护（per-thread 连接或 mutex）
  └── A4: 补齐关键路径的空指针检查

Phase B (中期, 3-5天)
  ├── B1: 集成 QtTest，编写计费逻辑 + FrameQueue 单元测试
  ├── B2: 提取 ParkingService（入库/出库/计费逻辑从 MainWindow 分离）
  └── B3: FrameQueue 从单帧改为环形缓冲区

Phase C (长期, 1-2周)
  ├── C1: 拆解 DatabaseManager 为 UserRepository / VehicleRepository / ParkingRepository
  ├── C2: 引入 Service 层（UserService, VehicleService, ParkingService）
  └── C3: 建立 CI/CD 流水线（GitHub Actions / 本地脚本）
```

---

## NOT in scope

以下问题在审查中被识别但视为超出当前范围：

- EasyPR → HyperLPR 迁移：库替换已完成，代码中仍保留 EasyPR 相关注释/路径引用
- 收费规则的"每日封顶"逻辑：设计文档提到但未实现，属于 Phase 6
- QSS 样式文件管理：已从 main.cpp 分离到 styles/，当前结构合理
- OpenCV 版本迁移（3.4.8 → 4.11）：已完成

---

## What already exists（设计良好、无需改动）

- **Signal/Slot 架构** — 使用 Qt 信号槽解耦组件（如 `parkingDataChanged()` 通知 UI 刷新）
- **FrameQueue 生产者-消费者模式** — 结构清晰，QWaitCondition + QMutex 使用正确
- **DatabaseManager 参数化查询** — 大部分方法使用 `prepare()` + `bindValue()`，防 SQL 注入
- **CameraManager 单例** — Meyer's Singleton 实现正确，`scanCameras` 可重入（`if(!m_threads.isEmpty()) return`）
- **文档覆盖** — 存在完整设计文档和多个版本的 spec

---

## 未解决问题

| # | 问题 | 待决策 |
|---|------|--------|
| 1 | DatabaseManager 拆分为 Repository 的时机 | Phase C 执行 |
| 2 | 测试框架选型（QtTest vs Catch2 vs gtest） | 需调研 OpenCV/HyperLPR 兼容性 |
| 3 | Service 层引入是否过度工程化 | 当前项目规模（实习项目）可能不需要 |

---

## GSTACK REVIEW REPORT

| Runs | Status | Findings |
|------|--------|----------|
| plan-eng-review (architecture) | ✅ | 4 findings (P1×3, P2×1) |
| plan-eng-review (code quality) | ✅ | 3 findings (P2×2, P3×1) |
| plan-eng-review (tests) | ⏭️ 跳过 | QtTest 后续手动引入 |
| plan-eng-review (performance) | ✅ | 2 findings (P3×2) |

**VERDICT:** 架构基本合格，适合实习项目规模。3 个 P1 问题需在后续迭代中解决（上帝对象、单例生命周期、线程安全）。代码注释质量高、文档覆盖完整，加分项。

**FINAL:** 建议先修复 Phase A（1-2 天安全加固），然后进行 Phase B（中期改善），Phase C 可择机进行。
