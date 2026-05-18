# UyaGUI Web 后端 TODO 文档

> 版本: v0.1.0  
> 日期: 2026-05-18  
> 状态: 规划中（尚未开始实现）

> 说明: 本文是“原生 Web 后端”实施路线图，目标是在浏览器中运行当前 `sim`/demo，并保持与现有 `SDL2` / `Framebuffer` 后端一致的分层方式。本文重点记录当前基线、缺口、实施顺序和验收标准。

---

## 目标

- 新增 `platform/web/disp_web.uya` 与 `platform/web/indev_web.uya`。
- 新增 Web 专用入口 `gui/sim_web_main.uya`，让 `sim` 逻辑运行到浏览器 `canvas`。
- 不复制一套网页专用 GUI 逻辑，只补宿主显示/输入/主循环差异层。
- 让 `SDL2` / `Framebuffer` / `Web` 三端尽量共享同一套 `SimRuntimeCore`。

## 非目标

- 当前阶段不做 DOM/CSS 渲染。
- 当前阶段不做 WebGPU。
- 当前阶段不把 SDL-to-WASM 当长期方案。
- 当前阶段不承诺移动端浏览器、IME、复杂文本输入、worker 多线程。

## 当前基线

| 条目 | 状态 | 说明 |
|------|------|------|
| 显示/帧缓冲抽象 | [x] 已有基线 | `gui/platform/disp.uya` 已提供 `FrameBuffer` / `DisplayCtx` / 像素格式 |
| 输入抽象与事件队列 | [x] 已有基线 | `gui/platform/indev.uya` 已有 `TouchDriver` / `KeyDriver` / `EncoderDriver` |
| 渲染上下文 | [x] 已有基线 | `gui/render/ctx.uya` 已能在 framebuffer 上渲染 |
| retained / dirty render | [x] 已有基线 | `gui/render/scheduler.uya` 与 sim retained 流程已可复用 |
| SDL2 显示后端 | [x] 已实现 | `gui/platform/sdl2/{disp_sdl.uya,indev_sdl.uya,sdl_host.c}` |
| Framebuffer 后端 | [x] 已实现 | `gui/platform/fb/{disp_fb.uya,indev_fb.uya,fb_host.c}` |
| 模拟器配置 | [x] 已实现 | `gui/sim/config.uya` 已支持 backend/demo/size/fps/screenshot 等参数 |
| 模拟器主循环 | [x] 已实现 | `gui/sim/runner.uya` 已支持 `SDL2` 与 `FB` |
| 浏览器后端 | [ ] 未实现 | 当前没有 `gui/platform/web/` |
| 共享 runtime core | [ ] 未实现 | 当前 `SDL2` / `FB` 主循环仍有较多重复逻辑 |
| Web 构建脚本 | [ ] 未实现 | 当前只有 `tools/build_gui_sim.sh` |

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
| W0 | 收口设计、整理文件布局、确定配置策略 | 进行中 |
| W1 | 抽共享 `SimRuntimeCore` | 未开始 |
| W2 | 新增 `platform/web` 目录与 host event 基线 | 未开始 |
| W3 | Web 显示 MVP（Canvas2D full present） | 未开始 |
| W4 | Web 输入 MVP（mouse/wheel/key） | 未开始 |
| W5 | Web runner 与浏览器主循环接线 | 未开始 |
| W6 | 资源打包、截图/录制、Makefile/构建脚本 | 未开始 |
| W7 | dirty present、resize、页面生命周期与 smoke | 未开始 |
| W8 | WebGL/worker/移动端兼容作为 backlog | 未开始 |

---

## W0: 方案收口与目录基线

### 目标

把“Web 后端需要哪些文件、哪些接口必须新增、哪些逻辑必须抽公共层”收口，不让后续实现边做边改结构。

### TODO

