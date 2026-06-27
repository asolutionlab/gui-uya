# UyaGUI - 详细开发 TODO 文档

> 版本: v0.1.0  
> 日期: 2026-04-26  
> 状态: Phase 5 仓库内优化/测试闭环已实现（目标板兼容待实机）

> 说明: 本文中的类型与函数签名默认以当前 Uya 语法为准；涉及未来 API 形态时，会显式写成“提案”而不是直接当成现行语法。

---

## 项目总览

| 阶段 | 状态 | 预计工期 | 实际工期 |
|------|------|---------|---------|
| Phase 0: 基础设施 | 已实现 | 3 周 | - |
| Phase 1: 核心系统 | 已实现（最小可编译/可测试基线） | 4 周 | - |
| Phase 2: 渲染引擎 | 已实现（最小可编译/可测试基线） | 3 周 | - |
| Phase 3: 组件库 | 已实现（最小可编译/可测试基线） | 3 周 | - |
| Phase 4: 高级特性 | 已实现（最小可编译/可测试基线） | 2 周 | - |
| Phase 5: 优化与测试 | 基本完成（仓库内可验证项已完成；实机兼容待硬件） | 3 周 | - |
| Phase 6: 文档与示例 | 进行中（文档/示例基线已完成，发布动作待收尾） | 2 周 | - |
| **总计** | | **20 周** | |

## 当前实现快照

- 2026-04-29 已补公共函数文档注释闭环: 为 `gui/**/*.uya` 的公开导出函数补齐源码 `///` 注释，并增强 `tools/gen_gui_api_docs.sh` 让 `make docs-api` 在 `docs/gui_uya_api_reference.md` 中同时展示函数签名与说明文字。
- 2026-04-29 已补 Week 17 Day 3-4 集成测试闭环: 新增 `tests/test_integration.uya`，覆盖 10+ 页面复杂场景导航回路、`24h` 级虚拟时钟 retained 长稳、重复场景批量渲染 RSS 增长 smoke，以及多线程事件注入后串行分发一致性；当前 `make test` 稳定通过 `183` 个 GUI/runtime 测试与 `31` 个 render 测试。
- 2026-04-29 已补 Week 17 Day 1-2 单元测试矩阵: 扩展 `tests/{test_rect,test_color,test_event,test_anim,test_widgets}.uya`，补齐 Rect / Color / 事件 / 动画 / 关键组件状态机与事件路径的高密度 case matrix；当前 `make test` 稳定通过 `179` 个 GUI/runtime 测试与 `31` 个 render 测试。
- 2026-04-29 已补 benchmark snapshot / threshold 闭环: `benchmarks/core_bench.uya` 新增 `empty frame` / `fullscreen`，仓库新增 `src/gui/benchmarks/phase5_bench_baseline.json`、`tools/check_gui_bench.py`、`make bench-json`、`make bench-verify`，CI 现默认校验 benchmark 回归阈值。
- 2026-04-29 已补 dashboard 同场景完整对照报告: 新增最小 `apps/dashboard_compare_main.uya` + `sim/dashboard_compare.uya` Uya runner、`tools/dashboard_compare_report.py` 与增强后的 `tools/lvgl_compare/dashboard.c`，`make dashboard-compare-report` 现会输出 `build/dashboard_compare/dashboard_compare_report.{md,json}`，包含帧耗时、启动时间、最大 RSS 与二进制体积对比。
- 2026-04-26 当前工作区已完成 Phase 4 最小基线: 在 Phase 3 组件库基础上，新增 `anim/{easing,tween,timeline}.uya`、`platform/tick.uya`、`res/fs.uya`，并增强 `platform/{disp,indev}.uya`、`res/cache.uya` 与 `widget/page.uya`，补齐页面导航、资源异步入口、输入校准/管理与平台 profile。
- 2026-04-26 当前 `make test` / `make build` / `make bench` / `make bench-report` / `make docs-api` 可用；默认 smoke 入口已切换为 `apps/phase6_smoke.uya`，对应 `examples/phase6_smoke.uya`、`tests/test_phase6_examples.uya` 与 Phase 4/5/6 demo 已接入。
- 2026-04-26 已完成 Phase 5 仓库内闭环: 优化 `core/{color,dirty_region}.uya`、`platform/disp.uya`、`render/{ctx,img,batch,gpu,zerocopy}.uya` 与 `res/cache.uya`，新增 `tests/test_phase5_runtime.uya`、旋转图像回归、CI benchmark、`build/phase5_bench.txt` 与 [gui_uya_phase5_report.md](./gui_uya_phase5_report.md)。
- 2026-04-26 已完成 Phase 6 第一轮仓库内闭环: 新增 `phase6_smoke`、`tests/test_phase6_examples.uya`、`examples/demo_{clock,music,settings,dashboard,game,perf}.uya`，补齐快速入门/API 索引/主题/性能/移植/架构文档，并新增 [gui_uya_phase6_report.md](./gui_uya_phase6_report.md)。
- 2026-04-26 已补文字渲染效果对比基线: 新增 `make text-compare`、`apps/text_render_compare.uya`、`examples/text_render_compare.uya` 与 [gui_uya_text_vs_lvgl.md](./gui_uya_text_vs_lvgl.md)，完成仓库内可复现样张；完整帧率/内存/体积对比仍待后续补齐。
- 2026-04-26 已补一条可运行的 LVGL 对照样张路径: 新增 `make lvgl-text-compare`，会用官方 LVGL + Tiny TTF 生成 `build/text_compare/lvgl_text_render_samples.bmp`，便于和当前 UyaGUI 样张直接比对。
- 2026-04-26 已补 dashboard 同场景 LVGL 性能对照入口: 新增 `make lvgl-dashboard-compare` 与 `make dashboard-compare`，Uya 侧 dashboard 默认走 retained 对象树，LVGL 侧用官方 LVGL + SDL + Tiny TTF 生成 `build/dashboard_compare/lvgl_dashboard.bmp`，并输出固定 `640x480`、14px 字体下的平均刷新耗时。
- 2026-04-26 已把默认 widget 文本接到 GUI 内部可缩放字体链路: `render/font.uya` 新增 GUI 层 DPI 逻辑字号换算、内嵌 `DejaVuSans` UI 字体兜底缓存、系统 CJK `TTC` 候选探测与 TTF 缺字回退；`widget/base.uya` 现会按 `font_size` 优先解析系统 UI/CJK TTF/TTC，不可用时再回退到内嵌 UI 字体，最后才落到内置 `5x7/8x8` 保底字形。
- Phase 1 已落地模块仍完整可见: `style/*`、`theme`、`event_dispatch`、`platform/indev`、`layout/*`、`dirty_region`、`benchmarks/core_bench.uya`、`examples/phase1_smoke.uya`。
- 以下 Phase 2 条目中的 `[x]` 表示“代码/接口已经写出或已有测试草案”，不代表当前工作区已经恢复绿色构建。
- 仍未开始或明显不足: 完整图片解码链路、硬件 GPU / DMA 后端实装、更多 LVGL / 真机场景对照、目标板/显示实机兼容验证、正式发布动作。

