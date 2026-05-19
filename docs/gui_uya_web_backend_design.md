# UyaGUI Web 后端详细设计

> 版本: v0.2.0  
> 日期: 2026-05-19  
> 状态: 设计已部分落地（以仓库当前实现为准）

> 说明: 本文描述“新增原生 Web 后端”的长期方案。`runtime_core`、`platform/web/*`、`runner_web`、Web 构建脚本已经在仓库中落地了一个 MVP；本文既记录目标结构，也记录当前实现与理想设计之间的差距。

## 0. 当前落地情况

- 已落地文件：
  - `gui/platform/web/disp_web.uya`
  - `gui/platform/web/indev_web.uya`
  - `gui/platform/web/web_common.uya`
  - `gui/platform/web/web_host.c`
  - `gui/sim/runtime_core.uya`
  - `gui/sim/runner_web.uya`
  - `gui/sim_web_main.uya`
  - `tools/build_gui_web.sh`
  - `tools/serve_gui_web.sh`
- 已落地策略：
  - Web backend 默认资源根为 `/app`
  - 资源探针改为 `.uya_sim_root_probe`
  - 默认截图/录制路径改为 `/tmp/last_frame.png` 与 `/tmp/last_input.uyarec`
  - 首版显示使用 `Canvas2D + full present`
  - 桌面 `runner.uya` 已改为复用 `SimRuntimeCore`
- 当前未闭环：
  - 本机未安装 `emcc`，因此未在当前环境产出最终 `wasm/js/html`
  - `present_dirty()` 在 Web 侧仍退化为 full present
  - 截图导出目前只保证写入 MEMFS，未补完 host 下载桥
  - 浏览器 smoke 与 headless 自动化尚未补齐

---

## 1. 目标

- 在浏览器中运行当前 `sim`/demo 逻辑，不单独维护一套网页版 GUI 逻辑。
- Web 构建使用独立入口 `gui/sim_web_main.uya`，但继续复用 `SimApp`、`render/*`、`widget/*`、`core/*`。
- 沿用当前 `platform/* + sim/* + render/*` 的职责边界，新增 `platform/web/disp_web.uya` 与 `platform/web/indev_web.uya`。
- 复用已有 `FrameBuffer`、`DirtyRegion`、`RenderCtx`、`SimApp`、`retained render`、截图/录制/profiler 逻辑。
- 先落地稳定的软件显示链路，再考虑 WebGL/WebGPU 加速。
- 尽量让 `SDL2` / `Framebuffer` / `Web` 三个后端共享同一套“每帧 update/render/present”核心逻辑，避免长期三份主循环分叉。

## 2. 非目标

- 不把 UyaGUI 直接改造成 DOM/CSS 渲染器。
- 不在第一阶段接入浏览器专用控件、IME、文本输入法、系统剪贴板等高级能力。
- 不在第一阶段直接实现 `gpu_web.uya`、WebGL batch、OffscreenCanvas worker、多线程。
- 不以“SDL 编到 Wasm”作为最终结构方案；SDL-to-WASM 只适合临时验证，不作为长期主线。

## 3. 当前基线

当前仓库已经具备的条件：

- `gui/platform/disp.uya`
  - 已有 `FrameBuffer`、`DisplayCtx`、像素格式、清屏/读写像素、双缓冲复制等基线。
- `gui/render/*`
  - 渲染核心已经面向 framebuffer，而不是直接面向 SDL 窗口。
- `gui/sim/app.uya`
  - demo 选择、交互、截图、profiler、录制与 retained 渲染入口已经具备。
- `gui/sim/config.uya`
  - 已有 `backend/gpu/cpu-render/demo/size/scale/fps/screenshot` 等运行参数。
- `gui/sim/runner.uya`
  - 已有 `SDL2` 与 `Framebuffer` 两套 host runner。
  - 目前主循环仍是宿主内同步 `while true` 模式。
- `gui/platform/sdl2/*`
  - 现有 `disp_sdl.uya + indev_sdl.uya + sdl_host.c` 提供了最直接的仿照对象。
