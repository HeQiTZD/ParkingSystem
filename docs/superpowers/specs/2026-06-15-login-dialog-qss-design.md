# LoginDialog QSS 样式设计规范

## 概述

为 `UI/Login/logindialog.ui` 设计专属的QSS样式，参考 `UI/ConfigInit/configinitdialog.ui` 的设计风格，保持项目视觉一致性。

**设计约束：**
- 禁止修改布局结构
- 仅通过QSS调整视觉样式
- 保持所有组件的min/max尺寸设置

## 设计方案

### 选择方案：完全一致风格

完全复用ConfigInitDialog的样式系统，确保项目视觉统一性。

## 详细设计

### 1. 整体对话框样式

```css
#LoginDialog {
    background-color: #ffffff;
    border-radius: 8px;
}
```

**设计要点：**
- 白色背景 (#ffffff)
- 圆角设计 (8px)
- 与ConfigInitDialog主容器一致

### 2. 标题栏按钮样式

#### 最小化按钮 (pushButton_4)

```css
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
```

#### 退出按钮 (pushButton_3)

```css
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
```

### 3. 输入框样式

#### 用户名输入框 (lineEdit_2)

```css
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
```

#### 密码输入框 (lineEdit_3)

```css
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
```

### 4. 按钮样式

#### 登录按钮 (pushButton)

```css
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
```

#### 注册按钮 (pushButton_2)

```css
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
```

### 5. 标签样式

#### 标题标签 (label)

```css
#LoginDialog QLabel#label {
    color: #1a1a2e;
    font-size: 18pt;
    font-weight: 600;
    padding: 10px 0;
}
```

#### 副标题标签 (label_2)

```css
#LoginDialog QLabel#label_2 {
    color: #475569;
    font-size: 10pt;
    padding: 5px 0;
}
```

#### 用户名标签 (label_3)

```css
#LoginDialog QLabel#label_3 {
    color: #475569;
    font-size: 14px;
    min-width: 411px;
    max-width: 411px;
    min-height: 25px;
    max-height: 25px;
}
```

#### 密码标签 (label_4)

```css
#LoginDialog QLabel#label_4 {
    color: #475569;
    font-size: 14px;
    min-width: 411px;
    max-width: 411px;
    min-height: 25px;
    max-height: 25px;
}
```

### 6. 辅助元素样式

#### 分隔线样式

```css
#LoginDialog QFrame#line,
#LoginDialog QFrame#line_2 {
    background-color: #e2e8f0;
    max-height: 1px;
}
```

#### "或"标签 (label_5)

```css
#LoginDialog QLabel#label_5 {
    color: #94a3b8;
    font-size: 12px;
    padding: 0 10px;
}
```

#### 记住我复选框 (checkBox)

```css
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
```

#### 忘记密码标签 (label_6)

```css
#LoginDialog QLabel#label_6 {
    color: #3b82f6;
    font-size: 14px;
    padding: 5px;
}

#LoginDialog QLabel#label_6:hover {
    color: #2563eb;
    text-decoration: underline;
}
```

### 7. 底部信息样式

#### 版权信息 (label_7)

```css
#LoginDialog QLabel#label_7 {
    color: #94a3b8;
    font-size: 12px;
    min-width: 100px;
    max-width: 300px;
}
```

#### 服务条款 (label_8)

```css
#LoginDialog QLabel#label_8 {
    color: #3b82f6;
    font-size: 12px;
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
```

#### 隐私政策 (label_9)

```css
#LoginDialog QLabel#label_9 {
    color: #3b82f6;
    font-size: 12px;
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

## 配色方案

| 用途 | 颜色值 | 说明 |
|------|--------|------|
| 主背景 | #ffffff | 白色 |
| 输入框背景 | #f8fafc | 浅灰 |
| 主题色 | #3b82f6 | 蓝色 |
| 标题文字 | #1a1a2e | 深蓝黑 |
| 正文文字 | #475569 | 深灰 |
| 次要文字 | #94a3b8 | 中灰 |
| 边框 | #e2e8f0 | 浅灰 |
| 悬浮边框 | #94a3b8 | 中灰 |
| 错误/关闭 | #ef4444 | 红色 |

## 交互状态

### 按钮状态
- **默认**: 透明背景 + 灰色文字
- **悬浮**: 浅灰背景 + 深灰文字
- **按下**: 更深灰背景
- **关闭按钮悬浮**: 红色背景 + 白色文字

### 输入框状态
- **默认**: 浅灰背景 + 浅灰边框
- **悬浮**: 边框变深
- **聚焦**: 白色背景 + 蓝色边框 + 边框加粗

### 链接状态
- **默认**: 蓝色文字
- **悬浮**: 深蓝文字 + 下划线

## 实现说明

1. **作用域限制**: 所有样式使用 `#LoginDialog` 作为前缀，确保仅影响登录对话框
2. **尺寸保持**: 所有min/max尺寸设置与UI文件一致，不修改布局
3. **字体设置**: 不设置font-size/font-weight，因为标签使用rich text，内联样式会覆盖QSS
4. **兼容性**: 样式与Qt 5/6兼容，使用标准QSS属性

## 文件结构

```
styles/
├── config.qss          # ConfigInitDialog样式
└── login.qss           # LoginDialog样式（新增）
```

## 下一步

1. 创建 `styles/login.qss` 文件
2. 在 `logindialog.cpp` 中加载QSS文件
3. 测试样式效果
4. 根据实际效果进行微调