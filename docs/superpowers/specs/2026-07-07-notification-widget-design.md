# 自定义统一提示框控件 — 设计规范

> **状态**: 设计定稿 · 待实现
> **作者**: Claude · 王工
> **日期**: 2026-07-07
> **范围**: 自定义控件的实现；不含 DatabaseManager 信号改造（后续迭代）

---

## 1. 背景与动机

### 1.1 当前问题

ParkingSystem 现有三套互不兼容的"错误报告方式"，造成**严重信息黑洞**：

| 方式 | 调用位置 | 可见性 |
|---|---|---|
| `qDebug()` | 全项目 107 处，其中 ~40 条是真正的错误 | **仅控制台可见**，普通用户不可见 |
| `QMessageBox::warning/critical/information` | Login / Register / ConfigInit / main.cpp 共 30 处 | 需用户点确认才能关闭，无统一风格 |
| 完全无提示 | MainWindow 中所有运行时操作 | 用户一无所知 |

### 1.2 核心矛盾

- DatabaseManager 有 46 处 `qDebug()` 报告 SQL 错误，但调用方拿到的只是一个 `bool false`
- 登录后进入 MainWindow 后，若 DB 操作失败，**用户完全无法感知**
- 现有 30 处 QMessageBox 散落在各 UI 类中，文案、按钮、风格都不统一

### 1.3 本次设计范围

**仅实现自定义控件本身**，提供对标 `QMessageBox` 的静态调用接口。后续迭代再：
- 给 DatabaseManager 加 `errorOccurred()` 信号
- 逐处迁移现有 `qDebug()` 到新的错误提示

### 1.4 不在本次范围内

- ~~数据库信号改造~~（后续迭代）
- ~~统一日志框架~~（后续迭代）
- ~~自动替换现有的 30 处 QMessageBox~~（后续逐处迁移）

---

## 2. 设计目标与原则

### 2.1 目标

1. **零学习成本** — 调用接口和 `QMessageBox` 静态方法一一对应，对照替换即可
2. **视觉统一** — 所有提示框遵循同一套 QSS 主题，与现有登录页/主窗口风格一致
3. **语义化调用** — 调用方不需要关心"用 Toast 还是弹窗"，只表达"这是成功警告还是错误"
4. **可扩展** — 未来要新增通知形态（如常驻横条）或新增信号源时，接口不变

### 2.2 设计原则

| 原则 | 说明 |
|---|---|
| **单一职责** | `ToastWidget` 只管消失式通知，`NotificationDialog` 只管模态弹窗 |
| **静态接口** | 所有公共调用都通过 `inline` 函数暴露，不需要手动 `new` 或管理内存 |
| **parent 感知** | 第一个参数 `QWidget *parent`，自动给父窗口盖遮罩、锚定 Toast 位置 |
| **QSS 主题化** | 控件外观完全由 QSS 控制，换主题不改 C++ |

---

## 3. 四种提示类型定义

### 3.1 信息提示（Toast 自动消失）

```
┌──────────────────────────────────────────────────────┐
│  MainWindow                                          │
│                                     ┌─────────────┐  │
│                                     │ ✓ 操作成功   │  ← 蓝色竖条 + 圆形图标
│                                     │  配置已保存  │  │
│                                     └─────────────┘  │
│                                     ┌─────────────┐  │
│                                     │ ✕ 操作失败   │  ← 红色竖条 + 圆形图标
│                                     │  用户名已存在 │  │
│                                     └─────────────┘  │
└──────────────────────────────────────────────────────┘
```

- **成功 Toast**: 蓝色 `#3498db` + ✓ 图标 + 白色卡片，3 秒后自动淡出
- **失败 Toast**: 红色 `#e74c3c` + ✕ 图标 + 白色卡片，3.5 秒后自动淡出
- **定位**: 父窗口右上角内侧（偏移量：右 20px，上 60px 避开标题栏）
- **堆叠**: 多条时向下排列（间距 10px），最多同时显示 5 条，超出丢弃最旧的
- **动画**: 从右侧滑入（200ms）→ 停留 → 向右滑出 + 淡出（250ms）
- **手动关闭**: 点击右侧 ✕ 立即关闭

### 3.2 警告提示（模态确认弹窗）