- `gui/platform/fb/*`
  - 现有 `disp_fb.uya + indev_fb.uya + fb_host.c` 提供了“第二套后端如何复用 sim runtime”的参考。

结论：

- 当前代码并不缺“浏览器显示控件”，缺的是“浏览器宿主后端”。
- 真正要新增的是 host glue、输入队列、present 机制与浏览器主循环接线。

## 4. 为什么选择原生 Web 后端，而不是 SDL-to-WASM

### 4.1 SDL-to-WASM 的优点

- 上手快。
- 现有 SDL2 代码改动最少。
- 适合做临时可视化验证。

### 4.2 SDL-to-WASM 的问题

- 浏览器里真正的能力边界仍然是 `canvas`、DOM 事件、浏览器文件系统、用户手势与页面生命周期。
- SDL 适配层会隐藏掉很多 Web 特有问题，后续一旦要做资源缓存、下载、页面 resize、持久化、worker、WebGL 优化，仍然要回到浏览器原生模型。
- 继续沿用 SDL host glue，会让长期架构里多出一层“桌面语义映射到网页语义”的间接层。

### 4.3 原生 Web 后端的长期收益

- `disp_web.uya` / `indev_web.uya` 的职责与 SDL/FB 对齐，结构最顺。
- 浏览器特性可以按需暴露，不必绕 SDL。
- `Canvas2D -> WebGL -> OffscreenCanvas` 的升级路径更清晰。
- 可以把资源、下载、持久化、URL 参数、页面生命周期控制在同一层处理。

## 5. 总体架构

```text
desktop:
  gui/sim_main.uya
    -> sim.runner.run_simulator()
      -> sim.config
      -> sim.runtime_core
      -> sdl2 / fb runner

web:
  gui/sim_web_main.uya
    -> sim.runner_web.run_simulator_web_bootstrap()
      -> sim.config
      -> sim.runtime_core
      -> web runner
      -> host requestAnimationFrame schedule

platform/web
  -> disp_web.uya
  -> indev_web.uya
  -> web_common.uya
  -> web_host.c

browser
  -> HTML canvas
  -> keyboard/mouse/touch/wheel events
  -> requestAnimationFrame
  -> MEMFS / IDBFS
```

核心原则：

- `render/*` 不感知浏览器。
- `SimApp` 不感知浏览器。
- `platform/web/*` 只负责“显示/输入/页面宿主差异”。
- `sim/runtime_core.uya` 负责统一三种 backend 的逐帧行为。

## 6. 目录与文件设计

建议新增或调整如下文件：

```text
gui/
  sim_web_main.uya
  platform/
    web/
      disp_web.uya
      indev_web.uya
      web_common.uya
      web_host.c
  sim/
    runtime_core.uya
    runner_web.uya

tools/
  build_gui_web.sh
  serve_gui_web.sh           # 可选，本地开发静态服务器

docs/
  gui_uya_web_backend_design.md
  gui_uya_web_backend_todo.md
```

各文件职责：

- `gui/platform/web/disp_web.uya`
  - 仿照 `disp_sdl.uya`，维护 front/back framebuffer、present/full/dirty、标题、全屏、resize 状态。
- `gui/platform/web/indev_web.uya`
  - 仿照 `indev_sdl.uya`，从 host 原始事件队列读取并转发给 `TouchDriver` / `KeyDriver` / `EncoderDriver`。
- `gui/platform/web/web_common.uya`
  - 定义 `WebHostEvent`、事件 kind 常量、hover/drag 辅助逻辑。
- `gui/platform/web/web_host.c`
  - 浏览器宿主 glue。
  - 负责 Emscripten / canvas / DOM 事件 / `requestAnimationFrame` / 下载 / 持久化桥接。
- `gui/sim_web_main.uya`
  - Web 专用程序入口。
  - 只负责调用 `run_simulator_web_bootstrap()`，不引入 SDL2 / FB 依赖。
