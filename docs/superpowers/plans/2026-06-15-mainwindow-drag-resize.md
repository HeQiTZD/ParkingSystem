# MainWindow 窗口拖动和拉伸功能 实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 MainWindow 添加窗口拖动（标题栏）和四边四角拉伸功能

**Architecture:** 通过重写 `nativeEvent` 拦截 Windows `WM_NCHITTEST` 消息，让操作系统处理拖动光标、拉伸光标和拉伸行为。只修改 `mainwindow.h` 和 `mainwindow.cpp` 两个文件。

**Tech Stack:** Qt 6 + Win32 API (`windows.h`, `windowsx.h`)

---

## 涉及文件

| 文件 | 操作 | 职责 |
|------|------|------|
| `UI/MainWindow/mainwindow.h` | 修改 | 添加 `nativeEvent` 声明、`m_borderWidth` 成员 |
| `UI/MainWindow/mainwindow.cpp` | 修改 | 添加 Windows 头文件、实现 `getHitArea` 辅助函数和 `nativeEvent` |

---

### Task 1: 修改 mainwindow.h — 添加声明和成员

**Files:**
- Modify: `UI/MainWindow/mainwindow.h`

- [ ] **Step 1: 添加 nativeEvent 声明**

在 `mainwindow.h` 的 `private slots:` 之前，添加 `protected` 区域：

```cpp
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
```

- [ ] **Step 2: 添加 m_borderWidth 成员**

在 `private` 区域的 `m_parkingChart` 之后添加：

```cpp
    int m_borderWidth = 8;  // 窗口边缘拉伸感应区域宽度
```

- [ ] **Step 3: 验证文件语法正确**

确认 `mainwindow.h` 的类声明结构完整，无语法错误。

- [ ] **Step 4: Commit**

```bash
git add UI/MainWindow/mainwindow.h
git commit -m "feat: 添加 nativeEvent 声明和 m_borderWidth 成员"
```

---

### Task 2: 修改 mainwindow.cpp — 实现 nativeEvent

**Files:**
- Modify: `UI/MainWindow/mainwindow.cpp`

- [ ] **Step 1: 添加 Windows 头文件**

在 `mainwindow.cpp` 顶部的 `#include` 区域末尾添加：

```cpp
#include <windows.h>
#include <windowsx.h>
```

- [ ] **Step 2: 实现 getHitArea 辅助函数**

在 `mainwindow.cpp` 的 `#include` 区域之后、构造函数之前，添加静态辅助函数：

```cpp
/**
 * @brief 根据鼠标在窗口中的位置，返回 Windows 命中区域值
 * @param pos   鼠标在窗口本地坐标系中的位置
 * @param rect  窗口矩形（0,0 为原点）
 * @param borderWidth  边缘拉伸感应宽度（像素）
 * @param titleBarHeight  标题栏高度（像素）
 * @return Windows HT* 命中区域值
 *
 * 优先级：角 > 边 > 标题栏 > 客户区
 */
static int getHitArea(const QPoint &pos, const QRect &rect,
                      int borderWidth, int titleBarHeight)
{
    int x = pos.x();
    int y = pos.y();
    int w = rect.width();
    int h = rect.height();

    bool onLeft   = (x >= 0 && x < borderWidth);
    bool onRight  = (x >= w - borderWidth && x < w);
    bool onTop    = (y >= 0 && y < borderWidth);
    bool onBottom = (y >= h - borderWidth && y < h);

    // 角部优先于边
    if (onTop && onLeft)        return HTTOPLEFT;
    if (onTop && onRight)       return HTTOPRIGHT;
    if (onBottom && onLeft)     return HTBOTTOMLEFT;
    if (onBottom && onRight)    return HTBOTTOMRIGHT;

    // 边
    if (onLeft)                 return HTLEFT;
    if (onRight)                return HTRIGHT;
    if (onTop)                  return HTTOP;
    if (onBottom)               return HTBOTTOM;

    // 标题栏区域（可拖动）
    if (y >= 0 && y < titleBarHeight) return HTCAPTION;

    // 其他区域
    return HTCLIENT;
}
```

