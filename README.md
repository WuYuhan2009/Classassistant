# ClassAssistant

班级桌面小助手（Qt Widgets + CMake）。

## 主要功能

- 侧栏展开态固定在屏幕右侧**垂直居中**。
- 收起态为右下角圆形悬浮球（置顶、可拖动、点击恢复侧栏）。
- 程序常驻托盘，窗口关闭仅隐藏，托盘右键菜单：
  - 打开设置
  - 展开侧边栏
  - 重载配置
  - 退出程序
- 首次启动欢迎向导（悬浮球透明度、考勤概览宽度、托盘点击行为、是否默认收起、默认路径）。
- 考勤流程：
  1. 点击侧栏“班级考勤”
  2. 弹出完整名单窗口勾选缺勤，可搜索、全选缺勤、清空勾选
  3. 保存后右下角底层概览仅展示：应到 / 实到 / 缺勤名单（固定底边、自动换行、向上增长）
- 设置页支持按钮管理（添加/删除/上移/下移，系统按钮不可删除）。

---

## 图标放置与命名规则（重要）

程序已内置在线图标地址（图床），启动时会自动检查并缓存到本地（仅每次启动尝试一次，仅下载缺失图标）。

若你希望覆盖默认图标，也可以继续放置在运行目录 `assets/` 下同名文件。

### 当前在线图标地址

- `icon_seewo.png`：https://upload.cc/i1/2026/02/08/Y6wmA8.png
- `icon_attendance.png`：https://upload.cc/i1/2026/02/08/HNo35p.png
- `icon_random.png`：https://upload.cc/i1/2026/02/08/Dt8WIg.png
- `icon_ai.png`：https://upload.cc/i1/2026/02/08/GeojsQ.png
- `icon_settings.png`：https://upload.cc/i1/2026/02/08/vCRlDF.png
- `icon_collapse.png`：https://upload.cc/i1/2026/02/08/BTjyOR.png
- `icon_expand.png`：https://upload.cc/i1/2026/02/08/N59bqp.png

### 推荐格式

- `PNG`（推荐）
- `SVG`
- `ICO`
- `JPG`

### 推荐命名

- `icon_tray.png`：托盘图标
- `icon_expand.png`：悬浮球中的“展开”图标
- `icon_collapse.png`：侧栏“收起”按钮图标
- `icon_settings.png`：侧栏“设置”按钮图标
- `icon_seewo.png`：希沃按钮
- `icon_attendance.png`：考勤按钮
- `icon_random.png`：随机点名按钮
- `icon_ai.png`：AI 按钮

### 200x200 图标适配说明

- 侧栏按钮大小固定为 `72x72`，图标绘制区域约 `40x40`，会自动居中显示。
- 悬浮球按钮大小固定为 `70x70`，图标绘制区域约 `40x40`，会自动居中显示。
- 建议图标主体在 `200x200` 画布中保持适当留白，避免边缘裁切。

> 如果图标缺失，程序会自动回退到文字或系统图标，不会崩溃。

---

## 数据存储位置

程序配置与数据使用 Qt 标准应用数据目录（`QStandardPaths::AppDataLocation`），保存文件：

- `config.json`：包含按钮配置、学生名单、启动偏好、透明度、宽度等。
- `icons/`：在线图标缓存目录（离线优先读取缓存）。

常见位置示例：

- Windows: `C:/Users/<用户名>/AppData/Roaming/<应用名>/config.json`
- Linux: `~/.local/share/<应用名>/config.json`
- macOS: `~/Library/Application Support/<应用名>/config.json`

---

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

如果提示找不到 `Qt5Config.cmake`，请安装 Qt5 开发环境并设置 `CMAKE_PREFIX_PATH`。

---

## 设置项说明

当前设置页可修改：

- 悬浮球透明度
- 考勤概览宽度
- 托盘单击是否展开侧栏
- 启动时是否显示考勤概览
- 启动时是否默认收起
- 默认程序路径
- 名单导入（CSV/TXT；XLS/XLSX 需先另存为 CSV/TXT）
- 自定义按钮管理
- 一键恢复默认设置（包含默认按钮与默认名单）
- 退出程序按钮

> 侧栏宽度与图标大小已固定，不允许在设置中修改，以优先保证稳定性。