---

## Phase 0: 基础设施 (Week 1-3)

> 注: 当前仓库已完成 Phase 0 的可编译可测试版本；除 `Rect.union_rect` 这类仍受语言关键字限制的命名点外，大部分编译器相关历史绕法已同步撤销，例如 `EventData` union 载荷、`obj_pool_new<T>()`、接口回调绑定、直接结构体比较与相对 `-o` 输出路径。少量组合路径在真实 GUI 构建里仍保留兼容层，例如 `EventOption` 出队、`ByteSlice` 返回值与 `Chart.add_point()` 的逐句调用。相关编译器修复清单见 [gui_uya_compiler_fixlist.md](./gui_uya_compiler_fixlist.md)。

### Week 1: 项目搭建与环境配置

#### Day 1-2: 项目初始化
- [x] 创建 git 仓库和目录结构
  - [x] `mkdir -p gui/{core,render,widget,layout,anim,style,res,platform,tests,benchmarks,examples}`
  - [x] 初始化 `uya.toml` 项目配置文件
  - [x] 设置 CI/CD 工作流 (GitHub Actions)
- [x] 配置开发环境
  - [x] 安装 Uya 编译器 v0.1.0+
  - [x] 配置 VS Code / Cursor 编辑环境
  - [x] 安装 uyaFmt 格式化工具（当前阶段不需要，保持可选）
  - [x] 配置 git hooks (pre-commit 格式化)
- [x] 编写构建脚本
  - [x] `Makefile` - 定义 build/test/bench/clean 目标
  - [x] 支持 ARM Cortex-M C99 交叉编译交接产物
  - [x] 调试/发布模式配置

#### Day 3-4: 基础类型与工具函数
- [x] `core/point.uya` - 2D点/向量基础
  - [x] `struct Point { x: i16, y: i16 }`
  - [x] 结构体方法: `add`, `sub`, `scale`, `distance`, `lerp`
  - [x] 常量: `POINT_ZERO`, `POINT_ONE`
- [x] `core/rect.uya` - 矩形区域运算
  - [x] `struct Rect { x: i16, y: i16, w: u16, h: u16 }`
  - [x] 结构体方法: `is_empty`, `contains`, `intersect`, `union`, `inflate`, `offset`
  - [x] 实现 `Copy` trait (Uya 中通过 `Copy` 标记)
  - [x] 说明: 当前实现使用 `union_rect` / `rect_union` 规避关键字冲突
- [x] `core/color.uya` - 颜色空间
  - [x] `struct Color { r: u8, g: u8, b: u8, a: u8 }`
  - [x] 宏 `COLOR(hex)` 编译期颜色常量
  - [x] 结构体方法: `blend`, `to_rgb565`, `to_argb8888`, `lerp`
  - [x] 预定义颜色常量: `BLACK`, `WHITE`, `RED`, `GREEN`, `BLUE`, `TRANSPARENT`
- [x] `core/size.uya` - 尺寸类型
  - [x] `struct Size { w: u16, h: u16 }`
  - [x] 结构体方法: `is_empty`, `area`, `aspect_ratio`

#### Day 5: 单元测试框架
- [x] 编写 `tests/test_utils.uya`
  - [x] `assert_eq` 泛型断言宏
  - [x] `assert_near` 浮点比较
  - [x] `test_suite` 测试套件宏
- [x] 为核心类型编写单元测试
  - [x] `test_rect.uya` - 矩形运算测试 (20+ 用例)
  - [x] `test_color.uya` - 颜色混合测试 (10+ 用例)

### Week 2: 内存管理与对象池

#### Day 1-2: 位图分配器
- [x] `core/bitmap.uya` - 位图分配器
  - [x] `struct BitmapAllocator`
  - [x] 结构体方法: `alloc`, `free`, `is_used`, `find_first_zero`
  - [x] 使用 `ctz` 指令优化查找 (O(1))
  - [x] 线程安全: `atomic` 操作
- [x] 单元测试
  - [x] 分配/释放循环测试
  - [x] 并发分配测试

#### Day 3-4: 内存池
- [x] `res/pool.uya` - 内存池
  - [x] `struct MemPool` - 固定大小块分配
  - [x] `struct PoolManager` - 多级内存池管理
  - [x] 泛型接口: `TypedPool<T: Sized>`
  - [x] 结构体方法: `alloc`, `free`, `used_count`, `free_count`
- [x] `res/buf.uya` - 缓冲区管理
  - [x] `struct Buffer` - 动态缓冲区
  - [x] 结构体方法: `resize`, `clear`, `append`
  - [x] `Slice<T>` 类型安全切片
- [x] 单元测试
  - [x] 内存池压力测试 (10000+ 次分配/释放)
  - [x] 内存碎片测试
  - [x] 边界测试

#### Day 5: 对象池
- [x] `core/obj_pool.uya` - GUI 对象池
  - [x] `struct ObjPool<T: IGuiObj>` - 泛型对象池
  - [x] 结构体方法: `alloc_obj`, `free_obj`, `get_obj`, `foreach`
  - [x] 对象槽位状态管理 (Free/Used/Recycling)
- [x] 性能基准测试
  - [x] 对象创建延迟测试
  - [x] 与裸 malloc/free 对比

### Week 3: 对象系统核心

#### Day 1-2: 基础对象
- [x] `core/obj.uya` - GuiObj 核心
  - [x] 定义 `IGuiObj` 接口
    - [x] `fn type_id(self: &Self) u32`
    - [x] `fn parent(self: &Self) &GuiObj`
    - [x] `fn area(self: &Self) Rect`
    - [x] `fn set_area(self: &Self, r: Rect) void`
    - [x] `fn screen_area(self: &Self) Rect`
    - [x] `fn invalidate(self: &Self) void`
    - [x] `fn visible(self: &Self) bool`
    - [x] `fn layout(self: &Self, available: Rect) void`
    - [x] `fn render(self: &Self, ctx: &RenderCtx) void`
    - [x] `fn handle_event(self: &Self, evt: &Event) bool`
  - [x] 定义 `struct GuiObj: IGuiObj`
    - [x] 树形结构字段 (parent, children 链表)
    - [x] 几何字段 (x, y, w, h)
    - [x] 标志字段 (ObjFlags 位域)
    - [x] 样式引用
  - [x] 结构体方法实现
    - [x] `fn default() GuiObj` - 默认构造
    - [x] `fn drop(self: &GuiObj) void` - RAII 析构
    - [x] `fn screen_area(self: &GuiObj) Rect` - 绝对坐标计算
    - [x] `fn invalidate(self: &GuiObj) void` - 脏区域传播
    - [x] `fn move_to(self: &GuiObj, x: i16, y: i16) void`
    - [x] `fn set_size(self: &GuiObj, w: u16, h: u16) void`
