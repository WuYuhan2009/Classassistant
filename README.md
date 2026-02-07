# ClassAssistant（班级小助手）

一个基于 **Qt Widgets + CMake** 的桌面助手项目，面向课堂场景，提供：

- 悬浮球 + 侧边栏快捷入口
- 班级考勤面板
- 随机点名动画
- 设置面板（希沃路径、名单导入）
- 托盘驻留运行

> 本项目为模块化源码结构，可直接复制并在本地 Qt 环境编译。

---

## 1. 功能概览

### 1.1 核心交互
- **悬浮球**：可拖动，点击展开侧边栏。
- **侧边栏**：根据配置动态生成功能按钮，支持 `exe` / `url` / `func` 三类动作。
- **托盘菜单**：显示侧边栏、退出程序。

### 1.2 教学场景功能
- **班级考勤**：点击姓名切换“出勤/缺勤”，实时统计应到/实到/缺勤。
- **随机点名**：滚动动画随机抽取名单中的学生。
- **设置窗口**：
  - 修改希沃白板启动路径
  - 导入名单（支持 txt/csv，逗号/分号拆分）

### 1.3 配置持久化
- 自动在系统 `AppDataLocation` 下生成 `config.json`。
- 首次启动自动写入默认配置（按钮、学生名单、希沃路径）。
- 配置异常或字段缺失时会自动回退到安全默认值。

---

## 2. 目录结构

```text
ClassAssistant/
├── CMakeLists.txt
├── resources.qrc
├── build.sh                 # Linux/macOS 构建脚本
├── build.bat                # Windows 构建脚本
├── assets/                  # 图标资源（可选）
│   └── .gitkeep
├── config/                  # 占位目录（运行时配置实际写入系统目录）
│   └── .gitkeep
└── src/
    ├── main.cpp
    ├── Utils.h
    ├── Utils.cpp
    └── ui/
        ├── FloatingBall.h
        ├── FloatingBall.cpp
        ├── Sidebar.h
        ├── Sidebar.cpp
        ├── Tools.h
        └── Tools.cpp
```

---

## 3. 环境要求

- CMake >= 3.10
- C++17 编译器（GCC / Clang / MSVC）
- Qt5（推荐 5.15.x）
  - Core
  - Gui
  - Widgets
  - Network
  - WinExtras（仅 Windows）

> 如果你用 Qt6，请自行调整 `CMakeLists.txt` 的 `find_package` 和链接目标。

---

## 4. 快速开始

## 4.1 Linux / macOS

```bash
chmod +x build.sh
./build.sh clean release
```

调试构建：

```bash
./build.sh clean debug
```

构建后立即运行：

```bash
./build.sh clean release run
```

可选环境变量：

```bash
GENERATOR="Ninja" BUILD_TYPE=Debug ./build.sh
```

## 4.2 Windows（CMD）

```bat
build.bat clean release
```

调试构建：

```bat
build.bat clean debug
```

构建后运行：

```bat
build.bat clean release run
```

---

## 5. 常见问题（FAQ）

### 5.1 CMake 报错找不到 Qt5Config.cmake
说明当前环境没有安装 Qt 开发包，或 CMake 没有找到 Qt 安装路径。

可尝试：

- 安装 Qt SDK（含开发组件）
- 设置 `CMAKE_PREFIX_PATH` 指向 Qt 安装目录（包含 `lib/cmake/Qt5`）

示例（Linux）：

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/5.15.2/gcc_64
```

### 5.2 侧边栏图标不显示
- 若未在资源系统或本地提供图标，按钮会自动回退为文字显示（正常行为）。
- 建议后续在 `resources.qrc` 中登记图标，或在配置里填写可访问的本地路径。

### 5.3 托盘图标为空
如果 `:/assets/icon.png` 不存在，程序会自动使用系统标准图标兜底。

### 5.4 名单导入后无变化
请确认文件编码与内容：
- 支持逐行姓名
- 支持 CSV 一行多个（逗号`,` 或 分号`;` 分隔）
- 空行会自动忽略

---

## 6. 配置文件说明

配置文件位置：
- `QStandardPaths::AppDataLocation/config.json`

典型字段：

```json
{
  "seewoPath": "C:/Program Files (x86)/Seewo/EasiNote5/swenlauncher/swenlauncher.exe",
  "students": ["张三", "李四"],
  "buttons": [
    {
      "name": "班级考勤",
      "icon": ":/assets/check.png",
      "action": "func",
      "target": "ATTENDANCE",
      "isSystem": true
    }
  ]
}
```

按钮动作说明：
- `exe`：启动本地程序（target 为路径，或 `SEEWO` 特殊值）
- `url`：打开网址/协议
- `func`：内部功能（如 `ATTENDANCE` / `RANDOM_CALL` / `SETTINGS`）

---

## 7. 代码模块说明

- `src/main.cpp`
  - 应用入口
  - 托盘与悬浮球/侧边栏联动
  - 高分屏属性与窗口显示策略

- `src/Utils.*`
  - `Config` 单例
  - 配置读写与默认值回退
  - 名单导入（txt/csv）

- `src/ui/FloatingBall.*`
  - 自绘悬浮球
  - 拖拽和点击事件

- `src/ui/Sidebar.*`
  - 动态创建按钮
  - 动作分发处理

- `src/ui/Tools.*`
  - 考勤窗口
  - 随机点名对话框
  - 设置对话框

---

## 8. 近期修复与改进

本次相较初始版本，重点增强：

1. **配置鲁棒性**：
   - 处理 JSON 异常/缺失字段，避免空配置导致功能失效。
   - 默认值自动回填。
2. **名单导入增强**：
   - 支持 txt/csv 混合格式。
   - 支持逗号/分号拆分。
   - 自动去重与空值过滤。
3. **随机点名稳定性**：
   - 使用 `QRandomGenerator`，替代未播种 `rand()`。
4. **图标显示容错**：
   - 按钮优先使用 `QIcon`，失败后自动文字兜底。
   - 托盘图标不存在时使用系统标准图标。

---

## 9. 后续可扩展建议

- 将按钮配置改为可视化编辑（增删改查）
- 加入每日考勤数据落盘与导出（CSV/Excel）
- 增加多显示器位置记忆
- 增加国际化（i18n）与主题切换

---

## 10. License

当前仓库未声明 License。若用于公开分发，请补充 `LICENSE` 文件并明确授权协议。
