# UyaGUI Linux 模拟器 TODO 文档

> 版本: v0.2.0  
> 日期: 2026-04-26  
> 状态: 已完成首版 SDL2 MVP 落地，并已在 2026-04-26 的 Linux + SDL2 2.32.4 环境完成实窗 smoke 与截图验证

> 说明: 本文已从“方案/示意代码文档”转换为“可执行 TODO 文档”。这里重点记录当前基线、缺口、实施顺序和验收标准，不再内嵌大段 SDL2 / Framebuffer 伪实现。若需要旧版方案细节，可从 git 历史查看。

---

## 目标

- 在 Linux PC 上运行 UyaGUI smoke/demo，用于 UI 预览、输入调试和性能采样。
- 优先落地 SDL2 后端，先解决“能跑、能看、能交互”，再考虑 Framebuffer 验证模式。
- 尽量复用当前仓库的 `platform` / `render` / `res` 基线，不重复造一套平台抽象。
- 不影响当前 `make build`、`make test`、`make bench` 工作流。

## 非目标

- 不在本文中定义 Uya 语言语法。
- 不把 Linux 模拟器当成真实硬件验证的替代品。
- 当前阶段不实现 X11 专用后端。
- 当前阶段不承诺完整 GPU 加速、热重载、可视化 Inspector。

## 当前基线

| 条目 | 状态 | 说明 |
|------|------|------|
| 显示/帧缓冲抽象 | [x] 已有基线 | `src/gui/platform/disp.uya` 已提供 `FrameBuffer`、`DisplayCtx`、像素格式与读写接口 |
| 输入抽象与事件队列 | [x] 已有基线 | `src/gui/platform/indev.uya` 已有 `IInputDev`、`TouchDriver`、`MouseDriver`、`KeyDriver`、`EncoderDriver` |
| Linux 计时接口 | [x] 已有基线 | `src/gui/platform/tick.uya` 已提供 `get_tick_ms`、`get_tick_us`、`sleep_ms` |
| 渲染上下文 | [x] 已有基线 | `src/gui/render/ctx.uya` 已可对 framebuffer 做基础绘制 |
| 资源与文件系统 | [x] 已有基线 | `src/gui/res/fs.uya`、`src/gui/res/cache.uya` 已有 Phase 4 入口 |
| Phase 4 I/O 回归 | [x] 已覆盖 | `tests/test_phase4_io.uya` 已验证 tick / host fs / async read / cache |
| Smoke / demo 入口 | [x] 已有离屏版本 | `apps/phase4_smoke.uya`、`apps/phase6_smoke.uya` 已可作为模拟器接入目标 |
| SDL2 显示后端 | [x] 已实现 | 已新增 `src/gui/platform/sdl2/{disp_sdl.uya,sdl_host.c}` |
| SDL2 OpenGL ES 2.0 后端 | [x] 已实现 | 已支持 `--gpu auto|software|gles2`，并在 `--gpu gles2 --cpu-render batch` 下直连 GLES2 batch 绘制 |
| CPU 渲染后端切换 | [x] 已实现 | 已支持 `--cpu-render immediate|batch`，默认 `immediate` |
| SDL2 输入后端 | [x] 已实现 | 已新增 `src/gui/platform/sdl2/indev_sdl.uya` |
| Linux 模拟器主循环 | [x] 已实现 | 已新增 `src/gui/sim/{main,runner,app,config}.uya` 与 `apps/sim_main.uya` 构建入口 |
| 截图/录制/Profiler 工具 | [x] 已实现 | 已新增 `src/gui/sim/{screenshot,recorder,profiler}.uya` |
| 模拟器构建命令 | [x] 已实现 | `Makefile` 已新增 `sim-build` / `sim-run` / `sim-debug` |

## 关键决策

- 主方案: SDL2
  - 原因: 最容易先做出窗口、输入、缩放和跨平台开发体验。
- 次方案: Linux Framebuffer
  - 只在 SDL2 MVP 稳定后再做，主要用于更贴近嵌入式环境的验证。
- 默认接入目标: 先 `apps/phase4_smoke.uya`，后 `apps/phase6_smoke.uya`
  - 原因: `Phase 4` 已覆盖 tick / fs / async cache / input 基线，更适合作为模拟器 MVP 的第一目标。
- 抽象策略: 复用现有 `src/gui/platform/{disp,indev,tick}.uya`
  - 除非现有接口明显不够，否则不新增一套 `platform/interface/*` 平行层。