- `gui/sim/runtime_core.uya`
  - 把当前 `runner.uya` 里与 backend 无关的 shared frame flow 抽出来。
- `gui/sim/runner_web.uya`
  - Web backend 初始化、生命周期管理、导出给 `web_host.c` 的 frame callback。
  - 必须只依赖 `runtime_core.uya` 与 `platform/web/*`，不得 `use platform/sdl2/*` 或 `platform/fb/*`。
- `tools/build_gui_web.sh`
  - 负责 `uya -> C -> emcc -> wasm/js/html` 的构建流程。
- `tools/serve_gui_web.sh`
  - 可选，本地 dev server，便于调试静态资源、MIME、跨域头与 wasm 加载。

## 7. 配置设计

### 7.1 `SimBackendKind`

提案：

```uya
export enum SimBackendKind {
    Sdl2,
    FrameBuffer,
    Web,
}
```

### 7.2 `SimConfig`

不建议一开始就塞太多 Web 专有字段。首版只补浏览器运行真正必要的少量参数：

```uya
export struct SimConfig {
    ...
    backend: SimBackendKind,
    ...
    web_canvas_id: &const byte,
    web_auto_resize: bool,
    web_persist_data: bool,
}
```

建议默认值：

- `web_canvas_id = "uya-canvas"`
- `web_auto_resize = true`
- `web_persist_data = false`

### 7.3 参数来源

首版延续现有 `argv` 解析模型，不重新设计一套浏览器专用配置系统。

具体策略：

- 浏览器启动时由 host/boot JS 组装 `Module.arguments`
- `sim_config_from_runtime()` 继续从 `argc/argv` 读取
- 新增支持：
  - `--backend web`
  - `--canvas-id uya-canvas`
  - `--fit-window` / `--fixed-window`
  - `--persist-data` / `--no-persist-data`

这样可以保持 `SDL2` / `FB` / `Web` 三端的 demo 选择、尺寸、fps、截图参数一致。

## 8. 共享 runtime 核心改造

### 8.1 问题

当前 `runner.uya` 中：

- `run_simulator_sdl()` 与 `run_simulator_fb()` 各自维护一套主循环。
- update/render/present 逻辑大段重复。
- Web 不能直接复用同步 `while true`，因为浏览器需要 callback 驱动。

如果继续沿用当前模式，新增 `run_simulator_web()` 之后会形成第三套分叉。

### 8.2 目标结构

把 backend 无关逻辑抽成统一状态机：

```text
SimRuntimeCore
  - init(cfg, app)
  - seed_if_needed(back_fb, gpu)
  - drain_input(input)
  - update(now_ms, delta_ms)
  - render_frame(back_fb, gpu, batch)
  - build_present_plan()
  - record_profiler()
  - should_exit()
  - finish(front_fb)
```

### 8.3 建议的核心接口

```uya
export struct SimRuntimeCore {
    app: &SimApp,
    cfg: SimConfig,
    first_frame: bool,
    last_dirty_render: bool,
    pending_seed_present: bool,
    last_tick_ms: u32,
    loop_frame_count: u32,
}

export struct SimFrameOutput {
    retained_result: RenderFrameResult,
    dirty_render: bool,
    should_present_full: bool,
    should_present_dirty: bool,
    should_refresh_front: bool,
    frame_start_ms: u32,
    after_update_ms: u32,
    after_render_ms: u32,
}
```

原则：

- `runtime_core` 只依赖 `FrameBuffer`、`RenderCtx`、`DrawBatch`、`IGpuCtx`、输入系统抽象结果。
- backend runner 只负责：
  - 创建 display/input/gpu
  - 提供 back/front framebuffer
  - 调用 `present()` / `present_dirty()`
  - 接入宿主主循环

### 8.4 对三端的影响

- `SDL2`
  - 行为不变，只是主循环变薄。
- `Framebuffer`
  - 行为不变，只是主循环变薄。
