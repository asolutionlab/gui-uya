# UyaGUI Web 后端 TODO 文档

> 版本: v0.2.0  
> 日期: 2026-05-19  
> 状态: 已完成主线实现（真实 `wasm/js/html`、dirty present、无头 smoke 已打通）

> 说明: 本文是“原生 Web 后端”实施路线图，目标是在浏览器中运行当前 `sim`/demo，并保持与现有 `SDL2` / `Framebuffer` 后端一致的分层方式。本文重点记录当前基线、缺口、实施顺序和验收标准。

---

## 目标

- 新增 `platform/web/disp_web.uya` 与 `platform/web/indev_web.uya`。
- 新增 Web 专用入口 `apps/sim_web_main.uya`，让 `sim` 逻辑运行到浏览器 `canvas`。
- 不复制一套网页专用 GUI 逻辑，只补宿主显示/输入/主循环差异层。
- 让 `SDL2` / `Framebuffer` / `Web` 三端尽量共享同一套 `SimRuntimeCore`。

## 非目标

- 当前阶段不做 DOM/CSS 渲染。
- 当前阶段不做 WebGPU。
- 当前阶段不把 SDL-to-WASM 当长期方案。
- 当前阶段不承诺移动端浏览器、完整 IME 预编辑体验、复杂文本输入、worker 多线程。

## 当前基线

| 条目 | 状态 | 说明 |
|------|------|------|
| 显示/帧缓冲抽象 | [x] 已有基线 | `src/gui/platform/disp.uya` 已提供 `FrameBuffer` / `DisplayCtx` / 像素格式 |
| 输入抽象与事件队列 | [x] 已有基线 | `src/gui/platform/indev.uya` 已有 `TouchDriver` / `KeyDriver` / `EncoderDriver` |
| 渲染上下文 | [x] 已有基线 | `src/gui/render/ctx.uya` 已能在 framebuffer 上渲染 |
| retained / dirty render | [x] 已有基线 | `src/gui/render/scheduler.uya` 与 sim retained 流程已可复用 |
| SDL2 显示后端 | [x] 已实现 | `src/gui/platform/sdl2/{disp_sdl.uya,indev_sdl.uya,sdl_host.c}` |
| Framebuffer 后端 | [x] 已实现 | `src/gui/platform/fb/{disp_fb.uya,indev_fb.uya,fb_host.c}` |
| 模拟器配置 | [x] 已实现 | `src/gui/sim/config.uya` 已支持 backend/demo/size/fps/screenshot 等参数 |
| 模拟器主循环 | [x] 已实现 | `src/gui/sim/runner.uya` 已支持 `SDL2` 与 `FB` |
| 浏览器后端 | [x] 已有 MVP | 已新增 `src/gui/platform/web/{disp_web.uya,indev_web.uya,web_common.uya,web_host.c}` |
| 共享 runtime core | [x] 已实现 | 已新增 `src/gui/sim/runtime_core.uya`，SDL2/FB 已切到共享帧逻辑 |
| Web 构建脚本 | [x] 已实现 | 已新增 `tools/build_gui_web.sh` 与 `tools/serve_gui_web.sh` |

## 关键决策

- 主方案：原生 Web backend，不走 SDL-to-WASM 作为长期结构。
- 首版显示：`Canvas2D + software present`。
- 首版输入：mouse/touch/wheel/keyboard 标准化为 `WebHostEvent`。
- 首版资源：MEMFS 预打包；持久化作为可选扩展。
- 结构前置：先抽 `SimRuntimeCore`，再接 `Web` backend。
- ownership 固定：Uya 负责一次性 bootstrap，host 只负责 rAF 调度和浏览器事件桥接。

## 里程碑总览

