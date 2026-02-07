# ClassAssistant

班级助手（Qt Widgets + CMake），支持：

- 侧栏展开 / 悬浮球收起（悬浮球置顶、可拖动、点击恢复）
- 系统托盘常驻（设置 / 展开 / 重载配置 / 退出）
- 首次启动向导（主题、侧栏宽度、图标大小、默认路径）
- 设置中心（外观、路径、名单导入、按钮管理）
- 班级考勤（右下角底层展示，应到/实到/请假名单，日切自动清空）
- 自定义按钮（添加 / 删除 / 上下移动；系统默认按钮不可删除）

## 构建

### Linux / macOS

```bash
chmod +x build.sh
./build.sh clean release
```

### Windows

```bat
build.bat clean release
```

> 若 CMake 提示找不到 `Qt5Config.cmake`，请安装 Qt5 开发包并设置 `CMAKE_PREFIX_PATH`。

## 首次启动

首次运行会自动弹出欢迎向导，要求完成：

1. 深色/浅色模式
2. 侧栏大小
3. 图标大小
4. 默认程序路径（希沃）

完成后会写入 `config.json`（位于 `QStandardPaths::AppDataLocation`）。

## 托盘机制

程序默认常驻托盘：

- 窗口关闭按钮只会隐藏窗口，不会退出进程
- 右键托盘菜单：
  - 打开设置
  - 展开侧边栏
  - 重载配置
  - 退出程序

## 名单导入说明

设置页支持导入文件类型：

- `CSV`
- `TXT`
- `XLS/XLSX`（当前版本会提示先另存为 CSV/TXT 再导入）

> 说明：当前实现未引入第三方 Excel 解析库，以保证项目轻量和可移植。

## 自定义按钮系统

设置页提供按钮管理：

- 添加按钮：设置名称、图标、动作类型、目标
- 删除按钮：仅可删除自定义按钮
- 顺序调整：上移/下移

动作类型：

- `exe`：打开程序/文件
- `url`：打开链接
- `func`：调用内置功能（如 `ATTENDANCE` / `RANDOM_CALL` / `SETTINGS`）

## 项目结构

```text
ClassAssistant/
├── CMakeLists.txt
├── resources.qrc
├── build.sh
├── build.bat
└── src/
    ├── main.cpp
    ├── Utils.h
    ├── Utils.cpp
    └── ui/
        ├── FloatingBall.h/.cpp
        ├── Sidebar.h/.cpp
        └── Tools.h/.cpp
```
