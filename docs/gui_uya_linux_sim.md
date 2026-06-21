# UyaGUI Linux 模拟器方案

> 版本: v0.1.0  
> 日期: 2026-04-24  
> 目标: 在 Linux PC 上完整模拟 UyaGUI 运行环境，支持调试、性能分析和 UI 预览

> 说明: 本文是 Linux 模拟器方案文档，不定义 Uya 语言语法。现行 Uya 语法以 `uya/docs/grammar_formal.md`、`uya/docs/uya.md` 和 `uya/tests/` 为准；本文中的导入、宿主 FFI 和调度片段主要用于表达方案结构。

> 2026-04-26 实现更新：仓库里已经落下首版 SDL2 MVP。真实入口为 `apps/sim_main.uya`（为了让 Uya 模块根保持在 `gui/`），模拟器逻辑位于 `src/gui/sim/{runner,app,config}.uya`，SDL2 host glue 位于 `src/gui/platform/sdl2/sdl_host.c`。当前 CI 已通过 `SDL_VIDEODRIVER=dummy` 跑最小 smoke；同一天也在本地 Linux + SDL2 2.32.4 完成了实窗 smoke，`make sim-run SIM_ARGS="--max-frames 3 --screenshot build/sim/makerun.bmp"` 可成功退出并生成截图。

## 0. 当前落地状态

- 已有命令：`make sim-build`、`make sim-run`、`make sim-debug`、`make sim-fb-run`
- 已有 headless 命令：`make sim-headless`
- 已有 SDL2 后端：`src/gui/platform/sdl2/{disp_sdl.uya, indev_sdl.uya, sdl_host.c}`
- 已有 SDL2 OpenGL ES 2.0 绘制/显示后端：支持 `--gpu auto|software|gles2`
- 已有 CPU 渲染后端选择：支持 `--cpu-render immediate|batch`
- 已有调试工具：`src/gui/sim/{screenshot,profiler,recorder}.uya`
- 已有无 SDL2 单测：`tests/test_sim_app.uya`、`tests/test_sim_tools.uya`
- 已有 CI smoke：`.github/workflows/gui-phase0.yml` 会安装 `libsdl2-dev`，并在 dummy video 下跑 `make sim-headless SIM_HEADLESS_ARGS="--max-frames 2 --screenshot build/sim/ci.bmp"`
- 已有本地实机 smoke：
  - `./build/sim/gui_uya_sim --max-frames 5 --screenshot build/sim/manual.bmp`
  - `./build/sim/gui_uya_sim --gpu gles2 --max-frames 5`
  - `make sim-run SIM_ARGS="--max-frames 3 --screenshot build/sim/makerun.bmp"`

## 0.1 构建与运行

```bash
# Debian / Ubuntu
sudo apt-get update
sudo apt-get install -y libsdl2-dev

# 构建
make sim-build

# 默认运行（phase4 场景）
make sim-run

# 强制使用 OpenGL ES 2.0 绘制/显示后端
make sim-run SIM_ARGS="--gpu gles2 --max-frames 120"

# 切换 CPU 渲染后端
make sim-run SIM_ARGS="--cpu-render batch --demo dashboard --max-frames 120"

# 显式启用 GLES2 batch 绘制
make sim-run SIM_ARGS="--gpu gles2 --cpu-render batch --demo dashboard --max-frames 120"

# 运行 phase6 并限定帧数，适合 CI / smoke
make sim-run SIM_ARGS="--demo phase6 --max-frames 120"

# 直接显示更复杂的单独 demo
make sim-run SIM_ARGS="--demo dashboard --scale 1"
make sim-run SIM_ARGS="--demo music --scale 1"
make sim-run SIM_ARGS="--demo novel --scale 1"

# 打开调试 HUD / profiler
make sim-debug

# 无窗口运行并导出截图
make sim-headless

# Linux framebuffer 运行（需要 /dev/fb0 权限）
make sim-fb-run
```

## 0.1.1 已验证输出

- `build/sim/manual.bmp`
- `build/sim/makerun.bmp`
- `build/sim/headless.bmp`
- `build/sim/ci.bmp`

## 0.2 运行参数

- `--demo phase4|phase6|clock|music|settings|dashboard|game|perf|novel`
- `--gpu auto|software|gles2`（默认 `auto`，`auto` 会在 GLES2 不可用时回退到 software）
- `--cpu-render immediate|batch`（默认 `immediate`，`batch` 会把基础几何命令汇总后再提交）
- `--width N --height N --scale N`（默认 `1920x1080`，`scale=1`）
- `--fullscreen | --windowed`
- `--root PATH`
- `--title TEXT`
- `--screenshot PATH`
- `--record PATH`
- `--playback PATH`
- `--max-frames N`
- `--profile-every N`
- `--hud | --no-hud`

常用切换热键：

- `1` / `6`：切到 `phase4` / `phase6`
- `C` / `M` / `S` / `D` / `G` / `B` / `N`：切到 `clock` / `music` / `settings` / `dashboard` / `game` / `perf` / `novel`
- `Space`：在 `novel` demo 里切换自动滚屏

### Headless 专用入口

- `make sim-headless`
- 可覆盖参数：`SIM_HEADLESS_ARGS="--max-frames 5 --screenshot build/sim/custom.bmp"`
- 实现方式：通过 `SDL_VIDEODRIVER=dummy` 复用 SDL2 主线，不额外分叉一套 headless runtime
- 说明：`--gpu auto` 在 dummy video / 无 GLES2 context 的环境下会自动回退到 software；`--gpu gles2` 会严格要求成功创建 GLES2 context
- 说明：当 `--gpu gles2 --cpu-render batch` 同时启用时，batch 内的矩形 / 描边矩形 / 线段 / 图像会直接走 GLES2 绘制；文本会提交 A8 字形覆盖图到 GLES2，复杂矢量描边字形仍保留 CPU fallback

### Framebuffer 专用入口

- `make sim-fb-run`
- 默认参数：`SIM_FB_ARGS="--backend fb --max-frames 60"`
- 可覆盖设备：
  - `SIM_FB_ARGS="--backend fb --fb-dev /dev/fb1 --max-frames 60"`
  - `SIM_FB_ARGS="--backend fb --fb-dev /dev/fb1 --fb-tty /dev/tty1 --max-frames 60"`
  - `SIM_FB_ARGS="--backend fb --fb-dev /dev/fb1 --fb-input /dev/input/event3 --fb-tty /dev/tty1 --max-frames 60"`
  - 或 `UYA_GUI_FB_DEV=/dev/fb1`
- 也可通过 `UYA_GUI_FB_TTY=/dev/tty1` 覆盖输入终端
- 也可通过 `UYA_GUI_FB_INPUT=/dev/input/event3` 指定 `evdev` 输入设备
- 当前 `fb` 输入已支持控制终端键盘热键与方向键，以及可选 `evdev` 指针/触摸/滚轮

## 0.3 当前交互

- 鼠标左键：通过 `TouchDriver` 进入现有事件系统
- 鼠标移动：驱动 hover 更新
- 鼠标滚轮：映射到 `EncoderDriver`，当前用于调节 slider
- 键盘：
  - `Esc` 退出
  - `P` 抓取截图（按扩展名导出 `PNG` / `BMP` / `.uyafb` 原始 dump）
  - `R` 开始/结束输入录制
  - `L` 读取并回放录制
  - `F11` 切换全屏
  - `1` / `6` 重新跑 `phase4_smoke` / `phase6_smoke`
- Framebuffer 控制终端：
  - 支持上述键盘热键
  - 支持方向键调节 slider
- Framebuffer `evdev` 设备：
  - 支持指针/触摸位置更新
  - 支持按下/抬起点击
  - 支持滚轮映射到 `EncoderDriver`

## 0.4 本次实机验证补到的兼容点

- `SdlInputSystem` 里的 `IInputDev` 接口值必须在 `init()` 内绑定到 `self.touch/self.key/self.encoder`，不能引用构造函数的局部变量，否则第一次 `poll()` 就会读到悬空事件队列
- SDL2 初始化期间不能误绑定到可执行里的 Uya `malloc/realloc/free/pthread_*` 符号
  - `tools/build_gui_sim.sh` 现在会带 `-fvisibility=hidden`
  - `src/gui/platform/sdl2/sdl_host.c` 会显式把 SDL2 内部内存分配切回宿主 libc allocator

## 0.5 当前已知现象

- `make sim-build` 仍会打印不少来自 Uya 生成 C 文件的 warning；当前不影响链接与运行
- 截图目前已支持 `PNG`、`BMP` 与原始 framebuffer dump（`.uyafb`）
- Framebuffer 专用后端已具备首版显示链路，并已支持控制终端键盘热键与方向键，以及可选 `evdev` 指针/触摸/滚轮；更完整校准与多点手势仍未实现
- 当前机器上 `/dev/fb0` 存在但普通用户无权限，`--backend fb` 会清晰返回 `Permission denied`
- `SDL2 + --gpu auto` 在无 OpenGL ES 2.0 context 的环境下会自动回退到 software，并继续完成 headless smoke / 截图链路
- 默认文本渲染已切到内置位图字体，不再显示统一占位方框
  - ASCII：`5x7`
  - 中文常用字：`U+4E00..U+9FFF` 的 `8x8` 内置点阵
  - 彩色与灰度 framebuffer 会对字形边缘做轻量 alpha 抗锯齿软化
  - 常见全角标点会归一化到 ASCII 标点
  - 单色 `I1` framebuffer 仍保持硬边渲染
  - 其余未覆盖 Unicode 字符仍会退化为 `?`

---

## 目录