## 里程碑总览

| 阶段 | 目标 | 当前状态 |
|------|------|----------|
| S0 | 收口接口、建立目录、确定 MVP 入口 | 已完成 |
| S1 | SDL2 显示/输入 MVP | 已完成 |
| S2 | 模拟器主程序与帧循环 | 已完成 |
| S3 | 调试工具（截图/录制/Profiler） | 已完成 |
| S4 | 资源/文件系统接线与 demo 扩展 | 已完成 |
| S5 | 构建脚本、调试工作流、文档收尾 | 已完成 |
| S6 | Framebuffer / Headless 扩展（可选） | 部分完成（headless 已落地，Framebuffer 显示后端已落地首版） |

---

## S0: 基线整理与接口收口

### 目标

把“仓库现状”和“模拟器真正需要的接入点”对齐，避免一上来就新增过多抽象层。

### TODO

- [x] 创建目录骨架
  - [x] `src/gui/platform/sdl2/`
  - [x] `src/gui/platform/fb/`
  - [x] `src/gui/sim/`
- [x] 盘点模拟器最小依赖接口
  - [x] 显示: framebuffer 获取、present/flush、窗口标题、缩放
  - [x] 输入: 非阻塞轮询、鼠标/键盘/滚轮事件映射
  - [x] 时钟: 毫秒/微秒 tick 与睡眠
  - [x] 文件系统: host fs 路径读取
- [x] 评估现有 `DisplayCtx` 与模拟器 present 流程之间的胶水层
- [x] 决定 MVP 默认入口使用 `apps/phase4_smoke.uya`
- [x] 明确 `phase6_smoke` 为第二阶段接入目标，而不是首个阻塞项

### 验收

- [x] 目录存在且命名定稿
- [x] 文档中不再出现与当前仓库脱节的伪目录结构
- [x] 模拟器 MVP 所需接口表能对应到真实文件

---

## S1: SDL2 显示/输入 MVP

### 目标

先让 Linux 上出现真实窗口，并把鼠标/键盘输入接进现有事件系统。

### TODO

- [x] `src/gui/platform/sdl2/disp_sdl.uya`
  - [x] SDL2 初始化与窗口创建
  - [x] renderer / texture 创建
  - [x] 把 `FrameBuffer` 内容上传到 SDL texture
  - [x] 支持窗口缩放倍率
  - [x] 支持窗口标题
  - [x] 基础全屏切换
  - [x] 支持全刷；脏区刷新当前退化为整帧 present
- [x] `src/gui/platform/sdl2/indev_sdl.uya`
  - [x] 鼠标左键映射到 touch press/release
  - [x] 鼠标移动映射到 touch/mouse move
  - [x] 键盘映射到 `KeyDriver`
  - [x] 滚轮映射到 `EncoderDriver`
  - [x] 退出事件映射到模拟器主循环停止条件
- [x] 计时策略
  - [x] 优先复用 `src/gui/platform/tick.uya`
  - [x] 仅在确有必要时补 SDL2 专用适配，不单独分叉一套 tick 实现

### 验收

- [x] Linux 上可打开 SDL2 窗口
- [x] 鼠标点击能驱动至少一个可见交互
- [x] `Esc` 或窗口关闭按钮能正常退出
- [x] 在没有 SDL2 依赖时，构建失败信息清晰可读

> 2026-04-26 实测记录：
> - `pkg-config --modversion sdl2` = `2.32.4`
> - `make sim-build` 成功
> - `make sim-run SIM_ARGS="--max-frames 3 --screenshot build/sim/makerun.bmp"` 成功退出并生成截图
> - 启动期真实踩到的 2 个问题已经修复：
>   1. `SdlInputSystem` 的接口值不能绑定到构造函数局部变量，否则 `poll()` 时会悬空
>   2. SDL2 不能误绑定到可执行里的 Uya `malloc/pthread_*` 符号；构建脚本已加 `-fvisibility=hidden`，host glue 也切回宿主 libc allocator

---

## S2: 模拟器主程序与帧循环

### 目标

把现有 smoke/demo、输入、渲染和 tick 串成一个真正可运行的 Linux 模拟器入口。

### TODO

- [x] `src/gui/sim/main.uya`
- [x] 初始化 display / input / tick / fs / cache
- [x] 增加基础配置项
  - [x] demo 入口
  - [x] width / height
  - [x] scale
  - [x] fullscreen
