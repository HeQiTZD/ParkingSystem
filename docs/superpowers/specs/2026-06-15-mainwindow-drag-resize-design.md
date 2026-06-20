# MainWindow 窗口拖动和拉伸功能设计

## 概述

为 MainWindow（无边框窗口）添加窗口拖动和拉伸功能，使用 Windows 原生 `WM_NCHITTEST` 消息实现。

## 当前状态

- MainWindow 已设置 `Qt::FramelessWindowHint`，无系统标题栏
- 顶部有自定义 `topBarWidget`（31px 高），包含时间、设置、最小化、最大化、关闭按钮
- 最小化/最大化/关闭按钮已连接信号，功能正常
- **缺失：** 窗口拖动和拉伸功能

## 需求

- **拖动区域：** 顶部标题栏 `topBarWidget`
- **拉伸方式：** 四边 + 四角拉伸（8 个方向）
- **拉伸感应宽度：** 8px
- **最小窗口尺寸：** 1000x700（保持现有设定）
- **目标窗口：** 仅 MainWindow

## 方案选择

**选择：Windows 原生消息方案（WM_NCHITTEST）**

理由：
- Windows 原生处理拉伸光标和拉伸行为，体验最流畅
- 代码量最少（约 80 行）
- 拉伸动画由操作系统管理，不会卡顿
- 项目本身是 Windows 专用系统，无需跨平台兼容

## 技术设计

### 核心机制

在 MainWindow 中重写 `nativeEvent` 方法，拦截 Windows 的 `WM_NCHITTEST` 消息。

`WM_NCHITTEST` 是 Windows 在鼠标移动时发送给窗口的消息，询问"鼠标当前在窗口的什么位置"。返回不同的命中区域值后，Windows 会自动：
- 显示对应的拉伸光标（↔ ↕ ↗ ↘ 等）
- 处理拉伸行为
- 处理标题栏拖动

### 涉及修改的文件

1. `UI/MainWindow/mainwindow.h` — 添加 `nativeEvent` 声明和辅助成员
2. `UI/MainWindow/mainwindow.cpp` — 实现 `nativeEvent` 逻辑

不需要修改 UI 文件或其他窗口文件。

### 命中测试逻辑

| 鼠标位置 | 返回值 | 效果 |
|---------|--------|------|
| 顶部标题栏 `topBarWidget` 区域（排除按钮） | `HTCAPTION` | 可拖动窗口 |
| 左边缘 8px | `HTLEFT` | 左拉伸光标 |
| 右边缘 8px | `HTRIGHT` | 右拉伸光标 |
| 上边缘 8px | `HTTOP` | 上拉伸光标 |
| 下边缘 8px | `HTBOTTOM` | 下拉伸光标 |
| 左上角 8px | `HTTOPLEFT` | 左上拉伸光标 |
| 右上角 8px | `HTTOPRIGHT` | 右上拉伸光标 |
| 左下角 8px | `HTBOTTOMLEFT` | 左下拉伸光标 |
| 右下角 8px | `HTBOTTOMRIGHT` | 右下拉伸光标 |
| 窗口按钮区域 | `HTCLIENT` | 正常点击 |
| 其他区域 | `HTCLIENT` | 正常交互 |

### 代码结构

#### mainwindow.h

```cpp
// 新增 protected 方法声明
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

// 新增 private 成员
private:
    int m_borderWidth = 8;  // 拉伸感应区域宽度
```

#### mainwindow.cpp

```cpp
#include <windows.h>
#include <windowsx.h>

// 辅助函数：根据鼠标位置返回 Windows 命中区域
static int getHitArea(const QPoint &pos, const QRect &rect,
                      int borderWidth, int titleBarHeight)
{
    // 1. 判断是否在标题栏区域（y < titleBarHeight）
    // 2. 判断是否在 8 个边缘/角部区域
    // 3. 角部优先于边（左上角返回 HTTOPLEFT 而非 HTLEFT）
    // 4. 其他返回 HTCLIENT
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType != "windows_generic_MSG") return false;

    MSG *msg = static_cast<MSG*>(message);
    if (msg->message != WM_NCHITTEST) return false;

    // 获取鼠标全局坐标，转换为窗口本地坐标
    // 调用 getHitArea 判断区域
    // 如果在标题栏且不在按钮上 → *result = HTCAPTION; return true;
    // 如果在边缘/角部 → *result = 对应值; return true;
    // 其他 → return false;
}
```

### 边界情况

1. **按钮区域排除：** 将 setButton、minimizeButton、maximizeButton、closeButton 的全局坐标映射到窗口坐标，判断鼠标是否在按钮上。在按钮上时返回 `HTCLIENT`。

2. **最大化状态：** 窗口最大化时禁用拉伸功能（只保留拖动恢复）。Windows 对 `HTCAPTION` 区域双击自动切换最大化/还原。

3. **高 DPI 支持：** `borderWidth` 和 `titleBarHeight` 使用逻辑像素，Qt 的坐标系统已处理 DPI 缩放。

4. **最小尺寸保护：** UI 文件已设定 `minimumSize` 为 1000x700，Windows 拉伸自动遵守。

5. **与 LoginDialog 的关系：** LoginDialog 已有自己的拖动实现，不受本次改动影响。
