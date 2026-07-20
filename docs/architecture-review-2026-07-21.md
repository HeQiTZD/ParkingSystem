# ParkingSystem 架构审查报告

**审查日期:** 2026-07-21 &nbsp;|&nbsp; **最后更新:** 2026-07-21
**审查范围:** 当前分支 master diff（83 文件, 10,968+ / 725-）
**审查方式:** gstack /plan-eng-review 流程
**参考文档:** [设计文档](docs/LENOVO-master-design-20260623-214343.md)
**实施记录:** [实现计划](superpowers/plans/2026-07-21-architecture-improvement.md)

---

## 项目架构总览（改进后）

```
┌─────────────────────────────────────────────────────────────┐
│                          UI Layer                            │
│  LoginDialog  RegisterDialog  MainWindow                     │
│  VehicleInformation  UserManagement  CameraManagement        │
│       └─ 持 Service& 引用, 不再持 DatabaseManager* ─────────┘ │
├──────────────────────────▼───────────────────────────────────┤
│                    Service Layer                              │
│  ParkingService  UserService  VehicleService                 │
│   (业务流程编排, 发业务信号, 统一错误出口)                      │
├──────────────────────────▼───────────────────────────────────┤
│                  Data Access Layer                           │
│  DatabaseManager (纯 CRUD, 无业务流程)                       │
│   └─ DbConnectionPool: per-thread QSqlDatabase 管理          │
└─────────────────────────────────────────────────────────────┘

Singletons (统一 Meyer's, 显式 shutdown)
  InitFile  PlateRecognize  CameraManager  ThreadPoolManager
```

### 设计模式分布（改进后）

| 模式 | 使用位置 | 评估 |
|------|---------|------|
| Singleton | InitFile, PlateRecognize, CameraManager, ThreadPoolManager | ✅ Meyer's + shutdown 逆序序列 |
| Service Layer | ParkingService, UserService, VehicleService | ✅ 业务流程与数据访问分离 |
| Per-thread Connection | DbConnectionPool | ✅ Qt 官方推荐方式 |
| Producer-Consumer | CameraThread → FrameQueue → RecognizeThread | ✅ 模式正确 |
| Observer (Signal/Slot) | 全局 Qt 信号槽 | ✅ Qt 标准模式 |

### 已修复问题（2026-07-21）

| # | 等级 | 问题 | 修复方式 |
|---|------|------|---------|
| #1 | P1 | DatabaseManager 上帝对象 | Service 层提取, DB 瘦身为纯 CRUD |
| #2 | P1 | Singleton 泛滥无生命周期 | 4 单例统一 Meyer's + ApplicationManager 逆序 shutdown |
| #3 | P1 | DatabaseManager 无线程安全 | DbConnectionPool per-thread 连接 + 线程退出 close |
| #4 | P2 | 裸指针传播无所有权 | UI 构造函数改为 Service& 引用 |
| #7 | P2 | UI 类混合展示与数据访问 | 业务流程下沉 ParkingService/UserService/VehicleService |
| #8 | P3 | 空指针检查不一致 | UI 引用天然非空 + Service 内部统一检查 |
| #9 | P3 | executeQuery 死代码 | 已删除 |

---

## 剩余未处理发现

### 发现 #10 [P3] 定时轮询而非事件驱动

**置信度:** 7/10

停车位统计使用 QTimer 每 5 秒轮询数据库。车位变化由入库/出库事件确定——已存在 `parkingDataChanged()` 信号可驱动更新。当数据库在远程服务器时，每次轮询都是网络往返。

---

### 发现 #11 [P3] 视频帧经过双重格式转换

**置信度:** 6/10

CameraThread 捕获 BGR → QImage → cv::Mat → QImage，每帧经历 2-3 次格式转换和内存拷贝。多路摄像头（最多 4 路）时 CPU 开销显著。

**建议:** 统一使用 cv::Mat，QImage 转换仅在实际显示时进行。

---

### 已忽略

| # | 等级 | 问题 | 原因 |
|---|------|------|------|
| #5 | P2 | FrameQueue 单帧缓冲 | 业务设计——当前只一个消费者 |
| #6 | P2 | 零测试覆盖 | QtTest 后续手动引入 |

---

## Section 3: 测试审查

| 指标 | 当前值 | 目标 |
|------|--------|------|
| 单元测试 | 0 | ≥80% 核心逻辑覆盖 |
| 集成测试 | 0 | 数据库 CRUD 全覆盖 |
| E2E 测试 | 0 | 至少入库→出库→计费完整流程 |
| CI/CD | 无 | GitHub Actions 或本地脚本 |

### 关键测试路径

```ascii
P0 — 计费正确性:
  checkIn(plate) → [等N分钟] → checkOut(plate)
  └─ 免费时段(0-15min) → fee=0
  └─ 45分钟 → fee = hourlyRate * 0.5

P1 — 线程安全:
  FrameQueue: push(线程A) / take(线程B, timeout) / clear(主线程)
  多线程同时调用 DatabaseManager::checkIn()

P2 — UI 交互:
  登录失败 → 错误提示 → 重试
  入库重复车牌 → 拒绝提示
  翻页 → 数据加载 → 控件状态更新
```

---

## NOT in scope

- EasyPR → HyperLPR 迁移：已完成
- 收费规则"每日封顶"：设计文档 Phase 6
- OpenCV 版本迁移（3.4.8 → 4.11）：已完成

---

## What already exists（设计良好、无需改动）

- **Signal/Slot 架构** — 使用 Qt 信号槽解耦组件
- **FrameQueue 生产者-消费者模式** — QWaitCondition + QMutex 使用正确
- **DatabaseManager 参数化查询** — `prepare()` + `bindValue()`，防 SQL 注入
- **文档覆盖** — 完整设计文档和多个版本 spec

---

## 后续建议

```ascii
Phase B (中期, 后续)
  ├── 集成 QtTest，编写计费逻辑 + FrameQueue 单元测试
  ├── 停车位统计改为事件驱动 (parkingDataChanged 替代 QTimer)
  └── 视频帧格式统一为 cv::Mat

Phase C (长期, 后续)
  ├── DatabaseManager 拆解为 Repository
  └── CI/CD 流水线
```

---

## 未解决问题

| # | 问题 | 决策时机 |
|---|------|---------|
| 1 | DatabaseManager 进一步拆解为 Repository | Phase C |
| 2 | 测试框架选型（QtTest vs Catch2 vs gtest） | Phase B |
| 3 | P3 轮询 + 格式转换的优先级 | Phase B |

---

## GSTACK REVIEW REPORT

| Runs | Status | Findings |
|------|--------|----------|
| plan-eng-review (architecture) | ✅ | P1×3 已修复 / P2×2 已修复 / P3×2 剩余 |
| plan-eng-review (code quality) | ✅ | 已修复 |
| plan-eng-review (tests) | ⏭️ | 后续手动引入 |
| plan-eng-review (performance) | ✅ | P3×2 剩余 |

**VERDICT (更新 2026-07-21):** 原始 11 项发现 — 5 项计划修复 + 2 项顺手修复 + 1 项实施补丁(MySQLInit) = 8 项已关闭。剩余 2 项 P3 低优先级 + 测试框架后续。**编译通过, 功能验证完成。**
