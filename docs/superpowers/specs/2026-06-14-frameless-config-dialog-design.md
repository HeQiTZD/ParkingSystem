# ConfigInitDialog 无边框窗口设计

## 概述

为 ConfigInitDialog 实现无边框窗口，包含自定义最小化和关闭按钮，支持顶部区域拖拽移动。

## 需求

- 移除系统标题栏（Qt::FramelessWindowHint）
- 禁止添加系统标题栏
- 右上角添加最小化按钮（悬浮时浅灰色）和关闭按钮（悬浮时红色）
- 窗口顶部区域支持拖拽移动
- 窗口带阴影效果

## 设计决策

### 方案选择

选择**纯 QSS + 事件过滤器**方案：
- 实现简单，代码量少
- 与现有 config.qss 风格统一
- 无需额外类或文件

### 标题栏设计

- **背景色**：白色 `#ffffff`，与主体一致
- **左侧**：无标题文字
- **右侧**：最小化按钮（`─`）和关闭按钮（`×`）
- **高度**：40px
- **底部**：1px 分割线 `#f1f5f9`

### 按钮样式

| 状态 | 最小化按钮 | 关闭按钮 |
|------|-----------|---------|
| 默认 | `#94a3b8` 灰色文字 | `#94a3b8` 灰色文字 |
| 悬浮 | `#f1f5f9` 浅灰背景 | `#ef4444` 红色背景 |
| 按下 | `#e2e8f0` 背景 | `#dc2626` 背景 |

### 窗口阴影

使用 QGraphicsDropShadowEffect：
- 模糊半径：20px
- 颜色：`rgba(0,0,0,0.3)`
- 偏移：0, 0

## 技术实现

### 文件变更

1. **configinitdialog.ui** — 添加标题栏 QWidget、最小化/关闭按钮
2. **configinitdialog.h** — 添加鼠标事件处理函数声明
3. **configinitdialog.cpp** — 实现窗口标志设置、阴影效果、拖拽逻辑、按钮功能
4. **config.qss** — 添加标题栏和按钮样式

### 关键实现

```cpp
// 构造函数中
setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
setAttribute(Qt::WA_TranslucentBackground);

// 阴影效果
QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
shadow->setBlurRadius(20);
shadow->setColor(QColor(0, 0, 0, 80));
shadow->setOffset(0, 0);
ui->mainContainer->setGraphicsEffect(shadow);

// 拖拽逻辑（事件过滤器）
bool ConfigInitDialog::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->titleBar) {
        // 处理鼠标按下、移动、释放
    }
    return QDialog::eventFilter(obj, event);
}
```

### QSS 样式

```css
/* 标题栏 */
#titleBar {
    background-color: #ffffff;
    border-bottom: 1px solid #f1f5f9;
}

/* 按钮基础样式 */
QPushButton#btnMinimize,
QPushButton#btnClose {
    background: transparent;
    border: none;
    border-radius: 4px;
    color: #94a3b8;
    font-size: 14px;
    min-width: 36px;
    max-width: 36px;
    min-height: 28px;
    max-height: 28px;
}

/* 最小化悬浮 */
QPushButton#btnMinimize:hover {
    background-color: #f1f5f9;
    color: #64748b;
}

/* 关闭悬浮 */
QPushButton#btnClose:hover {
    background-color: #ef4444;
    color: #ffffff;
}
```

## 验证标准

- [ ] 窗口无系统标题栏
- [ ] 最小化按钮悬浮时显示浅灰色背景
- [ ] 关闭按钮悬浮时显示红色背景
- [ ] 顶部区域可拖拽移动窗口
- [ ] 窗口有柔和阴影效果
- [ ] 点击最小化按钮窗口最小化
- [ ] 点击关闭按钮窗口关闭
- [ ] 现有表单功能不受影响