- [ ] 确认最终目录布局
  - [ ] `gui/sim_web_main.uya`
  - [ ] `gui/platform/web/disp_web.uya`
  - [ ] `gui/platform/web/indev_web.uya`
  - [ ] `gui/platform/web/web_common.uya`
  - [ ] `gui/platform/web/web_host.c`
  - [ ] `gui/sim/runtime_core.uya`
  - [ ] `gui/sim/runner_web.uya`
  - [ ] `tools/build_gui_web.sh`
- [ ] 明确不新增平行 `platform/interface/*` 抽象层
- [ ] 确认 Web 入口改为 `gui/sim_web_main.uya`
- [ ] 定稿启动 ownership
  - [ ] `main()` / bootstrap 只执行一次
  - [ ] host 不负责反向调用 boot
  - [ ] host 只负责 frame schedule 与单次 shutdown
- [ ] 确认浏览器配置仍以 `argv` 为主，host 侧组装 `Module.arguments`
- [ ] 确认默认资源根目录策略
  - [ ] backend=`web` 时默认 `resource_root=/app`
- [ ] 确认资源根探针不再依赖 `Makefile`
  - [ ] 改为 sentinel 文件 `.uya_sim_root_probe`
- [ ] 确认 Web 默认路径
  - [ ] screenshot=`/tmp/last_frame.png`
  - [ ] record=`/tmp/last_input.uyarec`
  - [ ] playback=`/tmp/last_input.uyarec`

### 验收

- [ ] 所有新增文件路径定稿
- [ ] `SimConfig` 增量字段列表定稿
- [ ] 共享 runtime core 是必须项，不再作为可选优化项
- [ ] Web 构建与桌面构建的入口/host glue 隔离方案定稿

---

## W1: 抽共享 `SimRuntimeCore`

### 目标

把当前 `SDL2` / `Framebuffer` backend 共用的 update/render/profiler/dirty logic 提取出来，为 Web callback 模式铺路。

### TODO

- [ ] 新增 `gui/sim/runtime_core.uya`
- [ ] 抽离 `runner.uya` 中 backend 无关逻辑
  - [ ] app init/finish
  - [ ] retained seed frame
  - [ ] input drain
  - [ ] update
  - [ ] render
  - [ ] profiler record
  - [ ] max-frames/should-exit
- [ ] 设计 `SimFrameOutput` 或等价结果结构
- [ ] 让 `run_simulator_sdl()` 先切到新 core
- [ ] 再让 `run_simulator_fb()` 切到新 core
- [ ] 约束 `runtime_core.uya` 不依赖任何 `platform/{sdl2,fb,web}/*`
- [ ] 保证现有 `sim-run` / `sim-headless` / `sim-fb-run` 行为不回归

### 验收

- [ ] `SDL2` 和 `FB` 不再各自持有大段重复 frame logic
- [ ] `run_simulator_sdl()` / `run_simulator_fb()` 变成“初始化 + present + loop 驱动”的薄封装
- [ ] 现有 sim 相关测试和 smoke 仍通过

---

## W2: `platform/web` 目录与 host event 基线

### 目标

建立与 SDL/FB 对齐的 Web host event 与输入公共数据结构。

### TODO

- [ ] 创建 `gui/platform/web/`
- [ ] 新增 `gui/platform/web/web_common.uya`
  - [ ] 定义 `WEB_EVT_*` 常量
  - [ ] 定义 `WebHostEvent`
  - [ ] 定义 `web_host_event_none()`
  - [ ] 定义 `web_feed_host_event(...)`
  - [ ] 定义 `web_hover_point_default()`
- [ ] 设计 host 事件环形缓冲区容量
- [ ] 约定键值映射
  - [ ] `Esc`
  - [ ] `Enter`
  - [ ] `Space`
  - [ ] 方向键
  - [ ] `F11`
- [ ] 明确 blur/visibility/pointercancel/touchcancel 的清状态规则

### 验收

