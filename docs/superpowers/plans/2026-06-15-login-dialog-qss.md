# LoginDialog QSS 样式实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 LoginDialog 添加专属的QSS样式，参考 ConfigInitDialog 的设计风格，保持项目视觉一致性。

**Architecture:** 创建独立的 `styles/login.qss` 文件，包含所有 LoginDialog 的样式定义。在 `logindialog.cpp` 中加载该QSS文件，确保样式仅作用于登录对话框。

**Tech Stack:** Qt QSS (Qt Style Sheets), C++

---

## 文件结构

```
styles/
├── config.qss          # ConfigInitDialog样式（已存在）
└── login.qss           # LoginDialog样式（新建）

UI/Login/
└── logindialog.cpp     # 修改：加载QSS文件
```

---

### Task 1: 创建 login.qss 样式文件

**Files:**
- Create: `styles/login.qss`

- [ ] **Step 1: 创建 login.qss 文件**

创建 `styles/login.qss` 文件，包含所有 LoginDialog 的样式定义：

```css
/* ==================== LoginDialog 样式 ==================== */
/* 作用域限制：仅影响 LoginDialog (#LoginDialog) */

/* -------------------- 对话框容器 -------------------- */
#LoginDialog {
    background-color: #ffffff;
    border-radius: 8px;
}

/* -------------------- 标题栏按钮样式 -------------------- */
/* 最小化按钮 */
#LoginDialog QPushButton#pushButton_4 {
    background: transparent;
    border: none;
    border-radius: 4px;
    color: #94a3b8;
    font-size: 14px;
    font-weight: bold;
    min-width: 25px;
    max-width: 25px;
    min-height: 25px;
    max-height: 25px;
    padding: 0px;
}

#LoginDialog QPushButton#pushButton_4:hover {
    background-color: #f1f5f9;
    color: #64748b;
}

#LoginDialog QPushButton#pushButton_4:pressed {
    background-color: #e2e8f0;
}

/* 退出按钮 */
#LoginDialog QPushButton#pushButton_3 {
    background: transparent;
    border: none;
    border-radius: 4px;
    color: #94a3b8;
    font-size: 14px;
    font-weight: bold;
    min-width: 30px;
    max-width: 30px;
    min-height: 30px;
    max-height: 30px;
    padding: 0px;
}

#LoginDialog QPushButton#pushButton_3:hover {
    background-color: #ef4444;
    color: #ffffff;
}

#LoginDialog QPushButton#pushButton_3:pressed {
    background-color: #dc2626;
}

/* -------------------- 输入框样式 -------------------- */
/* 用户名输入框 */
#LoginDialog QLineEdit#lineEdit_2 {
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 14px;
    font-size: 14px;
    color: #1e293b;
    background-color: #f8fafc;
    min-height: 41px;
    max-height: 41px;
}

#LoginDialog QLineEdit#lineEdit_2:hover {
    border-color: #94a3b8;
}

#LoginDialog QLineEdit#lineEdit_2:focus {
    border-color: #3b82f6;
    background-color: #ffffff;
    border-width: 2px;
    padding: 9px 13px;
}

/* 密码输入框 */
#LoginDialog QLineEdit#lineEdit_3 {
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 14px;
    font-size: 14px;
    color: #1e293b;
    background-color: #f8fafc;
    min-height: 41px;
    max-height: 41px;
}

#LoginDialog QLineEdit#lineEdit_3:hover {
    border-color: #94a3b8;
}

#LoginDialog QLineEdit#lineEdit_3:focus {
    border-color: #3b82f6;
    background-color: #ffffff;
    border-width: 2px;
    padding: 9px 13px;
}

/* -------------------- 按钮样式 -------------------- */
/* 登录按钮 */
#LoginDialog QPushButton#pushButton {
    background-color: #3b82f6;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
    font-weight: 600;
    min-height: 41px;
    max-height: 41px;
}

#LoginDialog QPushButton#pushButton:hover {
    background-color: #2563eb;
}

#LoginDialog QPushButton#pushButton:pressed {
    background-color: #1d4ed8;
}

/* 注册按钮 */
#LoginDialog QPushButton#pushButton_2 {
    background-color: #ffffff;
    color: #475569;
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
    min-height: 41px;
    max-height: 41px;
}

#LoginDialog QPushButton#pushButton_2:hover {
    background-color: #f8fafc;
    border-color: #94a3b8;
}

#LoginDialog QPushButton#pushButton_2:pressed {
    background-color: #f1f5f9;
}

/* -------------------- 标签样式 -------------------- */
/* 标题标签 */
#LoginDialog QLabel#label {
    color: #1a1a2e;
    padding: 10px 0;
}

/* 副标题标签 */
#LoginDialog QLabel#label_2 {
    color: #475569;
    padding: 5px 0;
}

/* 用户名标签 */
#LoginDialog QLabel#label_3 {
    color: #475569;
    font-size: 14px;
    min-width: 411px;
    max-width: 411px;
    min-height: 25px;
    max-height: 25px;
}

/* 密码标签 */
#LoginDialog QLabel#label_4 {
    color: #475569;
    font-size: 14px;
    min-width: 411px;
    max-width: 411px;
    min-height: 25px;
    max-height: 25px;
}

/* -------------------- 辅助元素样式 -------------------- */
/* 分隔线 */
#LoginDialog QFrame#line,
#LoginDialog QFrame#line_2 {
    background-color: #e2e8f0;
    max-height: 1px;
}

/* "或"标签 */
#LoginDialog QLabel#label_5 {
    color: #94a3b8;
    padding: 0 10px;
}

/* 记住我复选框 */
#LoginDialog QCheckBox#checkBox {
    color: #475569;
    font-size: 14px;
    spacing: 8px;
}

#LoginDialog QCheckBox#checkBox::indicator {
    width: 16px;
    height: 16px;
    border: 1px solid #e2e8f0;
    border-radius: 3px;
    background-color: #f8fafc;
}

#LoginDialog QCheckBox#checkBox::indicator:checked {
    background-color: #3b82f6;
    border-color: #3b82f6;
}

#LoginDialog QCheckBox#checkBox::indicator:hover {
    border-color: #94a3b8;
}

/* 忘记密码标签 */
#LoginDialog QLabel#label_6 {
    color: #3b82f6;
    padding: 5px;
}

#LoginDialog QLabel#label_6:hover {
    color: #2563eb;
    text-decoration: underline;
}

/* -------------------- 底部信息样式 -------------------- */
/* 版权信息 */
#LoginDialog QLabel#label_7 {
    color: #94a3b8;
    min-width: 100px;
    max-width: 300px;
}

/* 服务条款 */
#LoginDialog QLabel#label_8 {
    color: #3b82f6;
    min-width: 50px;
    max-width: 50px;
    min-height: 25px;
    max-height: 25px;
    padding: 5px;
}

#LoginDialog QLabel#label_8:hover {
    color: #2563eb;
    text-decoration: underline;
}

/* 隐私政策 */
#LoginDialog QLabel#label_9 {
    color: #3b82f6;
    min-width: 50px;
    max-width: 50px;
    min-height: 25px;
    max-height: 25px;
    padding: 5px;
}

#LoginDialog QLabel#label_9:hover {
    color: #2563eb;
    text-decoration: underline;
}
```

