# ParkingSystem — Smart Parking Management System

A desktop parking management system built with **Qt/C++**, integrating the **HyperLPR** license plate recognition engine, providing a complete business loop from camera capture and real-time plate detection to fee management and user access control.

## 📋 Table of Contents

- [About](#-about)
- [Tech Stack](#-tech-stack)
- [Features](#-features)
- [Project Structure](#-project-structure)
- [Prerequisites](#-prerequisites)
- [Build](#-build)
- [Running](#-running)
- [License](#-license)

## 📖 About

This is a desktop application for parking lot management scenarios. It uses the Qt framework for the graphical interface and HyperLPR deep learning library for real-time license plate detection from camera feeds. The system integrates MySQL for data persistence and supports multi-camera management, role-based user access, vehicle entry/exit records, and configurable fee rules.

**Tech positioning**: A Qt/C++ license plate recognition management system integrating HyperLPR for camera-based plate detection, user permission management, and local database storage.

## 🛠 Tech Stack

| Category | Technology |
|----------|------------|
| **Framework** | Qt 6.10.2 + Qt Charts + Qt Multimedia + Qt SVG |
| **Language** | C++11 |
| **Build System** | qmake (main project) + CMake (third-party deps) |
| **Plate Recognition** | HyperLPR (based on Caffe deep learning framework) |
| **Image Processing** | OpenCV 4.11 |
| **Database** | MySQL |
| **Compiler Optimizations** | OpenMP parallel acceleration |
| **Style System** | QSS (Qt Style Sheets), modularized per component |
| **UI Design** | Qt Designer (`.ui` files) + custom hand-written widgets |

## ✨ Features

### License Plate Recognition
- HyperLPR-based deep learning plate detection and recognition
- Multi-threaded frame capture with a dedicated recognition thread — never blocks the UI
- Auto-confirm and correction mechanism for recognition results (plate confirm tracker)
- Model files: Caffe SSD detection + RefineNet + SegmentationFree-Inception

### Camera Management
- Multi-camera live preview with drag-and-drop sorting
- Per-camera configuration: resolution, FPS, IP/port
- Role conflict validation (e.g., primary vs. auxiliary channel)
- Property settings dialog (global parameters + per-camera fine-tuning)

### Vehicle Entry/Exit Management
- Automatic recording of vehicle entry (plate number, timestamp, snapshot)
- Exit billing (based on parking duration × rate)
- Real-time entry/exit display widget

### Users & Permissions
- Login/registration UI with password strength validation and custom input controls
- Admin / regular user role hierarchy
- User management and role assignment

### Data Management
- MySQL-based data persistence
- Vehicle record query (by plate number, date range)
- Configurable fee rules (time-based rates, free minutes, etc.)
- Paginated query widget

### UI/UX
- Modular QSS style system (login, main window, settings, camera management, etc.)
- Notification system (Toast, modal notifications)
- SVG icon rendering
- Custom controls: circular progress bar, calendar picker, time selector, pagination, etc.

## 📁 Project Structure

```
ParkingSystem/
├── ParkingSystem.pro          # qmake main project file
│
├── src/
│   ├── app/                   # App core: main entry, plate recognition, recognition thread
│   ├── camera/                # Camera capture, frame queue, manager
│   ├── database/              # Connection pool, database manager, initialization
│   ├── service/               # Business service layer (parking, user, vehicle)
│   └── utils/                 # Utility components (init config, pagination, date pickers, etc.)
│
├── UI/
│   ├── Login/                 # Login interface
│   ├── MainWindow/            # Main window + vehicle entry/exit display
│   ├── CameraManagement/      # Camera management + settings dialog
│   ├── UserManager/           # User management
│   ├── VehicleInformation/    # Vehicle info query
│   ├── Register/              # Registration interface
│   ├── ConfigInit/            # First-run configuration wizard
│   ├── Settings/              # System settings
│   └── imageQrc/              # Icons and image resources
│
├── styles/
│   ├── main.qss               # Main window styles
│   ├── login.qss              # Login styles
│   ├── register.qss           # Registration styles
│   ├── settings.qss           # Settings styles
│   ├── cameramanagement.qss   # Camera management styles
│   ├── camerasettings.qss     # Camera settings styles
│   ├── vehicleInformation.qss # Vehicle info styles
│   ├── userManagement.qss     # User management styles
│   ├── config.qss             # Base config styles
│   └── styles.qrc             # Style resource file
│
├── thirdparty/
│   ├── hyperlpr/              # HyperLPR license plate recognition engine (compiled from source)
│   │   ├── include/           #   Headers
│   │   └── src/               #   Sources
│   └── opencv4/               # OpenCV 4.11 prebuilt libraries
│       ├── include/           #   Headers
│       └── lib/               #   Import libs + DLL
│
├── model/                     # License plate recognition model files
│   ├── mininet_ssd_v1.caffemodel / .prototxt
│   ├── refinenet.caffemodel / .prototxt
│   ├── SegmenationFree-Inception.caffemodel / .prototxt
│   └── cascade_double.xml
│
└── generated/                 # Build artifacts (moc, rcc, uic)
```

## 🔧 Prerequisites

### Operating System
- **Windows** 10/11 (primary development environment, MinGW toolchain)

### Required Software

| Dependency | Version | Notes |
|-----------|---------|-------|
| Qt | 6.10.2 | Requires `widgets` `sql` `gui` `charts` `multimedia` `multimediawidgets` `svg` modules |
| Compiler | MinGW 13.1.0 | Hard-coded to `C:/Qt/Tools/mingw1310_64/bin/g++.exe` |
| MySQL | 8.x | Database server |
| OpenCV | 4.11 (bundled) | Prebuilt libraries in `thirdparty/opencv4/` |

### Model Files

The following Caffe model files are required for plate recognition and are located in the `model/` directory (bundled with the project):

| File | Purpose |
|------|---------|
| `mininet_ssd_v1.caffemodel` + `.prototxt` | SSD plate detection |
| `refinenet.caffemodel` + `.prototxt` | RefineNet refinement network |
| `SegmenationFree-Inception.caffemodel` + `.prototxt` | Segmentation-Free recognition network |
| `cascade_double.xml` | OpenCV cascade classifier |

> **Model deployment**: After building, `QMAKE_POST_LINK` automatically copies the model files from `debug/model/` to the `model/` subdirectory next to the executable.

## 🔨 Build

### 1. Clone the Repository

```bash
git clone https://github.com/HeQiTZD/ParkingSystem.git
cd ParkingSystem
```

### 2. Third-Party Libraries

The project bundles OpenCV 4.11 prebuilt libraries (`thirdparty/opencv4/`) and HyperLPR source code (`thirdparty/hyperlpr/`). No additional download is normally required.

> ⚠️ Make sure the OpenCV DLL path is valid: `thirdparty/opencv4/lib/libopencv_world4110.dll`
>
> It is automatically copied to the output directory after building.

### 3. Configure MySQL Database

On first run, a configuration wizard will prompt you for:
- MySQL host (default `localhost`)
- Port (default `3306`)
- Username / Password
- Database name

The configuration is saved locally and loaded automatically on subsequent launches.

### 4. Build with Qt Creator (Recommended)

1. Open `ParkingSystem.pro` in **Qt Creator**
2. Select the **MinGW 13.1.0** toolchain (enforced by the project)
3. **Enable Shadow Build** — in-source builds are prohibited
4. Press Build (`Ctrl+B`)

### 5. Command-Line Build

```bash
# Clean
C:/Qt/6.10.2/mingw1310_64/bin/qmake.exe -r ParkingSystem.pro -spec win32-g++ CONFIG+=debug
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe clean

# Build
C:/Qt/6.10.2/mingw1310_64/bin/qmake.exe -r ParkingSystem.pro -spec win32-g++ CONFIG+=debug
C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe -j8
```

### Build Output

| Directory | Contents |
|-----------|----------|
| `debug/ParkingSystem.exe` | Executable (Debug mode) |
| `debug/*.dll` | Runtime dependencies (OpenCV, etc.) |
| `debug/model/*` | License plate recognition model files |
| `generated/` | Intermediate files (moc, rcc, uic) |

## 🚀 Running

### First Launch

1. Run `ParkingSystem.exe`
2. The program detects no config file and automatically opens the **Configuration Wizard (ConfigInitDialog)**
3. Enter MySQL database connection details
4. Configure parking lot parameters (name, capacity, fee rates, free minutes)
5. Database tables are created automatically

### Login

- Register an account first, then log in
- Password visibility toggle and strength indicator supported

### Main Interface

- **Dashboard**: Parking occupancy overview, today's traffic
- **Camera Management**: Add/configure cameras, real-time plate recognition
- **Vehicle Entry/Exit**: Manual entry/exit recording, auto-recognition logging
- **Vehicle Query**: Search history by plate number or date range
- **User Management**: Add, delete, and modify user permissions
- **System Settings**: Modify parking lot parameters and fee rules

### Model File Notes

> The project's recent commit (`83e92bd`) fixed the model file deployment issue:
>
> Model files are automatically copied via `QMAKE_POST_LINK` from `debug/model/` to the `model/` directory next to the executable after building.
>
> If deploying manually, copy all files from the `model/` directory to a `model/` subdirectory next to the executable.

## 📄 License

This project is for learning and reference purposes only.