- [ ] Web host 事件结构不依赖浏览器具体 API 类型
- [ ] Uya 侧已经能独立编译和单测输入事件映射逻辑

---

## W3: Web 显示 MVP

### 目标

先把 framebuffer 显示到 `canvas` 上，优先 full present，不一开始追求所有优化。

### TODO

- [ ] 新增 `gui/platform/web/disp_web.uya`
- [ ] 仿照 `disp_sdl.uya` 实现双缓冲
  - [ ] `front_mem`
  - [ ] `back_mem`
  - [ ] `DisplayCtx`
- [ ] `init()`
  - [ ] 分配 ARGB8888 双缓冲
  - [ ] 打开/绑定 HTML canvas
- [ ] `present()`
  - [ ] swap buffers
  - [ ] 调用 host 全帧上传
- [ ] `refresh_front()`
  - [ ] 不交换缓冲，仅重传 front
- [ ] `consume_refresh_request()`
  - [ ] resize/restore/context invalidation 后可触发
- [ ] `set_title()`
  - [ ] 映射到 `document.title`
- [ ] `toggle_fullscreen()`
  - [ ] 请求 Fullscreen API
- [ ] `set_dirty_overlay()`
  - [ ] 接口占位，首版可先不画 overlay

### 验收

- [ ] 浏览器中可见首帧画面
- [ ] framebuffer 逻辑尺寸与 canvas 显示尺寸可以分离
- [ ] 全屏请求失败时不会导致 sim 崩溃

---

## W4: Web 输入 MVP

### 目标

打通最小可用交互：点击、拖动、滚轮、热键。

### TODO

- [ ] 新增 `gui/platform/web/indev_web.uya`
- [ ] host 侧注册浏览器事件
  - [ ] mouse down/up/move
  - [ ] touch start/end/move/cancel
  - [ ] wheel
  - [ ] keydown/keyup
  - [ ] blur / visibilitychange
- [ ] `WebInputSystem.init()`
  - [ ] 绑定 `TouchDriver` / `KeyDriver` / `EncoderDriver`
- [ ] `pump(timestamp)`
  - [ ] 从 host 队列拉取 `WebHostEvent`
- [ ] `poll()`
  - [ ] 转发到现有 `InputManager`
- [ ] `poll_hover_coords()`
  - [ ] 支持 hover 更新
- [ ] 完成 CSS rect -> framebuffer 坐标映射
- [ ] pointer down 后自动 focus canvas 容器
- [ ] 补默认行为抑制策略
  - [ ] 被 sim 消费的热键调用 `preventDefault()`
  - [ ] `wheel` / `touchmove` 监听允许 `preventDefault()`
  - [ ] canvas 容器设置 `tabindex=0`
  - [ ] canvas 容器样式设置 `touch-action: none`
  - [ ] canvas 容器样式设置 `overscroll-behavior: contain`

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

- [ ] 新增 `gui/sim/runner_web.uya`
- [ ] 新增 `gui/sim_web_main.uya`
  - [ ] `main()` 只负责调用 `run_simulator_web_bootstrap()`
- [ ] 设计并实现导出 callback
  - [ ] `run_simulator_web_bootstrap()`
  - [ ] `sim_web_frame(now_ms)`
  - [ ] `sim_web_shutdown()`
- [ ] 新增 `gui/platform/web/web_host.c`
  - [ ] 提供 `uya_gui_web_host_start_loop()`
  - [ ] 注册 `requestAnimationFrame` 主循环
  - [ ] 每帧调用 `sim_web_frame(now_ms)`
  - [ ] 收到停止信号后取消主循环
  - [ ] `sim_web_shutdown()` 只调用一次
- [ ] 在 `gui/sim/config.uya` 中新增 `backend=web`
- [ ] 保持 `gui/sim/runner.uya` 为桌面 runner
- [ ] `runner_web.uya` 不允许导入 `runner.uya`
- [ ] 兼容 `--max-frames`
- [ ] 兼容 screenshot request / finish path