- `Web`
  - 直接复用相同 runtime core。
  - 唯一差异只剩“回调驱动、页面事件、宿主 present”。

### 8.5 模块隔离约束

Web 方案必须显式解决“桌面后端被一并编进 Wasm”的问题。

约束如下：

- `gui/sim_main.uya`
  - 保持桌面入口，只服务 `SDL2` / `FB` 构建。
- `gui/sim_web_main.uya`
  - 作为 Web 唯一入口。
- `gui/sim/runner.uya`
  - 允许继续 `use platform/sdl2/*`、`platform/fb/*`。
- `gui/sim/runner_web.uya`
  - 不允许依赖 `runner.uya`。
  - 不允许依赖 `platform/sdl2/*`、`platform/fb/*`。
- `gui/sim/runtime_core.uya`
  - 必须保持 backend-agnostic。
  - 不允许依赖 `platform/web/*`、`platform/sdl2/*`、`platform/fb/*`。

这样做的目的不是“语义更漂亮”，而是保证 Web 构建时生成的 C 不会因为桌面 runner 的顶层导入而携带 SDL2 / FB extern 依赖。

## 9. 显示后端设计

## 9.1 `WebDisplay` 目标

`WebDisplay` 应尽量与 `SdlDisplay` 保持同型：

- 有 front/back framebuffer
- 有 `present()` / `present_dirty()`
- 有 `refresh_front()`
- 有 `consume_refresh_request()`
- 有 `set_title()` / `toggle_fullscreen()` / `set_dirty_overlay()`

提案：

```uya
export struct WebDisplay {
    host: &void,
    front_mem: &byte,
    back_mem: &byte,
    alloc_bytes: usize,
    scale: u8,
    fullscreen: bool,
    dirty_overlay_enabled: bool,
    auto_resize: bool,
    ctx: DisplayCtx,
}
```

### 9.2 像素格式

首版继续使用当前 sim 主链路的：

- `PixelFormat.ARGB8888`
- 双缓冲
- retained dirty region

原因：

- 现有 sim 路径已经围绕 `ARGB8888` 打通。
- `Framebuffer` 和截图链路已经适配。
- Web 侧只需解决 `ARGB8888 -> canvas RGBA` 的上传转换。

### 9.3 Canvas2D 软件上传路径

首版只做 Canvas 2D 软件显示，不引入 WebGL batch。

渲染路径：

1. Uya 在 back buffer 上渲染。
2. `present()` 或 `present_dirty()` 交换 front/back。
3. `web_host.c` 把 front buffer 中对应区域从 `ARGB8888` 转成 `RGBA` scratch buffer。
4. 通过浏览器 `canvas` 的 `putImageData` 写入。

这样可以完全复用现有 CPU 渲染器。

### 9.4 全帧与脏区 present

`present()`：

- 用于首帧、整帧重绘、或脏区覆盖率过大时。
- 一次性上传整张 framebuffer。

`present_dirty()`：

- 复用当前 `DirtyRegion`。
- 调用 `dirty.merge()` 后决定是否退化为 full present。
- 对每个 dirty rect 执行局部 swizzle + `putImageData`。

建议的退化条件：

- dirty rect 数量大于阈值，例如 `8`
- dirty 区域总面积大于屏幕面积一定比例，例如 `35%`
- 浏览器当前 canvas 发生 resize，需要整帧重种

### 9.5 缩放与 CSS 尺寸

需要区分两层尺寸：

- framebuffer 逻辑尺寸：`cfg.width x cfg.height`
- canvas CSS 显示尺寸：逻辑尺寸乘 `scale`，或 fit-window 自动计算

建议：

- `canvas.width/height` 始终等于 framebuffer 逻辑尺寸
- `canvas.style.width/height` 负责页面显示缩放
- 默认启用 `image-rendering: pixelated`，避免软件放大模糊

好处：

- framebuffer 不需要因 DPR 或容器变化而频繁重建
- 输入坐标映射更稳定

### 9.6 resize 行为