- [x] `IContainer` 接口定义
  - [x] `fn add_child(self: &Self, child: &GuiObj) void`
  - [x] `fn remove_child(self: &Self, child: &GuiObj) void`
  - [x] `fn child_count(self: &Self) i32`
  - [x] `fn child_at(self: &Self, index: i32) &GuiObj`

#### Day 3: 对象树管理
- [x] `core/obj_tree.uya` - 对象树操作
  - [x] `struct ObjTree`
  - [x] 结构体方法:
    - [x] `attach(parent_idx, child_idx)` - 附加子对象
    - [x] `detach(child_idx)` - 分离子对象
    - [x] `foreach_child(parent_idx, callback)` - 遍历子对象
    - [x] `post_order(root_idx, callback)` - 后序遍历
    - [x] `pre_order(root_idx, callback)` - 前序遍历
    - [x] `find_at_point(root_idx, point)` - 命中测试
    - [x] `bubble_path(target_idx)` - 冒泡路径构建
- [x] 单元测试
  - [x] 树的构建/销毁
  - [x] 遍历顺序验证
  - [x] 深度/广度测试

#### Day 4-5: 事件系统基础
- [x] `core/event.uya` - 事件定义
  - [x] `enum EventType: u8` - 事件类型枚举
  - [x] `enum InputDev: u8` - 输入设备类型
  - [x] `struct Point` - 坐标点
  - [x] `struct Event` - 事件结构体
    - [x] 基础字段 (type, target, timestamp)
    - [x] `union EventData` 载荷字段 (point/key/value)
    - [x] 传播控制 (stop_bubble, handled)
  - [x] `struct EventQueue` - 环形缓冲区队列
    - [x] `fn push(self, evt) bool` - 入队
    - [x] `fn pop(self: &Self) EventOption` - 出队
    - [x] `fn is_empty(self) bool`
    - [x] `fn is_full(self) bool`
- [x] 手势识别器基础
  - [x] `struct GestureDetector`
  - [x] 结构体方法: `process_touch`

---

## Phase 1: 核心系统 (Week 4-7)

> 注: 当前仓库已完成 Phase 1 的最小可编译/可测试基线，`style`、`theme`、`event_dispatch`、`layout`、`dirty_region` 与 smoke/bench 骨架均已落地；但不少条目仍属于 MVP 范围，后续重点是补齐渲染能力、扩展测试覆盖、完善布局与事件边界行为。

### Week 4: 样式系统

> 当前状态: 已实现最小样式/主题系统，`test_style.uya` 已覆盖核心读写、merge/clone 与 builder/macro；后续继续扩充属性覆盖与组件级主题规则。

#### Day 1-2: 样式定义
- [x] `style/prop.uya` - 样式属性枚举
  - [x] 已覆盖颜色组 (BgColor, FgColor, BorderColor, ShadowColor, ...)
  - [x] 已覆盖尺寸组 (Width, Height, Padding, Margin, Min/Max, ...)
  - [x] 已覆盖字体组 (Font, FontSize)
  - [x] 已覆盖效果/布局组 (Radius, Shadow, Opacity, Layout/Flex/Grid/Overflow)
- [x] `style/style.uya` - 样式实现
  - [x] `union StyleValue` - 样式值
  - [x] `struct StyleEntry` - 属性-值对
  - [x] `struct Style` - 样式表
    - [x] 内联常用属性 (O(1)访问)
    - [x] 扩展属性数组
  - [x] 结构体方法
    - [x] `fn get(self, prop, out_value) bool` - 当前稳定接口
    - [x] `fn set(self, prop, value) void`
    - [x] `fn merge(self, other) void` - 样式合并
    - [x] `fn clone(self) Style` - 样式复制
- [x] 单元测试（核心路径）
- [x] 扩充到 50+ 属性读写/回归用例

#### Day 3-4: 主题系统
- [x] `style/theme.uya` - 主题管理
  - [x] `enum ThemeVariant` (Light/Dark/System)
  - [x] `struct Theme`
    - [x] 配色方案 (primary, secondary, accent, surface, text, ...)
    - [x] 组件样式引用 (button/panel/text/compact)
  - [x] `struct ThemeManager`
    - [x] `fn register_theme(self, theme) void`
    - [x] `fn switch_theme(self, variant) void`
    - [x] `fn current(self) &Theme`
    - [x] `fn apply_to_tree(self, root) void`
- [x] 预定义主题
  - [x] Material Light 主题
  - [x] Material Dark 主题
  - [x] 紧凑主题 (资源受限设备)
- [x] 细化到更多组件级主题映射策略

#### Day 5: 样式构建器
- [x] 实现 Fluent API 构建器
  - [x] `StyleBuilder::new()`
  - [x] `.bg_color(c)`, `.fg_color(c)`, `.border(w, c)`
  - [x] `.radius(r)`, `.padding(v)`, `.margin(v)`
  - [x] `.font(f)`, `.font_size(s)`
  - [x] `.opacity(o)`, `.shadow(x, y, blur, color)`
  - [x] `.build() -> Style`
- [x] 宏 `style(...)` 简化定义
- [x] 单元测试
- [x] 扩展更完整的声明式 DSL

### Week 5: 事件系统增强（MVP 已落地）

> 当前状态: 事件定义、手势识别、分发器、输入设备抽象与对象回调接口均已接通，现阶段主要缺的是更完整的行为覆盖与并发细节。

#### Day 1-2: 手势识别
- [x] `core/event.uya` 内嵌 `GestureDetector`
  - [x] 点击识别 (Press + Release, 时间/距离阈值)
  - [x] 长按识别 (时间阈值)
  - [x] 滑动识别 (方向阈值)
  - [x] 拖拽识别
  - [x] 双击识别
  - [x] 捏合识别 (多点触摸)
  - [x] 配置参数结构体
    - [x] `click_max_duration: u16` (默认 500ms)
    - [x] `long_press_duration: u16` (默认 800ms)
    - [x] `swipe_threshold: u16` (默认 30px)
    - [x] `double_click_interval: u16` (默认 300ms)
    - [x] `drag_threshold: u16`
    - [x] `pinch_threshold: u16`
- [x] 单元测试（点击、滑动）
- [x] 补齐长按/拖拽/双击/捏合模拟输入序列