- [ ] **Step 3: 实现 nativeEvent 方法**

在 `mainwindow.cpp` 的 `updateTime()` 方法之后，添加：

```cpp
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    // 只处理 Windows 原生消息
    if (eventType != "windows_generic_MSG")
        return QMainWindow::nativeEvent(eventType, message, result);

    MSG *msg = static_cast<MSG*>(message);
    if (msg->message != WM_NCHITTEST)
        return QMainWindow::nativeEvent(eventType, message, result);

    // 获取鼠标全局坐标并转换为窗口本地坐标
    QPoint globalPos(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam));
    QPoint localPos = mapFromGlobal(globalPos);

    // 窗口矩形（本地坐标系，原点为 0,0）
    QRect rect(0, 0, width(), height());

    // 标题栏高度：topBarWidget 的高度（31px，与 UI 文件一致）
    int titleBarHeight = 31;

    // 获取命中区域
    int hitArea = getHitArea(localPos, rect, m_borderWidth, titleBarHeight);

    // 如果命中标题栏，需要排除按钮区域
    if (hitArea == HTCAPTION) {
        // 检查鼠标是否在窗口控制按钮上
        QWidget *buttons[] = {
            ui->setButton,
            ui->minimizeButton,
            ui->maximizeButton,
            ui->closeButton
        };
        for (QWidget *btn : buttons) {
            QRect btnRect = btn->geometry();
            // 将按钮坐标转换为相对于 centralwidget 的坐标
            QPoint btnGlobal = btn->mapTo(this, QPoint(0, 0));
            QRect btnRectInWindow(btnGlobal, btn->size());
            if (btnRectInWindow.contains(localPos)) {
                hitArea = HTCLIENT;
                break;
            }
        }
    }

    // 最大化时禁用拉伸，只保留拖动（HTCAPTION）
    if (isMaximized() && hitArea != HTCAPTION && hitArea != HTCLIENT) {
        hitArea = HTCLIENT;
    }

    // 如果是客户区，交给 Qt 默认处理
    if (hitArea == HTCLIENT)
        return QMainWindow::nativeEvent(eventType, message, result);

    *result = hitArea;
    return true;
}
```

- [ ] **Step 4: Commit**

```bash
git add UI/MainWindow/mainwindow.cpp
git commit -m "feat: 实现 nativeEvent 支持窗口拖动和拉伸"
```

---

### Task 3: 构建和手动测试

**Files:**
- 无文件修改

- [ ] **Step 1: 构建项目**

在 Qt Creator 中构建项目，确认无编译错误。

- [ ] **Step 2: 测试窗口拖动**

1. 启动应用程序
2. 鼠标放在顶部标题栏（时间显示区域）按住拖动，确认窗口跟随移动
3. 鼠标放在设置/最小化/最大化/关闭按钮上，确认按钮可正常点击（不会触发拖动）

- [ ] **Step 3: 测试四边拉伸**

1. 鼠标移到窗口左边缘，确认显示 `↔` 光标，按住拖动可拉伸
2. 对右边缘、上边缘、下边缘重复测试
3. 确认拉伸时光标方向正确（水平/垂直）

- [ ] **Step 4: 测试四角拉伸**

1. 鼠标移到窗口左上角，确认显示 `⤡` 光标，按住拖动可拉伸
2. 对右上角、左下角、右下角重复测试
3. 确认拉伸时光标方向正确（对角线）

- [ ] **Step 5: 测试最大化状态**

1. 双击标题栏或点击最大化按钮使窗口最大化
2. 确认边缘和角部不再触发拉伸
3. 确认标题栏仍可拖动（拖动时自动还原窗口大小）

- [ ] **Step 6: 测试最小尺寸**

1. 尝试将窗口拉伸到很小
2. 确认窗口不会小于 1000x700

- [ ] **Step 7: 测试高 DPI**

1. 如果有高 DPI 显示器，确认拉伸感应区域和光标在不同缩放下表现正常