| 阶段 | 目标 | 当前状态 |
|------|------|----------|
| W0 | 收口设计、整理文件布局、确定配置策略 | 已完成 |
| W1 | 抽共享 `SimRuntimeCore` | 已完成 |
| W2 | 新增 `platform/web` 目录与 host event 基线 | 已完成 |
| W3 | Web 显示 MVP（Canvas2D full present） | 已完成 |
| W4 | Web 输入 MVP（mouse/wheel/key） | 已完成 |
| W5 | Web runner 与浏览器主循环接线 | 已完成 |
| W6 | 资源打包、截图/录制、Makefile/构建脚本 | 已完成 |
| W7 | dirty present、resize、页面生命周期与 smoke | 已完成 |
| W8 | WebGL/worker/移动端兼容作为 backlog | 未开始 |

## 当前实现状态

- 已完成：
  - 新增 `src/gui/sim/runtime_core.uya`，统一 SDL2 / FB / Web 的帧循环核心。
  - 新增 `src/gui/platform/web/` 目录与 `disp_web.uya` / `indev_web.uya` / `web_common.uya` / `web_host.c`。
  - 新增 `src/gui/platform/web/shell.html`，由 host 组装 `Module.arguments`、fit-window、下载导出与 `IDBFS` 挂载。
  - 新增 `src/gui/sim/runner_web.uya` 与 `apps/sim_web_main.uya`。
  - `SimConfig` 已支持 `backend=web` 与 `persist_data` 占位开关。
  - 资源根探针已改为 `.uya_sim_root_probe`。
  - Web 默认路径已切到 `/app`、`/tmp/last_frame.png`、`/tmp/last_input.uyarec`。
  - Make 目标与脚本已补齐：`sim-web-build` / `sim-web-run` / `sim-web-serve` / `sim-web-smoke`。
  - Web dirty present、overlay、resize/visibility refresh 与 screenshot host 导出已补齐。
  - Web 单测已补入 `tests/test_web_backend.uya` / `tests/test_web_config.uya` / `tests/test_web_present_plan.uya`。
- 已新增验证脚本：
  - `tools/smoke_gui_web.sh`
  - `tools/smoke_gui_web.py`
- 已验证：
  - `make test` 通过，其中 `tests/web_present_plan_suite.uya` 的 `4` 个测试通过。
  - `make sim-web-build` 可真实产出 `build/web/{index.html,index.js,index.wasm,index.data}`。
  - `make sim-web-smoke` 可无头打开页面，跑完 `--max-frames 3` 并校验 `/tmp/last_frame.png`。
- 当前阻塞：
  - 无硬阻塞；剩余主要是更强的交互式验收与视觉回归确认。

---

## W0: 方案收口与目录基线

### 目标

把“Web 后端需要哪些文件、哪些接口必须新增、哪些逻辑必须抽公共层”收口，不让后续实现边做边改结构。

### TODO

- [x] 确认最终目录布局
  - [x] `apps/sim_web_main.uya`
  - [x] `src/gui/platform/web/disp_web.uya`
  - [x] `src/gui/platform/web/indev_web.uya`
  - [x] `src/gui/platform/web/web_common.uya`
  - [x] `src/gui/platform/web/web_host.c`
  - [x] `src/gui/sim/runtime_core.uya`
  - [x] `src/gui/sim/runner_web.uya`
  - [x] `tools/build_gui_web.sh`
- [x] 明确不新增平行 `platform/interface/*` 抽象层
- [x] 确认 Web 入口改为 `apps/sim_web_main.uya`
- [x] 定稿启动 ownership
  - [x] `main()` / bootstrap 只执行一次
  - [x] host 不负责反向调用 boot
  - [x] host 只负责 frame schedule 与单次 shutdown
- [x] 确认浏览器配置仍以 `argv` 为主，host 侧组装 `Module.arguments`
- [x] 确认默认资源根目录策略
  - [x] backend=`web` 时默认 `resource_root=/app`
- [x] 确认资源根探针不再依赖 `Makefile`
  - [x] 改为 sentinel 文件 `.uya_sim_root_probe`
- [x] 确认 Web 默认路径
  - [x] screenshot=`/tmp/last_frame.png`
  - [x] record=`/tmp/last_input.uyarec`
  - [x] playback=`/tmp/last_input.uyarec`

