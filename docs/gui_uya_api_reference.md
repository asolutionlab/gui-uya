# UyaGUI API Reference

> 该文件由 `tools/gen_gui_api_docs.sh` 从 `src/gui/**/*.uya`、`apps/**/*.uya`、`examples/**/*.uya`、`tests/**/*.uya` 自动生成。
> 内容聚焦公开符号索引，详细设计说明请结合 `docs/gui_uya_*.md` 系列文档阅读。

## `apps/dashboard_compare_main.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行 dashboard 对照程序。

## `apps/phase0_smoke.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行当前模块的默认流程。

## `apps/phase1_smoke.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行当前模块的默认流程。

## `apps/phase2_smoke.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行当前模块的默认流程。

## `apps/phase3_smoke.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行当前模块的默认流程。

## `apps/phase4_smoke.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行当前模块的默认流程。

## `apps/phase6_smoke.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行当前模块的默认流程。

## `apps/sim_main.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，启动 Linux 模拟器程序。

## `apps/sim_web_main.uya`

- L5: `export fn main() i32 {`

## `apps/text_render_compare.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，生成文字渲染对比样张。

## `examples/custom/gauge.uya`

- L37: `export fn run_custom_gauge_example() i32 {`
  说明: 运行 `custom_gauge_example` 示例并返回退出码。

## `examples/custom/keyboard.uya`

- L10: `export fn run_custom_keyboard_example() i32 {`
  说明: 运行 `custom_keyboard_example` 示例并返回退出码。

## `examples/demo_animation.uya`

- L606: `export struct AnimationRetained {`
- L662: `export fn animation_retained_new() AnimationRetained {`
  说明: 创建默认的 `AnimationRetained` 实例。
- L720: `export fn animation_demo_section_rect(index: i32) Rect {`
  说明: 返回 section 按钮在 demo 本地坐标系中的区域。
- L725: `export fn animation_demo_replay_rect() Rect {`
  说明: 返回 `Replay` 按钮在 demo 本地坐标系中的区域。
- L730: `export fn animation_demo_toggle_rect() Rect {`
  说明: 返回 `Play/Pause` 按钮在 demo 本地坐标系中的区域。
- L735: `export fn animation_retained_init(anim: &AnimationRetained) void {`
  说明: 初始化 `animation_retained` 的内部状态。
- L991: `export fn animation_retained_set_view(anim: &AnimationRetained, view: i32) void {`
  说明: 切换到指定动画视图。
- L1012: `export fn animation_retained_next_view(anim: &AnimationRetained) void {`
  说明: 切换到下一个动画视图。
- L1025: `export fn animation_retained_prev_view(anim: &AnimationRetained) void {`
  说明: 切换到上一个动画视图。
- L1038: `export fn animation_retained_sync(anim: &AnimationRetained) void {`
  说明: 将外部状态同步到 `animation_retained`。
- L1048: `export fn animation_retained_replay(anim: &AnimationRetained) void {`
  说明: 重播整段动画序列。
- L1061: `export fn animation_retained_toggle(anim: &AnimationRetained) void {`
  说明: 切换动画播放状态。
- L1078: `export fn animation_retained_handle_touch(anim: &AnimationRetained, evt: &Event, local: Point) bool {`
  说明: 处理动画 demo 的本地触摸事件。
- L1118: `export fn animation_retained_update(anim: &AnimationRetained, dt_ms: u32) void {`
  说明: 推进动画 demo 的时间轴。