1. [方案概述](#1-方案概述)
2. [模拟器架构](#2-模拟器架构)
3. [显示模拟](#3-显示模拟)
4. [输入模拟](#4-输入模拟)
5. [时钟与定时器](#5-时钟与定时器)
6. [文件系统模拟](#6-文件系统模拟)
7. [音频模拟（可选）](#7-音频模拟)
8. [模拟器核心代码](#8-模拟器核心代码)
9. [调试工具](#9-调试工具)
10. [构建与运行](#10-构建与运行)
11. [完整示例](#11-完整示例)

---

## 1. 方案概述

### 1.1 为什么需要模拟器

在嵌入式开发中，模拟器至关重要：

| 痛点 | 模拟器解决方案 |
|------|--------------|
| 硬件未就绪 | 在 PC 上提前开发和调试 |
| 调试困难 | GDB + 可视化调试工具 |
| 编译刷写慢 | PC 上秒级编译运行 |
| 性能分析难 | 内置 Profiler 和性能统计 |
| UI 迭代慢 | 实时热重载 |
| 团队协作 | 无需硬件即可并行开发 |

### 1.2 模拟器能力矩阵

| 功能 | SDL2 后端 | Framebuffer 后端 | X11 后端 |
|------|----------|------------------|----------|
| 显示输出 | ✅ 窗口 | ✅ 全屏/终端 | ✅ 窗口 |
| 鼠标输入 | ✅ | ✅ | ✅ |
| 键盘输入 | ✅ | ✅ | ✅ |
| 触摸屏模拟 | ✅ | ❌ | ❌ |
| GPU 加速 | ✅ OpenGL ES 2.0 / software 自动回退 | ❌ | ❌ |
| 跨平台 | ✅ Linux/Mac/Win | ❌ Linux only | ❌ Linux only |
| 依赖复杂度 | 中 | 低 | 低 |
| **推荐度** | **★★★★★** | ★★★ | ★★ |

### 1.3 推荐方案

**主方案: SDL2** — 功能完整、跨平台、支持 OpenGL ES 2.0 加速与 software 回退  
**备选方案: Linux Framebuffer** — 零依赖、最接近嵌入式环境  
**双模式: SDL2 + Framebuffer 切换** — 开发用 SDL2，验证用 FB

---

## 2. 模拟器架构

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                        你的 UyaGUI 应用                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │  Button  │  │  Label   │  │  Chart   │  │  Slider  │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
├─────────────────────────────────────────────────────────────┤
│                      UyaGUI 框架层                            │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐          │
│  │  Widget │ │  Layout │ │  Render │ │  Event  │          │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘          │
├─────────────────────────────────────────────────────────────┤
│                    平台抽象层 (Platform Abstraction)           │
│  ┌────────────┐ ┌────────────┐ ┌────────────┐              │
│  │   Display  │ │   Input    │ │    Tick    │              │
│  │  (接口)    │ │  (接口)    │ │  (接口)    │              │
│  └─────┬──────┘ └─────┬──────┘ └─────┬──────┘              │
├────────┼──────────────┼──────────────┼──────────────────────┤
│        │              │              │                       │
│  ┌─────▼──────┐ ┌─────▼──────┐ ┌─────▼──────┐              │
│  │  SDL2 实现  │ │  SDL2 实现  │ │ SDL2/Clock │              │
│  │  disp_sdl  │ │  indev_sdl │ │  tick_sdl  │              │
│  └─────┬──────┘ └─────┬──────┘ └─────┬──────┘              │
│        │              │              │                       │
├────────┼──────────────┼──────────────┼──────────────────────┤
│        │              │              │                       │
│  ┌─────▼──────┐ ┌─────▼──────┐ ┌─────▼──────┐              │
│  │ SDL2 Video │ │ SDL2 Event │ │  Linux     │              │
│  │  (Window)  │ │ (Mouse/Key)│ │  gettimeofday                                    │
│  └────────────┘ └────────────┘ └────────────┘              │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 目录结构

```
gui/
├── platform/
│   ├── interface/           # 平台抽象接口
│   │   ├── disp_if.uya      # 显示接口
│   │   ├── indev_if.uya     # 输入接口
│   │   ├── tick_if.uya      # 时钟接口
│   │   └── fs_if.uya        # 文件接口
│   ├── sdl2/                # SDL2 后端实现
│   │   ├── disp_sdl.uya     # SDL2 显示
│   │   ├── indev_sdl.uya    # SDL2 输入
│   │   ├── tick_sdl.uya     # SDL2 时钟
│   │   └── sdl_common.uya   # SDL2 公共代码
│   ├── fb/                  # Framebuffer 后端
│   │   ├── disp_fb.uya      # FB 显示
│   │   ├── indev_fb.uya     # FB 输入（TTY 键盘 + 可选 evdev）
│   │   └── tick_linux.uya   # Linux 时钟
│   └── raw/                 # 裸机接口 (嵌入式用)
│       ├── disp_raw.uya
│       ├── indev_raw.uya
│       └── tick_raw.uya
└── sim/                     # 模拟器工具
    ├── profiler.uya         # 性能分析器
    ├── screenshot.uya       # 截图工具
    ├── recorder.uya         # 输入录制/回放
    └── inspector.uya        # UI 检查器
```

### 2.3 接口定义

```uya
//========================================
// platform/interface/disp_if.uya
// 显示驱动抽象接口 - 所有显示后端必须实现
//========================================

/// 显示驱动接口
interface IDisplay {
    /// 初始化显示
    fn init(self: &Self, width: u16, height: u16, format: PixelFormat) !void;

    /// 关闭显示
    fn deinit(self: &Self) void;

    /// 获取帧缓冲区（用于绘制）
    fn get_framebuffer(self: &Self) FrameBuffer;

    /// 刷新指定区域到屏幕
    fn flush_region(self: &Self, region: Rect) void;

    /// 全屏刷新
    fn flush(self: &Self) void;

    /// 进入全屏模式
    fn set_fullscreen(self: &Self, enabled: bool) void;

    /// 设置窗口标题
    fn set_title(self: &Self, title: *byte) void;

    /// 获取显示尺寸
    fn get_size(self: &Self) Size;

    /// 设置模拟像素密度（用于模拟小屏）
    fn set_scale(self: &Self, scale: u8) void;

    /// 是否支持 GPU 加速
    fn has_gpu(self: &Self) bool;

    /// 获取 GPU 上下文（如果有）
    fn get_gpu_ctx(self: &Self) &GpuCtx;
}

/// 帧缓冲描述符 - 跨平台通用
struct FrameBuffer {
    buf:        &u8,            // 像素数据
    w:          u16,
    h:          u16,
    stride:     u16,            // 每行字节数
    format:     PixelFormat,
}

/// 像素格式枚举
enum PixelFormat: u8 {
    RGB565,
    RGB888,
    ARGB8888,
    ARGB4444,
    L8,
    A8,
    I1,
    I4,
}
```

```uya
//========================================
// platform/interface/indev_if.uya
// 输入设备抽象接口
//========================================

/// 输入设备接口
interface IInputDev {
    /// 初始化输入设备
    fn init(self: &Self) !void;

    /// 关闭输入设备
    fn deinit(self: &Self) void;

    /// 读取输入数据（非阻塞，返回 true 表示有新数据）
    fn read(self: &Self, data: &InputData) bool;

    /// 获取设备类型
    fn dev_type(self: &Self) InputDevType;

    /// 设置校准参数（触摸屏用）
    fn set_calibration(self: &Self, cal: &CalibrationData) void;
}

/// 输入设备类型
enum InputDevType: u8 {
    Touch,
    Mouse,
    Keypad,
    Encoder,
    Button,
}

/// 输入数据联合体
union InputData {
    touch:  TouchData,
    mouse:  MouseData,
    key:    KeyData,
    encoder: EncoderData,
}

/// 触摸/鼠标数据
struct TouchData {
    x:          i16,
    y:          i16,
    pressure:   u16,
    is_pressed: bool,
}

struct MouseData {
    x:          i16,
    y:          i16,
    buttons:    u8,         // 位掩码: bit0=左键, bit1=右键, bit2=中键
    wheel:      i8,         // 滚轮增量
}

struct KeyData {
    code:       u16,        // 键码 (SDL/USB HID 编码)
    is_pressed: bool,
    is_repeat:  bool,
}

struct EncoderData {
    delta:      i16,        // 旋转增量
    button:     bool,       // 按下状态
}

/// 校准数据（触摸屏）
struct CalibrationData {
    x_min:  u16,
    x_max:  u16,
    y_min:  u16,
    y_max:  u16,
}
```

```uya
//========================================
// platform/interface/tick_if.uya
// 时钟接口
//========================================

/// 时钟接口
interface ITick {
    /// 获取当前 tick (毫秒)
    fn get_ms(self: &Self) u32;

    /// 获取当前 tick (微秒)
    fn get_us(self: &Self) u64;

    /// 延迟指定毫秒
    fn delay_ms(self: &Self, ms: u32) void;

    /// 延迟指定微秒
    fn delay_us(self: &Self, us: u32) void;

    /// 获取自启动以来的秒数
    fn uptime_s(self: &Self) u32;
}
```

---

## 3. 显示模拟

### 3.1 SDL2 显示后端

```uya
//========================================
// platform/sdl2/disp_sdl.uya
// SDL2 显示后端实现
//========================================

import sdl "SDL2/SDL.h";  // C 导入: #include <SDL.h>

/// SDL2 显示驱动
struct SdlDisplay: IDisplay {
    window:         &sdl.Window,
    renderer:       &sdl.Renderer,
    texture:        &sdl.Texture,
    surface:        &sdl.Surface,

    // 模拟器配置
    screen_w:       u16,
    screen_h:       u16,
    pixel_scale:    u8,
    format:         PixelFormat,

    // 帧缓冲
    fb_buf:         [u8: MAX_FB_SIZE],
    fb:             FrameBuffer,

    // GPU 上下文
    gpu_available:  bool,
    gpu_ctx:        &GpuCtx,
}

SdlDisplay {
    /// 创建 SDL2 显示驱动
    fn new() SdlDisplay {
        return SdlDisplay {
            window:         null,
            renderer:       null,
            texture:        null,
            surface:        null,
            screen_w:       480,
            screen_h:       320,
            pixel_scale:    2,          // 默认 2x 放大
            format:         PixelFormat.RGB565,
            fb_buf:         [],
            fb:             FrameBuffer { buf: null, w: 0, h: 0, stride: 0, format: PixelFormat.RGB565 },
            gpu_available:  false,
            gpu_ctx:        null,
        };
    }

    /// 初始化 SDL2 窗口和渲染器
    fn init(self: &SdlDisplay, width: u16, height: u16, format: PixelFormat) !void {
        self.screen_w = width;
        self.screen_h = height;
        self.format = format;

        // 初始化 SDL Video 子系统
        if sdl.Init(sdl.INIT_VIDEO) < 0 {
            return error.SdlInitFailed;
        }

        // 计算窗口大小
        const window_w: i32 = width as i32 * self.pixel_scale as i32;
        const window_h: i32 = height as i32 * self.pixel_scale as i32;

        // 创建窗口
        self.window = sdl.CreateWindow(
            "UyaGUI Simulator",
            sdl.WINDOWPOS_CENTERED,
            sdl.WINDOWPOS_CENTERED,
            window_w,
            window_h,
            sdl.WINDOW_SHOWN | sdl.WINDOW_RESIZABLE
        );

        if self.window == null {
            return error.WindowCreateFailed;
        }

        errdefer {
            sdl.DestroyWindow(self.window);
            sdl.Quit();
        }

        // 创建渲染器
        self.renderer = sdl.CreateRenderer(
            self.window, -1,
            sdl.RENDERER_ACCELERATED | sdl.RENDERER_PRESENTVSYNC
        );

        if self.renderer == null {
            // 回退到软件渲染
            self.renderer = sdl.CreateRenderer(self.window, -1, sdl.RENDERER_SOFTWARE);
            if self.renderer == null {
                return error.RendererCreateFailed;
            }
        }

        errdefer {
            sdl.DestroyRenderer(self.renderer);
        }

        // 获取 SDL 像素格式
        const sdl_format: u32 = uya_to_sdl_format(format);

        // 创建纹理（用于上传像素数据）
        self.texture = sdl.CreateTexture(
            self.renderer,
            sdl_format,
            sdl.TEXTUREACCESS_STREAMING,
            width as i32,
            height as i32
        );

        if self.texture == null {
            return error.TextureCreateFailed;
        }

        // 初始化帧缓冲
        const stride: u16 = width * bytes_per_pixel(format);
        const fb_size: u32 = stride as u32 * height as u32;

        if fb_size > MAX_FB_SIZE {
            return error.FrameBufferTooLarge;
        }

        self.fb = FrameBuffer {
            buf:    &self.fb_buf[0],
            w:      width,
            h:      height,
            stride: stride,
            format: format,
        };

        // 清空帧缓冲
        for 0..fb_size |i| {
            self.fb_buf[i] = 0;
        }

        // 设置纹理缩放模式（像素完美）
        sdl.SetTextureScaleMode(self.texture, sdl.SCALEMODE_NEAREST);

        // 设置渲染器缩放
        sdl.RenderSetLogicalSize(self.renderer, width as i32, height as i32);

        log_info("SDL2 Display initialized: ${width}x${height}, scale=${self.pixel_scale}");
    }

    /// 关闭并清理资源
    fn deinit(self: &SdlDisplay) void {
        if self.texture != null {
            sdl.DestroyTexture(self.texture);
            self.texture = null;
        }
        if self.renderer != null {
            sdl.DestroyRenderer(self.renderer);
            self.renderer = null;
        }
        if self.window != null {
            sdl.DestroyWindow(self.window);
            self.window = null;
        }
        sdl.Quit();

        log_info("SDL2 Display deinitialized");
    }

    /// 获取帧缓冲（供渲染引擎绘制）
    fn get_framebuffer(self: &SdlDisplay) FrameBuffer {
        return self.fb;
    }

    /// 刷新脏区域到屏幕
    fn flush_region(self: &SdlDisplay, region: Rect) void {
        if region.is_empty() {
            return;
        }

        // 更新纹理的指定区域
        const rect = sdl.Rect {
            x: region.x as i32,
            y: region.y as i32,
            w: region.w as i32,
            h: region.h as i32,
        };

        // 锁定纹理并复制像素数据
        var pixels: &void = null;
        var pitch: i32 = 0;

        if sdl.LockTexture(self.texture, &rect, &pixels, &pitch) == 0 {
            // 逐行复制像素数据
            const bpp: i32 = bytes_per_pixel(self.format) as i32;
            const src_stride: i32 = self.fb.stride as i32;

            for 0..region.h |row| {
                const src_row: &u8 = self.fb.buf 
                    + (region.y as i32 + row as i32) * src_stride 
                    + region.x as i32 * bpp;
                const dst_row: &u8 = pixels as &u8 + row as i32 * pitch;

                // 快速内存拷贝
                memcpy(dst_row, src_row, region.w as u32 * bpp as u32);
            }

            sdl.UnlockTexture(self.texture);
        }

        // 渲染到窗口
        sdl.RenderClear(self.renderer);
        sdl.RenderCopy(self.renderer, self.texture, null, null);
        sdl.RenderPresent(self.renderer);
    }

    /// 全屏刷新（简单调用 flush_region）
    fn flush(self: &SdlDisplay) void {
        self.flush_region(Rect {
            x: 0, y: 0,
            w: self.screen_w,
            h: self.screen_h,
        });
    }

    /// 切换全屏模式
    fn set_fullscreen(self: &SdlDisplay, enabled: bool) void {
        const flag: u32 = if enabled { sdl.WINDOW_FULLSCREEN_DESKTOP } else { 0 };
        sdl.SetWindowFullscreen(self.window, flag);
    }

    /// 设置窗口标题
    fn set_title(self: &SdlDisplay, title: *byte) void {
        sdl.SetWindowTitle(self.window, title);
    }

    /// 获取显示尺寸
    fn get_size(self: &SdlDisplay) Size {
        return Size { w: self.screen_w, h: self.screen_h };
    }

    /// 设置模拟像素密度
    fn set_scale(self: &SdlDisplay, scale: u8) void {
        self.pixel_scale = scale;
        const window_w: i32 = self.screen_w as i32 * scale as i32;
        const window_h: i32 = self.screen_h as i32 * scale as i32;
        sdl.SetWindowSize(self.window, window_w, window_h);
    }

    fn has_gpu(self: &SdlDisplay) bool {
        return self.gpu_available;
    }

    fn get_gpu_ctx(self: &SdlDisplay) &GpuCtx {
        return if self.gpu_ctx != null { ?self.gpu_ctx } else { null };
    }
}

/// Uya 像素格式转 SDL 像素格式
fn uya_to_sdl_format(format: PixelFormat) u32 {
    match format {
        PixelFormat.RGB565   => { return sdl.PIXELFORMAT_RGB565; }
        PixelFormat.RGB888   => { return sdl.PIXELFORMAT_RGB24; }
        PixelFormat.ARGB8888 => { return sdl.PIXELFORMAT_ARGB8888; }
        PixelFormat.ARGB4444 => { return sdl.PIXELFORMAT_ARGB4444; }
        else => { return sdl.PIXELFORMAT_RGB565; }
    }
}

/// 每像素字节数
fn bytes_per_pixel(format: PixelFormat) u8 {
    match format {
        PixelFormat.I1       => { return 1; }  // 位图，按字节对齐
        PixelFormat.I4       => { return 1; }
        PixelFormat.L8       => { return 1; }
        PixelFormat.A8       => { return 1; }
        PixelFormat.RGB565   => { return 2; }
        PixelFormat.ARGB4444 => { return 2; }
        PixelFormat.RGB888   => { return 3; }
        PixelFormat.ARGB8888 => { return 4; }
    }
}
```

### 3.2 Framebuffer 后端（更接近嵌入式）

```uya
//========================================
// platform/fb/disp_fb.uya
// Linux Framebuffer 后端 - 最接近嵌入式环境
//========================================

import "linux/fb.h";   // #include <linux/fb.h>
import "fcntl.h";      // #include <fcntl.h>
import "sys/mman.h";     // #include <sys/mman.h>
import "unistd.h";    // #include <unistd.h>

/// Framebuffer 显示驱动
struct FbDisplay: IDisplay {
    fb_fd:          i32,            // /dev/fb0 文件描述符
    fb_mem:         &u8,            // mmap 映射的帧缓冲内存
    fb_size:        u32,

    // 固定和可变屏幕信息
    fix_info:       fb_fix_screeninfo,
    var_info:       fb_var_screeninfo,

    // 模拟器帧缓冲（与硬件格式可能不同）
    sim_buf:        [u8: MAX_FB_SIZE],
    sim_fb:         FrameBuffer,

    screen_w:       u16,
    screen_h:       u16,
}

FbDisplay {
    /// 初始化 Framebuffer
    fn init(self: &FbDisplay, width: u16, height: u16, format: PixelFormat) !void {
        // 打开 framebuffer 设备
        self.fb_fd = open("/dev/fb0", O_RDWR);
        if self.fb_fd < 0 {
            return error.FbOpenFailed;
        }

        errdefer {
            close(self.fb_fd);
        }

        // 获取固定屏幕信息
        if ioctl(self.fb_fd, FBIOGET_FSCREENINFO, &self.fix_info) < 0 {
            return error.FbIoctlFailed;
        }

        // 获取可变屏幕信息
        if ioctl(self.fb_fd, FBIOGET_VSCREENINFO, &self.var_info) < 0 {
            return error.FbIoctlFailed;
        }

        // 设置虚拟分辨率
        self.var_info.xres_virtual = width as u32;
        self.var_info.yres_virtual = height as u32;
        self.var_info.xres = width as u32;
        self.var_info.yres = height as u32;

        // 根据格式设置位深
        match format {
            PixelFormat.RGB565 => {
                self.var_info.bits_per_pixel = 16;
                self.var_info.red.offset = 11;    self.var_info.red.length = 5;
                self.var_info.green.offset = 5;   self.var_info.green.length = 6;
                self.var_info.blue.offset = 0;    self.var_info.blue.length = 5;
            }
            PixelFormat.ARGB8888 => {
                self.var_info.bits_per_pixel = 32;
                self.var_info.red.offset = 16;    self.var_info.red.length = 8;
                self.var_info.green.offset = 8;   self.var_info.green.length = 8;
                self.var_info.blue.offset = 0;    self.var_info.blue.length = 8;
                self.var_info.transp.offset = 24; self.var_info.transp.length = 8;
            }
            else => {
                return error.UnsupportedFormat;
            }
        }

        // 应用设置
        if ioctl(self.fb_fd, FBIOPUT_VSCREENINFO, &self.var_info) < 0 {
            return error.FbSetModeFailed;
        }

        // mmap 帧缓冲
        self.fb_size = self.fix_info.smem_len;
        self.fb_mem = mmap(
            null, self.fb_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, self.fb_fd, 0
        ) as &u8;

        if self.fb_mem == null {
            return error.FbMmapFailed;
        }

        errdefer {
            munmap(self.fb_mem, self.fb_size);
        }

        // 初始化模拟器帧缓冲
        self.screen_w = width;
        self.screen_h = height;
        const stride: u16 = width * bytes_per_pixel(format);

        self.sim_fb = FrameBuffer {
            buf:    &self.sim_buf[0],
            w:      width,
            h:      height,
            stride: stride,
            format: format,
        };

        log_info("Framebuffer initialized: ${width}x${height}, ${self.fb_size} bytes");
    }

    /// 关闭 Framebuffer
    fn deinit(self: &FbDisplay) void {
        if self.fb_mem != null {
            munmap(self.fb_mem, self.fb_size);
            self.fb_mem = null;
        }
        if self.fb_fd >= 0 {
            close(self.fb_fd);
            self.fb_fd = -1;
        }
    }

    fn get_framebuffer(self: &FbDisplay) FrameBuffer {
        return self.sim_fb;
    }

    /// 刷新 - memcpy 到硬件帧缓冲
    fn flush(self: &FbDisplay) void {
        const sim_stride: i32 = self.sim_fb.stride as i32;
        const hw_stride: i32 = self.var_info.xres as i32 
                             * self.var_info.bits_per_pixel as i32 / 8;

        for 0..self.screen_h |row| {
            const src: &u8 = self.sim_fb.buf + row as i32 * sim_stride;
            const dst: &u8 = self.fb_mem + row as i32 * hw_stride;
            memcpy(dst, src, sim_stride as u32);
        }
    }

    /// 区域刷新（FB 通常只能全刷）
    fn flush_region(self: &FbDisplay, region: Rect) void {
        // Framebuffer 通常只支持全屏刷新
        // 如果需要部分刷新，可以通过 pan_display 或自定义 ioctl
        self.flush();
    }

    fn set_fullscreen(self: &FbDisplay, enabled: bool) void {
        // Framebuffer 本身就是全屏
    }

    fn set_title(self: &FbDisplay, title: *byte) void {
        // Framebuffer 没有窗口标题
    }

    fn get_size(self: &FbDisplay) Size {
        return Size { w: self.screen_w, h: self.screen_h };
    }

    fn set_scale(self: &FbDisplay, scale: u8) void {
        // Framebuffer 不支持缩放
    }

    fn has_gpu(self: &FbDisplay) bool {
        return false;
    }

    fn get_gpu_ctx(self: &FbDisplay) &GpuCtx {
        return null;
    }
}
```

---

## 4. 输入模拟

### 4.1 SDL2 输入后端

```uya
//========================================
// platform/sdl2/indev_sdl.uya
// SDL2 输入后端 - 鼠标模拟触摸屏，键盘模拟按键
//========================================

import sdl "SDL2/SDL.h";

/// SDL2 输入设备聚合器
struct SdlInput {
    // 触摸屏（鼠标模拟）
    touch:  SdlTouchInput,
    // 键盘
    keypad: SdlKeypadInput,
    // 鼠标滚轮（模拟编码器）
    encoder: SdlEncoderInput,

    // 事件轮询缓冲
    event_buf: [sdl.Event: SDL_EVENT_BUF_SIZE],
    event_count: i32,
}

SdlInput {
    fn new() SdlInput {
        return SdlInput {
            touch:       SdlTouchInput.new(),
            keypad:      SdlKeypadInput.new(),
            encoder:     SdlEncoderInput.new(),
            event_buf:   [],
            event_count: 0,
        };
    }

    /// 初始化所有输入设备
    fn init(self: &SdlInput) !void {
        try self.touch.init();
        try self.keypad.init();
        try self.encoder.init();
    }

    /// 轮询所有 SDL 事件
    fn poll(self: &SdlInput) void {
        self.event_count = 0;

        var evt: sdl.Event;
        while sdl.PollEvent(&evt) != 0 {
            if self.event_count < SDL_EVENT_BUF_SIZE {
                self.event_buf[self.event_count] = evt;
                self.event_count += 1;
            }

            // 分发给各输入设备
            self.touch.process_event(&evt);
            self.keypad.process_event(&evt);
            self.encoder.process_event(&evt);
        }
    }

    /// 读取下一个输入事件（聚合所有设备）
    fn read_next(self: &SdlInput, data: &InputData) ?InputDevType {
        // 优先读取触摸
        if self.touch.read(data) {
            return InputDevType.Touch;
        }
        // 然后键盘
        if self.keypad.read(data) {
            return InputDevType.Keypad;
        }
        // 最后编码器
        if self.encoder.read(data) {
            return InputDevType.Encoder;
        }
        return null;
    }
}

//========================================
// SDL2 触摸屏（鼠标模拟）
//========================================

/// 用鼠标模拟触摸屏
struct SdlTouchInput: IInputDev {
    // 当前状态
    x:              i16,
    y:              i16,
    is_pressed:     bool,
    has_update:     bool,

    // 模拟器配置
    sim_width:      u16,        // GUI 逻辑宽度
    sim_height:     u16,        // GUI 逻辑高度
    window_scale:   u8,         // 窗口放大倍数

    // 手势检测
    last_x:         i16,
    last_y:         i16,
    press_time:     u32,
}

SdlTouchInput {
    fn new() SdlTouchInput {
        return SdlTouchInput {
            x: 0, y: 0,
            is_pressed: false,
            has_update: false,
            sim_width: 480,
            sim_height: 320,
            window_scale: 2,
            last_x: 0, last_y: 0,
            press_time: 0,
        };
    }

    fn init(self: &SdlTouchInput) !void {
        // SDL 已在 display 中初始化
    }

    fn deinit(self: &SdlTouchInput) void {}

    fn dev_type(self: &SdlTouchInput) InputDevType {
        return InputDevType.Touch;
    }

    fn set_calibration(self: &SdlTouchInput, cal: &CalibrationData) void {
        // 鼠标不需要校准
    }

    /// 处理 SDL 事件
    fn process_event(self: &SdlTouchInput, evt: &sdl.Event) void {
        match evt.type {
            sdl.MOUSEMOTION => {
                const mx: i32 = evt.motion.x;
                const my: i32 = evt.motion.y;

                // 从窗口坐标转换为模拟器坐标
                self.x = (mx / self.window_scale as i32) as i16;
                self.y = (my / self.window_scale as i32) as i16;

                // 边界裁剪
                self.x = clamp(self.x, 0, self.sim_width as i16 - 1);
                self.y = clamp(self.y, 0, self.sim_height as i16 - 1);

                if self.is_pressed {
                    self.has_update = true;
                }
            }

            sdl.MOUSEBUTTONDOWN => {
                if evt.button.button == sdl.BUTTON_LEFT {
                    self.is_pressed = true;
                    self.last_x = self.x;
                    self.last_y = self.y;
                    self.press_time = get_tick_ms();
                    self.has_update = true;
                }
            }

            sdl.MOUSEBUTTONUP => {
                if evt.button.button == sdl.BUTTON_LEFT {
                    self.is_pressed = false;
                    self.has_update = true;
                }
            }

            else => {}
        }
    }

    /// 读取触摸数据
    fn read(self: &SdlTouchInput, data: &InputData) bool {
        if !self.has_update {
            return false;
        }

        data.touch = TouchData {
            x:          self.x,
            y:          self.y,
            pressure:   if self.is_pressed { 1000 } else { 0 },
            is_pressed: self.is_pressed,
        };

        self.has_update = false;
        return true;
    }
}

//========================================
// SDL2 键盘输入
//========================================

/// 键盘到 GUI 键码的映射表
const KEY_MAP: [(u16, u16): 32] = [
    // SDL 键码        -> GUI 键码
    (sdl.K_UP,        KEY_UP),
    (sdl.K_DOWN,      KEY_DOWN),
    (sdl.K_LEFT,      KEY_LEFT),
    (sdl.K_RIGHT,     KEY_RIGHT),
    (sdl.K_RETURN,    KEY_ENTER),
    (sdl.K_ESCAPE,    KEY_ESC),
    (sdl.K_TAB,       KEY_NEXT),
    (sdl.K_BACKSPACE, KEY_BACKSPACE),
    (sdl.K_HOME,      KEY_HOME),
    (sdl.K_END,       KEY_END),
    (sdl.K_0,         KEY_0),
    (sdl.K_1,         KEY_1),
    (sdl.K_2,         KEY_2),
    (sdl.K_3,         KEY_3),
    (sdl.K_4,         KEY_4),
    (sdl.K_5,         KEY_5),
    (sdl.K_6,         KEY_6),
    (sdl.K_7,         KEY_7),
    (sdl.K_8,         KEY_8),
    (sdl.K_9,         KEY_9),
];

struct SdlKeypadInput: IInputDev {
    pending_key:    KeyData,
    has_key:        bool,
}

SdlKeypadInput {
    fn new() SdlKeypadInput {
        return SdlKeypadInput {
            pending_key: KeyData { code: 0, is_pressed: false, is_repeat: false },
            has_key: false,
        };
    }

    fn init(self: &SdlKeypadInput) !void {}
    fn deinit(self: &SdlKeypadInput) void {}
    fn dev_type(self: &SdlKeypadInput) InputDevType {
        return InputDevType.Keypad;
    }
    fn set_calibration(self: &SdlKeypadInput, cal: &CalibrationData) void {}

    fn process_event(self: &SdlKeypadInput, evt: &sdl.Event) void {
        match evt.type {
            sdl.KEYDOWN => {
                const gui_code: u16 = sdl_to_gui_keycode(evt.key.keysym.sym);
                if gui_code != 0 {
                    self.pending_key = KeyData {
                        code:       gui_code,
                        is_pressed: true,
                        is_repeat:  evt.key.repeat != 0,
                    };
                    self.has_key = true;
                }
            }

            sdl.KEYUP => {
                const gui_code: u16 = sdl_to_gui_keycode(evt.key.keysym.sym);
                if gui_code != 0 {
                    self.pending_key = KeyData {
                        code:       gui_code,
                        is_pressed: false,
                        is_repeat:  false,
                    };
                    self.has_key = true;
                }
            }

            else => {}
        }
    }

    fn read(self: &SdlKeypadInput, data: &InputData) bool {
        if !self.has_key {
            return false;
        }
        data.key = self.pending_key;
        self.has_key = false;
        return true;
    }
}

/// SDL 键码转 GUI 键码
fn sdl_to_gui_keycode(sdl_key: sdl.Keycode) u16 {
    for KEY_MAP |entry| {
        if entry[0] == sdl_key as u16 {
            return entry[1];
        }
    }
    return 0;
}

//========================================
// SDL2 滚轮模拟编码器
//========================================

struct SdlEncoderInput: IInputDev {
    pending_delta:  i16,
    pending_button: bool,
    has_update:     bool,
}

SdlEncoderInput {
    fn new() SdlEncoderInput {
        return SdlEncoderInput {
            pending_delta: 0,
            pending_button: false,
            has_update: false,
        };
    }

    fn init(self: &SdlEncoderInput) !void {}
    fn deinit(self: &SdlEncoderInput) void {}
    fn dev_type(self: &SdlEncoderInput) InputDevType {
        return InputDevType.Encoder;
    }
    fn set_calibration(self: &SdlEncoderInput, cal: &CalibrationData) void {}

    fn process_event(self: &SdlEncoderInput, evt: &sdl.Event) void {
        match evt.type {
            sdl.MOUSEWHEEL => {
                self.pending_delta = evt.wheel.y as i16 * ENCODER_SENSITIVITY;
                self.has_update = true;
            }
            sdl.MOUSEBUTTONDOWN => {
                if evt.button.button == sdl.BUTTON_MIDDLE {
                    self.pending_button = true;
                    self.has_update = true;
                }
            }
            sdl.MOUSEBUTTONUP => {
                if evt.button.button == sdl.BUTTON_MIDDLE {
                    self.pending_button = false;
                    self.has_update = true;
                }
            }
            else => {}
        }
    }

    fn read(self: &SdlEncoderInput, data: &InputData) bool {
        if !self.has_update {
            return false;
        }
        data.encoder = EncoderData {
            delta:  self.pending_delta,
            button: self.pending_button,
        };
        self.pending_delta = 0;
        self.has_update = false;
        return true;
    }
}
```

---

## 5. 时钟与定时器

### 5.1 SDL2 时钟后端

```uya
//========================================
// platform/sdl2/tick_sdl.uya
// SDL2 时钟实现
//========================================

import sdl "SDL2/SDL.h";

/// SDL2 时钟实现
struct SdlTick: ITick {
    start_ms:   u32,        // 启动时的 tick
    start_us:   u64,        // 启动时的微秒
}

SdlTick {
    fn new() SdlTick {
        return SdlTick {
            start_ms: sdl.GetTicks(),
            start_us: get_time_us(),
        };
    }

    fn get_ms(self: &SdlTick) u32 {
        return sdl.GetTicks() - self.start_ms;
    }

    fn get_us(self: &SdlTick) u64 {
        return get_time_us() - self.start_us;
    }

    fn delay_ms(self: &SdlTick, ms: u32) void {
        sdl.Delay(ms);
    }

    fn delay_us(self: &SdlTick, us: u32) void {
        // SDL 没有微秒级延迟，使用忙等待
        const target: u64 = self.get_us() + us as u64;
        while self.get_us() < target {
            // 自旋等待
        }
    }

    fn uptime_s(self: &SdlTick) u32 {
        return self.get_ms() / 1000;
    }
}

/// 获取高精度时间（微秒）
extern fn gettimeofday(tv: &timeval, tz: &void) i32;

fn get_time_us() u64 {
    var tv: timeval;
    gettimeofday(&tv, null);
    return tv.tv_sec as u64 * 1000000 + tv.tv_usec as u64;
}

struct timeval {
    tv_sec:     i64,
    tv_usec:    i64,
}
```

### 5.2 Linux 通用时钟

```uya
//========================================
// platform/fb/tick_linux.uya
// Linux 通用时钟（不依赖 SDL）
//========================================

import "time.h";
import "unistd.h";

/// Linux 时钟实现
struct LinuxTick: ITick {
    start_monotonic: timespec,
}

LinuxTick {
    fn new() LinuxTick {
        var start: timespec;
        clock_gettime(CLOCK_MONOTONIC, &start);
        return LinuxTick { start_monotonic: start };
    }

    fn get_ms(self: &LinuxTick) u32 {
        var now: timespec;
        clock_gettime(CLOCK_MONOTONIC, &now);

        const elapsed_s: i64 = now.tv_sec - self.start_monotonic.tv_sec;
        const elapsed_ns: i64 = now.tv_nsec - self.start_monotonic.tv_nsec;

        return (elapsed_s * 1000 + elapsed_ns / 1000000) as u32;
    }

    fn get_us(self: &LinuxTick) u64 {
        var now: timespec;
        clock_gettime(CLOCK_MONOTONIC, &now);

        const elapsed_s: i64 = now.tv_sec - self.start_monotonic.tv_sec;
        const elapsed_ns: i64 = now.tv_nsec - self.start_monotonic.tv_nsec;

        return (elapsed_s * 1000000 + elapsed_ns / 1000) as u64;
    }

    fn delay_ms(self: &LinuxTick, ms: u32) void {
        usleep(ms * 1000);
    }

    fn delay_us(self: &LinuxTick, us: u32) void {
        usleep(us);
    }

    fn uptime_s(self: &LinuxTick) u32 {
        return self.get_ms() / 1000;
    }
}

const CLOCK_MONOTONIC: i32 = 1;

extern struct timespec {
    tv_sec:     i64,
    tv_nsec:    i64,
}

extern fn clock_gettime(clk_id: i32, tp: &timespec) i32;
extern fn usleep(usec: u32) i32;
```

---

## 6. 文件系统模拟

### 6.1 POSIX 文件系统后端

```uya
//========================================
// platform/sdl2/fs_posix.uya
// POSIX 文件系统 - 在 Linux 上直接使用真实文件系统
//========================================

import "fcntl.h";
import "unistd.h";
import "sys/stat.h";
import "dirent.h";

/// POSIX 文件系统接口
struct PosixFs {
    base_path: [u8: 256],   // 基础路径（chroot 到项目目录）
}

PosixFs {
    /// 打开文件
    fn open(path: *byte, flags: OpenFlags) !i32 {
        const fd: i32 = open(path, flags_to_posix(flags), 0644);
        if fd < 0 {
            return error.FileOpenFailed;
        }
        return fd;
    }

    /// 读取文件
    fn read(fd: i32, buf: &u8, count: u32) !i32 {
        const n: i32 = read(fd, buf, count);
        if n < 0 {
            return error.FileReadFailed;
        }
        return n;
    }

    /// 写入文件
    fn write(fd: i32, buf: &u8, count: u32) !i32 {
        const n: i32 = write(fd, buf, count);
        if n < 0 {
            return error.FileWriteFailed;
        }
        return n;
    }

    /// 关闭文件
    fn close(fd: i32) void {
        close(fd);
    }

    /// 获取文件大小
    fn file_size(path: *byte) !u32 {
        var st: stat;
        if stat(path, &st) < 0 {
            return error.FileNotFound;
        }
        return st.st_size as u32;
    }

    /// 文件是否存在
    fn exists(path: *byte) bool {
        return access(path, F_OK) == 0;
    }

    /// 列出目录
    fn list_dir(path: *byte, callback: fn(*byte) void) void {
        const dir: &DIR = opendir(path);
        if dir == null {
            return;
        }
        defer {
            closedir(dir);
        }

        var entry: &dirent;
        while (entry = readdir(dir)) != null {
            if entry.d_name[0] != '.' {  // 跳过 . 和 ..
                callback(&entry.d_name[0]);
            }
        }
    }
}

/// 文件打开标志
struct OpenFlags {
    read:       bool,
    write:      bool,
    create:     bool,
    truncate:   bool,
    append:     bool,
}

/// 转换为 POSIX 标志
fn flags_to_posix(flags: OpenFlags) i32 {
    var posix_flags: i32 = 0;
    if flags.read && flags.write {
        posix_flags |= O_RDWR;
    } else if flags.write {
        posix_flags |= O_WRONLY;
    } else {
        posix_flags |= O_RDONLY;
    }
    if flags.create   { posix_flags |= O_CREAT; }
    if flags.truncate { posix_flags |= O_TRUNC; }
    if flags.append   { posix_flags |= O_APPEND; }
    return posix_flags;
}
```

---

## 7. 模拟器调试工具

### 7.1 性能分析器

```uya
//========================================
// sim/profiler.uya
// 内置性能分析器
//========================================

/// 性能统计
struct Profiler {
    // 帧统计
    frame_count:        u32,
    frame_time_min:     u32,
    frame_time_max:     u32,
    frame_time_avg:     u32,
    frame_time_accum:   u64,

    // 渲染统计
    render_time:        u32,
    pixels_drawn:       u64,
    draw_calls:         u32,
    dirty_regions:      u32,

    // 事件统计
    events_processed:   u32,

    // 内存统计
    obj_count:          u32,
    obj_pool_used:      u32,
    mem_used:           u32,

    // 显示
    overlay_visible:    bool,
    font:               &Font,
}

Profiler {
    fn new() Profiler {
        return Profiler {
            frame_count:        0,
            frame_time_min:     0xFFFFFFFF,
            frame_time_max:     0,
            frame_time_avg:     0,
            frame_time_accum:   0,
            render_time:        0,
            pixels_drawn:       0,
            draw_calls:         0,
            dirty_regions:      0,
            events_processed:   0,
            obj_count:          0,
            obj_pool_used:      0,
            mem_used:           0,
            overlay_visible:    true,
            font:               &FONT_MONO_12,
        };
    }

    /// 开始帧计时
    fn begin_frame(self: &Profiler) void {
        self.frame_start = get_tick_us();
    }

    /// 结束帧计时
    fn end_frame(self: &Profiler) void {
        const elapsed: u32 = (get_tick_us() - self.frame_start) as u32;

        self.frame_count += 1;
        self.frame_time_accum += elapsed as u64;

        if elapsed < self.frame_time_min { self.frame_time_min = elapsed; }
        if elapsed > self.frame_time_max { self.frame_time_max = elapsed; }

        // 每秒更新平均
        if self.frame_count % 60 == 0 {
            self.frame_time_avg = (self.frame_time_accum / self.frame_count as u64) as u32;
        }
    }

    /// 绘制性能叠加层（在屏幕角落显示）
    fn draw_overlay(self: &Profiler, ctx: &RenderCtx) void {
        if !self.overlay_visible {
            return;
        }

        const line_h: i16 = 14;
        const x: i16 = 5;
        var y: i16 = 5;

        // 背景半透明矩形
        ctx.fill_rect(Rect { x: 0, y: 0, w: 140, h: 90 }, COLOR(0x80000000));

        // FPS
        const fps: u32 = if self.frame_time_avg > 0 { 1000000 / self.frame_time_avg } else { 0 };
        ctx.draw_text(x, y, "FPS: ${fps}", self.font, COLOR(0x00FF00));
        y += line_h;

        // 帧时间
        ctx.draw_text(x, y, "Frame: ${self.frame_time_avg / 1000}.${self.frame_time_avg % 1000}ms", 
                     self.font, COLOR(0xFFFFFF));
        y += line_h;

        // 渲染时间
        ctx.draw_text(x, y, "Render: ${self.render_time / 1000}ms", 
                     self.font, COLOR(0xFFFFFF));
        y += line_h;

        // 对象数
        ctx.draw_text(x, y, "Objs: ${self.obj_count}", 
                     self.font, COLOR(0xFFFFFF));
        y += line_h;

        // 内存
        ctx.draw_text(x, y, "Mem: ${self.mem_used / 1024}KB", 
                     self.font, COLOR(0xFFFFFF));
        y += line_h;

        // 绘制调用
        ctx.draw_text(x, y, "Draws: ${self.draw_calls}", 
                     self.font, COLOR(0xFFFFFF));
    }

    /// 切换显示
    fn toggle(self: &Profiler) void {
        self.overlay_visible = !self.overlay_visible;
    }

    /// 打印统计到控制台
    fn print_stats(self: &Profiler) void {
        printf("
=== UyaGUI Profiler ===
");
        printf("Frames:     %u
", self.frame_count);
        printf("Frame time: min=%uus, max=%uus, avg=%uus
",
               self.frame_time_min, self.frame_time_max, self.frame_time_avg);
        printf("Render:     %uus
", self.render_time);
        printf("Pixels:     %llu
", self.pixels_drawn);
        printf("Draw calls: %u
", self.draw_calls);
        printf("Objects:    %u
", self.obj_count);
        printf("Memory:     %u bytes
", self.mem_used);
        printf("=======================
");
    }
}
```

### 7.2 输入录制与回放

```uya
//========================================
// sim/recorder.uya
// 输入录制与回放 - 用于自动化测试和调试
//========================================

/// 录制的事件条目
struct RecordedEvent {
    timestamp:  u32,    // 相对录制开始的毫秒
    type:       InputDevType,
    data:       InputData,
}

/// 输入录制器
struct InputRecorder {
    events:     [RecordedEvent: MAX_RECORDED_EVENTS],
    count:      u32,
    is_recording: bool,
    start_ms:   u32,

    // 回放
    is_playing: bool,
    play_idx:   u32,
    play_start: u32,
}

InputRecorder {
    fn new() InputRecorder {
        return InputRecorder {
            events:         [],
            count:          0,
            is_recording:   false,
            start_ms:       0,
            is_playing:     false,
            play_idx:       0,
            play_start:     0,
        };
    }

    /// 开始录制
    fn start_recording(self: &InputRecorder) void {
        self.count = 0;
        self.is_recording = true;
        self.start_ms = get_tick_ms();
        log_info("Input recording started");
    }

    /// 停止录制
    fn stop_recording(self: &InputRecorder) void {
        self.is_recording = false;
        log_info("Input recording stopped: ${self.count} events");
    }

    /// 记录一个事件
    fn record(self: &InputRecorder, dev_type: InputDevType, data: &InputData) void {
        if !self.is_recording || self.count >= MAX_RECORDED_EVENTS {
            return;
        }

        self.events[self.count] = RecordedEvent {
            timestamp:  get_tick_ms() - self.start_ms,
            type:       dev_type,
            data:       *data,
        };
        self.count += 1;
    }

    /// 保存录制到文件
    fn save(self: &InputRecorder, path: *byte) !void {
        const fd: i32 = try PosixFs.open(path, OpenFlags { 
            write: true, create: true, truncate: true, read: false, append: false 
        });
        defer {
            PosixFs.close(fd);
        }

        // 写入文件头
        const header: RecordFileHeader = RecordFileHeader {
            magic:      RECORD_MAGIC,
            version:    1,
            event_count: self.count,
            screen_w:   SCREEN_WIDTH,
            screen_h:   SCREEN_HEIGHT,
        };
        PosixFs.write(fd, &header as &u8, @size_of(RecordFileHeader));

        // 写入事件
        for 0..self.count |i| {
            const evt: &RecordedEvent = &self.events[i];
            PosixFs.write(fd, evt as &u8, @size_of(RecordedEvent));
        }

        log_info("Recording saved: ${path} (${self.count} events)");
    }

    /// 从文件加载
    fn load(self: &InputRecorder, path: *byte) !void {
        const fd: i32 = try PosixFs.open(path, OpenFlags { 
            read: true, write: false, create: false, truncate: false, append: false 
        });
        defer {
            PosixFs.close(fd);
        }

        // 读取文件头
        var header: RecordFileHeader;
        PosixFs.read(fd, &header as &u8, @size_of(RecordFileHeader));

        if header.magic != RECORD_MAGIC {
            return error.InvalidRecordFile;
        }

        // 读取事件
        self.count = min(header.event_count, MAX_RECORDED_EVENTS);
        for 0..self.count |i| {
            PosixFs.read(fd, &self.events[i] as &u8, @size_of(RecordedEvent));
        }

        log_info("Recording loaded: ${path} (${self.count} events)");
    }

    /// 开始回放
    fn start_playback(self: &InputRecorder) void {
        if self.count == 0 {
            return;
        }
        self.is_playing = true;
        self.play_idx = 0;
        self.play_start = get_tick_ms();
        log_info("Input playback started");
    }

    /// 获取下一个回放事件（时间到了就返回）
    fn get_playback_event(self: &InputRecorder, dev_type: &InputDevType, data: &InputData) bool {
        if !self.is_playing || self.play_idx >= self.count {
            return false;
        }

        const now: u32 = get_tick_ms() - self.play_start;
        const evt: &RecordedEvent = &self.events[self.play_idx];

        if now >= evt.timestamp {
            *dev_type = evt.type;
            *data = evt.data;
            self.play_idx += 1;

            if self.play_idx >= self.count {
                self.is_playing = false;
                log_info("Input playback finished");
            }
            return true;
        }

        return false;
    }
}

const RECORD_MAGIC: u32 = 0x52434755;  // "UGCR" (UyaGui Capture Record)

struct RecordFileHeader {
    magic:      u32,
    version:    u32,
    event_count: u32,
    screen_w:   u16,
    screen_h:   u16,
}
```

---

## 8. 模拟器主程序

### 8.1 主循环

```uya
//========================================
// sim/main.uya
// 模拟器主程序
//========================================

import gui.core.*;
import gui.render.*;
import gui.widget.*;
import gui.layout.*;
import gui.anim.*;
import gui.platform.sdl2.*;
import gui.sim.*;

/// 模拟器配置
struct SimConfig {
    screen_w:       u16,        // 模拟屏幕宽度
    screen_h:       u16,        // 模拟屏幕高度
    pixel_format:   PixelFormat,
    pixel_scale:    u8,         // 窗口放大倍数
    target_fps:     u8,         // 目标帧率
    show_profiler:  bool,
    record_input:   bool,
    playback_file:  *byte,      // 回放文件路径
}

SimConfig {
    /// 默认配置 (480x320 通用嵌入式屏)
    fn default() SimConfig {
        return SimConfig {
            screen_w:       480,
            screen_h:       320,
            pixel_format:   PixelFormat.RGB565,
            pixel_scale:    2,
            target_fps:     60,
            show_profiler:  true,
            record_input:   false,
            playback_file:  null,
        };
    }

    /// 小屏配置 (240x320 竖屏)
    fn small_portrait() SimConfig {
        return SimConfig {
            screen_w:       240,
            screen_h:       320,
            pixel_format:   PixelFormat.RGB565,
            pixel_scale:    3,
            target_fps:     60,
            show_profiler:  true,
            record_input:   false,
            playback_file:  null,
        };
    }

    /// 大屏配置 (800x480)
    fn large() SimConfig {
        return SimConfig {
            screen_w:       800,
            screen_h:       480,
            pixel_format:   PixelFormat.RGB565,
            pixel_scale:    1,
            target_fps:     60,
            show_profiler:  true,
            record_input:   false,
            playback_file:  null,
        };
    }
}

/// 模拟器上下文
struct Simulator {
    // 平台驱动
    display:    SdlDisplay,
    input:      SdlInput,
    tick:       SdlTick,

    // GUI 系统
    ctx:        RenderCtx,
    tree:       ObjTree,
    dispatcher: EventDispatcher,
    anim_mgr:   AnimManager,

    // 调试工具
    profiler:   Profiler,
    recorder:   InputRecorder,

    // 配置
    config:     SimConfig,
    running:    bool,

    // 帧率控制
    frame_interval: u32,    // 每帧目标时间 (us)
}

Simulator {
    /// 创建模拟器
    fn new(config: SimConfig) Simulator {
        return Simulator {
            display:    SdlDisplay.new(),
            input:      SdlInput.new(),
            tick:       SdlTick.new(),
            ctx:        RenderCtx.dummy(),
            tree:       ObjTree.new(),
            dispatcher: EventDispatcher.new(),
            anim_mgr:   AnimManager.new(),
            profiler:   Profiler.new(),
            recorder:   InputRecorder.new(),
            config:     config,
            running:    false,
            frame_interval: 1000000 / config.target_fps as u32,
        };
    }

    /// 初始化模拟器
    fn init(self: &Simulator) !void {
        log_info("========================================");
        log_info("  UyaGUI Simulator v0.1.0");
        log_info("  Screen: ${self.config.screen_w}x${self.config.screen_h}");
        log_info("  Scale: ${self.config.pixel_scale}x");
        log_info("  Target FPS: ${self.config.target_fps}");
        log_info("========================================");

        // 初始化显示
        try self.display.init(
            self.config.screen_w,
            self.config.screen_h,
            self.config.pixel_format
        );
        errdefer {
            self.display.deinit();
        }

        self.display.set_scale(self.config.pixel_scale);

        // 初始化输入
        try self.input.init();

        // 初始化渲染上下文
        self.ctx = RenderCtx.init(self.display.get_framebuffer());

        // 初始化 GUI 系统
        self.tree = ObjTree.new();
        self.dispatcher = EventDispatcher.new();
        self.anim_mgr = AnimManager.new();

        // 初始化性能分析器
        self.profiler = Profiler.new();

        // 初始化录制器
        if self.config.record_input {
            self.recorder.start_recording();
        }
        if self.config.playback_file != null {
            try self.recorder.load(self.config.playback_file);
            self.recorder.start_playback();
        }

        self.running = true;
        log_info("Simulator initialized successfully");
    }

    /// 关闭模拟器
    fn deinit(self: &Simulator) void {
        if self.config.record_input {
            self.recorder.stop_recording();
            self.recorder.save("input_record.bin") catch |err| {
                log_error("Failed to save recording: ${err}");
            };
        }

        self.input.deinit();
        self.display.deinit();

        self.profiler.print_stats();

        log_info("Simulator shutdown");
    }

    /// 运行主循环
    fn run(self: &Simulator, app: &GuiApp) void {
        // 设置应用根对象
        self.tree.set_root(app.get_root());

        // 启动动画任务
        async {
            anim_task(&self.anim_mgr, self.frame_interval / 1000);
        }

        var last_frame: u64 = self.tick.get_us();

        while self.running {
            const frame_start: u64 = self.tick.get_us();

            // 1. 处理 SDL 事件（窗口关闭、输入等）
            self.input.poll();

            // 处理 SDL 退出事件
            self.handle_sdl_quit();

            // 2. 处理输入设备
            self.process_input();

            // 3. 处理 GUI 事件
            self.dispatcher.process_events(&self.tree);

            // 4. 执行布局
            if self.tree.is_layout_dirty() {
                self.tree.perform_layout();
            }

            // 5. 更新动画
            const now_ms: u32 = self.tick.get_ms();
            self.anim_mgr.update_all(now_ms);

            // 6. 渲染
            self.profiler.begin_frame();
            self.render_frame();
            self.profiler.end_frame();

            // 7. 刷新显示
            self.flush_display();

            // 8. 帧率控制
            self.frame_sync(frame_start);
        }
    }

    /// 处理输入事件
    fn process_input(self: &Simulator) void {
        var data: InputData;

        // 优先处理回放
        if self.recorder.is_playing {
            var dev_type: InputDevType;
            if self.recorder.get_playback_event(&dev_type, &data) {
                self.dispatcher.inject_event(dev_type, &data);
                return;
            }
        }

        // 读取实时输入
        while true {
            const dev_type_opt: ?InputDevType = self.input.read_next(&data);
            if dev_type_opt == null {
                break;
            }

            const dev_type: InputDevType = dev_type_opt.unwrap();

            // 录制
            if self.recorder.is_recording {
                self.recorder.record(dev_type, &data);
            }

            // 注入到 GUI 系统
            self.dispatcher.inject_event(dev_type, &data);
            self.profiler.events_processed += 1;
        }
    }

    /// 渲染一帧
    fn render_frame(self: &Simulator) void {
        const render_start: u64 = self.tick.get_us();

        // 获取脏区域
        const dirty: DirtyRegion = self.ctx.get_dirty_region();

        if dirty.is_empty() && !self.profiler.overlay_visible {
            return;  // 无脏区域，跳过渲染
        }

        // 渲染每个脏区域
        for 0..dirty.count |i| {
            const region: Rect = dirty.regions[i];
            self.ctx.set_clip(region);

            // 清空背景
            self.ctx.clear(region, app.bg_color);

            // 渲染对象树
            self.tree.render_region(&self.ctx, region);

            self.ctx.pop_clip();
        }

        // 绘制性能叠加层
        if self.config.show_profiler {
            self.profiler.draw_overlay(&self.ctx);
        }

        // 更新统计
        self.profiler.render_time = (self.tick.get_us() - render_start) as u32;
        self.profiler.draw_calls = self.ctx.stats.draw_calls;
        self.profiler.pixels_drawn = self.ctx.stats.pixels_drawn;
        self.profiler.dirty_regions = dirty.count as u32;
        self.profiler.obj_count = self.tree.obj_count;
    }

    /// 刷新显示
    fn flush_display(self: &Simulator) void {
        const dirty: DirtyRegion = self.ctx.get_dirty_region();

        if dirty.should_merge() {
            // 合并后全刷
            self.display.flush();
        } else {
            // 逐区域刷新
            for 0..dirty.count |i| {
                self.display.flush_region(dirty.regions[i]);
            }
        }

        self.ctx.clear_dirty();
    }

    /// 帧率同步
    fn frame_sync(self: &Simulator, frame_start: u64) void {
        const elapsed: u64 = self.tick.get_us() - frame_start;

        if elapsed < self.frame_interval as u64 {
            const sleep_us: u64 = self.frame_interval as u64 - elapsed;
            self.tick.delay_us(sleep_us as u32);
        }
    }

    /// 处理 SDL 退出
    fn handle_sdl_quit(self: &Simulator) void {
        // 检查窗口关闭事件已在 input.poll() 中处理
        // 这里检查特殊按键
        if is_key_pressed(sdl.K_F12) {
            self.profiler.toggle();
        }
        if is_key_pressed(sdl.K_F11) {
            self.display.set_fullscreen(true);
        }
        if is_key_pressed(sdl.K_ESCAPE) {
            self.running = false;
        }
        if is_key_pressed(sdl.K_F1) {
            // 截图
            self.save_screenshot();
        }
        if is_key_pressed(sdl.K_F2) {
            // 开始/停止录制
            if self.recorder.is_recording {
                self.recorder.stop_recording();
            } else {
                self.recorder.start_recording();
            }
        }
    }

    /// 保存截图
    fn save_screenshot(self: &Simulator) void {
        const filename: [u8: 64] = "screenshot_${get_tick_ms()}.bmp";

        // 保存帧缓冲为 BMP
        save_bmp(&self.display.fb, &filename[0]) catch |err| {
            log_error("Screenshot failed: ${err}");
            return;
        };

        log_info("Screenshot saved: ${filename}");
    }
}

/// 动画任务
@async_fn
fn anim_task(anim_mgr: &AnimManager, tick_ms: u32) void {
    while true {
        anim_mgr.update_all(tick_ms);
        @await sleep_ms(tick_ms);
    }
}

//========================================
// 应用程序接口
//========================================

/// GUI 应用接口
interface IGuiApp {
    fn get_root(self: &Self) &GuiObj;
    fn on_init(self: &Self) void;
    fn on_deinit(self: &Self) void;
}
```

---

## 9. 调试工具

### 9.1 截图工具

```uya
//========================================
// sim/screenshot.uya
// 截图功能
//========================================

/// 保存帧缓冲为 BMP 文件
fn save_bmp(fb: &FrameBuffer, path: *byte) !void {
    const fd: i32 = try PosixFs.open(path, OpenFlags {
        write: true, create: true, truncate: true, read: false, append: false
    });
    defer {
        PosixFs.close(fd);
    }

    // BMP 文件头
    const file_header: [u8: 14] = [
        B, M,                         // "BM"
        0, 0, 0, 0,                         // 文件大小（稍后填）
        0, 0, 0, 0,                         // 保留
        54, 0, 0, 0,                        // 数据偏移
    ];

    // DIB 头 (BITMAPINFOHEADER)
    const dib_header: [u8: 40] = [
        40, 0, 0, 0,                        // DIB 头大小
        fb.w as u8, (fb.w >> 8) as u8, 0, 0,   // 宽度
        fb.h as u8, (fb.h >> 8) as u8, 0, 0,   // 高度
        1, 0,                               // 颜色平面
        24, 0,                              // 位深 (24bpp)
        0, 0, 0, 0,                         // 压缩 (无)
        0, 0, 0, 0,                         // 图像大小
        0, 0, 0, 0,                         // X DPI
        0, 0, 0, 0,                         // Y DPI
        0, 0, 0, 0,                         // 颜色数
        0, 0, 0, 0,                         // 重要颜色
    ];

    // 写入文件头
    PosixFs.write(fd, &file_header[0], 14);
    PosixFs.write(fd, &dib_header[0], 40);

    // 计算行填充（BMP 行必须是 4 字节对齐）
    const row_bytes: i32 = fb.w as i32 * 3;
    const padding: i32 = (4 - row_bytes % 4) % 4;

    // 逐行写入像素（BMP 是 bottom-up）
    var row_buf: [u8: 800 * 3 + 4];  // 最大行缓冲

    var row: i32 = fb.h as i32 - 1;
    while row >= 0 {
        var col: i32 = 0;
        var buf_idx: i32 = 0;

        while col < fb.w as i32 {
            const pixel: Color = read_pixel(fb, col, row);
            row_buf[buf_idx + 0] = pixel.b;  // B
            row_buf[buf_idx + 1] = pixel.g;  // G
            row_buf[buf_idx + 2] = pixel.r;  // R
            buf_idx += 3;
            col += 1;
        }

        // 行填充
        for 0..padding |i| {
            row_buf[buf_idx + i] = 0;
        }

        PosixFs.write(fd, &row_buf[0], (row_bytes + padding) as u32);
        row -= 1;
    }

    log_info("BMP saved: ${path}");
}

/// 从帧缓冲读取像素
fn read_pixel(fb: &FrameBuffer, x: i32, y: i32) Color {
    const offset: i32 = y * fb.stride as i32 + x * bytes_per_pixel(fb.format) as i32;

    match fb.format {
        PixelFormat.RGB565 => {
            const pixel: u16 = (fb.buf[offset] as u16) | (fb.buf[offset + 1] as u16 << 8);
            return Color {
                r: ((pixel >> 11) & 0x1F) << 3 as u8,
                g: ((pixel >> 5) & 0x3F) << 2 as u8,
                b: (pixel & 0x1F) << 3 as u8,
                a: 255,
            };
        }
        PixelFormat.ARGB8888 => {
            return Color {
                b: fb.buf[offset + 0],
                g: fb.buf[offset + 1],
                r: fb.buf[offset + 2],
                a: fb.buf[offset + 3],
            };
        }
        else => {
            return COLOR(0);
        }
    }
}
```

---

## 10. 构建与运行

### 10.1 构建系统

```makefile
#========================================
# Makefile - UyaGUI Linux 模拟器构建
#========================================

# 配置
UYA         := uya
SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LIBS   := $(shell sdl2-config --libs)
TARGET      := uyagui_sim
BUILD_DIR   := build
SRC_DIR     := src

# 源文件
PLATFORM_SRC :=     $(SRC_DIR)/platform/sdl2/disp_sdl.uya     $(SRC_DIR)/platform/sdl2/indev_sdl.uya     $(SRC_DIR)/platform/sdl2/tick_sdl.uya     $(SRC_DIR)/platform/sdl2/sdl_common.uya     $(SRC_DIR)/platform/interface/disp_if.uya     $(SRC_DIR)/platform/interface/indev_if.uya     $(SRC_DIR)/platform/interface/tick_if.uya

CORE_SRC :=     $(SRC_DIR)/core/obj.uya     $(SRC_DIR)/core/rect.uya     $(SRC_DIR)/core/color.uya     $(SRC_DIR)/core/point.uya     $(SRC_DIR)/core/event.uya     $(SRC_DIR)/core/event_dispatch.uya     $(SRC_DIR)/core/dirty_region.uya     $(SRC_DIR)/core/bitmap.uya

RENDER_SRC :=     $(SRC_DIR)/render/ctx.uya     $(SRC_DIR)/render/draw.uya     $(SRC_DIR)/render/font.uya     $(SRC_DIR)/render/batch.uya     $(SRC_DIR)/render/img.uya

WIDGET_SRC :=     $(SRC_DIR)/widget/base.uya     $(SRC_DIR)/widget/btn.uya     $(SRC_DIR)/widget/lbl.uya     $(SRC_DIR)/widget/img.uya     $(SRC_DIR)/widget/slider.uya     $(SRC_DIR)/widget/chart.uya

ANIM_SRC :=     $(SRC_DIR)/anim/tween.uya     $(SRC_DIR)/anim/easing.uya     $(SRC_DIR)/anim/timeline.uya

STYLE_SRC :=     $(SRC_DIR)/style/style.uya     $(SRC_DIR)/style/theme.uya

SIM_SRC :=     $(SRC_DIR)/sim/profiler.uya     $(SRC_DIR)/sim/recorder.uya     $(SRC_DIR)/sim/screenshot.uya     $(SRC_DIR)/sim/main.uya

ALL_SRC := $(PLATFORM_SRC) $(CORE_SRC) $(RENDER_SRC) $(WIDGET_SRC) $(ANIM_SRC) $(STYLE_SRC) $(SIM_SRC)

# 编译标志
UYAFLAGS := --target x86_64-linux-gnu $(SDL2_CFLAGS) -O2
LDFLAGS  := $(SDL2_LIBS) -lm

# 目标
.PHONY: all clean run debug test profile

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(UYA) build $(UYAFLAGS) -o $@ $(SRC_DIR)/sim/main.uya $(LDFLAGS)
	@echo "Build complete: $@"

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

debug: UYAFLAGS := --target x86_64-linux-gnu $(SDL2_CFLAGS) -g -O0
debug: $(BUILD_DIR)/$(TARGET)_debug
	gdb ./$(BUILD_DIR)/$(TARGET)_debug

$(BUILD_DIR)/$(TARGET)_debug: $(ALL_SRC)
	@mkdir -p $(BUILD_DIR)
	$(UYA) build $(UYAFLAGS) -o $@ $(SRC_DIR)/sim/main.uya $(LDFLAGS)

test:
	$(UYA) test $(SRC_DIR)/tests/

profile: $(BUILD_DIR)/$(TARGET)
	# 使用 perf 进行性能分析
	perf record -g ./$(BUILD_DIR)/$(TARGET)
	perf report

clean:
	rm -rf $(BUILD_DIR)

# 安装依赖
setup:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev libsdl2-image-dev gdb valgrind perf-tools-unstable
	@echo "Dependencies installed"
```

### 10.2 运行脚本

```bash
#!/bin/bash
#========================================
# run.sh - 快速运行模拟器
#========================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
TARGET="$BUILD_DIR/uyagui_sim"

# 解析参数
SCREEN="480x320"
SCALE=2
FPS=60
RECORD=false
PLAYBACK=""
PROFILER=true

while [[ $# -gt 0 ]]; do
    case $1 in
        -s|--screen)
            SCREEN="$2"
            shift 2
            ;;
        --scale)
            SCALE="$2"
            shift 2
            ;;
        --fps)
            FPS="$2"
            shift 2
            ;;
        --record)
            RECORD=true
            shift
            ;;
        --playback)
            PLAYBACK="$2"
            shift 2
            ;;
        --no-profiler)
            PROFILER=false
            shift
            ;;
        --small)
            SCREEN="240x320"
            SCALE=3
            shift
            ;;
        --large)
            SCREEN="800x480"
            SCALE=1
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  -s, --screen SIZE   屏幕尺寸 (默认: 480x320)"
            echo "      --scale N       像素放大倍数 (默认: 2)"
            echo "      --fps N         目标帧率 (默认: 60)"
            echo "      --record        录制输入"
            echo "      --playback FILE 回放输入文件"
            echo "      --no-profiler   禁用性能分析器"
            echo "      --small         使用 240x320 小屏配置"
            echo "      --large         使用 800x480 大屏配置"
            echo "  -h, --help          显示帮助"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 解析分辨率
W=$(echo $SCREEN | cut -dx -f1)
H=$(echo $SCREEN | cut -dx -f2)

# 检查构建
if [ ! -f "$TARGET" ]; then
    echo "Building simulator..."
    make -C "$SCRIPT_DIR"
fi

echo "Starting UyaGUI Simulator..."
echo "  Screen: ${W}x${H}"
echo "  Scale: ${SCALE}x"
echo "  FPS: ${FPS}"
echo ""
echo "Controls:"
echo "  ESC        - 退出"
echo "  F1         - 截图"
echo "  F2         - 开始/停止录制"
echo "  F11        - 全屏切换"
echo "  F12        - 性能分析器开关"
echo "  Mouse      - 模拟触摸"
echo "  MouseWheel - 模拟编码器"
echo ""

# 运行
export SDL_VIDEODRIVER=x11
export SDL_RENDER_DRIVER=opengl

$TARGET     --width $W     --height $H     --scale $SCALE     --fps $FPS     $(if $RECORD; then echo "--record"; fi)     $(if [ -n "$PLAYBACK" ]; then echo "--playback $PLAYBACK"; fi)     $(if ! $PROFILER; then echo "--no-profiler"; fi)
```

### 10.3 VS Code 调试配置

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "UyaGUI Simulator Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/uyagui_sim_debug",
            "args": [
                "--width", "480",
                "--height", "320",
                "--scale", "2"
            ],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                { "name": "SDL_VIDEODRIVER", "value": "x11" }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build-debug"
        },
        {
            "name": "UyaGUI Valgrind",
            "type": "cppdbg",
            "request": "launch",
            "program": "valgrind",
            "args": [
                "--leak-check=full",
                "--show-leak-kinds=all",
                "--track-origins=yes",
                "${workspaceFolder}/build/uyagui_sim"
            ],
            "cwd": "${workspaceFolder}",
            "MIMode": "gdb"
        }
    ]
}
```

---

## 11. 完整示例

### 11.1 最小可运行示例

```uya
//========================================
// examples/sim_demo.uya
// 模拟器演示程序 - 可直接编译运行
//========================================

import gui.core.*;
import gui.render.*;
import gui.widget.*;
import gui.layout.*;
import gui.style.*;
import gui.anim.*;
import gui.platform.sdl2.*;
import gui.sim.*;

/// 颜色主题
const PALETTE: ThemeColors = ThemeColors {
    primary:    COLOR(0x1976D2),
    secondary:  COLOR(0xFF9800),
    bg:         COLOR(0xF5F5F5),
    surface:    COLOR(0xFFFFFF),
    text:       COLOR(0x212121),
    text_muted: COLOR(0x757575),
    success:    COLOR(0x4CAF50),
    danger:     COLOR(0xF44336),
};

/// 样式定义
const STYLE_PRIMARY: Style = StyleBuilder.new()
    .bg_color(PALETTE.primary)
    .fg_color(COLOR(0xFFFFFF))
    .radius(8)
    .build();

const STYLE_SECONDARY: Style = StyleBuilder.new()
    .bg_color(PALETTE.secondary)
    .fg_color(COLOR(0xFFFFFF))
    .radius(8)
    .build();

const STYLE_CARD: Style = StyleBuilder.new()
    .bg_color(PALETTE.surface)
    .fg_color(PALETTE.text)
    .radius(12)
    .build();

const STYLE_TITLE: Style = StyleBuilder.new()
    .fg_color(PALETTE.text)
    .font_size(20)
    .build();

const STYLE_BODY: Style = StyleBuilder.new()
    .fg_color(PALETTE.text_muted)
    .font_size(14)
    .build();

/// 演示应用
struct DemoApp {
    root:       Page,
    counter:    atomic i32,
    chart:      &Chart,
    label:      &Label,
}

DemoApp {
    fn create() DemoApp {
        // 创建根页面
        var root = Page.new("Demo");
        root.set_size(480, 320);
        root.set_bg_color(PALETTE.bg);
        root.set_layout(Layout.column().gap(10).padding(15));

        // 标题
        root.add(
            Label.new("UyaGUI Simulator Demo")
                .with_style(&STYLE_TITLE)
        );

        // 计数器标签
        const counter_lbl = root.add(
            Label.new("Count: 0")
                .with_style(&STYLE_BODY)
        );

        // 按钮行
        const btn_row = Container.new();
        btn_row.set_layout(Layout.row().gap(10));

        btn_row.add(
            Button.filled("Increment")
                .with_style(&STYLE_PRIMARY)
                .on_click(|app: &DemoApp, _| {
                    app.counter += 1;
                    app.label.set_text("Count: ${app.counter}");
                })
        );

        btn_row.add(
            Button.outlined("Animate")
                .with_style(&STYLE_SECONDARY)
                .on_click(|app: &DemoApp, _| {
                    app.chart.animate_to(100, 500);
                })
        );

        btn_row.add(
            Button.text_only("Reset")
                .on_click(|app: &DemoApp, _| {
                    app.counter = 0;
                    app.chart.clear();
                    app.label.set_text("Count: 0");
                })
        );

        root.add(btn_row);

        // 图表
        const chart = root.add(
            Chart.new()
                .set_size(450, 180)
                .set_chart_type(ChartType.Area)
                .with_style(&STYLE_CARD)
        );

        // 填充初始数据
        for 0..30 |i| {
            chart.add_point((sin(i as f32 * 0.3) * 40 + 50) as i32);
        }

        return DemoApp {
            root:       root,
            counter:    0,
            chart:      chart,
            label:      counter_lbl,
        };
    }

    fn get_root(self: &DemoApp) &GuiObj {
        return &self.root as &GuiObj;
    }
}

/// 程序入口
fn main() i32 {
    // 模拟器配置
    const config: SimConfig = SimConfig.default();

    // 创建模拟器
    var sim: Simulator = Simulator.new(config);

    // 初始化
    try sim.init() catch |err| {
        printf("Failed to initialize simulator: %d
", err);
        return 1;
    };
    defer {
        sim.deinit();
    }

    // 创建应用
    var app: DemoApp = DemoApp.create();

    // 运行
    sim.run(&app);

    return 0;
}
```

### 11.2 多分辨率测试

```bash
# 同时打开多个窗口测试不同分辨率
./run.sh --screen 240x320 --scale 3 &    # 小型竖屏
./run.sh --screen 480x320 --scale 2 &    # 横屏
./run.sh --screen 800x480 --scale 1 &    # 大屏
```

---

## 附录: 快速参考

### 快捷键

| 按键 | 功能 |
|------|------|
| `ESC` | 退出模拟器 |
| `F1` | 截图 (保存为 BMP) |
| `F2` | 开始/停止输入录制 |
| `F11` | 全屏切换 |
| `F12` | 性能分析器显示开关 |
| `鼠标左键` | 模拟触摸按下/释放 |
| `鼠标拖拽` | 模拟触摸滑动 |
| `鼠标滚轮` | 模拟编码器旋转 |
| `鼠标中键` | 模拟编码器按下 |
| `方向键` | 导航焦点 |
| `Enter` | 确认/点击 |
| `Tab` | 下一个焦点 |

### 环境变量

| 变量 | 说明 | 示例 |
|------|------|------|
| `SDL_VIDEODRIVER` | 视频驱动 | `x11`, `wayland` |
| `SDL_RENDER_DRIVER` | 渲染驱动 | `opengl`, `software` |
| `UYAGUI_SCALE` | 默认缩放 | `2` |
| `UYAGUI_FPS` | 默认帧率 | `60` |
| `UYAGUI_THEME` | 默认主题 | `light`, `dark` |

### 依赖安装

```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-image-dev

# Fedora/RHEL
sudo dnf install SDL2-devel SDL2_image-devel

# Arch Linux
sudo pacman -S sdl2 sdl2_image

# macOS
brew install sdl2 sdl2_image
```

---

*文档结束 - 最后更新: 2026-04-24*
