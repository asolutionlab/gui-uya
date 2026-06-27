# UyaGUI

`UyaGUI` 是一个基于 Uya 语言实现的嵌入式 GUI 运行时与 Linux 模拟器工程，仓库内包含 GUI 核心、渲染管线、布局系统、组件库、资源管理、平台抽象、SDL2/Framebuffer 模拟后端、示例、测试与性能基准。

项目目标是为资源受限场景提供零 GC、可移植、可测试、便于在 PC 上先行验证的 GUI 开发基线。

## 当前状态

- 截至 `2026-04-29`，Phase 0 到 Phase 5 的仓库内基线已具备，Phase 6 的文档与示例基线已补齐。
- 当前默认 smoke 入口为 `apps/phase6_smoke.uya`，`make build` 会生成 `build/phase6_smoke`。
- Linux 模拟器首版 SDL2 MVP 已落地，入口为 `apps/sim_main.uya`，可通过 `make sim-build`、`make sim-run`、`make sim-headless` 使用。
- `make bench-verify` 现会基于 `src/gui/benchmarks/phase5_bench_baseline.json` 校验 benchmark 回归阈值，`make dashboard-compare-report` 会生成 UyaGUI / LVGL dashboard 同场景对照报告。
- 默认文本渲染已从“占位方框”切换为内置 `5x7` 位图字形，英文、数字和常见标点可正常显示。
- 目标板实机验证、正式发布打包、真实字体/图片解码链路仍在后续范围内。

### 本地验证快照

以下结果基于 `2026-06-21` 在当前仓库工作区的实际执行：

- `uya --version` 输出 `v0.10.0`
- `make build` 通过
- `make sim-build SIM_BUILD_DIR=build/sim-check` 通过
- `make sim-web-build SIM_WEB_BUILD_DIR=build/web-check` 通过
- `make bench-verify` 通过
- `make test` 通过
  - `tests/test_suite.uya` 的 `165` 个测试通过
  - `tests/test_suite_extra.uya` 的 `57` 个测试通过
  - `tests/richtext_test_suite.uya` 的 `64` 个测试通过
  - `tests/web_present_plan_suite.uya` 的 `4` 个测试通过
  - `tests/render_test_suite.uya` 的 `38` 个测试通过

如果你当前只是想验证 GUI 核心与模拟器相关逻辑，可先单独执行：

```bash
make test
```

## 快速开始

### 1. 检查工具链

优先使用系统 `PATH` 中的 Uya 编译器：

```bash
uya --version
```

当前 `uya.toml` 要求的最小版本为 `0.9.4`。

### 2. 常用命令

```bash
make build
make test
make bench
make bench-report
make bench-json
make bench-verify
make dashboard-compare-report
make docs-api
make sim-web-build
make sim-web-pages
make release
```

说明：

- `make build`：构建默认 smoke 应用 `apps/phase6_smoke.uya`
- `make test`：运行 GUI 主测试集、扩展测试集、rich text、web present plan 和 render 测试集
- `make bench`：执行 benchmark
- `make bench-report`：生成 `build/phase5_bench.txt`
- `make bench-json`：把 `build/phase5_bench.txt` 解析为 `build/phase5_bench.json`
- `make bench-verify`：按 `src/gui/benchmarks/phase5_bench_baseline.json` 校验 benchmark 阈值
- `make dashboard-compare-report`：生成 `build/dashboard_compare/dashboard_compare_report.{md,json}`
- `make docs-api`：生成 API 索引文档 `docs/gui_uya_api_reference.md`
- `make sim-web-build`：构建 Web 模拟器产物到 `build/web/`
- `make sim-web-serve`：构建 Web 产物并启动本地静态服务
- `make sim-web-pages`：整理 GitHub Pages 可直接发布的站点文件到 `build/pages/`
- `make release`：执行 `ci` 后打包发布产物到 `build/release/uyagui-<version>.tar.gz`，并生成 `.sha256` 校验文件

### 3. 启用仓库 hooks

```bash
make hooks
```

仓库的 pre-commit hook 会在可用时格式化已暂存的 `*.uya` 文件，并执行 `make test`。

## Linux 模拟器

### 依赖

如果需要 SDL2 窗口模拟器，请先安装 SDL2 开发包。