#### Day 3-4: 事件分发器
- [x] `core/event_dispatch.uya` - 事件分发
  - [x] `struct EventDispatcher`
    - [x] 事件队列 (`EventQueue`)
    - [x] 手势识别器 (`GestureDetector`)
    - [x] 焦点管理 (`focus_obj: i32`)
    - [x] 捕获管理 (`capture_obj: i32`)
  - [x] 结构体方法
    - [x] `fn dispatch(self, evt, tree) bool` - 事件分发
    - [x] `fn find_target(self, tree, point) i32` - 命中测试
    - [x] `fn bubble_event(self, tree, path, evt) bool` - 冒泡处理
    - [x] `fn set_focus(self, obj_idx) void`
    - [x] `fn get_focus(self) i32`
    - [x] `fn process_touch(self, tree, point, is_down, timestamp) i32`
  - [x] 事件三阶段: Capture -> Target -> Bubble
- [x] 输入设备抽象
  - [x] `interface IInputDev`
  - [x] `struct TouchDriver: IInputDev`
  - [x] `struct MouseDriver: IInputDev`
  - [x] `struct KeyDriver: IInputDev`
  - [x] `struct EncoderDriver: IInputDev`
- [x] 焦点/捕获状态 atomic 化
- [x] Hover / capture 完整策略补齐

#### Day 5: 事件回调系统
- [x] 实现回调机制
  - [x] `fn on_click(self, callback) void`
  - [x] `fn on_press(self, callback) void`
  - [x] `fn on_release(self, callback) void`
  - [x] `fn on_long_press(self, callback) void`
  - [x] `fn on_value_change(self, callback) void`
  - [x] `fn on_focus(self, callback) void`
- [x] 回调上下文传递 (`user_data`)
- [x] 单元测试（点击、焦点、队列 touch 流程）
- [x] 补齐 `press` / `release` / `long_press` / `value_change` / `pinch` 全覆盖

### Week 6: 布局系统（MVP 已落地）

> 当前状态: `Flex`、`Grid`、`Absolute`、`AutoLayout` 与 `ObjTree::perform_layout()` 已打通，适合作为 Phase 1 基线；真正完整的 Flex/Grid 语义仍需继续补齐。

#### Day 1-2: Flex 布局
- [x] `layout/flex.uya` - Flex 布局引擎
  - [x] `enum FlexDir` / `Justify` / `Align`
  - [x] `struct FlexConfig`
    - [x] `direction`, `justify`, `align_items`, `align_content`
    - [x] `wrap`, `gap`, `row_gap`, `col_gap`
    - [x] `padding`
  - [x] `struct FlexLayout`
    - [x] `fn apply(self, container) void` - 执行布局
  - [x] Flex 属性
    - [x] `flex_grow`
    - [x] `align_self` (覆盖容器 `align_items`)
    - [x] `flex_shrink`
    - [x] `flex_basis`
- [x] 单元测试（基础行布局）
- [x] `RowReverse` / `ColumnReverse` 真正反向布局逻辑
- [x] `wrap` / `align_content` 生效逻辑
- [x] 扩展各种组合 30+ 用例（`test_layout.uya` 已补到 36 个 Flex 用例）

#### Day 3: Grid 布局
- [x] `layout/grid.uya` - Grid 布局
  - [x] `struct GridConfig`
    - [x] `columns: [u16: MAX_GRID_COLS]` (列宽模板)
    - [x] `rows: [u16: MAX_GRID_ROWS]` (行高模板)
    - [x] 行列间距 (`gap_x`, `gap_y`)
    - [x] `auto_flow: GridAutoFlow`
  - [x] `struct GridLayout`
    - [x] `fn apply(self, container) void`
  - [x] Grid 属性
    - [x] `grid_column`, `grid_row`
    - [x] `grid_column_span`, `grid_row_span`
    - [x] `grid_area`（`StyleBuilder.grid_area()` shorthand，映射到 `grid_column/grid_row/grid_*_span`）
- [x] 单元测试

#### Day 4: 绝对定位与混合布局
- [x] `layout/abs.uya` - 绝对定位
- [x] `layout/auto.uya` - 自动布局选择
- [x] 布局缓存系统基础
  - [x] 脏标记 (`dirty`)
  - [x] 基于对象树的递归布局刷新
  - [ ] 真正的子树级增量布局（当前仍递归整棵子树）
  - [ ] 布局约束传播
- [x] 基础性能验证（`core_bench` 已覆盖 `layout + dirty` 路径）

#### Day 5: 布局集成
- [x] 布局系统与对象树集成
- [x] `ObjTree::perform_layout()` 实现
- [x] 最小/最大尺寸约束
- [x] 溢出处理 (`visible` / `clip`)
- [ ] `scroll` 模式
- [x] 综合测试（Flex + Grid + AutoLayout 基础路径）

### Week 7: 脏区域管理

> 当前状态: 脏区域结构、合并策略与全刷启发式已可用；现已补齐仓库内渲染调度基线，包括脏区收集、受影响对象挑选、Z-order 排序、局部/全量绘制、显示提交、SDL VSync 接线与模拟器自适应帧率控制。

#### Day 1-2: 脏区域系统
- [x] `core/dirty_region.uya` - 脏区域管理
  - [x] `struct DirtyRegion`
    - [x] 精确区域数组
    - [x] 合并区域数组
    - [x] 全屏刷新阈值
  - [x] 结构体方法
    - [x] `fn add(self, rect) void` - 添加脏区域
    - [x] `fn merge(self) void` - 合并优化
    - [x] `fn should_full_refresh(self, screen) bool`
    - [x] `fn get_regions(self) DirtyRegionView`
    - [x] `fn clear(self) void`
- [x] 合并算法
  - [x] 交集/接触合并策略
  - [x] 面积启发式
  - [x] 数量限制 (超出则全屏刷新)
- [x] 单元测试
- [x] 收敛到更通用的切片/迭代视图接口（`DirtyRegionView ptr+len` + `is_empty/at/iter`）

#### Day 3-4: 渲染调度
- [x] 渲染流水线
  - [x] 收集脏区域
  - [x] 查找受影响对象
  - [x] 按 Z-order 排序
  - [x] 执行绘制
  - [x] 交换缓冲区
- [x] VSync 支持
- [x] 帧率控制 (自适应)

#### Day 5: 性能基准
- [x] 建立性能测试基线
  - [x] 空渲染帧时间
  - [x] 全屏刷新时间
  - [x] 脏区域处理时间
  - [x] 内存分配/释放时间
  - [x] 对象池/布局/事件分发时间
- [x] 编写 `benchmarks/core_bench.uya`
- [x] 固化 benchmark snapshot 与回归阈值

---

## Phase 2: 渲染引擎 (Week 8-10)

> 注: 当前仓库已完成 Phase 2 的最小可编译/可测试基线，实现了显示/帧缓冲抽象、渲染上下文与基础图元、图像与字体绘制、缓存/批处理/GPU/zero-copy 骨架，以及对应 smoke / bench / test 入口。