- [x] 实现主循环
  - [x] poll 输入
  - [x] update 业务/动画
  - [x] render 到 back buffer
  - [x] present 到 SDL2 窗口
  - [x] 控制退出与清理
- [x] 先接 `apps/phase4_smoke.uya`
- [x] 再接 `apps/phase6_smoke.uya`
- [x] 评估是否需要在模拟器层补一个简易 HUD
  - [x] FPS
  - [x] frame time
  - [x] dirty region 计数

### 验收

- [x] `phase4_smoke` 能在模拟器中完整跑通
- [x] 动画、输入、tick、host fs 至少各验证 1 条正向路径
- [x] 正常退出时无明显资源泄漏或重复释放问题

---

## S3: 调试工具

### 目标

让模拟器不仅“能跑”，还“值得拿来调试”。

### TODO

- [x] `src/gui/sim/screenshot.uya`
  - [x] 导出当前 framebuffer
  - [x] 支持原始像素导出与 `BMP` / `PNG` 编码
- [x] `src/gui/sim/profiler.uya`
  - [x] 统计 frame time
  - [x] 统计 update/render/present 时间分布
  - [x] 输出文本摘要
- [x] `src/gui/sim/recorder.uya`
  - [x] 录制鼠标/键盘/滚轮输入
  - [x] 回放固定输入序列
- [ ] Inspector 作为 backlog
  - [ ] 暂不阻塞 MVP

### 验收

- [x] 能抓一帧屏幕输出
- [x] 能打印基础性能统计
- [x] 能回放至少一段点击交互序列

---

## S4: 资源/文件系统接线与 demo 扩展

### 目标

把模拟器和现有 Phase 4/6 资源链路真正接起来，而不是只停留在空窗口。

### TODO

- [x] 复用 `src/gui/res/fs.uya` 现有 host fs / rom fs 基线
- [x] 复用 `tests/test_phase4_io.uya` 现有 I/O 回归
- [x] 约定模拟器运行时资源根目录
- [x] 明确示例资源路径解析规则
- [x] 若 `phase6_smoke` 需要额外资源，补齐缺失映射
- [x] 在模拟器中验证
  - [x] host fs 读取
  - [x] async read
  - [x] image cache 生命周期

### 验收

- [x] 资源路径规则在 Linux 下可复现
- [x] `phase4_smoke` 与 `phase6_smoke` 至少各跑通一条资源访问路径

---

## S5: 构建脚本、调试工作流、文档收尾

### 目标

让模拟器进入日常开发流程，而不是只能手工拼命令。

### TODO

- [x] 更新 `Makefile`
  - [x] `sim-build`
  - [x] `sim-run`
  - [x] `sim-debug`
- [x] 检测 SDL2 依赖
  - [x] `sdl2-config` 或 `pkg-config`
  - [x] 缺依赖时给出清晰提示
- [x] 增加 Linux 模拟器使用说明
  - [x] 依赖安装
  - [x] 构建
  - [x] 运行
  - [x] 常见问题
- [x] 视情况补 VS Code / Cursor 调试配置
- [x] 视情况增加 Linux 环境下的可选 CI job
  - [x] 至少保证模拟器入口可以编译

### 验收

- [x] 一条命令完成模拟器构建
- [x] 一条命令完成默认 demo 运行
- [x] 首次接手项目的人可按文档在 Linux 上复现

---

## S6: Framebuffer / Headless 扩展（可选）

### 目标

在 SDL2 稳定之后，再补更贴近嵌入式或更适合 CI 的运行模式。

### TODO

- [x] `src/gui/platform/fb/disp_fb.uya`
- [x] `src/gui/platform/fb/indev_fb.uya`
  - [x] 先补控制终端键盘热键与方向键
  - [x] 补 `evdev` 指针/触摸/滚轮最小接入
  - [ ] 多点手势与校准链路后续按需扩展
- [x] 评估 headless + 截图回归是否比 raw framebuffer 更实用
- [x] 若做 framebuffer 模式，补权限/设备节点说明
- [x] 若做 headless 模式，优先支持离屏渲染 + 导出截图

### 当前实现

- [x] `make sim-headless`
  - [x] 使用 `SDL_VIDEODRIVER=dummy`
  - [x] 支持 `SIM_HEADLESS_ARGS`
  - [x] 默认导出 `build/sim/headless.bmp`