### 验收

- [x] 所有新增文件路径定稿
- [x] `SimConfig` 增量字段列表定稿
- [x] 共享 runtime core 是必须项，不再作为可选优化项
- [x] Web 构建与桌面构建的入口/host glue 隔离方案定稿

---

## W1: 抽共享 `SimRuntimeCore`

### 目标

把当前 `SDL2` / `Framebuffer` backend 共用的 update/render/profiler/dirty logic 提取出来，为 Web callback 模式铺路。

### TODO

- [x] 新增 `src/gui/sim/runtime_core.uya`
- [x] 抽离 `runner.uya` 中 backend 无关逻辑
  - [x] app init/finish
  - [x] retained seed frame
  - [x] input drain
  - [x] update
  - [x] render
  - [x] profiler record
  - [x] max-frames/should-exit
- [x] 设计 `SimFrameOutput` 或等价结果结构
- [x] 让 `run_simulator_sdl()` 先切到新 core
- [x] 再让 `run_simulator_fb()` 切到新 core
- [x] 约束 `runtime_core.uya` 不依赖任何 `platform/{sdl2,fb,web}/*`
- [x] 保证现有 `sim-run` / `sim-headless` / `sim-fb-run` 行为不回归

### 验收

- [x] `SDL2` 和 `FB` 不再各自持有大段重复 frame logic
- [x] `run_simulator_sdl()` / `run_simulator_fb()` 变成“初始化 + present + loop 驱动”的薄封装
- [x] 现有 sim 相关测试和 smoke 仍通过

---

## W2: `platform/web` 目录与 host event 基线

### 目标

建立与 SDL/FB 对齐的 Web host event 与输入公共数据结构。

### TODO

- [x] 创建 `src/gui/platform/web/`
- [x] 新增 `src/gui/platform/web/web_common.uya`
  - [x] 定义 `WEB_EVT_*` 常量
  - [x] 定义 `WebHostEvent`
  - [x] 定义 `web_host_event_none()`
  - [x] 定义 `web_feed_host_event(...)`
  - [x] 定义 `web_hover_point_default()`
- [x] 设计 host 事件环形缓冲区容量
- [x] 约定键值映射
  - [x] `Esc`
  - [x] `Enter`
  - [x] `Space`
  - [x] 方向键
  - [x] `F11`
- [x] 明确 blur/visibility/pointercancel/touchcancel 的清状态规则

### 验收

- [x] Web host 事件结构不依赖浏览器具体 API 类型
- [x] Uya 侧已经能独立编译和单测输入事件映射逻辑

---

## W3: Web 显示 MVP

### 目标

先把 framebuffer 显示到 `canvas` 上，优先 full present，不一开始追求所有优化。

### TODO

- [x] 新增 `src/gui/platform/web/disp_web.uya`
- [x] 仿照 `disp_sdl.uya` 实现双缓冲
  - [x] `front_mem`
  - [x] `back_mem`
  - [x] `DisplayCtx`
- [x] `init()`
  - [x] 分配 ARGB8888 双缓冲
  - [x] 打开/绑定 HTML canvas
- [x] `present()`
  - [x] swap buffers
  - [x] 调用 host 全帧上传
- [x] `refresh_front()`
  - [x] 不交换缓冲，仅重传 front
- [x] `consume_refresh_request()`
  - [x] resize/restore/context invalidation 后可触发
- [x] `set_title()`
  - [x] 映射到 `document.title`
- [x] `toggle_fullscreen()`
  - [x] 请求 Fullscreen API
- [x] `set_dirty_overlay()`
  - [x] 接口占位，首版可先不画 overlay

### 验收

- [x] 浏览器中可见首帧画面
- [x] framebuffer 逻辑尺寸与 canvas 显示尺寸可以分离
- [x] 全屏请求失败时不会导致 sim 崩溃

---

## W4: Web 输入 MVP

### 目标

打通最小可用交互：点击、拖动、滚轮、热键。

### TODO