### Week 8: 基础渲染

#### Day 1-2: 帧缓冲抽象
- [x] `platform/disp.uya` - 显示接口
  - [x] `enum PixelFormat`
    - [x] RGB565, RGB888, ARGB8888, ARGB4444
    - [x] L8, A8, I1, I4
  - [x] `struct FrameBuffer`
    - [x] 像素数据指针、`size`、`stride`
    - [x] `format: PixelFormat`
  - [x] `struct DisplayCtx`
    - [x] 前后缓冲 (双缓冲)
    - [x] 显示帧计数
  - [x] 结构体方法
    - [x] `fn swap_buffers(self) void`
    - [x] `fn get_back_buffer(self) &FrameBuffer`
    - [x] `fn clear(self, color) void`
  - [x] 辅助函数
    - [x] `framebuffer_required_bytes`
    - [x] `framebuffer_get_pixel`
    - [x] `framebuffer_set_pixel`
    - [x] `framebuffer_clear`
- [x] 修复当前类型检查错误并接回稳定构建

#### Day 3-4: 渲染上下文
- [x] `render/ctx.uya` - 渲染上下文
  - [x] `struct RenderCtx`
    - [x] 目标帧缓冲 (`FrameBuffer`)
    - [x] 裁剪区域栈 (`clip_stack`)
    - [x] 当前脏区域 (`dirty`)
    - [x] 渲染统计 (`RenderStats`)
    - [x] 渲染模式 (`RenderMode`)
  - [x] 裁剪管理
    - [x] `fn push_clip(self, rect) void`
    - [x] `fn pop_clip(self) void`
    - [x] `fn current_clip(self) Rect`
  - [x] 基础绘制 API
    - [x] `fn draw_pixel(self, x, y, color) void`
    - [x] `fn fill_rect(self, rect, color) void`
    - [x] `fn draw_rect(self, rect, color, width) void`
    - [x] `fn draw_line(self, x1, y1, x2, y2, color, width) void`
    - [x] `fn draw_circle(self, cx, cy, r, color) void`
    - [x] `fn fill_circle(self, cx, cy, r, color) void`
    - [x] `fn fill_round_rect(self, rect, radius, color) void`
    - [x] `fn draw_round_rect(self, rect, radius, color, width) void`
    - [x] `fn draw_arc(self, cx, cy, r, start, end, width, color) void`
    - [x] `fn fill_arc(self, cx, cy, r, start, end, color) void`
  - [x] 调试辅助: `ctx_pixel()`
- [x] 基础 render 测试草案 (`test_render_ctx.uya`)
- [x] 修复当前 render 测试中的 move 语义问题

#### Day 5: 像素格式优化
- [x] 各像素格式特化读写路径
  - [ ] RGB565 快速填充 (32-bit 批量写入)
  - [x] ARGB8888 基础 Alpha 混合
  - [x] I1 单色位图绘制
  - [x] I4 4bpp 索引色
- [ ] 内联汇编优化 (关键路径)
- [x] 修复编译后补齐稳定单元测试

### Week 9: 高级绘制

#### Day 1-2: 图像绘制
- [x] `render/img.uya` - 图像处理
  - [x] `struct ImageData`
    - [x] 像素数据引用
    - [x] 宽度、高度、格式
    - [x] 引用计数
  - [x] 图像绘制方法
    - [x] `draw_image`
    - [x] `draw_image_scaled`
    - [x] `draw_image_clipped`
    - [x] `draw_image_rotated`
  - [x] 图像格式支持
    - [x] 原始像素 (RAW)
    - [ ] RLE 编码
    - [ ] BMP 解码
    - [ ] PNG 解码 (简化)
    - [ ] 自定义格式
- [x] 图像缓存 (`res/cache.uya`)
  - [x] LRU 风格缓存策略
  - [x] 引用计数管理
  - [x] 内存预算控制
- [x] 资源路径测试草案 (`test_render_assets.uya`)
- [x] 修复 move 语义后纳入稳定回归

#### Day 3: 文本渲染
- [x] `render/font.uya` - 字体引擎
  - [x] `struct Glyph`
    - [x] 字符编码
    - [x] 基础位图/几何占位数据
    - [x] 偏移、步进、尺寸
  - [x] `struct Font`
    - [x] 字体元数据 (高度、基线、行高)
    - [x] 字形查找表
    - [x] 字距调整表
  - [x] 字体绘制
    - [x] `draw_text`
    - [x] `draw_text_aligned`
    - [x] `text_width`
  - [x] 位图字体 (BMFont 兼容)
  - [x] 矢量字体 (TTF 简化光栅化)
  - [x] 内置字体 (系统字体)
- [x] 将当前几何占位字形替换为真实字库渲染
  - [x] 默认字体已支持 ASCII `5x7`、常用 CJK `8x8`、BMFont Alpha atlas 与简化矢量描边回退
  - [x] 已补 `kerning`、BMFont 解析、系统字体变体与专项渲染回归
  - [x] 已补 Uya 侧 TTF 运行时链路：`cmap4/cmap12`、`head/hhea/hmtx/maxp/loca/glyf/kern` 解析与 outline 扫描转换
  - [x] SDL2 模拟器文本显示继续复用 Uya framebuffer，不依赖 `SDL_ttf`
  - [x] 复合字形（TrueType `glyf` composite）支持
  - [x] 轻量 hinting / grid fitting 开关
  - [x] GUI 层 DPI 逻辑字号到像素高度映射
  - [x] TTF 缺字时的系统 CJK 高质量 fallback
  - [x] 更高性能 glyph cache（固定槽位缓存与命中统计）
  - [ ] CFF/OTF（Type 2 / CID-keyed）进一步支持

#### Day 4: 抗锯齿与效果
- [x] Alpha 混合优化
  - [x] 快速 Alpha 混合 (查表法)
  - [x] 预乘 Alpha
- [x] 圆角抗锯齿
- [x] 阴影效果
  - [x]  box-shadow 模拟
  - [x] 模糊阴影 (简化版)
- [x] 渐变填充
  - [x] 线性渐变
  - [x] 径向渐变

#### Day 5: 批处理渲染
- [x] `render/batch.uya` - 绘制批处理
  - [x] `struct DrawCmd` - 绘制命令
  - [x] `struct DrawBatch`
    - [x] 命令缓冲区
    - [x] 合并统计
  - [x] 优化策略
    - [x] 同色矩形合并
    - [x] 相邻水平线合并
    - [x] 按颜色排序减少状态切换
  - [x] `fn execute(self, ctx) void`
- [x] pipeline 测试草案 (`test_render_pipeline.uya`)
- [x] 修复编译后补性能对比测试

### Week 10: GPU 支持与优化

