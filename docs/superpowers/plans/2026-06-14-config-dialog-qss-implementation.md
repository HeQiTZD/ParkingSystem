# ConfigInitDialog QSS 样式实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为 ConfigInitDialog 创建专属的 QSS 样式文件，采用现代简约风格

**Architecture:** 创建独立的 config.qss 文件，在 ConfigInitDialog 构造函数中加载，通过 Qt 资源系统访问

**Tech Stack:** Qt 5, QSS, Qt Resource System

---

## 文件结构

- Create: `styles/config.qss` — 配置窗口专属样式
- Modify: `UI/imageQrc/image.qrc:1-6` — 添加 config.qss 到资源系统
- Modify: `UI/ConfigInit/configinitdialog.cpp:1-13` — 加载样式文件

---

### Task 1: 创建 config.qss 样式文件

**Files:**
- Create: `styles/config.qss`

- [ ] **Step 1: 创建 config.qss 文件**

```css
/* ==================== ConfigInitDialog 样式 ==================== */

/* 对话框容器 */
QDialog {
    background-color: #ffffff;
}

/* -------------------- 标题样式 -------------------- */
QLabel#titleLabel {
    color: #1a1a2e;
    font-size: 18pt;
    font-weight: bold;
    padding: 10px 0;
}

/* -------------------- 分组标题 -------------------- */
QLabel#dbSectionLabel,
QLabel#parkingSectionLabel {
    color: #1a1a2e;
    font-size: 12pt;
    font-weight: 600;
    padding: 8px 0;
    border-bottom: 2px solid #3b82f6;
    margin-top: 10px;
}

/* -------------------- 标签样式 -------------------- */
QLabel#lblIP,
QLabel#lblPort,
QLabel#lblDBName,
QLabel#lblUsername,
QLabel#lblPassword,
QLabel#lblParkingName,
QLabel#lblPrice,
QLabel#lblCapacity {
    color: #475569;
    font-size: 14px;
    min-width: 80px;
    max-width: 80px;
}

/* -------------------- 输入框样式 -------------------- */
QLineEdit {
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 14px;
    font-size: 14px;
    color: #1e293b;
    background-color: #f8fafc;
    min-height: 20px;
}

QLineEdit:hover {
    border-color: #94a3b8;
}

QLineEdit:focus {
    border-color: #3b82f6;
    background-color: #ffffff;
    border-width: 2px;
    padding: 9px 13px;
}

/* -------------------- 提交按钮 -------------------- */
QPushButton#btnSubmit {
    background-color: #3b82f6;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
    font-weight: 600;
    min-height: 20px;
}

QPushButton#btnSubmit:hover {
    background-color: #2563eb;
}

QPushButton#btnSubmit:pressed {
    background-color: #1d4ed8;
}

/* -------------------- 取消按钮 -------------------- */
QPushButton#btnCancel {
    background-color: #ffffff;
    color: #475569;
    border: 1px solid #e2e8f0;
    border-radius: 6px;
    padding: 10px 24px;
    font-size: 14px;
    min-height: 20px;
}

QPushButton#btnCancel:hover {
    background-color: #f8fafc;
    border-color: #94a3b8;
}

QPushButton#btnCancel:pressed {
    background-color: #f1f5f9;
}
```

- [ ] **Step 2: 验证文件创建**

Run: `ls -la styles/config.qss`
Expected: 文件存在，大小约 1.5KB

- [ ] **Step 3: Commit**

```bash
git add styles/config.qss
git commit -m "feat: 创建 ConfigInitDialog 专属 QSS 样式文件"
```

---

### Task 2: 更新资源文件

**Files:**
- Modify: `UI/imageQrc/image.qrc:1-6`

- [ ] **Step 1: 更新 image.qrc 添加 config.qss**

```xml
<RCC>
    <qresource prefix="/new/prefix1">
        <file alias="brandImage">login.png</file>
        <file alias="config.qss">../styles/config.qss</file>
    </qresource>
</RCC>
```

- [ ] **Step 2: 验证资源文件**

Run: `cat UI/imageQrc/image.qrc`
Expected: 包含 config.qss 条目

- [ ] **Step 3: Commit**

```bash
git add UI/imageQrc/image.qrc
git commit -m "feat: 将 config.qss 添加到 Qt 资源系统"
```

---