```
┌──────────────────────────────────────────────────────┐
│  ░░░░░░░░░░░ 半透明黑色遮罩 ░░░░░░░░░░░░░░░░░░░░░░  │
│  ░░  ┌────────────────────────────────────┐  ░░░░░  │
│  ░░  │            ⚠                       │  ░░░░░  │
│  ░░  │                                    │  ░░░░░  │
│  ░░  │        确认删除该用户？              │  ░░░░░  │
│  ░░  │     删除后无法撤销，请谨慎操作        │  ░░░░░  │
│  ░░  │                                    │  ░░░░░  │
│  ░░  │    [取消]            [确定]        │  ░░░░░  │
│  ░░  └────────────────────────────────────┘  ░░░░░  │
│  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  │
└──────────────────────────────────────────────────────┘
```

- **图标**: 黄色 `#f39c12` 圆形底 + ⚠ 字符
- **按钮**: "取消"（灰色边框）+ "确定"（主色填充，默认焦点）
- **返回值**: `bool`，确定 = true，取消 = false
- **Esc 键**: 等价于点"取消"，与 QMessageBox 一致
- **动画**: 缩放 0.9 → 1.0 + 淡入（150ms），关闭时反向

### 3.3 错误提示（模态错误弹窗）

```
┌──────────────────────────────────────────────────────┐
│  ░░░░░░░░░░░ 半透明黑色遮罩 ░░░░░░░░░░░░░░░░░░░░░░  │
│  ░░  ┌────────────────────────────────────┐  ░░░░░  │
│  ░░  │            ✕                       │  ░░░░░  │
│  ░░  │                                    │  ░░░░░  │
│  ░░  │        数据库连接失败                │  ░░░░░  │
│  ░░  │    127.0.0.1:3306 拒绝连接          │  ░░░░░  │
│  ░░  │    请检查数据库服务是否启动           │  ░░░░░  │
│  ░░  │                                    │  ░░░░░  │
│  ░░  │  [复制详情]    [我知道了]           │  ░░░░░  │
│  ░░  └────────────────────────────────────┘  ░░░░░  │
│  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  │
└──────────────────────────────────────────────────────┘
```

- **图标**: 红色 `#e74c3c` 圆形底 + ✕ 字符
- **按钮**: "复制详情"（可选，灰色）+ "我知道了"（主色填充）
- **作用**: 显示具体错误细节——如 SQL 错误文本、配置解析错误等
- **复制详情**: 把标题 + 正文复制到系统剪贴板，方便用户反馈给开发者
- **返回值**: `void`（单向告知，不需要用户决策）

### 3.4 询问提示（模态决策弹窗）

- **图标**: 蓝色 `#3498db` 圆形底 + ? 字符
- **按钮**: "是(Y)" + "否(N)"
- **返回值**: `bool`，是 = true，否 = false
- **Esc 键**: 等价于"否"

> 询问提示与警告提示结构相同，仅图标和按钮文案不同。实现上共用同一个 `NotificationDialog` 类，通过 `DialogType` 枚举区分。

---

## 4. 文件结构与模块划分

### 4.1 新增文件清单

```
src/utils/
├── notificationdialog.h          ← 对话框类声明
├── notificationdialog.cpp        ← 对话框类实现
├── toastwidget.h                ← Toast 控件声明
├── toastwidget.cpp              ← Toast 控件实现
└── notification_global.h        ← 统一便捷调用接口（inline 函数）
```

### 4.2 各文件职责

#### `notificationdialog.h` / `notificationdialog.cpp`

```cpp
class NotificationDialog : public QDialog
{
    Q_OBJECT
public:
    enum DialogType { Info, Warning, Error, Question };

    // 静态调用接口（对标 QMessageBox）
    static void info(QWidget *parent, const QString &title, const QString &msg);
    static bool warning(QWidget *parent, const QString &title, const QString &msg);
    static void error(QWidget *parent, const QString &title, const QString &msg);
    static bool question(QWidget *parent, const QString &title, const QString &msg);

    // 高级：错误弹窗附带"复制详情"
    static void errorWithDetail(QWidget *parent, const QString &title,
                                const QString &brief, const QString &detail);

private:
    explicit NotificationDialog(QWidget *parent, DialogType type,
                                const QString &title, const QString &msg);
    void setupUi();
    void playShowAnimation();
    void playCloseAnimation();

    // 内部成员
    DialogType m_type;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_msgLabel;
    QPushButton *m_primaryButton;    // 主按钮（确定/是/我知道了）
    QPushButton *m_secondaryButton;  // 次按钮（取消/否/复制详情）
};
```