Debian / Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev
```

### 构建与运行

```bash
make sim-build
make sim-run
make sim-debug
make sim-headless
make sim-fb-run
```

常见用法：

```bash
make sim-run SIM_ARGS="--demo phase6 --max-frames 120"
make sim-run SIM_ARGS="--demo dashboard --scale 1"
make sim-run SIM_ARGS="--demo music --scale 1"
make sim-run SIM_ARGS="--demo novel --scale 1"
make sim-run SIM_ARGS="--demo widgets --scale 1"
make sim-run SIM_ARGS="--demo custom --scale 1"
make sim-run SIM_ARGS="--max-frames 3 --screenshot build/sim/manual.bmp"
make sim-headless SIM_HEADLESS_ARGS="--max-frames 5 --screenshot build/sim/headless.bmp"
make uya-dashboard-compare
make lvgl-dashboard-compare
make dashboard-compare
make dashboard-compare-report
```

`make dashboard-compare` 会生成 `build/dashboard_compare/uya_dashboard.bmp`、
`build/dashboard_compare/lvgl_dashboard.bmp`，并分别输出 UyaGUI / LVGL 在同一 `640x480`
dashboard 场景下的启动与帧耗时。`make dashboard-compare-report` 会进一步补齐
`build/dashboard_compare/dashboard_compare_report.md`，把帧耗时、启动时间、最大 RSS 和
二进制体积汇总成一份对照报告。该目标默认用 `DASHBOARD_COMPARE_MODE=release` 构建 Uya，
避免 debug 数字误导；可用 `DASHBOARD_COMPARE_FRAMES=300` 调整采样帧数。如需查看 LVGL
每帧重建 dashboard 的成本，可运行
`make dashboard-compare-report LVGL_DASHBOARD_REBUILD=1`。

可用运行参数：

- `--demo phase4|phase6|clock|music|settings|dashboard|game|perf|novel|widgets|custom`
- `--width N --height N --scale N`（模拟器默认 `1920x1080`，`scale=1`）
- `--fullscreen | --windowed`
- `--root PATH`
- `--title TEXT`
- `--screenshot PATH`
- `--record PATH`
- `--playback PATH`
- `--max-frames N`
- `--profile-every N`
- `--hud | --no-hud`
- `--show-dirty | --hide-dirty`

当前 SDL2 模拟器常用快捷键：

- `Esc`：退出
- `P`：抓取截图（`.bmp` 或 `.uyafb`）
- `R`：开始或结束输入录制
- `L`：回放录制
- `F11`：切换全屏
- `1` / `6`：切换 `phase4` / `phase6` 场景
- `C` / `M` / `S` / `D` / `G` / `B` / `N` / `W` / `K`：切换 `clock` / `music` / `settings` / `dashboard` / `game` / `perf` / `novel` / `widgets` / `custom`
- `Space`：在 `novel` demo 里切换自动滚屏

当前内置字体说明：

- 默认字体为内置 `5x7` 位图字形
- 英文、数字和常见标点可正常显示
- 已支持 `U+4E00..U+9FFF` 常用 CJK 统一表意文字的内置 `8x8` 点阵显示
- 彩色与灰度 framebuffer 上会对字形斜边做轻量 alpha 抗锯齿软化
- 常见全角标点会归一化到 ASCII 标点显示
- 单色 `I1` framebuffer 仍保持硬边渲染
- 其余未覆盖的 Unicode 字符仍会回退为 `?`

## Web 模拟器

### 依赖

如果需要构建 `sim-web`，请先安装 Emscripten。Debian / Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y emscripten
```

### 构建与发布

```bash
make sim-web-build
make sim-web-run
make sim-web-pages
```

- `make sim-web-build` 会生成 `build/web/index.html`、`index.js`、`index.wasm`、`index.data`，以及独立的 `build/web/gui/render/generated/wqy_microhei_demo_*.{fnt,a8}` 外部字体资源
- `make sim-web-run` / `make sim-web-serve` 会生成 Web 产物并启动本地静态服务器
- `make sim-web-pages` 会把 GitHub Pages 需要的公开文件整理到 `build/pages/`，并写入 `.nojekyll`
- 仓库内置 `.github/workflows/sim-web-pages.yml`，推送到 `main` 后会自动构建并部署 Pages
- 页面支持用查询参数直接切换 demo，例如 `index.html?demo=dashboard`、`index.html?demo=music&debug=1`
- Web 构建默认使用仓库内固定的 [wqy-microhei.ttc](/media/winger/_dde_home/winger/uya/gui-uya/third_party/fonts/wqy/wqy-microhei.ttc)，避免 CI 依赖宿主机是否自带 CJK 字体
- Web 运行时不再 preload 整个 `gui/` 目录；`index.data` 仅保留最小运行时子集，WQY demo 位图字体会作为独立静态资源在启动前注入到 `/app/gui/render/generated/`
- 如需兼容旧版 Android WebView / 内置浏览器，可在构建前附加 `WEB_LEGACY_VM_SUPPORT=1`
- 如需改成别的字体，可在构建前覆盖 `WEB_CJK_FONT_SRC=/path/to/font.ttf`

