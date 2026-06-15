# AGENTS.md - ParkingSystem

## 构建系统

- **框架**: Qt 5.15.2, MinGW 8.1 (64-bit)
- **构建工具**: qmake
- **项目文件**: `ParkingSystem.pro`
- **构建命令**:
  ```bash
  qmake ParkingSystem.pro -spec win32-g++ "CONFIG+=debug"
  make debug
  ```
- **Release构建**: `make release`

## 关键依赖

- **OpenCV 3.4.8**: 硬编码路径 `C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64`
  - 修改需编辑 `ParkingSystem.pro` 中的 `INCLUDEPATH` 和 `LIBS`
- **MySQL**: 通过 Qt SQL 模块连接
- **Qt模块**: widgets, sql, gui, core, uitools, multimedia, multimediawidgets, charts

## 项目结构

```
ParkingSystem/
├── UI/                    # 界面模块（按功能分目录）
│   ├── Login/            # 登录窗口
│   ├── MainWindow/       # 主功能窗口
│   ├── HomePage/         # 主页
│   ├── VehicleInfo/      # 车辆信息页
│   ├── ManagePage/       # 管理页
│   ├── CameraPage/       # 摄像头页
│   └── ConfigInit/       # 配置初始化
├── styles/                # QSS样式文件
│   └── global.qss        # 全局样式（main.cpp中加载）
├── include/easypr/        # EasyPR车牌识别库头文件
├── src/core/              # EasyPR核心源码
├── thirdparty/            # 第三方依赖库
├── model/                 # 模型文件
└── docs/                  # 文档
```

## 架构要点

- **入口**: `main.cpp` → 加载全局QSS → 显示 LoginDialog
- **页面切换**: MainWindow 使用 QStackedWidget 管理子页面
- **摄像头**: CameraThread 在子线程运行，通过信号传递帧数据
- **数据库**: DatabaseManager 统一管理所有数据库操作
- **配置**: InitFile 管理 JSON 配置文件（路径由 QStandardPaths 定位）

## 样式系统

- 全局样式在 `styles/global.qss`，通过 `main.cpp` 加载
- 各页面可定义局部样式（如 `styles/login.qss`）
- 颜色常量定义见 `docs/界面开发文档.md` §5.1

## 注意事项

- **OpenCV路径**: 本地环境需确认 `C:\OpenCV-MinGW-Build-OpenCV-3.4.8-x64` 存在，否则需修改 `.pro` 文件
- **数据库配置**: 首次运行需通过 ConfigInit 配置 MySQL 连接信息
- **ui_*.h**: 由 Qt Designer 自动生成，不要手动编辑
- **Makefile**: 由 qmake 生成，不要手动修改

## 文档参考

- `docs/界面开发文档.md`: 完整的界面设计规范和实现指南
- `界面需求文档.md`: 功能需求说明