首版不做“浏览器窗口变化就重建 GUI framebuffer”。

首版只做：

- fit-window 时重新计算 CSS 缩放
- framebuffer 分辨率不变
- 输入坐标按最新 DOM rect 映射

后续再考虑：

- `--auto-resize-fb`
- 页面横竖屏切换时重建 framebuffer 并重新 layout

### 9.7 标题、全屏、刷新请求

- `set_title()`
  - 映射到 `document.title`
- `toggle_fullscreen()`
  - 请求 canvas 容器进入/退出 Fullscreen API
  - 浏览器要求用户手势；失败时返回 false，不视为致命错误
- `consume_refresh_request()`
  - 当页面从隐藏恢复、canvas 上下文刷新、或 resize 后需要重新 present 时置位

### 9.8 dirty overlay

保留与 SDL 相同的调试开关：

- host 在 `putImageData` 后额外画矩形 outline
- 仅在调试模式启用
- 不进入生产路径

## 10. 输入后端设计

## 10.1 目标

把浏览器原始输入标准化为与 SDL/FB 同级的 host 事件队列，再交给 `TouchDriver` / `KeyDriver` / `EncoderDriver`。

### 10.2 新增公共事件结构

提案：

```uya
export const WEB_EVT_NONE: u8 = 0u8;
export const WEB_EVT_QUIT: u8 = 1u8;
export const WEB_EVT_HOVER_MOVE: u8 = 2u8;
export const WEB_EVT_TOUCH_DOWN: u8 = 3u8;
export const WEB_EVT_TOUCH_UP: u8 = 4u8;
export const WEB_EVT_TOUCH_MOVE: u8 = 5u8;
export const WEB_EVT_KEY_DOWN: u8 = 6u8;
export const WEB_EVT_KEY_UP: u8 = 7u8;
export const WEB_EVT_WHEEL: u8 = 8u8;
export const WEB_EVT_VISIBILITY_HIDDEN: u8 = 9u8;
export const WEB_EVT_RESIZE: u8 = 10u8;

export struct WebHostEvent {
    kind: u8,
    x: i16,
    y: i16,
    value: i32,
    key_code: u16,
    reserved: u16,
}
```

`web_common.uya` 中提供：

- `web_host_event_none()`
- `web_feed_host_event(...)`
- `web_hover_point_default()`

结构上与 `fb/indev_fb_common.uya` 对齐。

### 10.3 输入来源

host 侧注册浏览器事件：

- 鼠标
  - down / up / move
- 触摸
  - start / end / move / cancel
- 滚轮
  - wheel
- 键盘
  - keydown / keyup
- 页面生命周期
  - blur
  - visibilitychange
- 容器变化
  - resize

这些事件统一写入 `WebHostEvent` 环形队列。

### 10.4 坐标映射

浏览器事件给出的通常是 CSS 像素坐标，GUI 需要 framebuffer 逻辑坐标。

映射规则：

```text
canvas css rect -> local css position -> normalize -> framebuffer position
```

具体做法：

- 读取 canvas `getBoundingClientRect()`
- 将事件位置减去 rect 左上角
- 按 `framebuffer_w / rect.width`、`framebuffer_h / rect.height` 缩放
- clamp 到 `[0, w-1]`、`[0, h-1]`

这能兼容：

- `scale != 1`
- fit-window
- DPR
- CSS 缩放

### 10.5 键盘策略

建议：

- pointer down 时尝试将 focus 置给 canvas 或包裹容器
- 键盘事件监听挂在 window/document
- 将常用按键转换为当前 sim 约定 key code

首版只保证：

- `Esc`
- `Enter`
- `Space`
- 方向键
- `F11` 等模拟器热键

### 10.5.1 默认浏览器行为抑制

如果不抑制浏览器默认行为，Web sim 很容易出现：

- 方向键/`Space` 触发页面滚动
- 滚轮优先滚页面而不是驱动 GUI encoder
- 触摸拖动触发浏览器滚动或回弹

因此 host 层需要明确以下规则：