## 交叉编译入口

仓库已提供几个便捷目标：

```bash
make build-arm
make build-riscv
make build-esp32
```

说明：

- `build-arm`：导出 C99，便于 Cortex-M 侧工具链继续处理
- `build-riscv`：导出 `rv32_baremetal_softvm` microapp
- `build-esp32`：导出 `xtensa_baremetal_softvm` microapp

## 目录结构

```text
.
├── docs/           # 架构、快速开始、模拟器、移植、性能、主题等文档
├── apps/           # 可执行入口与 smoke / simulator main
├── examples/       # smoke、demo 与示例业务逻辑
├── src/
│   └── gui/        # GUI 框架源码，保留 gui.* 模块命名空间
│       ├── core/   # 对象树、事件、脏区、几何与基础类型
│       ├── render/ # 渲染上下文、批处理、GPU、零拷贝、字体、图像
│       ├── widget/ # Button、Label、Page、Slider、Chart 等组件
│       ├── layout/ # abs / auto / flex / grid
│       ├── anim/   # tween / timeline / easing
│       ├── style/  # 主题、样式、属性系统
│       ├── res/    # pool、buffer、cache、fs
│       ├── platform/ # 显示、输入、时钟与 SDL2 / FB 后端
│       └── sim/    # 模拟器 runner、配置、录制、截图、profiler
├── tests/          # 单元测试、回归测试与测试 suite 入口
├── tools/          # 构建与文档脚本
├── Makefile
└── uya.toml
```

`uya.toml` 将 package source root 指向 `src`，因此发布源码只包含 `src/gui`，不会把顶层 `examples/` 和 `tests/` 一起作为库源码打包；同时源码仍以 `gui.*` 作为公开模块路径。

## 示例入口

- `examples/phase0_smoke.uya`
- `examples/phase1_smoke.uya`
- `examples/phase2_smoke.uya`
- `examples/phase3_smoke.uya`
- `examples/phase4_smoke.uya`
- `examples/phase6_smoke.uya`
- `examples/demo_clock.uya`
- `examples/demo_music.uya`
- `examples/demo_settings.uya`
- `examples/demo_dashboard.uya`
- `examples/demo_game.uya`
- `examples/demo_perf.uya`
- `examples/demo_novel.uya`
- `examples/demo_widgets.uya`
- `examples/demo_custom.uya`

## 文档导航

建议阅读顺序：

1. [快速开始](docs/gui_uya_quickstart.md)
2. [架构说明](docs/gui_uya_architecture.md)
3. [Linux 模拟器方案](docs/gui_uya_linux_sim.md)
4. [自定义组件指南](docs/gui_uya_custom_widget_guide.md)
5. [主题指南](docs/gui_uya_theme_guide.md)
6. [性能指南](docs/gui_uya_performance_guide.md)
7. [移植指南](docs/gui_uya_porting_guide.md)
8. [API 参考索引](docs/gui_uya_api_reference.md)

其他参考：

- [开发环境说明](docs/development.md)
- [设计文档](docs/gui_uya_design.md)
- [Phase 6 报告](docs/gui_uya_phase6_report.md)
- [项目 TODO](docs/gui_uya_todo.md)

## 已知问题

- `make sim-build` 能完成链接，但由 Uya 生成的 C 文件仍会产生较多编译 warning；目前不影响 `make sim-run` 正常使用。
- 当前截图已支持 `PNG`、`BMP` 导出；如需原始 framebuffer dump，可显式使用 `.uyafb`。
- Framebuffer 后端现已支持控制终端键盘热键，以及可选 `evdev` 指针/触摸/滚轮输入；更完整校准与多点手势仍未补齐。
- 默认内置字体已支持 ASCII 与 `U+4E00..U+9FFF` 中文常用字，但仍未覆盖完整 Unicode 字体排版链路。

## 相关文件

- [Makefile](Makefile)
- [项目配置](uya.toml)
- [模拟器构建脚本](tools/build_gui_sim.sh)
- [测试总入口](tests/test_suite.uya)
- [Render 测试总入口](tests/render_test_suite.uya)
- [模拟器入口](apps/sim_main.uya)