#### Day 1-2: GPU 抽象层
- [x] `render/gpu.uya` - GPU 抽象
  - [x] `interface IGpuCtx`
    - [x] `fn draw_rects(self, rects, colors, count) void`
    - [x] `fn draw_images(self, cmds, count) void`
    - [x] `fn draw_glyphs(self, cmds, count) void`
    - [x] `fn fill_rects(self, rects, colors, count) void`
    - [x] `fn sync(self) void`
  - [x] 软件回退后端 `SoftwareGpuCtx`
  - [ ] GPU 后端
    - [ ] STM32 DMA2D
    - [ ] ESP32-S3 LCD_CAM
    - [ ] NXP PXP
    - [x] 通用 OpenGL ES 2.0
  - [x] 自动检测与回退

#### Day 3-4: 零拷贝与 DMA
- [x] `render/zerocopy.uya` - 零拷贝优化
  - [x] 双缓冲管理
  - [x] 区域传输状态跟踪
  - [x] `present()` / `flush()` 基线路径
  - [ ] DMA 异步传输
  - [ ] 脏区域 DMA 传输
  - [ ] 传输完成中断
- [ ] 性能测试
  - [ ] DMA vs CPU memcpy
  - [ ] 帧完成延迟

#### Day 5: 渲染性能调优
- [x] 内联热点函数
- [x] SIMD 优化 (如果目标支持)
- [x] 查找表优化 (Alpha 混合、Sin/Cos)
- [x] 缓存友好数据布局
- [x] 修复构建后补完整渲染基准测试

---

## Phase 3: 组件库 (Week 11-13)

> 当前状态: 已实现 Phase 3 的最小可编译/可测试基线，基础组件、容器与高级组件均已落地，对应 `phase3_smoke` 与 `test_widgets` 已接入；少数“异步加载/动画”类条目当前采用占位接口或无动画实现，后续在 Phase 4 继续细化。

### Week 11: 基础组件

#### Day 1: 组件基类
- [x] `widget/base.uya` - Widget 基类
  - [x] `enum WidgetState: u8`
    - [x] Normal, Hovered, Pressed, Disabled, Focused, Checked
  - [x] `struct Widget: IStyled`
    - [x] `base: GuiObj`
    - [x] state: `WidgetState`
    - [x] anim_state: `&AnimState`
    - [x] 事件回调函数指针
  - [x] 结构体方法:
    - [x] `fn handle_input(self, evt) bool` - 状态机转换
    - [x] `fn set_state(self, state) void`
    - [x] `fn is_enabled(self) bool`
    - [x] `fn set_enabled(self, enabled) void`
    - [x] Fluent API: `.at()`, `.size()`, `.with_style()`
  - [x] `interface IStyled` 实现

#### Day 2: 按钮
- [x] `widget/btn.uya` - 按钮组件
  - [x] `enum BtnVariant: u8` (Filled/Outlined/Text/Icon/IconText)
  - [x] `struct Button`
    - [x] `widget: Widget`
    - [x] variant, label, icon
    - [x] click_ctx (闭包环境)
  - [x] 结构体方法:
    - [x] `fn filled(text) Button` - 构造宏
    - [x] `fn outlined(text) Button`
    - [x] `fn text_only(text) Button`
    - [x] `fn with_icon(icon, text) Button`
    - [x] `fn render(self, ctx) void`
    - [x] `fn on_click(self, callback) &Button`

#### Day 3: 标签
- [x] `widget/lbl.uya` - 标签组件
  - [x] 文本对齐枚举（已恢复 `TextAlign` 命名，覆盖 9 种对齐）
  - [x] `enum TextOverflow: u8` (Clip/Ellipsis/Wrap/Scroll)
  - [x] `struct Label`
    - [x] `widget: Widget`
    - [x] text, text_len, align, overflow
    - [x] 行缓存 (layout_dirty 标记)
  - [x] 结构体方法:
    - [x] `fn new(text) Label`
    - [x] `fn set_text(self, text) &Label`
    - [x] `fn set_align(self, align) &Label`
    - [x] `fn render(self, ctx) void`
    - [x] `fn recalc_layout(self, ctx) void` (缓存策略)

#### Day 4: 图像组件
- [x] `widget/img.uya` - 图像组件
  - [x] `enum ImageScale: u8` (Fit/Fill/Stretch/Center/Tile)
  - [x] `struct Image`
    - [x] `widget: Widget`
    - [x] src: `&ImageData`
    - [x] scale_mode
    - [x] corner_radius
  - [x] 结构体方法:
    - [x] `fn from_data(data) Image`
    - [x] `fn from_file(path) Image` (当前为占位异步入口)
    - [x] `fn set_source(self, src) &Image`
    - [x] `fn render(self, ctx) void`

#### Day 5: 滑块与开关
- [x] `widget/slider.uya` - 滑块
  - [x] `struct Slider`
    - [x] `widget: Widget`
    - [x] min, max, value, step
    - [x] orientation (Horizontal/Vertical)
    - [x] track_color, progress_color, thumb_color
  - [x] 结构体方法:
    - [x] `fn new(min, max, value) Slider`
    - [x] `fn set_value(self, v) void`
    - [x] `fn get_value(self) i32`
    - [x] `fn on_change(self, cb) &Slider`
    - [x] `fn render(self, ctx) void`
    - [x] `fn handle_drag(self, evt) void`
- [x] `widget/switch.uya` - 开关
  - [x] `struct Switch`
    - [x] `widget: Widget`
    - [x] checked: bool
    - [x] anim_progress: f32 (切换动画占位状态)
  - [x] 结构体方法:
    - [x] `fn new(checked) Switch`
    - [x] `fn toggle(self) void`
    - [x] `fn render(self, ctx) void`

### Week 12: 容器与列表

#### Day 1-2: 页面与面板
- [x] `widget/page.uya` - 页面容器
  - [x] `struct Page: IContainer`
    - [x] `widget: Widget`
    - [x] scrollable: bool
    - [x] scroll_x, scroll_y
    - [x] content_size
  - [x] 结构体方法:
    - [x] `fn new(name) Page`
    - [x] `fn add(self, child) &GuiObj`
    - [x] `fn remove(self, child) void`
    - [x] `fn set_layout(self, layout) void`
    - [x] `fn scroll_to(self, x, y) void`
    - [x] `fn render(self, ctx) void`
- [x] `widget/panel.uya` - 面板
  - [x] `struct Panel: IContainer`
    - [x] `widget: Widget`
  - [x] 支持背景、边框、阴影、圆角