#### `toastwidget.h` / `toastwidget.cpp`

```cpp
class ToastWidget : public QWidget
{
    Q_OBJECT
public:
    enum ToastType { Success, Failure, Info, Warning };

    explicit ToastWidget(QWidget *parent = nullptr);

    // 静态便捷调用
    static void show(QWidget *parent, const QString &msg,
                     ToastType type = Info, int durationMs = 3000);

private:
    void showToast(const QString &msg, ToastType type, int durationMs);
    void slideIn();
    void slideOut(std::function<void()> onFinished);

    // 内部：管理多条 Toast 的队列
    static QQueue<ToastWidget*> s_activeToasts;  // 全局活跃 Toast 列表
    static constexpr int MAX_ACTIVE = 5;
    static constexpr int SPACING = 10;
    static constexpr int MARGIN_RIGHT = 20;
    static constexpr int MARGIN_TOP = 60;  // 避开标题栏

    QLabel *m_iconLabel;
    QLabel *m_msgLabel;
    QPushButton *m_closeButton;
    QPropertyAnimation *m_animation;
};
```

#### `notification_global.h`

```cpp
#ifndef NOTIFICATION_GLOBAL_H
#define NOTIFICATION_GLOBAL_H

#include <QWidget>
#include "notificationdialog.h"
#include "toastwidget.h"

// ─── Toast（自动消失） ───────────────────────────────
inline void notifySuccess(QWidget *parent, const QString &msg, int ms = 3000) {
    ToastWidget::show(parent, msg, ToastWidget::Success, ms);
}
inline void notifyFailure(QWidget *parent, const QString &msg, int ms = 3500) {
    ToastWidget::show(parent, msg, ToastWidget::Failure, ms);
}
inline void notifyInfo(QWidget *parent, const QString &msg, int ms = 3000) {
    ToastWidget::show(parent, msg, ToastWidget::Info, ms);
}

// ─── 模态弹窗 ───────────────────────────────────────
inline void notifyWarning(QWidget *parent, const QString &title, const QString &msg) {
    NotificationDialog::warning(parent, title, msg);
}
inline void notifyError(QWidget *parent, const QString &title, const QString &msg) {
    NotificationDialog::error(parent, title, msg);
}
inline bool notifyConfirm(QWidget *parent, const QString &title, const QString &msg) {
    return NotificationDialog::question(parent, title, msg);
}

#endif // NOTIFICATION_GLOBAL_H
```

---

## 5. 视觉规范

### 5.1 统一风格参数

| 参数 | Toast | Modal |
|---|---|---|
| 圆角 | 10px | 14px |
| 阴影 | `0 4px 20px rgba(0,0,0,0.25)` | `0 8px 40px rgba(0,0,0,0.35)` |
| 最大宽度 | 320px | 400px |
| 内边距 | 12px 16px | 24px 28px |
| 图标尺寸 | 32×32px 圆形 | 48×48px 圆形 |
| 左侧色条 | 4px 宽，贯穿高度 | 无（用顶部图标区） |
| 字体（标题） | 14px semibold | 17px bold |
| 字体（正文） | 12px regular | 13px regular |
| 遮罩 | 无 | `rgba(0,0,0,0.45)` |

### 5.2 色彩体系

| 语义 | 图标底色 | 图标字符色 | 左侧竖条 / 按钮色 |
|---|---|---|---|
| 成功 Success | `#ebf3fd` | `#3498db` | `#3498db` |
| 信息 Info | `#ebf3fd` | `#3498db` | `#3498db` |
| 警告 Warning | `#fef5e7` | `#f39c12` | `#f39c12` |
| 失败 Failure | `#fdecea` | `#e74c3c` | `#e74c3c` |
| 错误 Error | `#fdecea` | `#e74c3c` | `#e74c3c` |

### 5.3 QSS 主题化方案