- [ ] **Step 2: 验证文件创建**

运行：`ls -la styles/login.qss`
预期：文件存在，大小约 4KB

- [ ] **Step 3: 提交QSS文件**

```bash
git add styles/login.qss
git commit -m "feat: 创建 LoginDialog QSS 样式文件"
```

---

### Task 2: 修改 logindialog.cpp 加载QSS

**Files:**
- Modify: `UI/Login/logindialog.cpp:1-15`

- [ ] **Step 1: 修改 logindialog.cpp 添加QSS加载**

修改 `UI/Login/logindialog.cpp` 文件，添加QSS文件加载逻辑：

```cpp
#include "logindialog.h"
#include "ui_logindialog.h"
#include <QFile>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 加载QSS样式文件
    QFile styleFile(":/styles/login.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
```

- [ ] **Step 2: 验证代码修改**

运行：`cat UI/Login/logindialog.cpp`
预期：文件包含QFile头文件和QSS加载逻辑

- [ ] **Step 3: 提交代码修改**

```bash
git add UI/Login/logindialog.cpp
git commit -m "feat: 在 LoginDialog 中加载 QSS 样式文件"
```

---

### Task 3: 更新资源文件

**Files:**
- Modify: `UI/imageQrc/image.qrc`

- [ ] **Step 1: 检查现有资源文件**