#### Day 3-4: 列表视图
- [x] `widget/list.uya` - 列表组件
  - [x] `struct ListView: IContainer, IScrollable, ISelectable`
    - [x] `widget: Widget`
    - [x] items: `[ListItem: MAX_ITEMS]`
    - [x] item_count, selected_idx
    - [x] scroll_y, content_h
    - [x] item_height
  - [x] 接口实现:
    - [x] `IScrollable`: scroll_to, scroll_by, content_size
    - [x] `ISelectable`: select, selected, select_next, select_prev
  - [x] 结构体方法:
    - [x] `fn add_item(self, text) void`
    - [x] `fn remove_item(self, index) void`
    - [x] `fn clear(self) void`
    - [x] `fn render(self, ctx) void` (虚拟滚动优化)
- [x] 单元测试 (1000+ 项性能的最小基线/行为回归已接入)

#### Day 5: 网格视图
- [x] `widget/grid_view.uya` - 网格视图
  - [x] `struct GridView: IContainer`
    - [x] `widget: Widget`
    - [x] columns: u8
    - [x] item_size: Size
    - [x] items 数组
  - [x] 虚拟滚动支持
  - [x] 单元测试

### Week 13: 高级组件

#### Day 1-2: 图表组件
- [x] `widget/chart.uya` - 图表
  - [x] `enum ChartType: u8` (Line/Bar/Area/Scatter/Pie)
  - [x] `struct Chart`
    - [x] `widget: Widget`
    - [x] data_points: `[i32: MAX_POINTS]`
    - [x] point_count, min_val, max_val
    - [x] chart_type
  - [x] 结构体方法:
    - [x] `fn add_point(self, value) &Chart`
    - [x] `fn clear(self) &Chart`
    - [x] `fn render_line(self, ctx) void`
    - [x] `fn render_bar(self, ctx) void`
    - [x] `fn render_area(self, ctx) void`
    - [x] `fn render(self, ctx) void`
- [x] 单元测试

#### Day 3: 画布组件
- [x] `widget/canvas.uya` - 画布
  - [x] `struct Canvas`
    - [x] `widget: Widget`
    - [x] draw_buffer: `&FrameBuffer`
    - [x] dirty: bool
  - [x] 结构体方法:
    - [x] `fn begin_draw(self) &RenderCtx` - 获取绘制上下文
    - [x] `fn end_draw(self) void` - 标记需要刷新
    - [x] `fn clear(self, color) void`
    - [x] `fn render(self, ctx) void` (将缓冲复制到屏幕)

#### Day 4-5: 自定义组件示例
- [x] `examples/custom/gauge.uya` - 仪表盘示例
  - [x] 演示如何实现自定义组件
  - [x] 使用所有核心特性
- [x] `examples/custom/keyboard.uya` - 虚拟键盘
- [x] 组件集成测试

---

## Phase 4: 高级特性 (Week 14-15)

> 当前状态: 已实现 Phase 4 的最小可编译/可测试基线，动画系统、页面导航过渡、资源/文件系统抽象、输入增强与 tick/disp 平台抽象均已落地；对应 `phase4_smoke`、`test_anim`、`test_phase4_io`、扩展后的 `test_input_dev` / `test_widgets` 与 benchmark 已接入。

### Week 14: 动画系统

#### Day 1-2: 补间动画
- [x] `anim/tween.uya` - 补间动画
  - [x] `enum EasingType: u8` (25+ 缓动函数)
  - [x] `enum AnimProp: u8` (X, Y, Width, Height, Opacity, ...)
  - [x] `union AnimValue` (i32, f32, Color)
  - [x] `struct Tween`
    - [x] target, property, from, to
    - [x] duration, delay, easing
    - [x] 运行时状态 (elapsed, running, reversed)
    - [x] repeat, yoyo
  - [x] 结构体方法:
    - [x] `fn new(target, prop, to, duration) Tween`
    - [x] `fn with_easing(self, easing) Tween`
    - [x] `fn with_delay(self, delay) Tween`
    - [x] `fn yoyo(self, enabled) Tween`
    - [x] `fn repeat(self, count) Tween`
    - [x] `fn on_complete(self, cb) Tween`
    - [x] `fn on_update(self, cb) Tween`
    - [x] `fn start(self) void`
    - [x] `fn update(self, dt) bool`
    - [x] `fn apply_value(self, value) void`
- [x] `anim/easing.uya` - 缓动函数实现
  - [x] 线性、二次、三次、四次
  - [x] Sine、Expo、Elastic、Back、Bounce
  - [x] 每种都有 In/Out/InOut 变体

#### Day 3: 动画管理器
- [x] `anim/timeline.uya` - 动画管理
  - [x] `struct AnimManager`
    - [x] tweens: `[Tween: MAX_ANIMATIONS]`
    - [x] active: `[bool: MAX_ANIMATIONS]`
  - [x] 结构体方法:
    - [x] `fn start_tween(self, tween) !i32`
    - [x] `fn stop_tween(self, id) void`
    - [x] `fn stop_all(self) void`
    - [x] `fn update_all(self, dt) void`
    - [x] `fn is_animating(self) bool`
  - [x] `@async_fn fn animation_loop(self: &Self, tick_ms: u32) Future<!void>`
- [x] 性能测试 (32 个并发动画)

#### Day 4-5: 页面过渡
- [x] 页面切换动画
  - [x] Slide (Left/Right/Up/Down)
  - [x] Fade
  - [x] Scale
  - [x] Custom
- [x] 导航栈管理
  - [x] Push/Pop 动画
  - [x] 手势返回 (Edge Swipe)

### Week 15: 资源管理与输入

#### Day 1-2: 资源系统
- [x] `res/cache.uya` - 图像缓存
  - [x] LRU 策略
  - [x] 引用计数
  - [x] 内存预算
  - [x] 异步加载
- [x] `res/fs.uya` - 文件系统抽象
  - [x] `interface IFileSystem`
  - [x] 内置 ROM FS
  - [x] FAT FS 适配
  - [x] 异步读取接口

#### Day 3-4: 输入系统完善
- [x] `platform/indev.uya` - 输入设备
  - [x] 触摸屏驱动
    - [x] 校准算法 (4点/9点)
    - [x] 中值滤波
    - [x] 手势识别集成
  - [x] 鼠标驱动
  - [x] 键盘驱动
  - [x] 编码器驱动
- [x] 输入设备管理器
  - [x] 多设备同时支持
  - [x] 设备热插拔

#### Day 5: 平台抽象
- [x] `platform/tick.uya` - 时钟接口
  - [x] `fn get_tick_ms() u32`
  - [x] `fn get_tick_us() u64`
  - [x] `fn sleep_ms(ms) void`
- [x] `platform/disp.uya` - 显示驱动接口
  - [x] 常见驱动适配
    - [x] ILI9341 (320x240)
    - [x] ST7789 (240x320)
    - [x] ST7735 (128x160)
    - [x] SSD1963 (480x272/800x480)
    - [x] RA8875 (800x480)

---

## Phase 5: 优化与测试 (Week 16-18)