控件通过 `Q_PROPERTY` + 属性选择器实现主题化：

```cpp
// C++ 中设置属性
iconLabel->setProperty("severity", "error");   // "success" | "warning" | "error" | "info"
```

```css
/* 写入 main.qss 或由控件动态加载 */
QLabel[severity="success"] { color: #3498db; background: #ebf3fd; border-radius:50%; }
QLabel[severity="warning"] { color: #f39c12; background: #fef5e7; border-radius:50%; }
QLabel[severity="error"]   { color: #e74c3c; background: #fdecea; border-radius:50%; }
QLabel[severity="info"]    { color: #3498db; background: #ebf3fd; border-radius:50%; }
```

---

## 6. 调用接口对照表

### 6.1 与 QMessageBox 的迁移对照

| 原 QMessageBox 调用 | 新调用 | 说明 |
|---|---|---|
| `QMessageBox::information(this, t, m)` | `notifyInfo(this, m)` | Toast 自动消失 |
| `QMessageBox::warning(this, t, m)` | `notifyWarning(this, t, m)` | 模态确认，返回 bool |
| `QMessageBox::critical(this, t, m)` | `notifyError(this, t, m)` | 模态错误，可复制详情 |
| `QMessageBox::question(this, t, m)` | `notifyConfirm(this, t, m)` | 模态询问，返回 bool |
| `qDebug() << "操作成功"` | `notifySuccess(this, "操作成功")` | 替代成功日志 |
| `qDebug() << "操作失败" << err` | `notifyFailure(this, "操作失败")` | 替代失败日志 |

### 6.2 典型调用示例

```cpp
// ─── 注册成功 ───
notifySuccess(this, "注册成功，欢迎使用！");

// ─── 注册失败（用户名重复） ───
notifyFailure(this, "用户名已存在，请更换");

// ─── 删除确认 ───
if (notifyWarning(this, "确认删除", "删除用户 admin 后无法撤销，确定继续？")) {
    m_db->deleteUser("admin");
    notifySuccess(this, "用户已删除");
}

// ─── 数据库错误 ───
if (!m_db->checkIn(plate)) {
    notifyError(this, "入库失败", "数据库写入失败，请稍后重试或联系管理员");
}

// ─── 带详情的错误 ───
NotificationDialog::errorWithDetail(this, "数据库错误",
    "无法连接到数据库",          // 简短描述（用户可见）
    lastErrorText);              // 技术详情（复制到剪贴板）
```

---

## 7. 技术实现要点

### 7.1 NotificationDialog 实现要点

1. **构造私有** — 只能通过静态方法创建，防止手动 `new` 导致内存泄漏
2. **自动遮罩** — 构造时获取 `parent->window()`，在其上盖一层 `QWidget` 遮罩（`Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint`）
3. **动画** — `QPropertyAnimation(this, "geometry")` 或 `QPropertyAnimation(this, "windowOpacity")`，150ms，`QEasingCurve::OutCubic`
4. **Esc 键** — 重写 `keyPressEvent`，`Qt::Key_Escape` → `reject()`
5. **复制详情** — `QApplication::clipboard()->setText(detail)` + 短暂 Toast 提示"已复制"

### 7.2 ToastWidget 实现要点

1. **窗口标志** — `Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WA_TranslucentBackground`
2. **位置计算** — `parent->rect().topRight() + QPoint(-width - MARGIN_RIGHT, MARGIN_TOP)`
3. **堆叠管理** — 静态 `QQueue<ToastWidget*>` 记录当前活跃的 Toast；新 Toast 弹出时，把已有的向下平移 `height + SPACING`
4. **动画** — `QPropertyAnimation(this, "pos")` 控制滑入滑出；`QPropertyAnimation(this, "windowOpacity")` 控制淡入淡出
5. **自动删除** — `deleteLater()` 在动画结束后调用，不泄漏
6. **parent 销毁时** — 重写 `eventFilter` 监听 parent 的 `QEvent::Destroy`，自动清理自己

### 7.3 线程安全

- 所有调用必须在 **UI 线程** 中（与 QMessageBox 一致）
- 如果未来需要从非 UI 线程调用，通过 `QMetaObject::invokeMethod(Qt::QueuedConnection)` 投递到 UI 线程

