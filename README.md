# ClassAssistant

班级桌面小助手（Qt Widgets + CMake）。

## 主要功能

- 侧栏展开态固定在屏幕右侧垂直居中，固定宽度，不可调整组件尺寸。
- 收起态为右下角悬浮球（置顶、可拖动、点击恢复）。
- 程序常驻托盘，关闭窗口仅隐藏；支持托盘菜单与点击展开。
- 考勤流程：名单勾选缺勤 → 保存 → 右上角考勤概览实时更新（固定顶边、向下延伸、自动换行，窗口置于底层）。
- 考勤增强：支持“全员到齐”快捷清空与缺勤名单 TXT 导出。
- 随机点名增强：支持开始/停止两段式、结果高亮、最近点名历史（最多 5 条）和一键复制结果。
- 设置页按“显示与启动 / 课堂工具 / 数据与按钮管理”分区，便于快速定位。

---

## 本地图标方案（Windows 7 兼容）

已完全移除在线图标与云端缓存逻辑，程序只读取本地图标。请将 PNG 图标放到主程序目录：

- `assets/icons/`

程序会按以下优先级查找：

1. 绝对路径（用于自定义按钮）
2. `<程序目录>/assets/icons/`
3. `<程序目录>/assets/`
4. `<当前工作目录>/assets/icons/`
5. `<当前工作目录>/assets/`

若图标缺失，会回退为文字或系统图标，不会崩溃。

### 建议文件命名（PNG）

- `icon_tray.png`：托盘图标
- `icon_expand.png`：悬浮球“展开”图标
- `icon_collapse.png`：侧栏“收起”图标
- `icon_settings.png`：设置图标
- `icon_seewo.png`：希沃按钮
- `icon_attendance.png`：考勤按钮
- `icon_random.png`：随机点名按钮
- `icon_ai.png`：AI 按钮

---

## 数据存储

使用 `QStandardPaths::AppDataLocation`：

- `config.json`：设置、按钮、名单

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

---

## 设置项说明

- 悬浮球透明度
- 考勤概览宽度（固定范围）
- 托盘单击展开侧栏
- 启动时显示考勤概览
- 随机点名无重复（点完一轮自动重置）
- 启动时默认收起
- 默认程序路径
- 名单导入（CSV/TXT）
- 缺勤名单导出（TXT）
- 自定义按钮管理
- 恢复默认设置
- 退出程序