- [x] 新增 `src/gui/platform/web/indev_web.uya`
- [x] host 侧注册浏览器事件
  - [x] mouse down/up/move
  - [x] touch start/end/move/cancel
  - [x] wheel
  - [x] keydown/keyup
  - [x] blur / visibilitychange
- [x] `WebInputSystem.init()`
  - [x] 绑定 `TouchDriver` / `KeyDriver` / `EncoderDriver`
- [x] `pump(timestamp)`
  - [x] 从 host 队列拉取 `WebHostEvent`
- [x] `poll()`
  - [x] 转发到现有 `InputManager`
- [x] `poll_hover_coords()`
  - [x] 支持 hover 更新
- [x] 完成 CSS rect -> framebuffer 坐标映射
- [x] pointer down 后自动 focus canvas 容器
- [x] 补默认行为抑制策略
  - [x] 被 sim 消费的热键调用 `preventDefault()`
  - [x] `wheel` / `touchmove` 监听允许 `preventDefault()`
  - [x] canvas 容器设置 `tabindex=0`
  - [x] canvas 容器样式设置 `touch-action: none`
  - [x] canvas 容器样式设置 `overscroll-behavior: contain`

### 验收

- [ ] 鼠标点击可驱动至少一个 demo 交互
- [ ] 滚轮可驱动 slider/encoder
- [ ] 方向键和 `Esc` 生效
- [ ] 页面切后台再回来不出现 stuck input

---

## W5: Web runner 与浏览器主循环

### 目标

用浏览器 callback 方式驱动 shared runtime，不再依赖同步 `while true`。

### TODO

- [x] 新增 `src/gui/sim/runner_web.uya`
- [x] 新增 `apps/sim_web_main.uya`
  - [x] `main()` 只负责调用 `run_simulator_web_bootstrap()`
- [x] 设计并实现导出 callback
  - [x] `run_simulator_web_bootstrap()`
  - [x] `sim_web_frame(now_ms)`
  - [x] `sim_web_shutdown()`
- [x] 新增 `src/gui/platform/web/web_host.c`
  - [x] 提供 `uya_gui_web_host_start_loop()`
  - [x] 注册 `requestAnimationFrame` 主循环
  - [x] 每帧调用 `sim_web_frame(now_ms)`
  - [x] 收到停止信号后取消主循环
  - [x] `sim_web_shutdown()` 只调用一次
- [x] 在 `src/gui/sim/config.uya` 中新增 `backend=web`
- [x] 保持 `src/gui/sim/runner.uya` 为桌面 runner
- [x] `runner_web.uya` 不允许导入 `runner.uya`
- [x] 兼容 `--max-frames`
- [x] 兼容 screenshot request / finish path

### 验收

- [x] `--backend web` 能进入并退出主循环
- [x] `max-frames=3` 的 smoke 可自动完成
- [x] `SDL2` / `FB` 分支行为不回归

---

## W6: 资源、打包与开发工作流

### 目标

让浏览器构建可以真正加载资源、导出截图，并进入日常开发流程。

### TODO

- [x] 新增 `tools/build_gui_web.sh`
  - [x] `APP=apps/sim_web_main.uya`
  - [x] `uya build --c99 --no-split-c`
  - [x] `emcc` 编译 generated C
  - [x] `emcc` 编译 `web_host.c`
  - [x] 若存在 `imports.sh` sidecar，改用 `emcc` 编译 sidecar 对象
  - [x] 资源预打包到 MEMFS
  - [x] 打包 `.uya_sim_root_probe` 到 `/app`
  - [x] 初始化 `/tmp`
  - [x] 不编译 `sdl_host.c`
  - [x] 不编译 `fb_host.c`
  - [x] 输出到 `build/web/`
- [x] 新增 Make 目标
  - [x] `sim-web-build`
  - [x] `sim-web-run`
  - [x] `sim-web-serve`
  - [x] `sim-web-smoke`
- [x] 资源根目录方案
  - [x] 约定 `/app`
  - [x] backend=`web` 时默认 root 回退到 `/app`