- 绑定到 canvas 或其容器的 `wheel` / `touchmove` 监听必须允许 `preventDefault()`
  - 也就是不能全部使用 passive listener
- 被 sim 消费的热键需要 `preventDefault()`
  - 至少包括 `Space`、方向键、`Enter`、用于 demo/调试的热键
- canvas 容器需要可聚焦
  - 例如设置 `tabindex=0`
- canvas 容器默认样式需要抑制触摸手势滚动
  - 推荐 `touch-action: none`
  - 推荐 `overscroll-behavior: contain`
- 未被 sim 消费的浏览器级快捷键不应无条件吞掉
  - 避免破坏刷新、开发者工具等常用浏览器操作

### 10.6 页面失焦与隐藏

浏览器里最容易出现“按下后没收到抬起”的 stuck state。

因此在以下事件发生时必须清状态：

- `blur`
- `visibilitychange -> hidden`
- `pointercancel`
- `touchcancel`

清理策略：

- 清空 hover pending
- 清空 pointer_down
- 为当前按下中的 pointer/key 注入 synthetic up/release

## 11. 浏览器主循环设计

## 11.1 约束

Web 后端不能复用同步阻塞式 `while true`：

- 浏览器要求通过 `requestAnimationFrame` 或宿主回调驱动
- 页面后台时帧率会被浏览器节流
- `sleep_ms()` 不能作为帧节拍主机制

### 11.2 设计原则

- `Web runner` 负责注册主循环
- 每帧只做一次 `runtime_core.step(now_ms)`
- update/render/present 仍由 Uya runtime 控制
- 浏览器只负责调度与输入采集

### 11.3 推荐实现

Web 生命周期的 ownership 需要固定为：

- Uya 负责一次性初始化
- `web_host.c` 只负责调度 frame callback 和宿主事件桥接
- shutdown 只能执行一次

不建议让 host 同时负责 `boot + frame + shutdown`，否则很容易和 `main()` 的生命周期打架。

推荐启动顺序：

1. `gui/sim_web_main.uya.main()`
2. `run_simulator_web_bootstrap()`
3. 初始化 `SimConfig` / `SimApp` / `WebDisplay` / `WebInputSystem` / `SimRuntimeCore`
4. 调用 host 提供的 `uya_gui_web_host_start_loop()`
5. host 注册 `requestAnimationFrame`
6. host 在每一帧调用 Uya 导出的 `sim_web_frame(now_ms)`
7. 当 `sim_web_frame()` 返回停止信号后，host 取消主循环并调用 `sim_web_shutdown()` 一次

建议由 `web_host.c` 通过 Emscripten 主循环 API 驱动，调用 Uya 导出的 frame callback。

提案：

```uya
// gui/sim_web_main.uya
export fn main() i32

// gui/sim/runner_web.uya
export fn run_simulator_web_bootstrap() i32
export fn sim_web_frame(now_ms: f64) i32
export fn sim_web_shutdown() void
```

其中：

- `main()`
  - 只负责进入 Web bootstrap
- `run_simulator_web_bootstrap()`
  - 做一次性初始化
  - 注册 host 主循环
  - 不能再次被 host 反向调用
- `sim_web_frame(now_ms)`
  - 执行一帧
  - 返回 `0/1` 表示是否继续
- `sim_web_shutdown()`
  - 做 `finish()`、资源释放、可选 download/persist flush
  - 必须保证幂等

`web_host.c` 负责：

- `emscripten_set_main_loop_arg(...)`
- 在每次 rAF 调用 Uya frame callback
- 结束时取消主循环
- 绝不负责重复初始化 `SimApp`

### 11.4 对 `max_frames`、fps、idle 的处理

Web 中不再主动 `sleep_after_frame()` 控速，而是改为：

- 以浏览器回调帧率为上限
- 在 Uya 侧根据 `now_ms - last_tick_ms` 计算 delta
- `max_frames` 仍保留，用于自动退出 smoke
- `target_fps` 仍可用于“低于 rAF 的逻辑帧跳过策略”，但首版不强制实现