- [x] `make sim-fb-run`
  - [x] 通过 `--backend fb` 复用同一套 simulator app/runtime
  - [x] `disp_fb.uya` + `fb_host.c` 已可打开 `/dev/fb0`、查询分辨率并做 ARGB8888 -> 设备像素格式写回
  - [x] `indev_fb.uya` + `fb_host.c` 已可从控制终端读取键盘热键与方向键
  - [x] 已可从可选 `evdev` 设备读取指针/触摸/滚轮
  - [x] 在当前机器上已验证“无权限访问 `/dev/fb0` 时返回清晰 `Permission denied`”

---

## 当前已验证命令

```bash
make test
make build
make sim-build
make sim-run SIM_ARGS="--max-frames 3 --screenshot build/sim/makerun.bmp"
./build/sim/gui_uya_sim --max-frames 5 --screenshot build/sim/manual.bmp
```

## 当前遗留项

- `make sim-build` 仍会打印较多由 Uya 生成 C 代码带来的 warning，但不阻塞链接与运行
- Framebuffer 模式已支持控制终端键盘热键，以及可选 `evdev` 指针/触摸/滚轮；多点手势与校准链路仍待后续补齐
- 默认内置字体已修复占位方框问题，已支持 `U+4E00..U+9FFF` 常用中文显示，并对彩色/灰度 framebuffer 启用轻量字形抗锯齿；完整 Unicode 字体排版链路仍待后续补齐

### 验收

- [ ] 可选后端不会反向拖慢 SDL2 主线
- [x] CI 或本地验证至少有一个“无窗口”运行模式

---

## 测试与验收矩阵

| 项目 | 当前状态 | 目标状态 |
|------|----------|----------|
| `make build` | [x] 已可用 | [x] 保持不回退 |
| `make test` | [x] 已可用 | [x] 保持不回退 |
| `make bench` | [x] 已可用 | [x] 保持不回退 |
| `test_phase4_io` | [x] 已覆盖 | [x] 继续作为模拟器 I/O 基线 |
| SDL2 模拟器构建 | [x] 已覆盖 | [x] `sim-build` |
| SDL2 模拟器运行 | [x] 已覆盖 | [x] `sim-run` |
| Headless 模拟器运行 | [x] 已覆盖 | [x] `sim-headless` |
| Framebuffer 模拟器运行 | [x] 已具备入口 | [x] `sim-fb-run` |
| 输入交互回归 | [x] 已覆盖 | [x] 点击/按键/滚轮 |
| 截图导出 | [x] 已覆盖 | [x] 至少一条正向回归 |
| 录制/回放 | [x] 已覆盖 | [x] 至少一条正向回归 |

## 推荐实施顺序

1. 先做 S0，收口接口与目录，避免抽象走偏。
2. 直接进入 S1，把 SDL2 窗口和输入打通。
3. 用 S2 把 `phase4_smoke` 跑起来，确认模拟器最小闭环。
4. 补 S5 的 `Makefile` 目标，让模拟器进入日常命令流。
5. 再做 S3 的调试工具，提升开发体验。
6. 最后再考虑 S4 扩资源细节和 S6 可选后端。

## 风险与依赖

- SDL2 FFI 的可用性依赖当前 Uya 编译器对宿主 C 导入链路的稳定性。
- 现有 `DisplayCtx` / `RenderCtx` 更偏离屏渲染，接入窗口 present 时可能需要额外胶水层。
- 真实图片/字体解码链路仍不完整，模拟器首阶段不应把这部分当阻塞项。
- Framebuffer 模式在 Linux 上涉及权限、设备节点和环境差异，不适合先做。

## Framebuffer 模式说明

- 默认设备节点：`/dev/fb0`
- 可覆盖：`--fb-dev /dev/fb1` 或环境变量 `UYA_GUI_FB_DEV=/dev/fb1`
- 当前机器实测：设备存在但普通用户权限不足，执行 `./build/sim/gui_uya_sim --backend fb --max-frames 1` 返回
  - `[sim] backend=fb`
  - `[sim] framebuffer init failed: Permission denied`
- 若需要本地验证，通常需要把当前用户加入 `video` 组或临时提升权限

## 相关文件

- 全局开发 TODO: `docs/gui_uya_todo.md`
- 平台显示基线: `src/gui/platform/disp.uya`
- 平台输入基线: `src/gui/platform/indev.uya`
- 平台时钟基线: `src/gui/platform/tick.uya`
- Phase 4 I/O 测试: `tests/test_phase4_io.uya`
- Smoke 入口: `apps/phase4_smoke.uya`、`apps/phase6_smoke.uya`
