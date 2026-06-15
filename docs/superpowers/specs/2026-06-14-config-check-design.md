# 配置文件检查功能设计文档

## 1. 概述

### 1.1 功能描述
在程序入口添加配置文件检查逻辑，根据配置文件是否存在决定显示登录窗口还是配置窗口。

### 1.2 需求分析
- 配置文件存在：直接显示登录窗口
- 配置文件不存在：显示配置窗口
- 配置完成后：自动进入登录窗口
- 取消配置：退出程序

### 1.3 设计目标
- 逻辑清晰，代码简洁
- 复用现有InitFile类
- 用户体验流畅
- 错误处理完善

## 2. 架构设计

### 2.1 整体流程
```
程序启动
    ↓
加载全局样式
    ↓
创建InitFile实例检查配置文件
    ↓
配置文件存在？
    ├── 是 → 显示登录窗口
    └── 否 → 显示配置窗口
                ↓
            配置完成？
                ├── 是 → 显示登录窗口
                └── 否 → 退出程序
```

### 2.2 组件关系
- **InitFile**：配置文件管理类，提供isConfigExists()方法
- **LoginDialog**：登录窗口
- **ConfigInitDialog**：配置初始化窗口
- **MainWindow**：主窗口

### 2.3 数据流
1. 程序启动时创建InitFile实例
2. 调用isConfigExists()检查配置文件
3. 根据检查结果创建相应对话框
4. 处理用户交互结果

## 3. 详细设计

### 3.1 main.cpp修改
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

### 3.2 关键逻辑说明

#### 3.2.1 配置文件检查
- 使用`InitFile::isConfigExists()`方法
- 检查应用程序目录下的config.json文件
- 返回布尔值表示文件是否存在

#### 3.2.2 对话框显示逻辑
1. **配置文件存在**：
   - 直接创建LoginDialog实例
   - 调用exec()显示模态对话框
   - 根据返回值决定是否显示主窗口

2. **配置文件不存在**：
   - 创建ConfigInitDialog实例
   - 调用exec()显示模态对话框
   - 根据返回值决定后续流程

#### 3.2.3 用户交互处理
- **配置窗口确认**：进入登录窗口
- **配置窗口取消**：退出程序
- **登录窗口确认**：进入主窗口
- **登录窗口取消**：退出程序

### 3.3 错误处理

#### 3.3.1 配置文件检查失败
- InitFile类内部处理文件不存在的情况
- 自动创建默认配置文件
- 不影响主流程

#### 3.3.2 对话框显示失败
- Qt框架保证对话框创建成功
- 异常情况由Qt事件循环处理

## 4. 实现细节

### 4.1 依赖关系
- **InitFile类**：提供配置文件检查功能
- **LoginDialog类**：登录窗口实现
- **ConfigInitDialog类**：配置窗口实现
- **MainWindow类**：主窗口实现

### 4.2 资源管理
- 对话框对象在栈上创建，自动管理生命周期
- InitFile对象在main函数作用域内，程序结束时自动销毁

### 4.3 线程安全
- 所有UI操作在主线程执行
- 无需额外的线程同步机制

## 5. 测试方案

### 5.1 功能测试
1. **配置文件存在测试**
   - 预期：直接显示登录窗口
   - 验证：检查登录窗口是否正常显示

2. **配置文件不存在测试**
   - 预期：显示配置窗口
   - 验证：检查配置窗口是否正常显示

3. **配置完成后测试**
   - 预期：自动进入登录窗口
   - 验证：检查配置完成后是否显示登录窗口

4. **取消配置测试**
   - 预期：程序退出
   - 验证：检查程序是否正常退出

### 5.2 边界测试
1. **配置文件损坏测试**
   - 预期：InitFile类处理异常，创建默认配置
   - 验证：检查程序是否正常运行

2. **权限问题测试**
   - 预期：InitFile类处理权限错误
   - 验证：检查错误处理是否正常

## 6. 部署说明

### 6.1 文件修改
- 仅修改`main.cpp`文件
- 无需修改其他文件

### 6.2 编译要求
- 确保所有依赖类已正确实现
- 确保Qt环境配置正确

### 6.3 运行环境
- 支持所有Qt支持的平台
- 需要图形界面环境

## 7. 维护说明

### 7.1 代码维护
- 逻辑集中在main.cpp中，便于维护
- 使用已有的InitFile类，减少重复代码

### 7.2 功能扩展
- 如需添加新的启动逻辑，可在main函数中扩展
- 如需修改配置文件格式，修改InitFile类即可

### 7.3 问题排查
- 配置文件检查问题：检查InitFile::isConfigExists()实现
- 对话框显示问题：检查相应对话框类的实现
- 流程问题：检查main函数中的条件判断逻辑