---

## 8. 与现有项目的集成点

### 8.1 需要修改的文件

| 文件 | 改动 |
|---|---|
| `ParkingSystem.pro` | 新增 `notificationdialog.cpp/h`、`toastwidget.cpp/h` 到 `SOURCES` / `HEADERS` |
| `main.cpp` | 可选：把 8 处 `QMessageBox::critical` 替换为 `notifyError`（非必须） |
| `UI/Login/logindialog.cpp` | 可选：4 处 QMessageBox 替换（非必须） |
| `UI/Register/registerdialog.cpp` | 可选：8 处 QMessageBox 替换（非必须） |
| `UI/ConfigInit/configinitdialog.cpp` | 可选：10 处 QMessageBox 替换（非必须） |

> 本次实现**不强制迁移**现有调用点，新控件和 QMessageBox 可以共存。后续逐处迁移。

### 8.2 需要新增的文件

| 文件 | 说明 |
|---|---|
| `src/utils/notificationdialog.h` | 对话框类声明 |
| `src/utils/notificationdialog.cpp` | 对话框类实现 |
| `src/utils/toastwidget.h` | Toast 控件声明 |
| `src/utils/toastwidget.cpp` | Toast 控件实现 |
| `src/utils/notification_global.h` | 统一便捷调用接口 |

---

## 9. 验收标准

### 9.1 功能验收

- [ ] `notifySuccess(parent, "msg")` 显示蓝色 Toast，3 秒后自动消失
- [ ] `notifyFailure(parent, "msg")` 显示红色 Toast，3.5 秒后自动消失
- [ ] `notifyWarning(parent, "title", "msg")` 显示黄色模态弹窗，返回 bool
- [ ] `notifyError(parent, "title", "msg")` 显示红色模态弹窗，可关闭
- [ ] `notifyConfirm(parent, "title", "msg")` 显示蓝色询问弹窗，返回 bool
- [ ] 多条 Toast 正确堆叠，不超过 5 条
- [ ] Esc 键可关闭 Modal
- [ ] 复制详情按钮正常工作
- [ ] parent 销毁时 Toast 不崩溃

### 9.2 视觉验收

- [ ] 圆角、阴影、色彩符合 §5 规范
- [ ] 动画流畅（滑入滑出、淡入淡出、缩放）
- [ ] 与现有登录页 QSS 风格协调
- [ ] 高 DPI 屏幕下不模糊

### 9.3 代码质量验收

- [ ] 无内存泄漏（`deleteLater()` 正确）
- [ ] 无 `qDebug()` 残留（新控件内部也不该用 `qDebug`）
- [ ] 头文件自包含（include 即可用）
- [ ] 接口注释完整

---

## 10. 后续迭代（不在本次范围）

1. **DatabaseManager 信号改造** — 加 `errorOccurred(QString msg, int level)` 信号，46 处 `qDebug()` 改为 `emit errorOccurred()`
2. **统一日志框架** — 所有 `qDebug()` 重定向到文件 + UI 双通道
3. **逐处迁移 QMessageBox** — 30 处现有调用替换为新控件
4. **错误码体系** — 给每种错误分配唯一 ID，方便搜索和国际化
5. **国际化** — 文案提取到 `tr()` 中

---

## 附录 A：决策记录

| 决策 | 选项 | 结论 | 理由 |
|---|---|---|---|
| 信息提示形态 | Toast / 常驻条 / 模态 | **Toast** | 不打断用户，操作成功/失败不需要确认 |
| 警告提示形态 | Toast / 常驻条 / 模态 | **模态** | 需要用户明确确认，防止误操作 |
| 错误提示形态 | Toast / 常驻条 / 模态 | **模态** | 必须让用户看到具体错误信息 |
| 视觉风格 | 圆角卡片 / 直角 / 药丸 | **圆角卡片** | 与现有 QSS 风格统一 |
| 实现方案 | 统一管理器 / 独立控件 / 综合 | **独立控件** | 用户要求"仅实现控件"，调用方式对标 QMessageBox |
| 是否改造 DB 层 | 是 / 否 | **否** | 后续迭代 |
| 是否迁移现有 QMessageBox | 是 / 否 | **否** | 后续逐处迁移 |