首版建议：

- `requestAnimationFrame` 每帧都调用
- Uya 侧保留 profiler 记录
- 真正的逻辑降帧作为后续优化项

## 12. 资源、文件系统与持久化设计

## 12.1 问题

当前 sim 允许：

- `--root PATH`
- 从 host fs 读取资源
- 把截图/录制结果写入文件

浏览器中不存在本地 POSIX 文件系统，必须改成虚拟 FS。

### 12.2 资源加载策略

首版建议：

- 构建时把 demo 所需资源预打包进 Emscripten MEMFS
- bootstrap 阶段显式确保存在：
  - `/app`
  - `/tmp`
  - `/data`（仅在启用持久化时挂载）
- Web backend 默认 `resource_root = "/app"`
- 若用户未显式传 `--root`，当 backend=`web` 时自动回退到 `"/app"`

这样可以保持现有资源 API 不变。

### 12.2.1 资源根探针修正

当前 `SimApp.probe_resource_root()` 会读取 `Makefile`，这隐含了“资源根就是仓库根目录”的桌面假设，不适合 Web。

因此 Web 方案必须同步修改探针策略：

- 不再把 `Makefile` 当成资源根有效性的默认探针
- 改为使用 backend-neutral sentinel 文件
  - 推荐文件名：`.uya_sim_root_probe`
- 桌面运行时：
  - sentinel 文件位于仓库根或构建时约定的 resource root
- Web 运行时：
  - `build_gui_web.sh` 必须把 `.uya_sim_root_probe` 打包到 `/app/.uya_sim_root_probe`

这样 `probe_resource_root()`、fs 状态标签和后续 smoke 都不再耦合桌面仓库结构。

### 12.3 持久化策略

默认：

- 只用 MEMFS
- 页面刷新后丢失

可选开启：

- `--persist-data`
- 将 `/data` 挂到 `IDBFS`
- 在关键时机触发同步

适合的对象：

- 输入录制文件
- 用户配置
- demo save data

不建议首版强制落地：

- screenshot 自动持久化
- 全量资源缓存

### 12.4 截图与下载

当前截图/录制逻辑可以继续写入虚拟路径，但必须先改掉桌面默认路径。

建议新增 backend-aware 默认路径解析：

- desktop screenshot 默认：`build/sim/last_frame.bmp`
- web screenshot 默认：`/tmp/last_frame.png`
- desktop record 默认：`build/sim/last_input.uyarec`
- web record 默认：`/tmp/last_input.uyarec`
- web playback 默认：`/tmp/last_input.uyarec`

Web 中建议的临时路径例如：

- `/tmp/last_frame.png`
- `/tmp/record.uyarec`

host 层再决定是否：

- 自动触发浏览器下载
- 暴露给 JS 回调
- 保留在 MEMFS 供测试读取

这样可以最大限度复用现有 `sim/screenshot.uya`。

## 13. 构建与部署设计

## 13.1 构建链路

推荐链路：

```text
uya build gui/sim_web_main.uya --c99 --no-split-c
  -> generated C
  -> emcc
  -> gui_uya_web.js + gui_uya_web.wasm + html shell
```

### 13.2 新脚本

建议新增：

```text
tools/build_gui_web.sh
```

职责：

- 调用 `uya build`
- 入口必须固定为 `APP=gui/sim_web_main.uya`
- 编译 `gui/platform/web/web_host.c`
- 若存在 `imports.sh` sidecar，必须继续编译并链接，但编译器从 `cc` 切换为 `emcc`
- 链接 wasm/js/html
- 打包资源到 MEMFS
- 绝不编译/链接：
  - `gui/platform/sdl2/sdl_host.c`
  - `gui/platform/fb/fb_host.c`
- 产出到 `build/web/`

建议产物：

```text
build/web/
  gui_uya_web.html
  gui_uya_web.js
  gui_uya_web.wasm
  gui_uya_web.data      # 若启用资源预打包
```

