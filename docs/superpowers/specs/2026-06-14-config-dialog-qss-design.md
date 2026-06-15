# ConfigInitDialog QSS 样式设计

## 概述

为 ConfigInitDialog 创建专属的 QSS 样式文件，采用现代简约风格，与现有 global.qss 保持一致的蓝色主题。

## 设计决策

### 风格选择
- **视觉风格**：现代简约风格（方案A）
- **输入框交互**：带阴影的聚焦效果
- **按钮样式**：标准样式（提交蓝色实心，取消灰色边框）
- **分组展示**：带分隔线的分组

### 实现方式
- **文件组织**：独立配置文件（方案A）
- **文件路径**：`styles/config.qss`
- **加载方式**：在 ConfigInitDialog 构造函数中加载

## 文件结构

```
styles/
├── global.qss          # 全局基础样式
├── login.qss           # 登录窗口样式
└── config.qss          # 新增：配置窗口样式
```

## 样式规则

### 对话框容器
```css
QDialog {
    background-color: #ffffff;
}
```

### 标题样式
```css
QLabel#titleLabel {
    color: #1a1a2e;
    font-size: 18pt;
    font-weight: bold;
    padding: 10px 0;
}
```

### 分组标题
```css
QLabel#dbSectionLabel,
QLabel#parkingSectionLabel {
    color: #1a1a2e;
    font-size: 12pt;
    font-weight: 600;
    padding: 8px 0;
    border-bottom: 2px solid #3b82f6;
    margin-top: 10px;
}
```

### 标签样式
```css
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
```

### 输入框样式
```css
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
```

### 按钮样式
```css
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

## 代码变更

### 1. 创建 styles/config.qss
包含上述所有样式规则。

### 2. 更新 UI/imageQrc/image.qrc
添加 config.qss 到资源文件：
```xml
<file>styles/config.qss</file>
```

### 3. 更新 UI/ConfigInit/configinitdialog.cpp
在构造函数中加载样式：
```cpp
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
```

## 验证标准

1. ConfigInitDialog 显示白色背景
2. 标题居中显示，18pt 粗体
3. 分组标题带蓝色底部分隔线
4. 输入框 focus 时显示蓝色边框
5. 提交按钮蓝色实心，取消按钮灰色边框
6. 所有控件与 global.qss 基础样式兼容

## 依赖关系

- 依赖现有 `styles/global.qss` 基础样式
- 依赖 `UI/imageQrc/image.qrc` 资源系统