运行：`cat UI/imageQrc/image.qrc`
预期：查看当前资源文件结构

- [ ] **Step 2: 添加QSS文件到资源**

在 `UI/imageQrc/image.qrc` 文件中添加 login.qss 资源。在现有的 `<qresource prefix="/icons">` 块之后添加新的资源块：

```xml
<RCC>
    <qresource prefix="/icons">
        <!-- 现有图标资源保持不变 -->
    </qresource>
    <qresource prefix="/styles">
        <file>../../styles/login.qss</file>
    </qresource>
</RCC>
```

- [ ] **Step 3: 验证资源文件修改**

运行：`cat UI/imageQrc/image.qrc`
预期：文件包含 login.qss 资源引用

- [ ] **Step 4: 提交资源文件修改**

```bash
git add UI/imageQrc/image.qrc
git commit -m "feat: 将 login.qss 添加到资源文件"
```

---

### Task 4: 测试样式效果

**Files:**
- Test: 运行应用程序验证样式

- [ ] **Step 1: 编译项目**

运行：`qmake && make` 或 `cmake --build .`
预期：编译成功，无错误

- [ ] **Step 2: 运行应用程序**

运行：`./ParkingSystem` 或相应可执行文件
预期：应用程序启动，显示登录对话框

- [ ] **Step 3: 验证样式效果**

检查以下内容：
- 对话框背景为白色，圆角设计
- 最小化和退出按钮透明背景，悬浮效果正常
- 输入框浅灰背景，聚焦时蓝色边框
- 登录按钮蓝色背景，注册按钮白色背景
- 所有标签颜色正确
- 复选框样式正确
- 底部链接蓝色，悬浮时下划线

- [ ] **Step 4: 测试交互状态**

测试以下交互：
- 鼠标悬浮在按钮上，检查颜色变化
- 点击输入框，检查聚焦样式
- 悬浮在忘记密码、服务条款等链接上，检查下划线效果
- 点击复选框，检查选中状态

- [ ] **Step 5: 记录测试结果**

记录任何样式问题或需要调整的地方。

---

### Task 5: 最终提交和清理

**Files:**
- 所有修改的文件

- [ ] **Step 1: 检查所有修改**

运行：`git status`
预期：显示所有修改的文件

- [ ] **Step 2: 添加所有修改**

```bash
git add -A
```

- [ ] **Step 3: 最终提交**

```bash
git commit -m "feat: 完成 LoginDialog QSS 样式实现

- 创建 styles/login.qss 样式文件
- 修改 logindialog.cpp 加载QSS
- 更新资源文件包含QSS
- 参考 ConfigInitDialog 设计风格
- 保持项目视觉一致性"
```

- [ ] **Step 4: 验证提交**

运行：`git log -1`
预期：显示最新的提交记录

---

## 完成检查清单

- [ ] login.qss 文件创建并包含所有样式定义
- [ ] logindialog.cpp 修改并正确加载QSS
- [ ] 资源文件更新包含QSS文件
- [ ] 项目编译成功
- [ ] 应用程序运行正常
- [ ] 所有样式效果符合设计规范
- [ ] 交互状态正常工作
- [ ] 代码已提交到git

---

## 注意事项

1. **作用域限制**：所有样式使用 `#LoginDialog` 作为前缀，确保仅影响登录对话框
2. **尺寸保持**：所有min/max尺寸设置与UI文件一致，不修改布局
3. **字体设置**：rich text 标签不设置font-size/font-weight，避免被内联样式覆盖
4. **兼容性**：样式与Qt 5/6兼容，使用标准QSS属性
5. **资源路径**：确保QSS文件正确添加到资源系统，路径为 `:/styles/login.qss`