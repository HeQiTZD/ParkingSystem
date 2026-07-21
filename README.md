# ParkingSystem — 智能停车场管理系统

基于 **Qt/C++** 开发的桌面级停车场管理系统，集成 **HyperLPR** 车牌识别引擎，提供从摄像头画面采集、车牌实时检测到收费管理、用户权限控制的完整业务闭环。

## 📋 目录

- [项目简介](#-项目简介)
- [技术栈](#-技术栈)
- [功能特性](#-功能特性)
- [项目结构](#-项目结构)
- [环境依赖](#-环境依赖)
- [编译构建](#-编译构建)
- [运行说明](#-运行说明)
- [许可证](#-许可证)

## 📖 项目简介

本项目是一个面向停车场管理场景的桌面应用程序，使用 Qt 框架构建图形界面，搭配 HyperLPR 深度学习车牌识别库实现摄像头画面中的车牌实时检测。系统集成了 MySQL 数据库用于数据持久化，支持多摄像头管理、用户权限分级、车辆进出记录查询、收费规则配置等完整功能。

**技术定位**：基于 Qt/C++ 开发的车牌识别管理系统，集成 HyperLPR 实现摄像头车牌检测、用户权限管理、本地数据库存储。

## 🛠 技术栈

| 类别 | 技术 |
|------|------|
| **框架** | Qt 5.15 + Qt Charts + Qt Multimedia + Qt SVG |
| **语言** | C++11 |
| **构建系统** | qmake（主工程）+ CMake（第三方依赖） |
| **车牌识别** | HyperLPR（基于 Caffe 深度学习框架） |
| **图像处理** | OpenCV 4.11 |
| **数据库** | MySQL |
| **编译优化** | OpenMP 并行加速 |
| **样式系统** | QSS（Qt Style Sheets），按模块拆分独立文件 |
| **UI 设计** | Qt Designer（`.ui` 文件）+ 手写自定义控件 |

## ✨ 功能特性

### 车牌识别
- 基于 HyperLPR 的深度学习车牌检测与识别
- 支持多线程帧采集 + 独立识别线程，不阻塞 UI
- 识别结果自动确认/修正机制（plate confirm tracker）
- 模型文件：Caffe SSD 检测模型 + RefineNet + SegmentationFree-Inception

### 摄像头管理
- 多摄像头实时画面预览（可拖拽排序）
- 每路摄像头独立配置：分辨率、帧率、IP/端口
- 角色互斥校验（如主通道/辅助通道）
- 属性设置窗口（全局参数 + 逐摄像头微调）

### 车辆出入管理
- 车辆入场自动记录（车牌、时间、抓拍图像）
- 出场结算（基于停车时长 × 费率）
- 入口/出口车辆实时展示组件

### 用户与权限
- 登录/注册界面（含密码强度校验、自定义输入控件）
- 管理员/普通用户权限分级
- 用户管理与角色分配

### 数据管理
- 基于 MySQL 的数据持久化
- 车辆信息查询（车牌搜索、时间范围筛选）
- 收费规则配置（时段费率、免费时长等）
- 分页查询组件

### UI/UX
- 模块化 QSS 样式系统（登录、主界面、设置、摄像头管理等）
- 通知/提示系统（Toast、模态通知）
- SVG 图标渲染
- 圆形进度条等自定义控件
- 日历选择器、时间选择器、分页等工具组件

## 📁 项目结构

```
ParkingSystem/
├── ParkingSystem.pro          # qmake 主工程文件
├── CLAUDE.md                  # 项目级 Claude 指令
│
├── src/
│   ├── app/                   # 应用核心：主入口、车牌识别、识别线程
│   ├── camera/                # 摄像头采集、帧队列、管理器
│   ├── database/              # 数据库连接池、管理器、初始化
│   ├── service/               # 业务服务层（停车、用户、车辆）
│   └── utils/                 # 工具组件（初始化配置、分页、日期选择等）
│
├── UI/
│   ├── Login/                 # 登录界面
│   ├── MainWindow/            # 主窗口 + 车辆出入展示
│   ├── CameraManagement/      # 摄像头管理 + 设置对话框
│   ├── UserManager/           # 用户管理
│   ├── VehicleInformation/    # 车辆信息查询
│   ├── Register/              # 注册界面
│   ├── ConfigInit/            # 首次配置向导
│   ├── Settings/              # 系统设置
│   └── imageQrc/              # 图标、图片资源
│
├── styles/
│   ├── main.qss               # 主界面样式
│   ├── login.qss              # 登录样式
│   ├── register.qss           # 注册样式
│   ├── settings.qss           # 设置界面样式
│   ├── cameramanagement.qss   # 摄像头管理样式
│   ├── camerasettings.qss     # 摄像头设置样式
│   ├── vehicleInformation.qss # 车辆信息样式
│   ├── userManagement.qss     # 用户管理样式
│   ├── config.qss             # 基础配置样式
│   └── styles.qrc             # 样式资源文件
│
├── thirdparty/
│   ├── hyperlpr/              # HyperLPR 车牌识别引擎（源码编译）
│   │   ├── include/           #   头文件
│   │   └── src/               #   源文件
│   └── opencv4/               # OpenCV 4.11 预编译库
│       ├── include/           #   头文件
│       └── lib/               #   导入库 + DLL
│
├── model/                     # 车牌识别模型文件
│   ├── mininet_ssd_v1.caffemodel / .prototxt
│   ├── refinenet.caffemodel / .prototxt
│   ├── SegmenationFree-Inception.caffemodel / .prototxt
│   └── cascade_double.xml
│
└── generated/                 # 构建生成文件（moc、rcc、uic）
```

## 🔧 环境依赖

### 操作系统
- **Windows** 10/11（主开发环境，MinGW 工具链）

### 必须安装

| 依赖 | 版本 | 说明 |
|------|------|------|
| Qt | Qt 6.10.2 | 需要 `widgets` `sql` `gui` `charts` `multimedia` `multimediawidgets` `svg` 模块 |
| 编译器 | MinGW 13.1.0 | 项目强制指定 `C:/Qt/Tools/mingw1310_64/bin/g++.exe` |
| MySQL | 8.x | 数据库服务端 |
| OpenCV | 4.11（已携带） | 项目 `thirdparty/opencv4/` 下提供预编译库 |

### 模型文件

车牌识别需要以下 Caffe 模型文件，放置于 `model/` 目录（项目已携带）：

| 文件 | 用途 |
|------|------|
| `mininet_ssd_v1.caffemodel` + `.prototxt` | SSD 车牌检测 |
| `refinenet.caffemodel` + `.prototxt` | RefineNet 精炼网络 |
| `SegmenationFree-Inception.caffemodel` + `.prototxt` | Segmentation-Free 识别网络 |
| `cascade_double.xml` | OpenCV 级联分类器 |

> **模型文件部署**：构建完成后，`QMAKE_POST_LINK` 自动将模型文件从 `debug/model/` 复制到可执行文件所在目录的 `model/` 子目录下。

## 🔨 编译构建

### 1. 克隆仓库

```bash
git clone https://github.com/HeQiTZD/ParkingSystem.git
cd ParkingSystem
```

### 2. 准备第三方库

项目已携带 OpenCV 4.11 预编译库（`thirdparty/opencv4/`）和 HyperLPR 源码（`thirdparty/hyperlpr/`），一般情况下无需额外下载。

> ⚠️ 确保 OpenCV DLL 路径正确：`thirdparty/opencv4/lib/libopencv_world4110.dll`
>
> 构建后会自动复制到可执行文件目录。

### 3. 配置 MySQL 数据库

首次运行时会弹出配置向导，需要填写：
- MySQL 主机地址（默认 `localhost`）
- 端口（默认 `3306`）
- 用户名 / 密码
- 数据库名

配置信息保存在本地配置文件中，之后启动自动加载。

### 4. 使用 Qt Creator 构建（推荐）

1. 用 **Qt Creator** 打开 `ParkingSystem.pro`
2. 选择 **MinGW 13.1.0** 工具链（项目已强制指定）
3. **启用影子构建（Shadow Build）**——项目禁止源内构建
4. 点击构建（`Ctrl+B`）

### 5. 命令行构建

```bash
# 清理
C:/Qt/6.10.2/mingw1310_64/bin/qmake.exe -r ParkingSystem.pro -spec win32-g++ CONFIG+=debug
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe clean

# 构建
C:/Qt/6.10.2/mingw1310_64/bin/qmake.exe -r ParkingSystem.pro -spec win32-g++ CONFIG+=debug
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe -j8
```

### 编译产出说明

| 目录 | 内容 |
|------|------|
| `debug/ParkingSystem.exe` | 可执行文件（Debug 模式） |
| `debug/*.dll` | 运行时依赖库（OpenCV 等） |
| `debug/model/*` | 车牌识别模型文件 |
| `generated/` | 中间文件（moc、rcc、uic） |

## 🚀 运行说明

### 首次启动

1. 运行 `ParkingSystem.exe`
2. 程序检测到无配置文件，自动弹出 **初始化配置向导（ConfigInitDialog）**
3. 填写 MySQL 数据库连接信息
4. 配置停车场基本参数（名称、容量、收费标准、免费时长）
5. 数据库表结构由程序自动创建

### 登录系统

- 默认需要注册账号后登录
- 支持密码可见性切换、密码强度显示

### 主界面功能

- **首页仪表盘**：车位占用概览、今日流量
- **摄像头管理**：添加/配置摄像头，实时识别车牌
- **车辆出入**：手动记录入场/出场，自动识别记录
- **车辆查询**：按车牌、日期范围搜索历史记录
- **用户管理**：添加/删除/修改用户权限
- **系统设置**：修改停车场参数、收费标准

## 📄 许可证

本项目仅供学习和参考。