### Task 3: 更新 ConfigInitDialog 加载样式

**Files:**
- Modify: `UI/ConfigInit/configinitdialog.cpp:1-13`

- [ ] **Step 1: 更新 configinitdialog.cpp 加载样式**

```cpp
#include "configinitdialog.h"
#include "ui_configinitdialog.h"
#include "initfile.h"
#include <QMessageBox>
#include <QFile>

ConfigInitDialog::ConfigInitDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigInitDialog)
{
    ui->setupUi(this);
    setFixedSize(500, 600);
    setWindowTitle("停车场数据初始化");

    // 加载配置窗口样式
    QFile styleFile(":/styles/config.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}

ConfigInitDialog::~ConfigInitDialog()
{
    delete ui;
}

bool ConfigInitDialog::validateInputs()
{
    if (ui->txtIP->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入IP地址");
        return false;
    }
    if (ui->txtPort->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入端口号");
        return false;
    }
    if (ui->txtDBName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入数据库名");
        return false;
    }
    if (ui->txtUsername->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入账户");
        return false;
    }
    if (ui->txtPassword->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入密码");
        return false;
    }
    if (ui->txtParkingName->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入停车场名");
        return false;
    }
    if (ui->txtPrice->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入价格");
        return false;
    }
    if (ui->txtCapacity->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入容量");
        return false;
    }
    return true;
}

void ConfigInitDialog::saveConfig()
{
    InitFile initFile;
    initFile.loadConfig();

    // 保存数据库配置
    initFile.setDbConfig(ui->txtIP->text(),
                         ui->txtPort->text().toInt(),
                         ui->txtDBName->text(),
                         ui->txtUsername->text(),
                         ui->txtPassword->text());

    // 保存停车场配置
    initFile.setParkingConfig(ui->txtParkingName->text(),
                              ui->txtPrice->text().toDouble(),
                              ui->txtCapacity->text().toInt());

    initFile.saveConfig();
}

void ConfigInitDialog::on_btnSubmit_clicked()
{
    if (validateInputs()) {
        saveConfig();
        QMessageBox::information(this, "成功", "配置初始化完成");
        accept();
    }
}

void ConfigInitDialog::on_btnCancel_clicked()
{
    reject();
}
```

- [ ] **Step 2: 验证代码修改**

Run: `grep -n "QFile" UI/ConfigInit/configinitdialog.cpp`
Expected: 第 5 行显示 `#include <QFile>`

- [ ] **Step 3: Commit**

```bash
git add UI/ConfigInit/configinitdialog.cpp
git commit -m "feat: 在 ConfigInitDialog 中加载专属 QSS 样式"
```

---

### Task 4: 构建和验证

**Files:**
- Test: 运行程序验证样式效果

- [ ] **Step 1: 清理并重新构建项目**

Run: `qmake ParkingSystem.pro -o Makefile && make clean && make`
Expected: 构建成功，无错误

- [ ] **Step 2: 运行程序验证样式**

Run: `./release/ParkingSystem.exe` 或 `./debug/ParkingSystem.exe`
Expected:
1. ConfigInitDialog 显示白色背景
2. 标题居中显示，18pt 粗体
3. 分组标题带蓝色底部分隔线
4. 输入框 focus 时显示蓝色边框
5. 提交按钮蓝色实心，取消按钮灰色边框

- [ ] **Step 3: 测试输入框交互**

操作：
1. 点击输入框，验证 focus 时蓝色边框和阴影效果
2. 悬停输入框，验证 hover 时边框变色
3. 点击提交按钮，验证 hover 和 pressed 状态
4. 点击取消按钮，验证 hover 和 pressed 状态

Expected: 所有交互效果符合设计规范

- [ ] **Step 4: 最终 Commit**

```bash
git add -A
git commit -m "feat: 完成 ConfigInitDialog QSS 样式实现"
```

---

## 验证清单

- [ ] config.qss 文件创建成功
- [ ] image.qrc 正确引用 config.qss
- [ ] configinitdialog.cpp 正确加载样式
- [ ] 程序构建成功
- [ ] 所有样式效果符合设计规范
- [ ] 交互效果正常工作

## 依赖关系

- 依赖现有 `styles/global.qss` 基础样式
- 依赖 `UI/imageQrc/image.qrc` 资源系统
- 依赖 Qt 5 构建系统