- [x] 资源探针方案
  - [x] `probe_resource_root()` 改读 `.uya_sim_root_probe`
- [x] 截图方案
  - [x] 空路径时使用 `/tmp/last_frame.png`
  - [x] 先写入 MEMFS
  - [x] 再由 host 触发下载或保留给 smoke 读取
- [x] 录制/回放默认路径方案
  - [x] record 默认 `/tmp/last_input.uyarec`
  - [x] playback 默认 `/tmp/last_input.uyarec`
- [x] `--persist-data` 方案
  - [x] 首版可先占位
  - [x] 若开启则挂 `IDBFS`

### 验收

- [x] 一条命令可以产出 `wasm/js/html`
- [x] 本地静态服务可正常打开页面
- [x] demo 所需资源在浏览器中可读
- [x] 截图链路可用

---

## W7: dirty present、resize、页面生命周期与 smoke

### 目标

从“能显示”提升到“能长期使用与回归验证”。

### TODO

- [x] `present_dirty()`
  - [x] dirty rect merge
  - [x] full/dirty 退化阈值
  - [x] 局部 swizzle 上传
- [x] dirty overlay 调试显示
- [x] resize/focus/visibility 生命周期处理
  - [x] 页面隐藏后 refresh request
  - [x] resize 后 refresh request
  - [x] canvas CSS fit-window
- [x] 单元测试
  - [x] `test_web_input_mapping.uya`
  - [x] `test_web_present_plan.uya`
  - [x] `test_web_config.uya`
- [x] 浏览器 smoke
  - [x] 启动静态 server
  - [x] 无头浏览器打开页面
  - [x] `--max-frames 3`
  - [x] 校验截图或 completion 标志

### 验收

- [ ] 页面缩放后点击位置仍正确
- [ ] dirty render 不会出现脏区残影/黑边
- [x] headless browser smoke 可自动跑通

---

## W8: Backlog 与性能增强

### 目标

把 Web backend 从“可用”逐步提升到“高性能/高体验”，但这些都不阻塞主线。

### Backlog

- [ ] `gpu_web.uya`
  - [ ] WebGL2 batch
  - [ ] 图像/字形直传 GPU
- [ ] OffscreenCanvas
- [ ] worker / pthread 模式
- [ ] `IDBFS` 自动同步策略
- [ ] 移动端浏览器专项适配
- [ ] Inspector / stats 面板
- [ ] 浏览器下载 API 与录制文件管理
- [ ] 更细粒度的 fps throttle / idle scheduling

### 验收

- [ ] backlog 项全部独立，不阻塞 W1-W7 合并

---

## 文件落点清单

- [x] `src/gui/platform/web/disp_web.uya`
- [x] `src/gui/platform/web/indev_web.uya`
- [x] `src/gui/platform/web/web_common.uya`
- [x] `src/gui/platform/web/web_host.c`
- [x] `apps/sim_web_main.uya`
- [x] `src/gui/sim/runtime_core.uya`
- [x] `src/gui/sim/runner_web.uya`
- [x] `tools/build_gui_web.sh`
- [x] `tools/serve_gui_web.sh`

## 下一步建议

1. 安装 Emscripten，先跑通 `sim-web-build` 的真实 `wasm/js/html` 产物。
2. 补浏览器 smoke，把 `--max-frames 3` 和 completion/screenshot 校验自动化。
3. 实现 Web `present_dirty()`，避免每帧全量 ARGB->RGBA swizzle 上传。
4. 把截图导出从“写入 MEMFS”补全到“host 下载或 smoke 读取”。

## 最小验收路径

第一条真正的里程碑链路应该是：

1. `sim-web-build`
2. 浏览器打开页面
3. `--backend web --demo dashboard --max-frames 3`
4. 第 1 帧可见
5. 鼠标点击与滚轮生效
6. 自动退出
7. 截图产物可导出

只要这条链路打通，Web backend 就已经进入“可持续迭代”状态。