- L1137: `export fn render_demo_animation_retained_state(anim: &AnimationRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 使用 retained 状态将 `animation` demo 渲染到指定上下文。
- L1153: `export fn render_demo_animation(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `animation` demo 渲染到指定上下文。
- L1159: `export fn run_demo_animation() i32 {`
  说明: 运行 `animation` demo 并返回退出码。

## `examples/demo_clock.uya`

- L121: `export struct ClockRetained {`
- L145: `export fn clock_retained_new() ClockRetained {`
  说明: 创建默认的 `ClockRetained` 实例。
- L239: `export fn clock_retained_sync(clock: &ClockRetained) void {`
- L284: `export fn clock_retained_init(clock: &ClockRetained) void {`
  说明: 初始化 `clock_retained` 的内部状态。
- L324: `export fn render_demo_clock_retained(clock: &ClockRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 使用 retained 状态将 `clock_retained` 渲染到指定上下文。
- L335: `export fn render_demo_clock(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `clock` demo 渲染到指定上下文。
- L341: `export fn run_demo_clock() i32 {`
  说明: 运行 `clock` demo 并返回退出码。

## `examples/demo_custom.uya`

- L103: `export struct CustomRetained {`
- L172: `export fn custom_retained_new() CustomRetained {`
  说明: 创建默认的 `CustomRetained` 实例。
- L243: `export fn custom_retained_init(custom: &CustomRetained) void {`
  说明: 初始化 `custom_retained` 的内部状态。
- L418: `export fn custom_retained_sync(`
  说明: 将外部状态同步到 `custom_retained`。
- L483: `export fn render_demo_custom_retained_state(`
  说明: 按 retained 状态参数将 `custom` demo 渲染到指定上下文。
- L504: `export fn render_demo_custom_state(`
  说明: 按显式状态参数将 `custom` demo 渲染到指定上下文。
- L520: `export fn render_demo_custom(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `custom` demo 渲染到指定上下文。
- L525: `export fn run_demo_custom() i32 {`
  说明: 运行 `custom` demo 并返回退出码。

## `examples/demo_dashboard.uya`

- L34: `export struct DashboardRetained {`
- L83: `export fn dashboard_retained_new() DashboardRetained {`
  说明: 创建默认的 `DashboardRetained` 实例。
- L101: `export fn dashboard_retained_init(dashboard: &DashboardRetained) void {`
  说明: 初始化 `dashboard_retained` 的内部状态。
- L159: `export fn render_dashboard_retained_state(dashboard: &DashboardRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16, metric: i32) void {`
  说明: 按 retained 状态渲染 `render_dashboard`。
- L174: `export fn render_demo_dashboard_state(ctx: &RenderCtx, origin_x: i16, origin_y: i16, metric: i32) void {`
  说明: 按显式状态参数将 `dashboard` demo 渲染到指定上下文。
- L236: `export fn render_demo_dashboard(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `dashboard` demo 渲染到指定上下文。
- L241: `export fn run_demo_dashboard() i32 {`
  说明: 运行 `dashboard` demo 并返回退出码。

## `examples/demo_doudizhu.uya`

- L81: `export struct DdzPageRetained {`
- L1103: `export fn doudizhu_page_retained_new() DdzPageRetained {`
- L1132: `export fn doudizhu_page_retained_init(page: &DdzPageRetained) void {`
- L1143: `export fn doudizhu_page_retained_update(page: &DdzPageRetained, now_ms: u32) void {`
- L1233: `export fn doudizhu_page_retained_autoplay_step(page: &DdzPageRetained, now_ms: u32) bool {`
- L1265: `export fn doudizhu_page_retained_toggle_autoplay(page: &DdzPageRetained) bool {`
- L1279: `export fn doudizhu_page_retained_next_wake_delay_ms(page: &DdzPageRetained, now_ms: u32) i32 {`
- L1291: `export fn doudizhu_page_retained_handle_touch(page: &DdzPageRetained, evt: &Event, local: Point) void {`
- L1350: `export fn render_demo_doudizhu_retained_state(page: &DdzPageRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
- L1359: `export fn run_demo_doudizhu() i32 {`
- L1378: `export fn run_demo_doudizhu_full_round() i32 {`

## `examples/demo_game.uya`

- L60: `export enum GameMoveDir {`
- L67: `export struct Game2048Retained {`
- L82: `export struct GamePageRetained {`
- L341: `export fn game_retained_seed(game: &Game2048Retained, seed: u32) void {`
  说明: 为 `game_retained` 设置确定性的随机种子。
- L353: `export fn game_retained_reset(game: &Game2048Retained) void {`
  说明: 重置 `game_retained` 的运行时状态。
- L406: `export fn game_retained_handle_touch(game: &Game2048Retained, evt: &Event, local: Point) void {`
  说明: 处理 `game_retained` 的触摸输入。
- L478: `export fn game_retained_move(game: &Game2048Retained, dir: GameMoveDir) bool {`
  说明: 对 `game_retained` 执行一次移动操作。
- L749: `export fn game_page_retained_new() GamePageRetained {`
  说明: 创建默认的 `GamePageRetained` 实例。
- L781: `export fn game_page_retained_init(page_state: &GamePageRetained, game: &Game2048Retained) void {`
  说明: 初始化 `game_page_retained` 的内部状态。
- L894: `export fn game_page_retained_sync(page_state: &GamePageRetained, game: &Game2048Retained) void {`
  说明: 将外部状态同步到 `game_page_retained`。
- L915: `export fn render_demo_game_retained_state(page_state: &GamePageRetained, game: &Game2048Retained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 按 retained 状态参数将 `game` demo 渲染到指定上下文。
- L925: `export fn render_demo_game_state(game: &Game2048Retained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 按显式状态参数将 `game` demo 渲染到指定上下文。
- L1013: `export fn render_demo_game(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `game` demo 渲染到指定上下文。
- L1019: `export fn run_demo_game() i32 {`
  说明: 运行 `game` demo 并返回退出码。
- L1039: `export fn game_retained_new() Game2048Retained {`
  说明: 创建默认的 `Game2048Retained` 实例。

## `examples/demo_music.uya`

- L37: `export struct MusicRetained {`
- L51: `export fn music_retained_new() MusicRetained {`
  说明: 创建默认的 `MusicRetained` 实例。
- L67: `export fn music_retained_init(music: &MusicRetained) void {`
  说明: 初始化 `music_retained` 的内部状态。
- L120: `export fn music_retained_sync(music: &MusicRetained, track_idx: i32, playing: bool, progress_value: i32) void {`
  说明: 将外部状态同步到 `music_retained`。
- L136: `export fn render_demo_music_retained_state(music: &MusicRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16, track_idx: i32, playing: bool, progress_value: i32) void {`
  说明: 按 retained 状态参数将 `music` demo 渲染到指定上下文。
- L146: `export fn render_demo_music_state(ctx: &RenderCtx, origin_x: i16, origin_y: i16, track_idx: i32, playing: bool, progress_value: i32) void {`
  说明: 按显式状态参数将 `music` demo 渲染到指定上下文。
- L152: `export fn render_demo_music(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `music` demo 渲染到指定上下文。
- L157: `export fn run_demo_music() i32 {`
  说明: 运行 `music` demo 并返回退出码。

## `examples/demo_novel.uya`

- L249: `export struct NovelRetained {`
- L264: `export struct NovelPageRetained {`
- L991: `export fn novel_retained_scroll_by(novel: &NovelRetained, delta: i32) void {`
  说明: 按给定增量滚动 `novel_retained`。
- L1000: `export fn novel_retained_scroll_to_top(novel: &NovelRetained) void {`
  说明: 将 `novel_retained` 滚动到顶部。
- L1009: `export fn novel_retained_scroll_to_end(novel: &NovelRetained) void {`
  说明: 将 `novel_retained` 滚动到底部。
- L1018: `export fn novel_retained_toggle_auto_scroll(novel: &NovelRetained) void {`
  说明: 切换 `novel_retained` 的自动滚动状态。
- L1028: `export fn novel_retained_progress_percent(novel: &NovelRetained) i32 {`
  说明: 返回 `novel_retained` 的当前进度百分比。
- L1041: `export fn novel_retained_reset_state(novel: &NovelRetained) void {`
  说明: 提供 `novel_retained_reset_state` 的公开辅助入口。
- L1052: `export fn novel_page_retained_refresh_font_runtime(page_state: &NovelPageRetained, novel: &NovelRetained) void {`
- L1072: `export fn novel_retained_handle_touch(novel: &NovelRetained, evt: &Event, local: Point) void {`
  说明: 处理 `novel_retained` 的触摸输入。
- L1123: `export fn novel_retained_update(novel: &NovelRetained, now_ms: u32) void {`
  说明: 推进 `novel_retained` 的一帧更新。
- L1360: `export fn novel_page_retained_new() NovelPageRetained {`
  说明: 创建默认的 `NovelPageRetained` 实例。
- L1400: `export fn novel_page_retained_init(page_state: &NovelPageRetained, novel: &NovelRetained) void {`
  说明: 初始化 `novel_page_retained` 的内部状态。
- L1549: `export fn novel_page_retained_sync(page_state: &NovelPageRetained, novel: &NovelRetained) void {`
  说明: 将外部状态同步到 `novel_page_retained`。
- L1605: `export fn render_demo_novel_retained_state(page_state: &NovelPageRetained, novel: &NovelRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 按 retained 状态参数将 `novel` demo 渲染到指定上下文。
- L1615: `export fn novel_retained_init(novel: &NovelRetained) void {`
  说明: 初始化 `novel_retained` 的内部状态。
- L1627: `export fn render_demo_novel_state(novel: &NovelRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 按显式状态参数将 `novel` demo 渲染到指定上下文。
- L1815: `export fn render_demo_novel(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `novel` demo 渲染到指定上下文。
- L1821: `export fn run_demo_novel() i32 {`
  说明: 运行 `novel` demo 并返回退出码。
- L1841: `export fn novel_retained_new() NovelRetained {`
  说明: 创建默认的 `NovelRetained` 实例。

## `examples/demo_perf.uya`

- L26: `export struct PerfRetained {`
- L38: `export fn perf_retained_new() PerfRetained {`
  说明: 创建默认的 `PerfRetained` 实例。
- L52: `export fn perf_retained_init(perf: &PerfRetained) void {`
  说明: 初始化 `perf_retained` 的内部状态。
- L100: `export fn render_demo_perf_retained(perf: &PerfRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 使用 retained 状态将 `perf_retained` 渲染到指定上下文。
- L110: `export fn render_demo_perf(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `perf` demo 渲染到指定上下文。
- L116: `export fn run_demo_perf() i32 {`
  说明: 运行 `perf` demo 并返回退出码。

## `examples/demo_richtext.uya`

- L33: `export struct RichTextRetained {`
- L48: `export fn richtext_retained_new() RichTextRetained {`
- L65: `export fn richtext_retained_init(demo: &RichTextRetained) void {`
- L155: `export fn richtext_retained_sync(demo: &RichTextRetained) void {`
- L162: `export fn render_demo_richtext_retained_state(`

## `examples/demo_settings.uya`

- L26: `export struct SettingsRetained {`
- L41: `export fn settings_retained_new() SettingsRetained {`
  说明: 创建默认的 `SettingsRetained` 实例。
- L58: `export fn settings_retained_init(settings: &SettingsRetained) void {`
  说明: 初始化 `settings_retained` 的内部状态。
- L106: `export fn settings_retained_sync(settings: &SettingsRetained, wifi_enabled: bool, alerts_enabled: bool, brightness_value: i32) void {`
  说明: 将外部状态同步到 `settings_retained`。
- L117: `export fn render_demo_settings_retained_state(settings: &SettingsRetained, ctx: &RenderCtx, origin_x: i16, origin_y: i16, wifi_enabled: bool, alerts_enabled: bool, brightness_value: i32) void {`
  说明: 按 retained 状态参数将 `settings` demo 渲染到指定上下文。
- L127: `export fn render_demo_settings_state(ctx: &RenderCtx, origin_x: i16, origin_y: i16, wifi_enabled: bool, alerts_enabled: bool, brightness_value: i32) void {`
  说明: 按显式状态参数将 `settings` demo 渲染到指定上下文。
- L133: `export fn render_demo_settings(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `settings` demo 渲染到指定上下文。
- L138: `export fn run_demo_settings() i32 {`
  说明: 运行 `settings` demo 并返回退出码。

## `examples/demo_widgets.uya`

- L121: `export struct WidgetsRetained {`
- L166: `export fn widgets_retained_new() WidgetsRetained {`
  说明: 创建默认的 `WidgetsRetained` 实例。
- L213: `export fn widgets_retained_init(widgets: &WidgetsRetained) void {`
  说明: 初始化 `widgets_retained` 的内部状态。
- L406: `export fn widgets_retained_sync(`
  说明: 将外部状态同步到 `widgets_retained`。
- L498: `export fn render_demo_widgets_retained_state(`
  说明: 按 retained 状态参数将 `widgets` demo 渲染到指定上下文。
- L519: `export fn render_demo_widgets_state(`
  说明: 按显式状态参数将 `widgets` demo 渲染到指定上下文。
- L546: `export fn render_demo_widgets(ctx: &RenderCtx, origin_x: i16, origin_y: i16) void {`
  说明: 将 `widgets` demo 渲染到指定上下文。
- L551: `export fn run_demo_widgets() i32 {`
  说明: 运行 `widgets` demo 并返回退出码。

## `examples/doudizhu/ai.uya`

- L33: `export const DDZ_PROMPT_BYTES: i32 = 32768;`
- L34: `export const DDZ_OPENAI_RESPONSE_BYTES: i32 = 1024;`
- L36: `export enum DdzAiSource {`
- L42: `export enum DdzAiStatus {`
- L50: `export struct DdzAiDecision {`
- L57: `export struct DdzAiState {`
- L142: `export fn ddz_ai_state_new() DdzAiState {`
- L155: `export fn ddz_ai_cancel_pending(state: &DdzAiState) void {`
- L170: `export fn ddz_ai_bid_score(hand: &DdzHand) i32 {`
- L327: `export fn ddz_ai_action_id_valid(legal: &DdzActionList, action_index: i32) bool {`
- L479: `export fn ddz_ai_choose_action(state: &DdzAiState, game: &DdzGameState, player: i32) DdzAiDecision {`
- L1261: `export fn ddz_ai_step_action(state: &DdzAiState, game: &DdzGameState, player: i32) DdzAiDecision {`

## `examples/doudizhu/rules.uya`

- L3: `export const DDZ_PLAYER_COUNT: i32 = 3;`
- L4: `export const DDZ_DECK_COUNT: i32 = 54;`
- L5: `export const DDZ_BOTTOM_COUNT: i32 = 3;`
- L6: `export const DDZ_INITIAL_HAND_COUNT: i32 = 17;`
- L7: `export const DDZ_MAX_CARDS_PER_HAND: i32 = 20;`
- L8: `export const DDZ_RANK_COUNT: i32 = 15;`
- L9: `export const DDZ_MAX_COMBO_CARDS: i32 = 20;`
- L10: `export const DDZ_MAX_ACTIONS: i32 = 128;`
- L11: `export const DDZ_MAX_HISTORY: i32 = 256;`
- L12: `export const DDZ_ACTION_LABEL_BYTES: i32 = 48;`
- L13: `export const DDZ_NO_PLAYER: i32 = -1;`
- L14: `export const DDZ_RANK_2: i32 = 12;`
- L15: `export const DDZ_RANK_BJ: i32 = 13;`
- L16: `export const DDZ_RANK_RJ: i32 = 14;`
- L18: `export enum DdzComboKind {`
- L32: `export enum DdzActionKind {`
- L37: `export enum DdzGamePhase {`
- L43: `export struct DdzCard {`
- L49: `export struct DdzHand {`
- L55: `export struct DdzCombo {`
- L65: `export struct DdzAction {`
- L74: `export struct DdzActionList {`
- L80: `export struct DdzHistoryEntry {`
- L94: `export struct DdzGameState {`
- L128: `export fn ddz_rank_name(rank: i32) &const byte {`
- L189: `export fn ddz_combo_invalid() DdzCombo {`
- L310: `export fn ddz_action_list_new() DdzActionList {`
- L331: `export fn ddz_history_entry_new() DdzHistoryEntry {`
- L400: `export fn ddz_hand_sort(hand: &DdzHand) void {`
- L429: `export fn ddz_hand_recount(hand: &DdzHand) void {`
- L504: `export fn ddz_action_label_format(action: &DdzAction) void {`
- L627: `export fn ddz_deck_init(game: &DdzGameState) bool {`
- L661: `export fn ddz_deck_shuffle(game: &DdzGameState, seed: u32) bool {`
- L690: `export fn ddz_game_deal(game: &DdzGameState) bool {`
- L721: `export fn ddz_combo_detect(card_ranks: &i32, count: i32) DdzCombo {`
- L883: `export fn ddz_combo_can_beat(candidate: DdzCombo, target: DdzCombo) bool {`
- L1200: `export fn ddz_generate_bid_actions(game: &DdzGameState, player: i32, out_list: &DdzActionList) bool {`
- L1215: `export fn ddz_generate_play_actions(game: &DdzGameState, player: i32, out_list: &DdzActionList) bool {`
- L1292: `export fn ddz_game_apply_bid(game: &DdzGameState, player: i32, bid: i32) bool {`
- L1369: `export fn ddz_game_apply_action(game: &DdzGameState, player: i32, action: &DdzAction) bool {`
- L1439: `export fn ddz_game_new() DdzGameState {`
- L1461: `export fn ddz_game_restart(game: &DdzGameState, seed: u32) bool {`
- L1482: `export fn ddz_action_validate_in_hand(hand: &DdzHand, action: &DdzAction) bool {`

## `examples/phase0_smoke.uya`

- L18: `export fn run_phase0_smoke() i32 {`
  说明: 运行 `phase0_smoke` 场景并返回退出码。

## `examples/phase1_smoke.uya`

- L22: `export fn run_phase1_smoke() i32 {`
  说明: 运行 `phase1_smoke` 场景并返回退出码。

## `examples/phase2_smoke.uya`

- L28: `export fn run_phase2_smoke() i32 {`
  说明: 运行 `phase2_smoke` 场景并返回退出码。

## `examples/phase3_smoke.uya`

- L34: `export fn run_phase3_smoke() i32 {`
  说明: 运行 `phase3_smoke` 场景并返回退出码。

## `examples/phase4_smoke.uya`

- L46: `export fn run_phase4_smoke() i32 {`
  说明: 运行 `phase4_smoke` 场景并返回退出码。

## `examples/phase6_smoke.uya`

- L15: `export fn run_phase6_smoke() i32 {`
  说明: 运行 `phase6_smoke` 场景并返回退出码。

## `examples/text_render_compare.uya`

- L170: `export fn run_text_render_compare() i32 {`
  说明: 运行 `text_render_compare` 并返回退出码。

## `src/gui/anim/easing.uya`

- L50: `export enum EasingType {`
- L79: `export fn easing_variant_count() i32 {`
  说明: 返回当前公开的缓动函数变体数量。
- L84: `export fn apply_easing(t: f32, easing: EasingType) f32 {`
  说明: 按指定缓动函数计算归一化进度。

## `src/gui/anim/timeline.uya`

- L6: `export const MAX_ANIMATIONS: i32 = 32;`
- L10: `export struct AnimManager {`
- L92: `export fn anim_manager_new() AnimManager {`
  说明: 创建默认的动画管理器实例。

## `src/gui/anim/tween.uya`

- L87: `export enum AnimProp {`
- L110: `export interface ITweenCompleteCallback {`
- L114: `export interface ITweenUpdateCallback {`
- L118: `export struct Tween {`
- L437: `export fn anim_value_i32(value: i32) AnimValue {`
  说明: 构造 `i32` 类型的动画值。
- L442: `export fn anim_value_f32(value: f32) AnimValue {`
  说明: 构造 `f32` 类型的动画值。
- L447: `export fn anim_value_color(value: Color) AnimValue {`
  说明: 构造 `Color` 类型的动画值。

## `src/gui/bench_suite.uya`

- L6: `export fn main() i32 {`
  说明: 模块入口，运行 GUI benchmark 套件。

## `src/gui/benchmarks/core_bench.uya`

- L572: `export fn run_core_bench() i32 {`
  说明: 运行 `core_bench` 并返回退出码。

## `src/gui/core/bitmap.uya`

- L6: `export const BITMAP_ALLOCATOR_MAX_BITS: i32 = 256;`
- L31: `export struct BitmapAllocator {`
- L125: `export fn bitmap_allocator_new(capacity: i32) BitmapAllocator {`
  说明: 按给定容量创建位图分配器。

## `src/gui/core/color.uya`

- L80: `export struct Color {`
- L87: `export const BLACK: Color = Color{ r: 0, g: 0, b: 0, a: 255 };`
- L88: `export const WHITE: Color = Color{ r: 255, g: 255, b: 255, a: 255 };`
- L89: `export const RED: Color = Color{ r: 255, g: 0, b: 0, a: 255 };`
- L90: `export const GREEN: Color = Color{ r: 0, g: 255, b: 0, a: 255 };`
- L91: `export const BLUE: Color = Color{ r: 0, g: 0, b: 255, a: 255 };`
- L92: `export const TRANSPARENT: Color = Color{ r: 0, g: 0, b: 0, a: 0 };`
- L94: `export mc COLOR(hex: expr) expr {`

## `src/gui/core/dirty_region.uya`

- L5: `export const MAX_DIRTY_PRECISE: i32 = 16;`
- L6: `export const MAX_DIRTY_MERGED: i32 = 8;`
- L34: `export struct DirtyRegionView {`
- L61: `export struct DirtyRegionIter {`
- L76: `export struct DirtyRegion {`
- L272: `export fn dirty_region_new() DirtyRegion {`
  说明: 创建空的脏区收集器。
- L285: `export fn dirty_region_view_empty() DirtyRegionView {`
  说明: 返回空的脏区视图。
- L293: `export fn dirty_region_iter_empty() DirtyRegionIter {`
  说明: 返回空的脏区迭代器。

## `src/gui/core/event.uya`

- L4: `export const EVENT_QUEUE_SIZE: i32 = 64;`
- L5: `export const EVENT_TEXT_INPUT_MAX_BYTES: i32 = 96;`
- L6: `export const EVENT_MOD_ALT: u16 = 1u16;`
- L7: `export const EVENT_MOD_CTRL: u16 = 2u16;`
- L8: `export const EVENT_MOD_META: u16 = 4u16;`
- L9: `export const EVENT_MOD_SHIFT: u16 = 8u16;`
- L25: `export enum EventType {`
- L57: `export enum EventPhase {`
- L63: `export enum InputDev {`
- L72: `export struct EventPointValue {`
- L77: `export struct EventTextValue {`
- L144: `export struct Event {`
- L231: `export struct EventOption {`
- L236: `export struct EventQueue {`
- L279: `export struct GestureDetector {`
- L404: `export struct GestureConfig {`
- L414: `export fn gesture_config_default() GestureConfig {`
  说明: 返回默认的手势识别配置。
- L426: `export fn gesture_detector_with_config(config: GestureConfig) GestureDetector {`
  说明: 按指定配置创建手势识别器。
- L443: `export fn event_with_phase(evt: Event, phase: EventPhase) Event {`
  说明: 返回带有指定传播阶段的事件副本。
- L450: `export fn event_point_value(kind: EventType, dev: InputDev, target: i32, timestamp: u32, point: Point, value: i32) Event {`
  说明: 构造同时携带坐标和值的事件。
- L469: `export fn event_focus(kind: EventType, target: i32, timestamp: u32) Event {`
  说明: 构造焦点进入或离开事件。
- L485: `export fn event_none() Event {`
  说明: 构造空事件值。
- L501: `export fn event_option_some(evt: Event) EventOption {`
  说明: 构造包含事件值的 `EventOption`。
- L509: `export fn event_option_none() EventOption {`
  说明: 构造空的 `EventOption`。
- L517: `export fn event_point(kind: EventType, dev: InputDev, target: i32, timestamp: u32, point: Point) Event {`
  说明: 构造携带坐标的事件。
- L533: `export fn event_key(kind: EventType, dev: InputDev, target: i32, timestamp: u32, key_code: u16) Event {`
  说明: 构造携带键码的事件。
- L538: `export fn event_key_repeat(kind: EventType, dev: InputDev, target: i32, timestamp: u32, key_code: u16, key_repeat: bool) Event {`
  说明: 构造携带键码并标记是否为重复触发的事件。
- L543: `export fn event_key_modifiers(kind: EventType, dev: InputDev, target: i32, timestamp: u32, key_code: u16, modifiers: u16, key_repeat: bool) Event {`
  说明: 构造携带键码、修饰键和重复触发标记的事件。
- L559: `export fn event_text_input(dev: InputDev, target: i32, timestamp: u32, text: &const byte, len: usize) Event {`
  说明: 构造携带 UTF-8 文本的输入事件。
- L586: `export fn event_value(kind: EventType, dev: InputDev, target: i32, timestamp: u32, value: i32) Event {`
  说明: 构造携带数值的事件。
- L602: `export fn event_queue_new() EventQueue {`
  说明: 创建空的事件队列。
- L613: `export fn gesture_detector_new() GestureDetector {`
  说明: 创建使用默认配置的手势识别器。

## `src/gui/core/event_dispatch.uya`

- L35: `export struct EventDispatcher {`
- L236: `export fn event_dispatcher_new() EventDispatcher {`
  说明: 创建默认的事件分发器。

## `src/gui/core/obj.uya`

- L9: `export const GUI_OBJ_REGISTRY_CAPACITY: i32 = 256;`
- L11: `export interface IGuiObj {`
- L24: `export interface IContainer {`
- L31: `export interface IGuiEventCallback {`
- L35: `export interface IGuiRenderCallback {`
- L39: `export interface IGuiLayoutCallback {`
- L43: `export interface IGuiInputCallback {`
- L47: `export struct EventCallbackSlot {`
- L52: `export struct ObjFlags {`
- L64: `export struct GuiObj : IGuiObj, IContainer {`
- L106: `export fn obj_flags_default() ObjFlags {`
  说明: 返回默认的 GUI 对象标志位集合。
- L121: `export fn event_callback_slot_default() EventCallbackSlot {`
  说明: 返回空的对象事件回调槽位。
- L478: `export fn gui_obj_default() GuiObj {`
  说明: 构造默认初始化的 `GuiObj`。
- L513: `export fn gui_obj_register(idx: i32, obj: &GuiObj) bool {`
  说明: 将对象注册到全局 GUI 对象表。
- L527: `export fn gui_obj_get(idx: i32) &GuiObj {`
  说明: 按注册索引获取 GUI 对象引用。
- L539: `export fn gui_obj_unregister(idx: i32) void {`
  说明: 从全局 GUI 对象表中移除对象。
- L551: `export fn gui_obj_reset_registry() void {`
  说明: 清空全局 GUI 对象注册表。

## `src/gui/core/obj_pool.uya`

- L1: `export const OBJ_POOL_CAPACITY: i32 = 128;`
- L5: `export enum ObjSlotState {`
- L11: `export struct ObjPoolIndices {`
- L16: `export struct ObjPool<T: IGuiObj> {`
- L99: `export fn obj_pool_new<T: IGuiObj>() ObjPool<T> {`
  说明: 创建指定 GUI 对象类型的对象池。

## `src/gui/core/obj_tree.uya`

- L14: `export const OBJ_TREE_MAX_PATH: i32 = 64;`
- L15: `export const OBJ_TREE_MAX_REPAINTS: i32 = 64;`
- L17: `export struct BubblePath {`
- L82: `export struct ObjTreeRepaintRequest {`
- L88: `export struct ObjTreeDirtyRenderPlan {`
- L232: `export struct ObjTree {`
- L534: `export fn obj_tree_new() ObjTree {`
  说明: 创建空的对象树管理器。

## `src/gui/core/point.uya`

- L20: `export struct Point {`
- L25: `export const POINT_ZERO: Point = Point{ x: 0, y: 0 };`
- L26: `export const POINT_ONE: Point = Point{ x: 1, y: 1 };`

## `src/gui/core/rect.uya`

- L21: `export struct Rect {`
- L28: `export const RECT_ZERO: Rect = Rect{ x: 0, y: 0, w: 0, h: 0 };`
- L149: `export fn rect_union(a: Rect, b: Rect) Rect {`
  说明: 返回两个矩形的并集。

## `src/gui/core/size.uya`

- L1: `export struct Size {`
- L6: `export const SIZE_ZERO: Size = Size{ w: 0, h: 0 };`

## `src/gui/layout/abs.uya`

- L4: `export enum OverflowMode {`
- L60: `export struct AbsLayout {`
- L77: `export fn abs_layout_default() AbsLayout {`
  说明: 返回默认的绝对布局实例。

## `src/gui/layout/auto.uya`

- L10: `export enum LayoutMode {`
- L17: `export struct AutoLayout {`
- L44: `export fn auto_layout_new() AutoLayout {`
  说明: 创建自动布局配置实例。

## `src/gui/layout/flex.uya`

- L4: `export const MAX_FLEX_CHILDREN: i32 = 64;`
- L5: `export const MAX_FLEX_LINES: i32 = 16;`
- L7: `export enum FlexDir {`
- L14: `export enum Justify {`
- L23: `export enum Align {`
- L31: `export struct FlexConfig {`
- L161: `export fn flex_config_row() FlexConfig {`
  说明: 返回按行排布的默认 Flex 配置。
- L176: `export fn flex_config_column() FlexConfig {`
  说明: 返回按列排布的默认 Flex 配置。
- L182: `export struct FlexLayout {`
- L459: `export fn flex_layout_row() FlexLayout {`
  说明: 创建按行方向工作的 Flex 布局器。
- L464: `export fn flex_layout_column() FlexLayout {`
  说明: 创建按列方向工作的 Flex 布局器。

## `src/gui/layout/grid.uya`

- L4: `export const MAX_GRID_COLS: i32 = 8;`
- L5: `export const MAX_GRID_ROWS: i32 = 8;`
- L7: `export enum GridAutoFlow {`
- L12: `export struct GridConfig {`
- L46: `export struct GridLayout {`
- L110: `export fn grid_layout_default() GridLayout {`
  说明: 返回默认的网格布局器。

## `src/gui/platform/disp.uya`

- L258: `export enum PixelFormat {`
- L269: `export struct FrameBuffer {`
- L276: `export struct DisplayCtx {`
- L302: `export fn bytes_per_pixel(format: PixelFormat) u8 {`
  说明: 返回指定像素格式的每像素字节数。
- L319: `export fn framebuffer_required_bytes(w: u16, h: u16, format: PixelFormat) usize {`
  说明: 计算指定帧缓冲配置所需的总字节数。
- L330: `export fn framebuffer_rect(fb: &FrameBuffer) Rect {`
  说明: 返回帧缓冲覆盖的完整矩形区域。
- L335: `export fn framebuffer_copy(dst: &FrameBuffer, src: &FrameBuffer) bool {`
  说明: 复制整个源帧缓冲到目标帧缓冲。
- L364: `export fn framebuffer_copy_rect(dst: &FrameBuffer, src: &FrameBuffer, rect: Rect) bool {`
  说明: 复制源帧缓冲的指定矩形区域到目标帧缓冲。
- L404: `export fn framebuffer_new(pixels: &byte, w: u16, h: u16, stride: u16, format: PixelFormat) FrameBuffer {`
  说明: 根据像素指针和尺寸信息构造帧缓冲描述符。
- L414: `export fn empty_framebuffer() FrameBuffer {`
  说明: 返回空的帧缓冲描述符。
- L424: `export fn display_ctx_new(front: FrameBuffer, back: FrameBuffer) DisplayCtx {`
  说明: 创建包含前后缓冲的显示上下文。
- L433: `export fn framebuffer_inside(fb: &FrameBuffer, x: i32, y: i32) bool {`
  说明: 判断坐标是否位于帧缓冲范围内。
- L438: `export fn framebuffer_get_pixel(fb: &FrameBuffer, x: i32, y: i32) Color {`
  说明: 读取帧缓冲指定坐标的颜色值。
- L497: `export fn framebuffer_set_pixel(fb: &FrameBuffer, x: i32, y: i32, color: &Color) void {`
  说明: 向帧缓冲指定坐标写入颜色值。
- L513: `export fn framebuffer_clear(fb: &FrameBuffer, color: Color) void {`
  说明: 使用指定颜色清空整个帧缓冲。
- L539: `export enum DisplayDriverKind {`
- L548: `export struct DisplayDriverProfile {`
- L559: `export fn display_driver_profile(kind: DisplayDriverKind) DisplayDriverProfile {`
  说明: 返回指定显示驱动类型的能力画像。
- L627: `export fn display_driver_required_bytes(kind: DisplayDriverKind) usize {`
  说明: 计算指定显示驱动默认缓冲所需的字节数。
- L633: `export fn framebuffer_new_for_driver(pixels: &byte, kind: DisplayDriverKind) FrameBuffer {`
  说明: 按驱动默认画像构造帧缓冲描述符。

## `src/gui/platform/fb/disp_fb.uya`

- L27: `export struct FbDisplay : IRuntimeDisplay {`
- L195: `export fn fb_display_new() FbDisplay {`
  说明: 创建 Framebuffer 显示驱动实例。
- L206: `export fn fb_display_last_error() &const byte {`
  说明: 返回最近一次 Framebuffer 显示驱动错误信息。

## `src/gui/platform/fb/indev_fb.uya`

- L23: `export struct FbInputSystem : IRuntimeInput {`
- L114: `export fn fb_input_system_new() FbInputSystem {`
  说明: 创建 Framebuffer 输入系统实例。
- L130: `export fn fb_input_last_error() &const byte {`
  说明: 返回最近一次 Framebuffer 输入系统错误信息。

## `src/gui/platform/fb/indev_fb_common.uya`

- L7: `export const FB_EVT_NONE: u8 = 0u8;`
- L8: `export const FB_EVT_KEY_DOWN: u8 = 1u8;`
- L9: `export const FB_EVT_HOVER_MOVE: u8 = 2u8;`
- L10: `export const FB_EVT_TOUCH_DOWN: u8 = 3u8;`
- L11: `export const FB_EVT_TOUCH_UP: u8 = 4u8;`
- L12: `export const FB_EVT_TOUCH_MOVE: u8 = 5u8;`
- L13: `export const FB_EVT_WHEEL: u8 = 6u8;`
- L15: `export struct FbHostEvent {`
- L25: `export fn fb_host_event_none() FbHostEvent {`
  说明: 构造空的 Framebuffer host 事件。
- L37: `export fn fb_feed_host_event(`
  说明: 向 Framebuffer host 注入一条原始输入事件。
- L89: `export fn fb_hover_point_default() Point {`
  说明: 返回 Framebuffer hover 状态的默认坐标。

## `src/gui/platform/indev.uya`

- L19: `export const TOUCH_FILTER_WINDOW: i32 = 5;`
- L20: `export const MAX_INPUT_DEVICES: i32 = 8;`
- L69: `export interface IInputDev {`
- L74: `export struct TouchCalibSample {`
- L79: `export struct TouchCalibration {`
- L110: `export struct TouchMedianFilter {`
- L151: `export struct TouchDriver : IInputDev {`
- L202: `export struct MouseDriver : IInputDev {`
- L223: `export struct KeyDriver : IInputDev {`
- L252: `export struct TextDriver : IInputDev {`
- L269: `export struct EncoderDriver : IInputDev {`
- L286: `export struct InputDeviceSlot {`
- L292: `export struct InputManager {`
- L366: `export fn touch_calibration_from_samples(samples: &TouchCalibSample, count: i32) TouchCalibration {`
  说明: 根据采样点拟合触摸校准参数。
- L415: `export fn touch_driver_new() TouchDriver {`
  说明: 创建触摸输入驱动实例。
- L431: `export fn touch_driver_configure_pointer_precision(driver: &TouchDriver) void {`
  说明: 将触摸驱动切换到更适合桌面指针的精度配置。
- L441: `export fn mouse_driver_new() MouseDriver {`
  说明: 创建鼠标输入驱动实例。
- L446: `export fn key_driver_new() KeyDriver {`
  说明: 创建键盘输入驱动实例。
- L451: `export fn text_driver_new() TextDriver {`
  说明: 创建文本输入驱动实例。
- L456: `export fn encoder_driver_new() EncoderDriver {`
  说明: 创建编码器输入驱动实例。
- L461: `export fn input_manager_new() InputManager {`
  说明: 创建输入设备管理器实例。
- L470: `export fn touch_calibration_identity() TouchCalibration {`
  说明: 返回不做变换的触摸校准参数。
- L475: `export fn touch_filter_default() TouchMedianFilter {`
  说明: 返回默认的触摸中值滤波器。

## `src/gui/platform/openai/chat.uya`

- L9: `export enum UyaOpenAiChatStatus {`
- L24: `export struct UyaOpenAiChatPollResult {`
- L66: `export fn uya_openai_chat_is_available() bool {`
- L70: `export fn uya_openai_chat_start_request(request_body: &const byte, request_len: usize) i32 {`
- L77: `export fn uya_openai_chat_poll_request(handle: i32, out_body: &byte, out_cap: i32) UyaOpenAiChatPollResult {`
- L89: `export fn uya_openai_chat_cancel_request(handle: i32) void {`
- L96: `export fn uya_openai_chat_model_name(out_model: &byte, out_cap: i32) bool {`
- L108: `export fn uya_openai_chat_parse_action_id(text: &const byte, out_action_id: &i32) bool {`

## `src/gui/platform/sdl2/disp_sdl.uya`

- L57: `export struct SdlDisplay : IRuntimeDisplay {`
- L456: `export fn sdl_display_new() SdlDisplay {`
  说明: 创建 SDL2 显示驱动实例。
- L472: `export fn sdl_display_last_error() &const byte {`
  说明: 返回最近一次 SDL2 显示驱动错误信息。

## `src/gui/platform/sdl2/gpu_sdl.uya`

- L17: `export struct SdlGles2GpuCtx : IGpuCtx {`
- L279: `export fn sdl_gles2_gpu_new(display: &SdlDisplay, mirror: &RenderCtx) SdlGles2GpuCtx {`
  说明: 基于 SDL GLES2 display 和共享 framebuffer 创建 GPU 上下文。

## `src/gui/platform/sdl2/indev_sdl.uya`

- L33: `export struct SdlHostEvent {`
- L52: `export struct SdlInputSystem : IRuntimeInput {`
- L184: `export fn sdl_input_system_new() SdlInputSystem {`
  说明: 创建 SDL2 输入系统实例。

## `src/gui/platform/text_host.uya`

- L5: `export const RICH_TEXT_HOST_CMD_NONE: u8 = 0u8;`
- L6: `export const RICH_TEXT_HOST_CMD_INSERT_TEXT: u8 = 1u8;`
- L7: `export const RICH_TEXT_HOST_CMD_COMPOSITION_START: u8 = 2u8;`
- L8: `export const RICH_TEXT_HOST_CMD_COMPOSITION_UPDATE: u8 = 3u8;`
- L9: `export const RICH_TEXT_HOST_CMD_COMPOSITION_END: u8 = 4u8;`
- L10: `export const RICH_TEXT_HOST_CMD_PASTE_TEXT: u8 = 5u8;`
- L11: `export const RICH_TEXT_HOST_CMD_CUT: u8 = 6u8;`
- L12: `export const RICH_TEXT_HOST_CMD_COPY: u8 = 7u8;`
- L13: `export const RICH_TEXT_HOST_CMD_BLUR: u8 = 8u8;`
- L14: `export const RICH_TEXT_HOST_QUEUE_CAPACITY: i32 = 32;`
- L15: `export const RICH_TEXT_HOST_TEXT_BYTES: i32 = 4096;`
- L17: `export struct RichTextHostCommand {`
- L122: `export fn rich_text_host_activate(session_id: u32, obj: &GuiObj) void {`
- L130: `export fn rich_text_host_deactivate(session_id: u32) void {`
- L141: `export fn rich_text_host_active_session() u32 {`
- L145: `export fn rich_text_host_caret_rect() Rect {`
- L149: `export fn rich_text_host_caret_rect_x() i16 {`
- L153: `export fn rich_text_host_caret_rect_y() i16 {`
- L157: `export fn rich_text_host_caret_rect_w() u16 {`
- L161: `export fn rich_text_host_caret_rect_h() u16 {`
- L165: `export fn rich_text_host_selection_text_ptr() &const byte {`
- L169: `export fn rich_text_host_selection_text_len() usize {`
- L173: `export fn rich_text_host_set_caret_rect(session_id: u32, rect: Rect) void {`
- L180: `export fn rich_text_host_set_selection_text(session_id: u32, text: &const byte, len: usize) void {`
- L188: `export fn rich_text_host_feed_command(kind: u8) bool {`
- L202: `export fn rich_text_host_feed_text_command(kind: u8, text: &const byte, len: usize) bool {`
- L215: `export fn rich_text_host_poll_for_session(session_id: u32, out_cmd: &RichTextHostCommand) bool {`

## `src/gui/platform/tick.uya`

- L22: `export fn get_tick_ms() u32 {`
  说明: 获取当前单调时钟的毫秒值。
- L35: `export fn get_tick_us() u64 {`
  说明: 获取当前单调时钟的微秒值。
- L41: `export fn sleep_ms(ms: u32) void {`
  说明: 以毫秒为单位休眠当前线程。

## `src/gui/platform/web/disp_web.uya`

- L33: `export struct WebDisplay : IRuntimeDisplay {`
- L207: `export fn web_display_new() WebDisplay {`
- L219: `export fn web_display_last_error() &const byte {`

## `src/gui/platform/web/indev_web.uya`

- L41: `export struct WebInputSystem : IRuntimeInput {`
- L260: `export fn web_input_system_new() WebInputSystem {`
- L287: `export fn web_input_system_init(input: &WebInputSystem) void {`
- L294: `export fn web_input_system_pump(input: &WebInputSystem, timestamp: u32) i32 {`
- L301: `export fn web_input_system_poll(input: &WebInputSystem, out_evt: &Event) bool {`
- L308: `export fn web_input_system_poll_hover(input: &WebInputSystem, out_x: &i16, out_y: &i16) bool {`

## `src/gui/platform/web/web_common.uya`

- L5: `export const WEB_EVT_NONE: u8 = 0u8;`
- L6: `export const WEB_EVT_POINTER_MOVE: u8 = 1u8;`
- L7: `export const WEB_EVT_POINTER_DOWN: u8 = 2u8;`
- L8: `export const WEB_EVT_POINTER_UP: u8 = 3u8;`
- L9: `export const WEB_EVT_WHEEL: u8 = 4u8;`
- L10: `export const WEB_EVT_KEY_DOWN: u8 = 5u8;`
- L11: `export const WEB_EVT_KEY_UP: u8 = 6u8;`
- L12: `export const WEB_EVT_RESET_INPUT: u8 = 7u8;`
- L13: `export const WEB_EVT_REFRESH: u8 = 8u8;`
- L14: `export const WEB_EVT_TOUCH_CANCEL: u8 = 9u8;`
- L15: `export const WEB_EVT_TEXT_INPUT: u8 = 10u8;`
- L16: `export const WEB_HOST_EVENT_CAPACITY: i32 = 256;`
- L17: `export const WEB_MODIFIER_KEY_REPEAT: u16 = 0x8000u16;`
- L19: `export struct WebHostEvent {`
- L35: `export fn web_host_event_none() WebHostEvent {`
- L48: `export fn web_feed_host_event_raw(raw: &WebHostEvent) bool {`
- L63: `export fn web_feed_host_event(kind: u8, x: i16, y: i16, value: i32, key_code: u16, modifiers: u16) bool {`
- L77: `export fn web_feed_host_text_event(text: &const byte, len: usize, modifiers: u16) bool {`
- L94: `export fn web_poll_host_event(out_evt: &WebHostEvent) bool {`
- L108: `export fn web_hover_point_default() Point {`
- L112: `export fn web_host_event_queue_len() u16 {`
- L116: `export fn web_host_event_is_key_repeat(raw: &WebHostEvent) bool {`
- L123: `export fn web_host_event_modifiers(raw: &WebHostEvent) u16 {`

## `src/gui/render/batch.uya`

- L8: `export const DRAW_BATCH_CAPACITY: i32 = 64;`
- L10: `export enum DrawCmdKind {`
- L17: `export struct DrawCmd {`
- L234: `export struct DrawBatch {`
- L376: `export fn draw_batch_new() DrawBatch {`
  说明: 创建空的绘制批处理缓冲区。

## `src/gui/render/cpu_backend.uya`

- L9: `export enum CpuRenderBackendKind {`
- L15: `export fn cpu_render_backend_name(kind: CpuRenderBackendKind) &const byte {`
  说明: 返回 CPU 渲染后端的显示名称。
- L23: `export fn cpu_render_batch_execute(ctx: &RenderCtx, batch: &DrawBatch) void {`
  说明: 使用 CPU 渲染路径执行一批绘制命令。

## `src/gui/render/ctx.uya`

- L25: `export const MAX_CLIP_STACK: i32 = 16;`
- L368: `export enum RenderMode {`
- L374: `export struct RenderStats {`
- L385: `export struct RenderCtx {`
- L1330: `export fn empty_framebuffer_color() Color {`
  说明: 返回空帧缓冲上下文使用的清屏颜色。
- L1335: `export fn render_ctx_new(fb: FrameBuffer) RenderCtx {`
  说明: 基于帧缓冲创建渲染上下文。
- L1360: `export fn empty_render_ctx() RenderCtx {`
  说明: 返回空的渲染上下文。
- L1365: `export fn default_ctx() RenderCtx {`
  说明: 返回带默认帧缓冲的渲染上下文。
- L1370: `export fn ctx_pixel(ctx: &RenderCtx, x: i16, y: i16) Color {`
  说明: 读取渲染上下文帧缓冲中的指定像素。
- L1400: `export fn render_ctx_attach_batch(ctx: &RenderCtx, batch: &DrawBatch) void {`
  说明: 附加 CPU batch 存储区，供 `RenderMode.Batch` 复用。
- L1412: `export fn render_ctx_attach_gpu(ctx: &RenderCtx, gpu: &IGpuCtx) void {`
  说明: 附加 GPU 设备，供 batch flush 时提交。
- L1421: `export fn render_ctx_set_mode(ctx: &RenderCtx, mode: RenderMode) void {`
  说明: 切换渲染上下文模式。
- L1430: `export fn render_ctx_flush(ctx: &RenderCtx) void {`
  说明: 刷新 CPU batch 中待执行的绘制命令。

## `src/gui/render/font.uya`

- L66: `export const UI_FONT_BASE_DPI: u16 = 96u16;`
- L67: `export const BITMAP_FONT_GLYPH_CAPACITY: i32 = 1024;`
- L68: `export const BITMAP_FONT_UNICODE_CAPACITY: i32 = 1024;`
- L69: `export const BITMAP_FONT_KERNING_CAPACITY: i32 = 128;`
- L70: `export const TTF_POINT_CAPACITY: i32 = 2048;`
- L71: `export const TTF_CONTOUR_CAPACITY: i32 = 256;`
- L72: `export const TTF_EDGE_CAPACITY: i32 = 16384;`
- L73: `export const TTF_COMPOSITE_DEPTH_MAX: i32 = 8;`
- L74: `export const TTF_CACHE_ENTRY_CAPACITY: i32 = 1024;`
- L75: `export const TTF_CACHE_SLOT_DIM: i32 = 64;`
- L76: `export const TTF_CACHE_SLOT_BYTES: i32 = TTF_CACHE_SLOT_DIM * TTF_CACHE_SLOT_DIM;`
- L77: `export const TTF_CACHE_STORAGE_BYTES: i32 = TTF_CACHE_ENTRY_CAPACITY * TTF_CACHE_SLOT_BYTES;`
- L78: `export const SYSTEM_UI_FONT_SLOT_CAPACITY: i32 = 16;`
- L84: `export enum TextAlign {`
- L90: `export enum FontBitmapFormat {`
- L96: `export enum FontRenderKind {`
- L108: `export struct Glyph {`
- L123: `export struct GlyphLookupEntry {`
- L128: `export struct GlyphLookupTable {`
- L135: `export struct KerningPair {`
- L141: `export struct Font {`
- L162: `export struct BitmapFontAsset {`
- L208: `export struct TtfFontAsset {`
- L2136: `export fn ttf_cache_reset() void {`
  说明: 重置运行时 TTF 字形缓存统计与内容。
- L2159: `export fn ttf_cache_hit_count() u32 {`
  说明: 返回当前 TTF 字形缓存命中次数。
- L2164: `export fn ttf_cache_miss_count() u32 {`
  说明: 返回当前 TTF 字形缓存未命中次数。
- L2238: `export fn ttf_font_set_hinting(asset: &TtfFontAsset, enabled: bool) void {`
  说明: 切换指定 TTF 字体资产的 hinting 开关。
- L2354: `export fn ttf_font_asset_new() TtfFontAsset {`
  说明: 创建空的 TTF 字体资产对象。
- L2429: `export fn ttf_font_native_backend_is_c() bool {`
  说明: 判断当前默认 TTF 后端是否使用 C 实现。
- L2434: `export fn ttf_font_set_default_native_backend_c(enabled: bool) void {`
  说明: 设置默认 TTF 后端是否优先使用 C 实现。
- L2827: `export fn font_system_ui_retry_refresh() bool {`
- L3345: `export fn ttf_font_load_memory(asset: &TtfFontAsset, name: &const byte, data: &const byte, len: usize, pixel_height: u16) bool {`
  说明: 从内存字节中加载一份 TTF 字体资产。
- L3484: `export fn bitmap_font_asset_new() BitmapFontAsset {`
  说明: 创建空的位图字体资产对象。
- L3505: `export fn bitmap_font_load_bmfont(asset: &BitmapFontAsset, desc: &const byte, len: usize, bitmap_data: &const byte, bitmap_stride: u16) bool {`
  说明: 从 BMFont 描述文本和位图数据加载位图字体。
- L3618: `export fn font_system_default() Font {`
  说明: 返回默认系统字体。
- L3643: `export fn font_system_compact() Font {`
  说明: 返回更紧凑的系统字体。
- L3668: `export fn font_system_vector() Font {`
  说明: 返回矢量系统字体。
- L3693: `export fn font_ui_default_ref(pixel_height: u8) &Font {`
  说明: 返回按像素高度解析后的默认 UI 字体引用。
- L3780: `export fn font_system_ui_ref(pixel_height: u8) &Font {`
  说明: 返回按像素高度解析后的系统 UI 字体引用。
- L3787: `export fn font_runtime_generation() u32 {`
- L3792: `export fn font_ui_set_dpi(dpi: u16) void {`
  说明: 设置 UI 字体逻辑到像素的 DPI 标尺。
- L3805: `export fn font_ui_dpi() u16 {`
  说明: 返回当前 UI 字体 DPI 标尺。
- L3810: `export fn font_ui_pixel_height(logical_height: u8) u8 {`
  说明: 将逻辑字号换算为运行时像素高度。
- L3818: `export fn font_ui_reset_runtime() void {`
  说明: 重置 UI 字体运行时缓存和 DPI 状态。
- L3828: `export fn font_set_builtin_cjk_fallback_enabled(enabled: bool) void {`
  说明: 设置是否启用内置 CJK 8x8 点阵 fallback。
- L3837: `export fn font_builtin_cjk_fallback_enabled() bool {`
  说明: 返回内置 CJK 8x8 点阵 fallback 的启用状态。
- L3842: `export fn font_set_latin_companion_enabled(enabled: bool) void {`
  说明: 设置是否启用 system-ui 字体的 latin companion 替换。
- L3851: `export fn font_latin_companion_enabled() bool {`
  说明: 返回 system-ui 字体 latin companion 替换的启用状态。
- L3856: `export fn font_default() Font {`
  说明: 返回默认字体值。
- L3861: `export fn glyph_for_char(font: &Font, code: u8) Glyph {`
  说明: 根据 ASCII 码点返回对应字形。
- L3866: `export fn text_width(text: &const byte, len: usize, font: &Font) i16 {`
  说明: 计算指定文本在给定字体下的显示宽度。
- L3874: `export fn draw_text(ctx: &RenderCtx, x: i16, y: i16, text: &const byte, len: usize, font: &Font, color: Color) void {`
  说明: 在渲染上下文中绘制一段文本。
- L3917: `export fn draw_text_aligned(ctx: &RenderCtx, rect: Rect, text: &const byte, len: usize, font: &Font, color: Color, align: TextAlign) void {`
  说明: 在指定矩形内按对齐方式绘制文本。

## `src/gui/render/gpu.uya`

- L8: `export struct GpuFillRectCmd {`
- L13: `export struct GpuStrokeRectCmd {`
- L19: `export struct GpuLineCmd {`
- L28: `export struct GpuImageCmd {`
- L39: `export struct GpuGlyphCmd {`
- L88: `export interface IGpuCtx {`
- L110: `export fn gpu_execute_batch(gpu: &IGpuCtx, batch: &DrawBatch) void {`
  说明: 通过 GPU 接口执行一批绘制命令。

## `src/gui/render/gpu_software.uya`

- L16: `export struct SoftwareGpuCtx : IGpuCtx {`
- L183: `export fn software_gpu_new(ctx: &RenderCtx) SoftwareGpuCtx {`
  说明: 创建软件 GPU 上下文实例。

## `src/gui/render/img.uya`

- L215: `export struct ImageData {`
- L224: `export fn image_data_new(pixels: &byte, w: u16, h: u16, stride: u16, format: PixelFormat) ImageData {`
  说明: 根据像素指针和尺寸信息构造图像描述符。
- L235: `export fn empty_image() ImageData {`
  说明: 返回空的图像描述符。
- L246: `export fn image_size_bytes(img: &ImageData) u32 {`
  说明: 返回图像像素数据占用的字节数。
- L257: `export fn image_retain(img: &ImageData) void {`
  说明: 增加图像的引用计数。
- L262: `export fn image_release(img: &ImageData) void {`
  说明: 释放图像的一次引用。
- L269: `export fn draw_image(ctx: &RenderCtx, x: i16, y: i16, img: &ImageData) void {`
  说明: 在指定坐标绘制原始图像。
- L314: `export fn draw_image_scaled(ctx: &RenderCtx, rect: Rect, img: &ImageData) void {`
  说明: 按目标矩形缩放绘制图像。
- L378: `export fn draw_image_clipped(ctx: &RenderCtx, dst_rect: Rect, img: &ImageData, src_rect: Rect) void {`
  说明: 按源矩形裁剪后绘制图像。
- L426: `export fn draw_image_rotated(ctx: &RenderCtx, x: i16, y: i16, img: &ImageData, angle_deg: i16) void {`
  说明: 按指定角度旋转绘制图像。

## `src/gui/render/scheduler.uya`

- L13: `export enum RenderPresentMode {`
- L19: `export interface IRenderScheduleFullCallback {`
- L23: `export interface IRenderScheduleOverlayCallback {`
- L27: `export struct RenderFrameResult {`
- L64: `export fn render_frame_result_idle() RenderFrameResult {`
  说明: 将 `frame_result_idle` 渲染到指定上下文。
- L153: `export fn render_schedule_tree_frame(`
  说明: 将 `schedule_tree_frame` 渲染到指定上下文。
- L209: `export struct RenderFramePacer {`
- L302: `export fn render_frame_pacer_new(target_fps: u8, vsync_enabled: bool, adaptive: bool) RenderFramePacer {`
  说明: 将 `frame_pacer_new` 渲染到指定上下文。
- L319: `export fn render_scheduler_default_clear_color() Color {`
  说明: 将 `scheduler_default_clear_color` 渲染到指定上下文。

## `src/gui/render/zerocopy.uya`

- L8: `export const ZERO_COPY_TRANSFER_CAPACITY: i32 = 8;`
- L22: `export struct ZeroCopyTransfer {`
- L28: `export struct ZeroCopyCtx {`
- L157: `export fn zerocopy_ctx_new(display: &DisplayCtx) ZeroCopyCtx {`
  说明: 创建零拷贝显示提交上下文。

## `src/gui/res/buf.uya`

- L3: `export struct Slice<T> {`
- L8: `export struct ByteSlice {`
- L13: `export struct Buffer {`
- L78: `export fn buffer_new(allocator: &MallocAllocator) Buffer {`
  说明: 创建基于指定分配器的动态缓冲区。
- L88: `export fn slice_from_ptr<T>(ptr: &T, len: usize) Slice<T> {`
  说明: 基于指针和长度构造类型安全切片。

## `src/gui/res/cache.uya`

- L8: `export const IMAGE_CACHE_CAPACITY: i32 = 16;`
- L10: `export enum CacheEntryState {`
- L17: `export struct CacheEntry {`
- L27: `export struct ImageCache {`
- L275: `export @async_fn fn image_cache_put_async(cache: &ImageCache, key: u32, image: ImageData) Future<!bool> {`
- L283: `export fn image_cache_new(capacity: u8, budget_bytes: u32) ImageCache {`
  说明: 创建图像缓存实例。

## `src/gui/res/fs.uya`

- L8: `export const ROM_FS_CAPACITY: i32 = 16;`
- L9: `export const FS_PATH_CAPACITY: i32 = 256;`
- L93: `export interface IFileSystem {`
- L98: `export @async_fn fn fs_read_async<T: IFileSystem>(fs: &T, path: &const byte, out: &byte, capacity: usize) Future<!usize> {`
- L102: `export struct RomFsEntry {`
- L109: `export struct RomFileSystem : IFileSystem {`
- L159: `export struct FatFileSystem : IFileSystem {`
- L197: `export fn rom_fs_new() RomFileSystem {`
  说明: 创建内存内只读文件系统实例。
- L205: `export fn fat_fs_new(base_dir: &const byte) FatFileSystem {`
  说明: 创建基于宿主目录的 FAT 文件系统适配器。
- L210: `export fn fs_path_len(path: &const byte) usize {`
  说明: 计算以 NUL 结尾的文件系统路径长度。

## `src/gui/res/pool.uya`

- L4: `export const MEM_POOL_BYTES: i32 = 4096;`
- L31: `export struct MemPool {`
- L79: `export struct PoolManager {`
- L110: `export struct TypedPool<T> {`
- L135: `export fn mem_pool_new(block_size: u16, block_count: u16) MemPool {`
  说明: 创建固定块大小的内存池。
- L158: `export fn pool_manager_new() PoolManager {`
  说明: 创建多级内存池管理器。
- L167: `export fn typed_pool<T>(pool: &MemPool) TypedPool<T> {`
  说明: 将基础内存池包装为指定类型的对象池视图。

## `src/gui/richtext/delta.uya`

- L21: `export struct RichBlockAttrs {`
- L26: `export struct RichDeltaAttrs {`
- L35: `export enum RichOpKind {`
- L42: `export struct RichDeltaOp {`
- L54: `export struct RichDelta {`
- L61: `export struct RichInlineToggleResult {`
- L94: `export fn rich_block_attrs(block_type: RichBlockType, indent: u8) RichBlockAttrs {`
- L101: `export fn rich_delta_attrs_none() RichDeltaAttrs {`
- L112: `export fn rich_delta_attrs_inline(marks: RichInlineMarks) RichDeltaAttrs {`
- L126: `export fn rich_delta_attrs_block(block_type: RichBlockType, indent: u8) RichDeltaAttrs {`
- L542: `export fn rich_delta_new() RichDelta {`
- L546: `export fn rich_delta_new_with_allocator(allocator: &MallocAllocator) RichDelta {`
- L694: `export fn rich_delta_invert(base_doc: &RichDocument, delta: &RichDelta, out: &RichDelta) !void {`
  说明: Builds the inverse patch for applying `delta` to `base_doc`. 参数: `base_doc` is the document before `delta`, `delta` is the patch to invert, and `out` receives the inverse patch. 返回值: succeeds after replacing `out` with the inverse delta, or returns an error if the patch exceeds `base_doc` bounds.
- L738: `export fn document_to_delta(doc: &RichDocument, out: &RichDelta) !void {`
- L771: `export fn apply_delta(doc: &RichDocument, delta: &RichDelta) !void {`
- L878: `export fn rich_delete_range(doc: &RichDocument, range: RichRange) !RichPos {`
- L883: `export fn rich_insert_text(doc: &RichDocument, range: RichRange, text: &const byte, text_len: usize, marks: RichInlineMarks) !RichPos {`
- L907: `export fn rich_backspace(doc: &RichDocument, range: RichRange) !RichPos {`
- L926: `export fn rich_delete_forward(doc: &RichDocument, range: RichRange) !RichPos {`
- L946: `export fn rich_insert_break(doc: &RichDocument, range: RichRange) !RichPos {`
- L979: `export fn rich_set_block_type(doc: &RichDocument, range: RichRange, block_type: RichBlockType, indent: u8) !RichRange {`
- L1031: `export fn rich_toggle_inline_mark(doc: &RichDocument, range: RichRange, pending_marks: RichInlineMarks, mark_bit: u16) !RichInlineToggleResult {`
- L1137: `export fn rich_delta_from_plain_text(text: &const byte) !RichDelta {`

## `src/gui/richtext/document.uya`

- L10: `export const RICH_MARK_BOLD: u16 = 1u16;`
- L11: `export const RICH_MARK_ITALIC: u16 = 2u16;`
- L12: `export const RICH_MARK_UNDERLINE: u16 = 4u16;`
- L13: `export const RICH_MARK_STRIKE: u16 = 8u16;`
- L14: `export const RICH_MARK_INLINE_CODE: u16 = 16u16;`
- L16: `export enum RichBlockType {`
- L27: `export struct RichInlineMarks {`
- L32: `export struct RichSpan {`
- L45: `export struct RichBlock {`
- L53: `export struct RichDocument {`
- L64: `export fn rich_inline_marks_none() RichInlineMarks {`
- L71: `export fn rich_inline_marks_eq(a: RichInlineMarks, b: RichInlineMarks) bool {`
- L75: `export fn rich_inline_marks_has_bit(marks: RichInlineMarks, bit: u16) bool {`
- L79: `export fn rich_inline_marks_with_bit(marks: RichInlineMarks, bit: u16) RichInlineMarks {`
- L86: `export fn rich_inline_marks_without_bit(marks: RichInlineMarks, bit: u16) RichInlineMarks {`
- L93: `export fn rich_utf8_span(first: u8) usize {`
- L109: `export fn rich_utf8_cont_byte(value: u8) bool {`
- L113: `export fn rich_utf8_is_boundary(text: &const byte, len: usize, offset: usize) bool {`
- L123: `export fn rich_utf8_next_index(text: &const byte, len: usize, index: usize) usize {`
- L141: `export fn rich_utf8_prev_index(text: &const byte, len: usize, index: usize) usize {`
- L157: `export fn rich_utf8_codepoint_count(text: &const byte, len: usize) u32 {`
- L170: `export fn rich_utf8_codepoint_offset_to_byte_offset(text: &const byte, len: usize, codepoint_offset: u32) !usize {`
- L189: `export fn rich_utf8_byte_offset_to_codepoint_offset(text: &const byte, len: usize, byte_offset: usize) !u32 {`
- L503: `export fn rich_document_new() RichDocument {`
- L507: `export fn rich_document_new_with_allocator(allocator: &MallocAllocator) RichDocument {`

## `src/gui/richtext/history.uya`

- L9: `export enum RichHistoryMergeKind {`
- L24: `export struct RichHistory {`
- L93: `export fn rich_history_new() RichHistory {`
- L97: `export fn rich_history_new_with_allocator(allocator: &MallocAllocator) RichHistory {`

## `src/gui/richtext/layout.uya`

- L48: `export struct RichVisualLine {`
- L59: `export struct RichLayoutCache {`
- L505: `export fn rich_layout_cache_new() RichLayoutCache {`
- L509: `export fn rich_layout_cache_new_with_allocator(allocator: &MallocAllocator) RichLayoutCache {`

## `src/gui/richtext/render.uya`

- L245: `export struct RichTextRenderer {}`
- L247: `export fn rich_text_renderer_new() RichTextRenderer {`

## `src/gui/richtext/selection.uya`

- L1: `export struct RichPos {`
- L8: `export struct RichRange {`
- L13: `export fn rich_pos_zero() RichPos {`
- L22: `export fn rich_range_collapsed(pos: RichPos) RichRange {`
- L29: `export fn rich_pos_compare(a: RichPos, b: RichPos) i32 {`
- L57: `export fn rich_range_is_collapsed(range: RichRange) bool {`
- L61: `export fn rich_range_normalized(range: RichRange) RichRange {`

## `src/gui/sim/app.uya`

- L233: `export struct SimApp {`
- L2516: `export fn sim_app_alloc() &SimApp {`
  说明: 在堆上创建模拟器应用状态对象，避免大型状态压占调用栈。
- L2527: `export fn sim_app_free(app: &SimApp) void {`
  说明: 释放 `sim_app_alloc` 创建的模拟器应用状态对象。
- L2534: `export fn sim_app_new() SimApp {`
  说明: 创建模拟器应用状态对象。

## `src/gui/sim/common.uya`

- L4: `export const SIM_TEXT_CAPACITY: i32 = 160;`
- L5: `export const SIM_PATH_CAPACITY: i32 = 256;`
- L6: `export const SIM_DEFAULT_RES_WIDTH: u16 = 1920u16;`
- L7: `export const SIM_DEFAULT_RES_HEIGHT: u16 = 1080u16;`
- L8: `export const SIM_LAYOUT_BASE_WIDTH: u16 = 640u16;`
- L9: `export const SIM_LAYOUT_BASE_HEIGHT: u16 = 480u16;`
- L11: `export const SIM_KEY_ESCAPE: u16 = 27;`
- L12: `export const SIM_KEY_ENTER: u16 = 13;`
- L13: `export const SIM_KEY_SPACE: u16 = 32;`
- L14: `export const SIM_KEY_LEFT: u16 = 1000;`
- L15: `export const SIM_KEY_RIGHT: u16 = 1001;`
- L16: `export const SIM_KEY_UP: u16 = 1002;`
- L17: `export const SIM_KEY_DOWN: u16 = 1003;`
- L18: `export const SIM_KEY_F11: u16 = 1011;`
- L25: `export fn sim_layout_x(value: i16) i16 {`
  说明: 提供 `sim_layout_x` 的公开辅助入口。
- L30: `export fn sim_layout_y(value: i16) i16 {`
  说明: 提供 `sim_layout_y` 的公开辅助入口。
- L35: `export fn sim_layout_w(value: u16) u16 {`
  说明: 提供 `sim_layout_w` 的公开辅助入口。
- L40: `export fn sim_layout_h(value: u16) u16 {`
  说明: 提供 `sim_layout_h` 的公开辅助入口。
- L45: `export fn sim_layout_font(value: u8) u8 {`
  说明: 提供 `sim_layout_font` 的公开辅助入口。
- L62: `export fn sim_layout_square(value: u16) u16 {`
  说明: 提供 `sim_layout_square` 的公开辅助入口。
- L72: `export fn sim_layout_rect(x: i16, y: i16, w: u16, h: u16) Rect {`
  说明: 返回 `sim_layout` 对应的矩形区域。
- L82: `export fn sim_layout_square_rect(x: i16, y: i16, size: u16) Rect {`
  说明: 返回 `sim_layout_square` 对应的矩形区域。
- L93: `export fn sim_layout_size(w: u16, h: u16) Size {`
  说明: 返回 `sim_layout` 对应的尺寸。
- L101: `export fn sim_cstr_empty(text: &const byte) bool {`
  说明: 判断 C 字符串是否为空。
- L106: `export fn sim_cstr_eq(a: &const byte, b: &const byte) bool {`
  说明: 比较两个 C 字符串是否内容相等。
- L123: `export fn sim_copy_cstr(dst: &byte, capacity: usize, src: &const byte) bool {`
  说明: 将 C 字符串复制到固定容量缓冲区。

## `src/gui/sim/config.uya`

- L19: `export enum SimDemoKind {`
- L36: `export enum SimBackendKind {`
- L42: `export enum SimGpuKind {`
- L48: `export struct SimConfig {`
- L77: `export fn sim_demo_name(kind: SimDemoKind) &const byte {`
  说明: 返回指定 demo 的显示名称。
- L121: `export fn sim_demo_default_width(kind: SimDemoKind) u16 {`
  说明: 返回指定 demo 的默认宽度。
- L127: `export fn sim_demo_default_height(kind: SimDemoKind) u16 {`
  说明: 返回指定 demo 的默认高度。
- L133: `export fn sim_demo_is_phase(kind: SimDemoKind) bool {`
  说明: 判断指定 demo 是否属于 phase 首页场景。
- L138: `export fn sim_backend_name(kind: SimBackendKind) &const byte {`
  说明: 返回指定模拟器后端的显示名称。
- L149: `export fn sim_gpu_name(kind: SimGpuKind) &const byte {`
  说明: 返回指定 GPU 模式的显示名称。
- L160: `export fn sim_cpu_backend_name(kind: CpuRenderBackendKind) &const byte {`
  说明: 返回指定 CPU 渲染后端的显示名称。
- L172: `export fn sim_config_default() SimConfig {`
  说明: 返回默认的模拟器配置。
- L202: `export fn sim_config_apply_backend_defaults(cfg: &SimConfig) void {`
- L216: `export fn sim_config_from_runtime() SimConfig {`
  说明: 从环境变量和命令行参数构建模拟器配置。

## `src/gui/sim/dashboard_compare.uya`

- L300: `export fn run_dashboard_compare() i32 {`
  说明: 运行 Uya 侧 dashboard 对照程序并返回退出码。

## `src/gui/sim/main.uya`

- L4: `export fn sim_entry() i32 {`
  说明: 作为模拟器子入口运行 `run_simulator`。

## `src/gui/sim/profiler.uya`

- L3: `export struct ProfileStat {`
- L30: `export struct ProfilerSnapshot {`
- L39: `export struct SimProfiler {`
- L89: `export fn profile_stat_new() ProfileStat {`
  说明: 创建空的性能统计对象。
- L100: `export fn sim_profiler_new(report_every_frames: u32) SimProfiler {`
  说明: 创建按指定汇报周期工作的模拟器 profiler。

## `src/gui/sim/recorder.uya`

- L10: `export const SIM_RECORDER_CAPACITY: i32 = 1024;`
- L11: `export const SIM_RECORDER_MAGIC: u32 = 0x52435955u32;`
- L12: `export const SIM_RECORDER_VERSION: u16 = 1u16;`
- L13: `export const RECORDER_EVENT_FLAG_KEY_REPEAT: u16 = 1u16;`
- L18: `export enum RecorderEventKind {`
- L28: `export struct RecordedEvent {`
- L44: `export struct SimRecorder {`
- L195: `export fn sim_recorder_new() SimRecorder {`
  说明: 创建输入录制器实例。
- L207: `export fn recorded_event_is_key_repeat(evt: &RecordedEvent) bool {`

## `src/gui/sim/runner.uya`

- L239: `export fn run_simulator() i32 {`

## `src/gui/sim/runner_web.uya`

- L121: `export fn run_simulator_web_bootstrap() i32 {`
- L187: `export fn sim_web_frame(now_ms: i32) i32 {`
- L211: `export fn sim_web_shutdown() void {`
- L227: `export fn sim_web_richtext_export_plain_text(out: &byte, cap: usize) usize {`
- L240: `export fn sim_web_richtext_export_html(out: &byte, cap: usize) usize {`
- L253: `export fn sim_web_richtext_insert_text(text: &const byte, text_len: usize) bool {`
- L270: `export fn sim_web_richtext_paste_plain(text: &const byte, text_len: usize) bool {`
- L287: `export fn sim_web_richtext_focus_editor() bool {`
- L301: `export fn sim_web_richtext_select_all() bool {`

## `src/gui/sim/runtime_core.uya`

- L24: `export interface IRuntimeDisplay {`
- L34: `export interface IRuntimeInput {`
- L43: `export interface IRuntimeRenderBinder {`
- L47: `export struct SimFrameOutput {`
- L58: `export struct SimRuntimeCore {`
- L227: `export fn sim_frame_output_default() SimFrameOutput {`
- L240: `export fn sim_runtime_core_new() SimRuntimeCore {`

## `src/gui/sim/screenshot.uya`

- L17: `export const SIM_SCREENSHOT_MAGIC: u32 = 0x46425559u32;`
- L18: `export const SIM_SCREENSHOT_VERSION: u16 = 1u16;`
- L52: `export fn screenshot_payload_bytes(fb: &FrameBuffer) usize {`
  说明: 计算原始截图载荷需要的字节数。
- L231: `export fn screenshot_write_png(path: &const byte, fb: &FrameBuffer) !usize {`
  说明: 将帧缓冲写出为 PNG 文件。
- L318: `export fn screenshot_write_bmp(path: &const byte, fb: &FrameBuffer) !usize {`
  说明: 将帧缓冲写出为 BMP 文件。
- L384: `export fn screenshot_write_raw(path: &const byte, fb: &FrameBuffer) !usize {`
  说明: 将帧缓冲写出为原始自定义格式文件。
- L417: `export fn screenshot_write(path: &const byte, fb: &FrameBuffer) !usize {`
  说明: 按输出路径扩展名选择合适的截图写出格式。

## `src/gui/sim/web_present_plan.uya`

- L7: `export enum WebPresentMode {`
- L13: `export struct WebPresentPlan {`
- L18: `export fn web_build_present_plan(fb: &FrameBuffer, dirty: &DirtyRegion) WebPresentPlan {`

## `src/gui/style/prop.uya`

- L1: `export const MAX_STYLE_EXT_PROPS: i32 = 24;`
- L3: `export enum StyleProp {`

## `src/gui/style/style.uya`

- L143: `export enum StyleValueKind {`
- L152: `export struct StyleEntry {`
- L162: `export struct Style {`
- L674: `export struct StyleBuilder {`
- L790: `export fn style_builder_new() StyleBuilder {`
  说明: 创建样式构建器实例。
- L794: `export const DEFAULT_STYLE: Style = Style{`
- L824: `export fn style_empty() Style {`
  说明: 返回空样式。
- L829: `export fn style_default() Style {`
  说明: 返回默认样式。
- L834: `export fn style_make(bg: Color, fg: Color, border_width: u8, border_color: Color, radius: u8, opacity: u8) Style {`
  说明: 按显式参数构造样式值。
- L844: `export mc style(bg: expr, fg: expr, border_width: expr, border_color: expr, radius: expr, opacity: expr) expr {`

## `src/gui/style/theme.uya`

- L11: `export const MAX_THEMES: i32 = 8;`
- L27: `export enum ThemeVariant {`
- L33: `export struct Theme {`
- L51: `export struct ThemeManager {`
- L182: `export fn theme_manager_new() ThemeManager {`
  说明: 创建主题管理器实例。
- L192: `export fn material_light_theme() Theme {`
  说明: 返回 Material Light 预设主题。
- L213: `export fn material_dark_theme() Theme {`
  说明: 返回 Material Dark 预设主题。
- L234: `export fn compact_theme() Theme {`
  说明: 返回面向资源受限设备的紧凑主题。

## `src/gui/widget/base.uya`

- L28: `export const WIDGET_TYPE_WIDGET: u32 = 0x1000u32;`
- L29: `export const WIDGET_TYPE_BUTTON: u32 = 0x1001u32;`
- L30: `export const WIDGET_TYPE_LABEL: u32 = 0x1002u32;`
- L31: `export const WIDGET_TYPE_IMAGE: u32 = 0x1003u32;`
- L32: `export const WIDGET_TYPE_SLIDER: u32 = 0x1004u32;`
- L33: `export const WIDGET_TYPE_SWITCH: u32 = 0x1005u32;`
- L34: `export const WIDGET_TYPE_PAGE: u32 = 0x1006u32;`
- L35: `export const WIDGET_TYPE_PANEL: u32 = 0x1007u32;`
- L36: `export const WIDGET_TYPE_LIST: u32 = 0x1008u32;`
- L37: `export const WIDGET_TYPE_GRID_VIEW: u32 = 0x1009u32;`
- L38: `export const WIDGET_TYPE_CHART: u32 = 0x100Au32;`
- L39: `export const WIDGET_TYPE_CANVAS: u32 = 0x100Bu32;`
- L40: `export const WIDGET_TYPE_TEXT_INPUT: u32 = 0x100Cu32;`
- L41: `export const WIDGET_TYPE_RICH_TEXT_INPUT: u32 = 0x100Du32;`
- L42: `export const WIDGET_TYPE_RICH_TOOLBAR: u32 = 0x100Eu32;`
- L435: `export enum WidgetState {`
- L444: `export enum WidgetOrientation {`
- L449: `export interface IStyled {`
- L455: `export interface IWidgetEventCallback {`
- L459: `export interface IWidgetValueCallback {`
- L463: `export interface IScrollable {`
- L469: `export interface ISelectable {`
- L476: `export struct AnimState {`
- L481: `export struct WidgetEventSlot {`
- L486: `export struct WidgetValueSlot {`
- L491: `export struct Widget : IStyled {`
- L663: `export fn widget_text_len(text: &const byte) usize {`
  说明: 计算以 NUL 结尾的组件文本长度。
- L671: `export fn widget_new(name_ptr: &const byte, type_tag: u32, clickable: bool, focusable: bool) Widget {`
  说明: 创建基础 `Widget` 并初始化交互标志。
- L690: `export fn widget_bind_callbacks(base: &GuiObj, render_cb: &IGuiRenderCallback, input_cb: &IGuiInputCallback, user_data: &void) &GuiObj {`
  说明: 为 `GuiObj` 绑定渲染与输入回调。
- L698: `export fn widget_font(style: &Style) &Font {`
  说明: 根据样式解析组件当前应使用的字体引用。
- L717: `export fn widget_text_color(style: &Style, state: WidgetState) Color {`
  说明: 按组件状态解析文本颜色。
- L733: `export fn widget_state_bg(style: &Style, state: WidgetState) Color {`
  说明: 按组件状态解析背景颜色。
- L751: `export fn widget_inner_rect(widget: &Widget) Rect {`
  说明: 计算组件去除 padding 后的内容区域。
- L774: `export fn widget_draw_surface(widget: &Widget, ctx: &RenderCtx) Rect {`
  说明: 绘制组件表面并返回内部内容区域。
- L803: `export fn widget_render_children(base: &GuiObj, ctx: &RenderCtx) void {`
  说明: 按当前坐标渲染组件的直接子节点。
- L812: `export fn widget_render_children_translated(base: &GuiObj, ctx: &RenderCtx, dx: i16, dy: i16) void {`
  说明: 带平移量渲染组件的直接子节点。
- L819: `export fn widget_measure_text(text: &const byte, font: &Font) i16 {`
  说明: 测量文本在指定字体下的像素宽度。
- L828: `export fn widget_clone_color(color: Color) Color {`
  说明: 复制颜色值。
- L833: `export fn widget_default_font() Font {`
  说明: 返回默认组件字体。

## `src/gui/widget/btn.uya`

- L69: `export enum BtnVariant {`
- L83: `export struct Button {`

## `src/gui/widget/canvas.uya`

- L30: `export struct Canvas {`

## `src/gui/widget/chart.uya`

- L13: `export const MAX_CHART_POINTS: i32 = 256;`
- L15: `export enum ChartType {`
- L43: `export struct Chart {`

## `src/gui/widget/grid_view.uya`

- L76: `export const MAX_GRID_ITEMS: i32 = 128;`
- L78: `export enum GridCellContentMode {`
- L83: `export enum GridContentAlign {`
- L93: `export struct GridView : IContainer {`

## `src/gui/widget/img.uya`

- L25: `export enum ImageScale {`
- L37: `export struct Image {`

## `src/gui/widget/lbl.uya`

- L32: `export const MAX_LABEL_LINES: i32 = 8;`
- L34: `export enum TextAlign {`
- L46: `export enum TextOverflow {`
- L67: `export struct Label {`
- L394: `export const TEXT_INPUT_BUFFER_BYTES: i32 = 128;`
- L482: `export struct TextInput {`

## `src/gui/widget/list.uya`

- L24: `export const MAX_LIST_ITEMS: i32 = 1024;`
- L37: `export struct ListView : IContainer, IScrollable, ISelectable {`

## `src/gui/widget/page.uya`

- L151: `export struct Page : IContainer {`
- L336: `export enum PageTransitionKind {`
- L347: `export interface IPageTransitionCallback {`
- L351: `export struct PageTransitionState {`
- L362: `export const PAGE_STACK_CAPACITY: i32 = 8;`
- L364: `export struct PageNavigator {`
- L563: `export fn page_transition_none() PageTransitionState {`
  说明: 返回空的页面过渡状态。
- L577: `export fn page_navigator_new(viewport: Rect) PageNavigator {`
  说明: 创建页面导航器实例。

## `src/gui/widget/panel.uya`

- L132: `export struct Panel : IContainer {`

## `src/gui/widget/rich_text_input.uya`

- L405: `export enum RichCommandKind {`
- L435: `export struct RichCommand {`
- L441: `export struct RichTextFormatState {`
- L449: `export struct RichTextInput {`

## `src/gui/widget/rich_toolbar.uya`

- L34: `export const RICH_TOOLBAR_MAX_FORMATS: i32 = 16;`
- L36: `export enum RichToolbarFormatKind {`
- L53: `export struct RichToolbarFormatSpec {`
- L63: `export struct RichFormatRegistry {`
- L89: `export fn rich_format_registry_new() RichFormatRegistry {`
- L140: `export struct RichToolbar {`

## `src/gui/widget/slider.uya`

- L35: `export struct Slider {`

## `src/gui/widget/switch.uya`

- L24: `export struct Switch {`

## `tests/test_anim.uya`

- L30: `export const TEST_ANIM_MODULE: i32 = 1;`

## `tests/test_bitmap.uya`

- L7: `export const TEST_BITMAP_MODULE: i32 = 1;`

## `tests/test_color.uya`

- L13: `export const TEST_COLOR_MODULE: i32 = 1;`

## `tests/test_core_types.uya`

- L12: `export const TEST_CORE_TYPES_MODULE: i32 = 1;`

## `tests/test_dirty_region.uya`

- L9: `export const TEST_DIRTY_REGION_MODULE: i32 = 1;`

## `tests/test_doudizhu_ai.uya`

- L24: `export const TEST_DOUDIZHU_AI_MODULE: i32 = 1;`

## `tests/test_doudizhu_rules.uya`

- L27: `export const TEST_DOUDIZHU_RULES_MODULE: i32 = 1;`

## `tests/test_event.uya`

- L27: `export const TEST_EVENT_MODULE: i32 = 1;`

## `tests/test_event_dispatch.uya`

- L25: `export const TEST_EVENT_DISPATCH_MODULE: i32 = 1;`

## `tests/test_input_dev.uya`

- L37: `export const TEST_INPUT_DEV_MODULE: i32 = 1;`

## `tests/test_integration.uya`

- L33: `export const TEST_INTEGRATION_MODULE: i32 = 1;`

## `tests/test_layout.uya`

- L25: `export const TEST_LAYOUT_MODULE: i32 = 1;`

## `tests/test_obj_tree.uya`

- L27: `export const TEST_OBJ_TREE_MODULE: i32 = 1;`

## `tests/test_phase4_io.uya`

- L23: `export const TEST_PHASE4_IO_MODULE: i32 = 1;`

## `tests/test_phase5_runtime.uya`

- L45: `export const TEST_PHASE5_RUNTIME_MODULE: i32 = 1;`

## `tests/test_phase6_examples.uya`

- L21: `export const TEST_PHASE6_EXAMPLES_MODULE: i32 = 1;`

## `tests/test_pool.uya`

- L20: `export const TEST_POOL_MODULE: i32 = 1;`

## `tests/test_rect.uya`

- L8: `export const TEST_RECT_MODULE: i32 = 1;`

## `tests/test_render_assets.uya`

- L70: `export const TEST_RENDER_ASSETS_MODULE: i32 = 1;`

## `tests/test_render_ctx.uya`

- L17: `export const TEST_RENDER_CTX_MODULE: i32 = 1;`

## `tests/test_render_pipeline.uya`

- L30: `export const TEST_RENDER_PIPELINE_MODULE: i32 = 1;`

## `tests/test_render_scheduler.uya`

- L24: `export const TEST_RENDER_SCHEDULER_MODULE: i32 = 1;`

## `tests/test_rich_toolbar.uya`

- L44: `export const TEST_RICH_TOOLBAR_MODULE: i32 = 1;`

## `tests/test_richtext_delta.uya`

- L30: `export const TEST_RICHTEXT_DELTA_MODULE: i32 = 1;`

## `tests/test_richtext_document.uya`

- L13: `export const TEST_RICHTEXT_DOCUMENT_MODULE: i32 = 1;`

## `tests/test_richtext_history.uya`

- L25: `export const TEST_RICHTEXT_HISTORY_MODULE: i32 = 1;`

## `tests/test_richtext_layout.uya`

- L18: `export const TEST_RICHTEXT_LAYOUT_MODULE: i32 = 1;`

## `tests/test_richtext_render.uya`

- L25: `export const TEST_RICHTEXT_RENDER_MODULE: i32 = 1;`

## `tests/test_richtext_widget.uya`

- L50: `export const TEST_RICHTEXT_WIDGET_MODULE: i32 = 1;`

## `tests/test_sim_app.uya`

- L57: `export const TEST_SIM_APP_MODULE: i32 = 1;`

## `tests/test_sim_tools.uya`

- L22: `export const TEST_SIM_TOOLS_MODULE: i32 = 1;`

## `tests/test_style.uya`

- L24: `export const TEST_STYLE_MODULE: i32 = 1;`

## `tests/test_text_compare.uya`

- L8: `export const TEST_TEXT_COMPARE_MODULE: i32 = 1;`

## `tests/test_utils.uya`

- L9: `export fn _assert_eq_impl<T>(actual: &T, expected: &T, actual_src: &const byte, expected_src: &const byte) !void {`
  说明: 执行测试用的泛型相等断言。
- L16: `export mc assert_eq(actual: expr, expected: expr) expr {`
- L21: `export fn _assert_near_impl(actual: f64, expected: f64, epsilon: f64, actual_src: &const byte, expected_src: &const byte) !void {`
  说明: 执行测试用的浮点近似断言。
- L28: `export mc assert_near(actual: expr, expected: expr, epsilon: expr) expr {`
- L33: `export fn _test_suite_impl(name: &const byte) void {`
  说明: 作为测试套件宏的运行时入口。
- L37: `export mc test_suite(name: expr) expr {`

## `tests/test_web_backend.uya`

- L28: `export const TEST_WEB_BACKEND_MODULE: i32 = 1;`

## `tests/test_web_config.uya`

- L9: `export const TEST_WEB_CONFIG_MODULE: i32 = 1;`

## `tests/test_web_present_plan.uya`

- L12: `export const TEST_WEB_PRESENT_PLAN_MODULE: i32 = 1;`

## `tests/test_widgets.uya`

- L59: `export const TEST_WIDGET_MODULE: i32 = 1;`