> 当前状态: 已完成当前仓库内可直接验证的 Phase 5 子项，包括脏区/批处理/加速渲染/零拷贝调度优化、缓存与内存可观测性、运行时回归/交错场景测试、benchmark/report 与 CI 补齐。仍待硬件环境完成的条目会继续保留未勾选状态。

### Week 16: 性能优化

#### Day 1-2: 渲染优化
- [x] 脏矩形合并算法优化
- [x] 绘制批处理增强
- [x] GPU 加速集成（软件 GPU 批执行路径）
- [x] DMA 零拷贝（部分刷新/传输队列基线，待真实 DMA 后端）
- [ ] 目标: 60fps @ 480x320 (120MHz MCU)

#### Day 3-4: 内存优化
- [x] 内存占用分析
- [ ] 结构体字段对齐优化
- [ ] 位域使用
- [ ] 只读数据放 Flash
- [ ] 目标: 空框架 < 8KB RAM

#### Day 5: 启动优化
- [ ] 延迟初始化
- [ ] 预计算查找表
- [ ] 编译期常量展开
- [ ] 目标: 启动 < 10ms

### Week 17: 全面测试

#### Day 1-2: 单元测试
- [x] Phase 5 运行时回归用例 (`tests/test_phase5_runtime.uya`)
- [x] 组件状态/事件/缓存/渲染加速回归
- [x] 核心模块测试（仓库内高密度 case matrix 已补齐；精确覆盖率量化待后续 coverage 工具接入）
  - [x] Rect 运算 (50+ 用例)
  - [x] Color 混合 (30+ 用例)
  - [x] 事件系统 (40+ 用例)
  - [x] 布局引擎 (50+ 用例)
  - [x] 动画系统 (30+ 用例)
- [x] 组件测试
  - [x] 关键组件回归矩阵（Button / Label / Image / Slider / Switch / Page / List / Grid / Chart / Canvas / Panel）
  - [x] 状态转换测试
  - [x] 事件处理测试

#### Day 3-4: 集成测试
- [x] 完整 UI 场景测试
  - [x] 10+ 页面复杂应用
  - [x] 长时间运行稳定性 (`24h` 虚拟时钟 retained 回归)
  - [x] 内存泄漏检测（RSS 增长 smoke）
- [x] 并发安全测试
  - [x] 多线程事件注入
  - [x] 动画 + 输入并发

#### Day 5: 性能基准
- [x] 与 LVGL 对比测试
  - [x] 文字渲染效果对比样张与报告
  - [x] dashboard 相同场景帧率对比入口
  - [x] 内存占用对比
  - [x] 启动时间对比
  - [x] 代码体积对比
- [x] 编写性能报告

### Week 18: 兼容性测试

#### Day 1-2: 目标平台测试
- [ ] STM32F4xx (Cortex-M4, 168MHz)
- [ ] STM32H7xx (Cortex-M7, 480MHz)
- [ ] ESP32-S3 (Xtensa LX7, 240MHz)
- [ ] Raspberry Pi Pico (RP2040, 133MHz)
- [ ] RISC-V GD32V (108MHz)

#### Day 3-4: 显示适配测试
- [ ] 128x160 (1.8" SPI)
- [ ] 240x320 (3.2" SPI)
- [ ] 320x480 (3.5" SPI)
- [ ] 480x320 (3.5" Parallel)
- [ ] 800x480 (5" RGB)

#### Day 5: 修复与稳定
- [x] Bug 修复
- [x] 回归测试
- [x] 性能微调

---

## Phase 6: 文档与示例 (Week 19-20)

> 当前状态: 已完成当前仓库内可直接落地的文档、示例与演示基线，包含快速入门、API 索引生成、主题/性能/移植/架构文档，以及 6 个 demo 与 `phase6_smoke` 聚合入口。正式发布动作与硬件验证仍保留未完成。

### Week 19: 文档完善

#### Day 1-2: API 文档
- [x] 自动生成 API 文档
- [x] 每个公共函数文档注释
- [x] 使用示例代码
- [x] 类型说明

#### Day 3-4: 开发指南
- [x] 快速入门指南
- [x] 自定义组件教程
- [x] 主题定制指南
- [x] 性能优化指南
- [x] 移植指南

#### Day 5: 架构文档
- [x] 系统架构图
- [x] 数据流图
- [x] 模块依赖图
- [x] 内存布局图

### Week 20: 示例与发布

#### Day 1-3: 示例应用
- [x] `examples/demo_clock.uya` - 时钟应用
- [x] `examples/demo_music.uya` - 音乐播放器
- [x] `examples/demo_settings.uya` - 设置界面
- [x] `examples/demo_dashboard.uya` - 数据仪表板
- [x] `examples/demo_game.uya` - 简单游戏

#### Day 4: 性能演示
- [x] 帧率计数器
- [x] 内存使用显示
- [x] 渲染统计

#### Day 5: 发布准备
- [ ] 版本标记 v1.0.0
- [x] 发布说明
- [ ] 二进制分发
- [ ] 社区公告

---

## 持续集成与质量保障

### 自动化测试
- [x] 每次提交自动运行单元测试
- [x] 每晚集成测试
- [x] 每周性能基准
- [ ] 代码覆盖率报告

### 代码质量
- [ ] 代码格式化 (uyaFmt)
- [ ] 静态分析
- [ ] 文档完整性检查
- [ ] API 兼容性检查

### 性能监控
- [x] 帧率追踪
- [x] 内存使用追踪
- [x] 渲染时间分析
- [x] 回归检测

---

## 风险与缓解

| 风险 | 可能性 | 影响 | 缓解措施 |
|------|--------|------|---------|
| Uya 编译器 Bug | 中 | 高 | 及时反馈社区，准备 C 回退方案 |
| 目标平台内存不足 | 中 | 高 | 分级组件，可选特性编译开关 |
| GPU 驱动不稳定 | 低 | 中 | 完善的软件回退路径 |
| 性能未达预期 | 中 | 高 | 持续 Profiling，算法优化 |
| 编译时间过长 | 低 | 低 | 增量编译，模块化设计 |

---

## 度量指标

### 代码度量
- 总代码行数目标: < 15000 行 (不含测试和示例)
- 测试代码行数: > 5000 行
- 文档行数: > 3000 行
- 公共 API 数量: ~200 个函数

### 质量度量
- 单元测试覆盖率: > 90%
- 集成测试场景: > 20 个
- 文档覆盖率: 100% 公共 API
- 零编译警告

### 性能度量
- 空框架内存: < 8KB
- 启动时间: < 10ms
- 渲染帧率: 60fps @ 480x320
- 最大组件数: 256+
- 最大动画数: 32 (并发)

---

*TODO 文档结束 - 最后更新: 2026-04-26*
