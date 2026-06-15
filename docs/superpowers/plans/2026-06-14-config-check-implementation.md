# 配置文件检查功能实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 在程序入口添加配置文件检查逻辑，根据配置文件是否存在决定显示登录窗口还是配置窗口

**Architecture:** 在main.cpp中使用InitFile类检查配置文件，根据检查结果创建相应的对话框（LoginDialog或ConfigInitDialog），处理用户交互结果

**Tech Stack:** Qt C++, QDialog, InitFile类

---

## 文件结构

### 修改的文件
- `main.cpp` - 程序入口，添加配置文件检查逻辑

### 依赖的文件（只读）
- `initfile.h` / `initfile.cpp` - 提供isConfigExists()方法
- `UI/Login/logindialog.h` - 登录窗口类
- `UI/ConfigInit/configinitdialog.h` - 配置初始化窗口类
- `UI/MainWindow/mainwindow.h` - 主窗口类

---

## 实现任务

### Task 1: 修改main.cpp添加配置文件检查

**Files:**
- Modify: `main.cpp`

- [ ] **Step 1: 读取当前main.cpp内容**

读取main.cpp文件，了解当前实现，确定修改位置。

- [ ] **Step 2: 添加配置文件检查逻辑**

在main函数中，加载全局样式之后添加配置文件检查逻辑：

```cpp
#include "initfile.h"
#include "UI/Login/logindialog.h"
#include "UI/MainWindow/mainwindow.h"
#include "UI/ConfigInit/configinitdialog.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 加载全局样式
    QFile styleFile(":/styles/global.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
        styleFile.close();
    }

    // 创建InitFile实例检查配置文件
    InitFile initFile;
    
    // 检查配置文件是否存在
    if (initFile.isConfigExists()) {
        // 配置文件存在，直接显示登录窗口
        LoginDialog loginDialog;
        if (loginDialog.exec() == QDialog::Accepted) {
            MainWindow mainWindow;
            mainWindow.show();
            return app.exec();
        }
    } else {
        // 配置文件不存在，显示配置窗口
        ConfigInitDialog configDialog;
        if (configDialog.exec() == QDialog::Accepted) {
            // 配置完成，显示登录窗口
            LoginDialog loginDialog;
            if (loginDialog.exec() == QDialog::Accepted) {
                MainWindow mainWindow;
                mainWindow.show();
                return app.exec();
            }
        } else {
            // 用户取消配置，退出程序
            return 0;
        }
    }

    return 0;
}
```

- [ ] **Step 3: 保存修改后的main.cpp**

保存文件，确保代码格式正确。

- [ ] **Step 4: 编译测试**

运行编译命令，确保代码没有语法错误：

```bash
qmake ParkingSystem.pro
make
```

预期：编译成功，没有错误

- [ ] **Step 5: 功能测试**

运行程序，测试以下场景：

1. **配置文件存在时**：
   - 删除config.json文件（如果存在）
   - 运行程序
   - 预期：显示登录窗口

2. **配置文件不存在时**：
   - 确保config.json不存在
   - 运行程序
   - 预期：显示配置窗口

3. **配置完成后**：
   - 在配置窗口中完成配置
   - 点击确认
   - 预期：自动进入登录窗口

4. **取消配置时**：
   - 在配置窗口中点击取消
   - 预期：程序退出

- [ ] **Step 6: 提交代码**

```bash
git add main.cpp
git commit -m "feat: 添加配置文件检查逻辑，根据配置文件存在与否显示不同窗口"
```

---

## 测试验证

### 测试场景

1. **配置文件存在测试**
   - 前置条件：config.json文件存在
   - 操作：运行程序
   - 预期结果：直接显示登录窗口
   - 验证方法：检查登录窗口是否正常显示

2. **配置文件不存在测试**
   - 前置条件：config.json文件不存在
   - 操作：运行程序
   - 预期结果：显示配置窗口
   - 验证方法：检查配置窗口是否正常显示

3. **配置完成后测试**
   - 前置条件：config.json文件不存在
   - 操作：在配置窗口完成配置并确认
   - 预期结果：自动进入登录窗口
   - 验证方法：检查配置完成后是否显示登录窗口

4. **取消配置测试**
   - 前置条件：config.json文件不存在
   - 操作：在配置窗口点击取消
   - 预期结果：程序退出
   - 验证方法：检查程序是否正常退出

### 边界测试

1. **配置文件损坏测试**
   - 前置条件：config.json文件内容损坏
   - 操作：运行程序
   - 预期结果：InitFile类处理异常，创建默认配置
   - 验证方法：检查程序是否正常运行

2. **权限问题测试**
   - 前置条件：config.json文件权限受限
   - 操作：运行程序
   - 预期结果：InitFile类处理权限错误
   - 验证方法：检查错误处理是否正常

---

## 部署说明

### 文件修改
- 仅修改`main.cpp`文件
- 无需修改其他文件

### 编译要求
- 确保所有依赖类已正确实现
- 确保Qt环境配置正确

### 运行环境
- 支持所有Qt支持的平台
- 需要图形界面环境

---

## 维护说明

### 代码维护
- 逻辑集中在main.cpp中，便于维护
- 使用已有的InitFile类，减少重复代码

### 功能扩展
- 如需添加新的启动逻辑，可在main函数中扩展
- 如需修改配置文件格式，修改InitFile类即可

### 问题排查
- 配置文件检查问题：检查InitFile::isConfigExists()实现
- 对话框显示问题：检查相应对话框类的实现
- 流程问题：检查main函数中的条件判断逻辑