### 13.3 Makefile 目标

建议新增：

- `sim-web-build`
- `sim-web-run`
- `sim-web-serve`
- `sim-web-smoke`

其中：

- `sim-web-run`
  - 依赖 `sim-web-build`
  - 启动本地 HTTP 服务
- `sim-web-smoke`
  - 面向 CI 的浏览器无头 smoke

## 14. GPU 与后续升级路径

首版明确不做 `gpu_web.uya`，但设计上要留出口。

推荐升级顺序：

1. `Canvas2D` 软件 present 稳定
2. `present_dirty()` 性能调优
3. `gpu_web.uya` 接 WebGL2 batch
4. 可选 worker + OffscreenCanvas

注意：

- 一旦走 `WebGL + worker + OffscreenCanvas + pthreads`，部署将需要额外跨源隔离头。
- 这不适合首版阻塞主线。

## 15. 测试设计

### 15.1 单元/仓库内测试

建议新增：

- `gui/tests/test_web_input_mapping.uya`
  - 验证 CSS rect 到 framebuffer 坐标映射
- `gui/tests/test_web_present_plan.uya`
  - 验证 dirty/full 退化策略
- `gui/tests/test_web_config.uya`
  - 验证 `backend=web`、`root=/app`、web 参数解析

### 15.2 浏览器 smoke

建议新增一条无头浏览器 smoke：

- 构建 web 产物
- 启动静态 server
- 打开页面
- 传 `--demo dashboard --max-frames 3 --screenshot /tmp/smoke.png`
- 等待退出或 JS completion 标志
- 检查截图文件或 canvas 像素摘要

### 15.3 人工回归项

- 首页/phase demo 是否可见
- 鼠标点击/拖动/滚轮是否生效
- 键盘热键是否生效
- 页面缩放后坐标是否仍准确
- 页面切后台再切回前台是否黑屏或 stuck input

## 16. 风险与关键决策

### 16.1 风险

- 浏览器不支持阻塞式主循环，必须重构 runtime loop。
- 当前 `resource_root="."` 的桌面假设在浏览器里不成立。
- `ARGB8888 -> RGBA` swizzle 会引入额外 CPU 开销。
- Fullscreen、下载、持久化都带浏览器手势/权限限制。
- 如果过早引入 WebGL/worker，会显著增加调试与部署复杂度。

### 16.2 关键决策

- 必须先抽 `sim/runtime_core.uya`，否则 Web 会成为第三套主循环分叉。
- 必须使用独立 Web 入口 `gui/sim_web_main.uya`，否则 Web 构建很容易把桌面后端 extern 一并拖入。
- 首版显示只做 `Canvas2D` 软件路径。
- 资源默认打包进 MEMFS，`--persist-data` 再单独启用 `IDBFS`。
- 必须替换当前 `Makefile` 资源根探针，改用 backend-neutral sentinel 文件。
- 必须提供 Web 专用默认 screenshot/record 路径，不能继续沿用 `build/sim/*`。
- 浏览器首要目标是“桌面 Chrome/Firefox 开发调试可用”，不是一开始就覆盖所有移动端浏览器。

## 17. 开放问题

- `SimConfig` 的 web 专用参数是放进 `argv`，还是通过 HTML `data-*`/JS 初始化对象注入。
- `screenshot/record/playback` 在浏览器里是否保持“写文件再下载”语义，还是直接改成 host callback。
- `scale` 与 `fit-window` 是否同时保留，还是统一成“固定像素 + 自适应 CSS 缩放”。

## 18. 推荐实施顺序

1. 先抽 `sim/runtime_core.uya`
2. 再新增 `platform/web/{web_common,disp_web,indev_web}.uya`
3. 再新增 `runner_web.uya + web_host.c`
4. 再补 `build_gui_web.sh + Makefile` 目标
5. 最后补浏览器 smoke 与文档

这样能把最大结构风险前置，避免后面为 Web 再拆一遍 runner。
