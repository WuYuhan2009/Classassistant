# Classassistant

Classassistant 是一个运行在班级电脑上的跨平台桌面侧边栏工具，定位为**班级管理小助手**。项目使用 **C++17 + Qt Widgets (Qt 5.12~5.15 LTS)** 开发，面向：

- Windows 7（建议 Qt 5.15 LTS + MSVC 2019 / MinGW）
- Linux（Ubuntu / Debian / Fedora 等支持 Qt5 的发行版）

核心功能支持本地离线运行（首次可自动准备本地图标资源）。

---

## 功能概览

- 右侧竖向侧边栏（展开 / 收起）
- 右下角悬浮球（置顶、可拖动、点击恢复侧栏）
- 系统托盘常驻，右键菜单支持：
  - 打开设置
  - 展开侧边栏
  - 重载配置
  - 退出程序
- 首次启动欢迎向导：深浅色、侧栏宽度、图标大小
- 默认 6 按钮：
  1. 希沃白板启动
  2. 班级考勤
  3. ClassIsland 快捷换课
  4. 课堂随机点名
  5. AI 快捷按钮（豆包）
  6. 设置（固定底部逻辑）
- 自定义按钮系统：添加 / 删除（默认按钮不可删）/ 调序
- 名单导入（CSV/TXT；Excel 建议另存为 CSV）
- 考勤浮层（桌面右下角，显示应到/实到/请假）
- 每日自动清空考勤异常记录

---

## 目录结构

```text
Classassistant/
├─ CMakeLists.txt
├─ README.md
├─ src/
│  ├─ include/
│  │  ├─ appcontroller.h
│  │  ├─ attendanceoverlay.h
│  │  ├─ configmanager.h
│  │  ├─ custombuttondialog.h
│  │  ├─ floatingballwindow.h
│  │  ├─ models.h
│  │  ├─ randomnamedialog.h
│  │  ├─ resourcedownloader.h
│  │  ├─ settingsdialog.h
│  │  ├─ sidebarwindow.h
│  │  └─ welcomedialog.h
│  ├─ appcontroller.cpp
│  ├─ attendanceoverlay.cpp
│  ├─ configmanager.cpp
│  ├─ custombuttondialog.cpp
│  ├─ floatingballwindow.cpp
│  ├─ main.cpp
│  ├─ randomnamedialog.cpp
│  ├─ resourcedownloader.cpp
│  ├─ settingsdialog.cpp
│  ├─ sidebarwindow.cpp
│  └─ welcomedialog.cpp
├─ assets/
│  └─ icons/            # 首次运行自动补齐本地图标
├─ config/              # 预留目录
├─ plugins/             # 预留目录
└─ third_party/         # 预留目录
```

---

## 开发环境要求

## 1) 必需软件

- CMake >= 3.10
- Qt 5.12 ~ 5.15 LTS（建议 5.15.2）
- C++17 编译器
  - Windows: MSVC 2019 / MinGW 8+
  - Linux: GCC 9+ / Clang 10+

## 2) 推荐下载链接

- Qt: https://www.qt.io/download
- CMake: https://cmake.org/download/
- Visual Studio: https://visualstudio.microsoft.com/
- MinGW-w64: https://www.mingw-w64.org/

---

## 编译步骤

## Windows（Qt + MSVC 示例）

```bat
cd Classassistant
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" ..
cmake --build .
```

> 若使用 MinGW，请替换为对应 Qt MinGW 套件和生成器。

运行：

```bat
Classassistant.exe
```

## Linux（Qt5 + GCC 示例）

安装依赖（Debian/Ubuntu）：

```bash
sudo apt update
sudo apt install -y build-essential cmake qtbase5-dev
```

构建：

```bash
cd Classassistant
mkdir -p build
cd build
cmake ..
cmake --build . -j
```

运行：

```bash
./Classassistant
```

---

## 运行说明

- 程序默认常驻托盘；关闭窗口不会直接结束进程。
- 退出请通过托盘菜单“退出程序”。
- 配置文件位于：
  - Windows: `%APPDATA%/Classassistant/config.json`（Qt 平台映射路径）
  - Linux: `~/.config/Classassistant/config.json`

---

## 后续可扩展方向

- 真正的毛玻璃/高斯模糊（按平台启用）
- 更完整的 Excel 解析（接入 QXlsx / libxlsxwriter）
- IconPark SVG 图标包离线缓存器
- ClassIsland IPC 状态检测
- 更平滑的侧栏动画与 GPU 加速效果