### 验收

- [ ] `--backend web` 能进入并退出主循环
- [ ] `max-frames=3` 的 smoke 可自动完成
- [ ] `SDL2` / `FB` 分支行为不回归

---

## W6: 资源、打包与开发工作流

### 目标

让浏览器构建可以真正加载资源、导出截图，并进入日常开发流程。

### TODO

- [ ] 新增 `tools/build_gui_web.sh`
  - [ ] `APP=gui/sim_web_main.uya`
  - [ ] `uya build --c99 --no-split-c`
  - [ ] `emcc` 编译 generated C
  - [ ] `emcc` 编译 `web_host.c`
  - [ ] 若存在 `imports.sh` sidecar，改用 `emcc` 编译 sidecar 对象
  - [ ] 资源预打包到 MEMFS
  - [ ] 打包 `.uya_sim_root_probe` 到 `/app`
  - [ ] 初始化 `/tmp`
  - [ ] 不编译 `sdl_host.c`
  - [ ] 不编译 `fb_host.c`
  - [ ] 输出到 `build/web/`
- [ ] 新增 Make 目标
  - [ ] `sim-web-build`
  - [ ] `sim-web-run`
  - [ ] `sim-web-serve`
  - [ ] `sim-web-smoke`
- [ ] 资源根目录方案
  - [ ] 约定 `/app`
  - [ ] backend=`web` 时默认 root 回退到 `/app`
- [ ] 资源探针方案
  - [ ] `probe_resource_root()` 改读 `.uya_sim_root_probe`
- [ ] 截图方案
  - [ ] 空路径时使用 `/tmp/last_frame.png`
  - [ ] 先写入 MEMFS
  - [ ] 再由 host 触发下载或保留给 smoke 读取
- [ ] 录制/回放默认路径方案
  - [ ] record 默认 `/tmp/last_input.uyarec`
  - [ ] playback 默认 `/tmp/last_input.uyarec`
- [ ] `--persist-data` 方案
  - [ ] 首版可先占位
  - [ ] 若开启则挂 `IDBFS`

### 验收

- [ ] 一条命令可以产出 `wasm/js/html`
- [ ] 本地静态服务可正常打开页面
- [ ] demo 所需资源在浏览器中可读
- [ ] 截图链路可用

---

## W7: dirty present、resize、页面生命周期与 smoke

### 目标

从“能显示”提升到“能长期使用与回归验证”。

### TODO

- [ ] `present_dirty()`
  - [ ] dirty rect merge
  - [ ] full/dirty 退化阈值
  - [ ] 局部 swizzle 上传
- [ ] dirty overlay 调试显示
- [ ] resize/focus/visibility 生命周期处理
  - [ ] 页面隐藏后 refresh request
  - [ ] resize 后 refresh request
  - [ ] canvas CSS fit-window
- [ ] 单元测试
  - [ ] `test_web_input_mapping.uya`
  - [ ] `test_web_present_plan.uya`
  - [ ] `test_web_config.uya`
- [ ] 浏览器 smoke
  - [ ] 启动静态 server
  - [ ] 无头浏览器打开页面
  - [ ] `--max-frames 3`
  - [ ] 校验截图或 completion 标志

### 验收

- [ ] 页面缩放后点击位置仍正确
- [ ] dirty render 不会出现脏区残影/黑边
- [ ] headless browser smoke 可自动跑通

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

- [ ] `gui/platform/web/disp_web.uya`
- [ ] `gui/platform/web/indev_web.uya`
- [ ] `gui/platform/web/web_common.uya`
- [ ] `gui/platform/web/web_host.c`
- [ ] `gui/sim_web_main.uya`
- [ ] `gui/sim/runtime_core.uya`
- [ ] `gui/sim/runner_web.uya`
- [ ] `tools/build_gui_web.sh`
- [ ] `tools/serve_gui_web.sh`

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
