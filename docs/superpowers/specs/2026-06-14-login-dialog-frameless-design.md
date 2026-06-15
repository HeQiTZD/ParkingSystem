# 登录窗口无边框样式设计文档

## 概述

将 LoginDialog 改造为与 ConfigInitDialog 相同样式的无边框窗口，保留原有的左右分栏布局（左侧品牌面板、右侧登录表单）。

## 设计目标

1. **样式一致性**：与 ConfigInitDialog 保持完全相同的无边框窗口样式
2. **功能完整性**：保留原有的登录功能和布局结构
3. **用户体验**：支持窗口拖动、最小化、关闭等操作

## 架构设计

### UI 结构改造

**当前结构：**
```
LoginDialog (QDialog)
└── QGridLayout
    ├── brandPanel (品牌面板)
    └── loginPanel (登录表单)
```

**目标结构：**
```
LoginDialog (QDialog, FramelessWindowHint)
└── mainContainer (QWidget, 圆角+阴影)
    ├── titleBar (QWidget, 支持拖动)
    │   └── systemBtnsPanel (QWidget)
    │       ├── btnMinimize (QPushButton)
    │       └── btnClose (QPushButton)
    └── contentArea (QWidget)
        └── QHBoxLayout
            ├── brandPanel (品牌面板)
            └── loginPanel (登录表单)
```

### C++ 代码改造

**新增成员变量：**
- `m_dragging: bool` - 拖动状态标志
- `m_dragPosition: QPoint` - 拖动起始位置

**新增方法：**
- `setupWindowFlags()` - 设置无边框窗口标志和透明背景
- `setupShadow()` - 为 mainContainer 添加阴影效果
- `setupTitleBar()` - 安装事件过滤器到 titleBar

**新增槽函数：**
- `on_btnMinimize_clicked()` - 最小化窗口
- `on_btnClose_clicked()` - 关闭窗口

**事件处理：**
- 重写 `eventFilter()` 方法，处理 titleBar 的鼠标事件实现窗口拖动

## 组件详细设计

### 1. mainContainer (主容器)

- **样式**：白色背景，圆角 12px
- **阴影**：QGraphicsDropShadowEffect
  - 模糊半径：20px
  - 颜色：rgba(0, 0, 0, 80)
  - 偏移：(0, 0)
- **布局**：垂直布局（titleBar + contentArea）

### 2. titleBar (标题栏)

- **高度**：40px
- **背景**：#f8f8f8
- **功能**：支持鼠标拖动移动窗口
- **内容**：右侧系统按钮面板

### 3. systemBtnsPanel (系统按钮面板)

- **布局**：水平布局，间距 8px
- **按钮**：
  - btnMinimize：最小化按钮
  - btnClose：关闭按钮
- **样式**：圆形按钮，使用 macOS 风格颜色
  - 最小化：#ffbd2e (黄色)
  - 关闭：#ff5f57 (红色)

### 4. contentArea (内容区域)

- **布局**：水平布局
- **左侧 brandPanel**：
  - 宽度：300px
  - 背景：渐变 (#667eea → #764ba2)
  - 内容：品牌图标 + 系统名称
- **右侧 loginPanel**：
  - 宽度：自适应
  - 内容：欢迎文字、用户名输入框、密码输入框、忘记密码链接、登录按钮

## 数据流

1. 用户点击标题栏 → eventFilter 捕获鼠标事件 → 移动窗口
2. 用户点击最小化按钮 → showMinimized()
3. 用户点击关闭按钮 → close()
4. 用户输入凭据 → 登录验证逻辑

## 错误处理

- 保持原有的登录验证逻辑不变
- 窗口拖动时的边界检查由 Qt 框架处理

## 测试策略

1. **功能测试**：
   - 验证窗口可以正常显示
   - 验证窗口拖动功能
   - 验证最小化和关闭功能
   - 验证登录功能正常工作

2. **视觉测试**：
   - 验证无边框样式正确应用
   - 验证阴影效果显示正常
   - 验证圆角效果正确
   - 验证与 ConfigInitDialog 样式一致

3. **兼容性测试**：
   - 在不同 DPI 设置下测试
   - 在不同 Windows 版本下测试

## 实施计划

### 阶段 1：UI 文件改造

1. 修改 `logindialog.ui` 文件
2. 添加 mainContainer、titleBar、systemBtnsPanel
3. 调整布局结构

### 阶段 2：C++ 代码改造

1. 修改 `logindialog.h` 文件
   - 添加成员变量
   - 添加方法声明
   - 添加槽函数声明

2. 修改 `logindialog.cpp` 文件
   - 实现 setupWindowFlags()
   - 实现 setupShadow()
   - 实现 setupTitleBar()
   - 实现 eventFilter()
   - 实现槽函数

### 阶段 3：样式调整

1. 确保样式表兼容新的 UI 结构
2. 测试并调整视觉效果

## 依赖关系

- 依赖 ConfigInitDialog 的样式参考
- 保持与现有登录逻辑的兼容性

## 风险和缓解措施

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| UI 结构改动导致布局错乱 | 高 | 逐步调整，保持原有布局比例 |
| 阴影效果在某些系统上不显示 | 低 | 提供降级方案（无阴影） |
| 窗口拖动与输入框焦点冲突 | 中 | 确保事件过滤器正确处理 |

## 参考资料

- ConfigInitDialog 实现：`UI/ConfigInit/configinitdialog.ui`、`configinitdialog.h`、`configinitdialog.cpp`
- Qt 无边框窗口文档：Qt::FramelessWindowHint
- Qt 阴影效果文档：QGraphicsDropShadowEffect
