# ClassAssistant

班级桌面小助手（Qt Widgets + CMake，离线优先，兼容 **Windows 7** 与 **Linux**）。

## 设计目标

- 统一为现代化、简洁的白色卡片 UI。
- 全部核心控件使用圆角设计（避免直角视觉）。
- 主界面与工具窗口采用丝滑过渡动画。
- 保持离线优先，不依赖网络功能。

## 当前功能

### 主界面快捷功能

- 希沃白板（本地程序启动）
- 班级考勤
- 随机点名
- 课堂计时器
- 课堂便签（本地保存）
- 分组抽签
- 课堂计分板
- 设置

### 课堂工具

- 考勤：支持搜索、缺勤勾选、导出缺勤名单。
- 随机点名：支持无重复模式与历史记录条数。
- 计时器：开始/暂停/重置。
- 便签：离线保存到本地配置文件。
- 分组抽签：按每组人数自动随机分组。
- 计分板：双队伍名称可配置，支持加减分。

## 设置页结构（类似 iPadOS）

- 左侧：一级目录（显示与启动 / 课堂工具 / 数据管理 / 安全与离线）
- 右侧：对应二级/三级详细项
- 底部一级操作区：**保存设置 / 还原默认设置 / 退出应用**

> 说明：保存、还原、退出不再放在某个二级页面中，避免逻辑分散。

## 动画与 UI 说明

- 侧边栏展开/收起：位移 + 透明度动画。
- 工具窗口：统一淡入淡出。
- 主界面收起可联动隐藏工具窗口（可配置）。
- 全局字体优先：`HarmonyOS Sans SC` / `HarmonyOS Sans`，自动回退系统字体。

## 图标文件存放路径（必须）

请将图标文件放在程序目录下：

- `assets/icons/`

程序图标查找顺序：

1. 绝对路径（自定义按钮）
2. `<程序目录>/assets/icons/`
3. `<程序目录>/assets/`
4. `<当前工作目录>/assets/icons/`
5. `<当前工作目录>/assets/`

## 图标文件命名规则（推荐）

文件格式建议：`PNG`（兼容性最佳）。

- `icon_tray.png`：托盘图标
- `icon_expand.png`：悬浮球展开图标
- `icon_collapse.png`：侧栏收起图标
- `icon_settings.png`：设置图标
- `icon_seewo.png`：希沃白板
- `icon_attendance.png`：班级考勤
- `icon_random.png`：随机点名
- `icon_timer.png`：课堂计时器（可选）
- `icon_note.png`：课堂便签（可选）
- `icon_group.png`：分组抽签（可选）
- `icon_score.png`：课堂计分（可选）

命名建议规范：

- 统一前缀：`icon_`
- 仅使用小写字母、数字、下划线
- 避免空格与中文文件名

## 数据存储

使用 `QStandardPaths::AppDataLocation`：

- `config.json`：保存设置、按钮、名单、便签等数据

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

## 兼容性说明

- 编译目标兼容 Windows 7（`_WIN32_WINNT=0x0601`）与 Linux。
- 不使用在线图标下载与网络依赖逻辑。
- 若目标环境缺少 Qt 开发包，请先安装 Qt5 SDK 后再构建。
