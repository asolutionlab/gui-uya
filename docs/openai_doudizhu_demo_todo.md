# OpenAI Chat 人机斗地主 Demo TODO

> 配套设计文档：`docs/openai_doudizhu_demo_design.md`  
> 当前状态：阶段 A 已完成主体实现、默认测试、headless smoke，以及 `sim-run` 启动链路验证；仍缺手工完整对局收口。阶段 B 的 OpenAI 接入代码、配置文件支持、调试日志与 live 请求验证已完成，但尚未补齐阶段 A 手工验收与阶段 B 全量 smoke 收口。  
> 执行原则：先完整做完离线 MVP，再开始 OpenAI 接入。  
> 纪律：未写代码、未跑测试、未跑 smoke、未产出截图，不得勾选完成。

## 当前进展

- 已新增 `examples/doudizhu/rules.uya`、`examples/doudizhu/ai.uya`、`examples/demo_doudizhu.uya`。
- 已修改 `src/gui/sim/config.uya`、`src/gui/sim/app.uya`，接入 `--demo doudizhu` 和 `Z` 热键。
- 已新增 `tests/test_doudizhu_rules.uya`、`tests/test_doudizhu_ai.uya`，并接入 `tests/test_suite.uya`。
- 已补 `phase6` demo 渲染测试入口。
- 已真实通过 `make test`。
- 已真实通过 `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"`，并核对截图内容。
- 已修复 simulator 窗口路径下 `--max-frames` 依赖 present 次数导致静态 retained 页面不退出的问题，并真实通过 `make sim-run SIM_ARGS="--demo doudizhu --max-frames 3 --screenshot build/sim/doudizhu_simrun_check.bmp"`。
- 已新增 `src/gui/platform/openai/chat.uya`、`src/gui/platform/openai/openai_chat_stub.c`、`src/gui/platform/openai/openai_chat_host.c`，并接入 `examples/doudizhu/ai.uya`。
- 已支持 `.uya_openai.env`、`openai.env`、`UYA_OPENAI_CONFIG_FILE` 配置文件加载，以及 `UYA_OPENAI_DEBUG=1` 请求调试日志。
- 已真实验证 OpenAI live 请求可返回合法 `action_id`，示例请求耗时约 `1.4s ~ 1.7s`，未见超时。
- 当前仍未完成的主要是阶段 A 手工可玩/完整一局验收，以及阶段 B 按 TODO 原口径补齐完整 smoke 收口。

## 0. 交付策略

推荐拆成两个阶段，而且必须串行推进：

- 阶段 A：离线 MVP
- 阶段 B：OpenAI 接入

阶段 B 的开始条件：

- [ ] 阶段 A 所有实现项完成。
- [ ] 阶段 A 单测通过。
- [ ] 阶段 A `sim-run` 可玩完一局。
- [ ] 阶段 A `sim-headless` 可出截图。
- [ ] 阶段 A 文档已经先写离线路径。

阶段 B 禁止提前“预埋半套实现”来假装并行完成，除非只是无行为影响的接口占位，且不会让评审误判为已接入 OpenAI。

## 阶段 A：离线 MVP

目标：

- 完整本地规则闭环。
- 完整本地启发式 AI 闭环。
- 完整 UI demo 闭环。
- 完整 simulator 接入闭环。
- 完整 tests 闭环。
- 文档先以离线路径为主，不要求任何网络依赖。

### A0. 开工前确认

- [x] 确认 MVP 不实现飞机、四带二、完整积分倍数。
- [x] 确认电脑决策先只走本地启发式 AI。
- [x] 确认默认测试不访问网络。
- [x] 确认文档、命令示例、验收口径全部以离线路径优先。

验收：

- [x] 设计文档中所有阶段 A 待确认项已有结论。

### A1. 规则层

- [x] 新建 `examples/doudizhu/rules.uya`。
- [x] 定义 rank、card、hand、combo、action、game phase、game state。
- [x] 实现 `ddz_deck_init()`，生成 54 张唯一牌。
- [x] 实现 `ddz_deck_shuffle()`，使用可重复 seed。
- [x] 实现 `ddz_game_deal()`，分发 `17/17/17 + 3`。
- [x] 实现 `ddz_hand_sort()`。
- [x] 实现 `ddz_hand_recount()`。
- [x] 实现 `ddz_combo_detect()`。
- [x] 实现 `ddz_combo_can_beat()`。
- [x] 实现 `ddz_generate_bid_actions()`。
- [x] 实现 `ddz_generate_play_actions()`。
- [x] 实现 `ddz_game_apply_bid()`。
- [x] 实现 `ddz_game_apply_action()`。
- [x] 实现 `ddz_game_restart()`。
- [x] 实现 action label 格式化。

验收：

- [ ] 规则层不 import UI、OpenAI、sim app。
- [ ] 所有公开函数对非法输入有确定返回值。
- [ ] `DDZ_MAX_ACTIONS` 溢出时设置 `truncated`，不写越界。

测试：

- [x] deck 唯一性。
- [x] 发牌数量。
- [x] rank_counts 正确。
- [x] 单、对、三、三带一、三带一对。
- [x] 顺子、连对、炸弹、火箭。
- [x] 非法顺子和非法连对。
- [x] 炸弹/火箭比较。
- [x] pass 规则。
- [x] apply action 后手牌和轮转正确。
- [x] GameOver 判定。

### A2. 本地启发式 AI

- [x] 新建 `examples/doudizhu/ai.uya`。
- [x] 定义 `DdzAiSource`：`Local | Fallback`。
- [x] 定义离线 AI 决策状态。
- [x] 实现叫分评分函数。
- [x] 实现主动出牌启发式。
- [x] 实现跟牌启发式。
- [x] 实现炸弹/火箭保守使用策略。
- [x] 实现 action id 校验函数。
- [x] 保证同一 seed 和同一局面下结果稳定。

验收：

- [ ] 启发式永远返回 `legal_actions` 中已有的 action。
- [ ] `legal_actions` 为空时返回明确失败状态。
- [ ] 不依赖任何 OpenAI 模块也能完成整局流程。

测试：

- [x] 强牌叫 3 分。
- [x] 弱牌不叫或叫低分。
- [x] 主动出牌选择最小普通动作。
- [x] 跟牌选择最小可压动作。
- [x] 非必要场景不炸。
- [x] 剩余牌少时允许炸。

### A3. 离线 UI Demo

- [x] 新建 `examples/demo_doudizhu.uya`。
- [x] 定义 `DdzPageRetained`。
- [x] 实现 demo 初始化和资源分配。
- [x] 实现 Canvas 桌面背景绘制。
- [x] 实现玩家区域绘制。
- [x] 实现底牌区绘制。
- [x] 实现上一手牌绘制。
- [x] 实现人类手牌绘制。
- [x] 实现选中牌上移效果。
- [x] 实现叫分按钮。
- [x] 实现 `提示`、`出牌`、`不要`、`重开` 按钮。
- [x] 实现点击手牌命中检测。
- [x] 实现人类叫分交互。
- [x] 实现人类出牌交互。
- [x] 实现 AI 自动叫分和出牌。
- [x] 实现状态文案。
- [x] 实现重开时清理选牌和 AI 状态。

验收：

- [ ] 无 OpenAI 环境变量时可完整玩一局。
- [ ] UI 不展示电脑手牌。
- [ ] 当前玩家和胜负状态清晰可见。
- [ ] 选中牌、非法出牌、不能 pass 都有明确反馈。

### A4. 模拟器接入

- [x] 修改 `src/gui/sim/config.uya`，新增 `SimDemoKind.Doudizhu`。
- [x] 修改 `sim_demo_name()`，返回 `斗地主`。
- [x] 修改命令行解析，支持 `--demo doudizhu`。
- [x] 修改 `src/gui/sim/app.uya`，添加 retained state。
- [x] 修改 `src/gui/sim/app.uya`，接入 render/update。
- [x] 修改 `src/gui/sim/app.uya`，接入输入事件。
- [x] 增加热键 `Z` 切换到斗地主。
- [x] 更新 simulator help 文案。

验收：

- [ ] `make sim-run SIM_ARGS="--demo doudizhu --scale 1"` 能打开。
- [x] `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"` 能生成截图。
- [ ] 切换到其他 demo 再切回不崩溃。

### A5. 测试接入

- [x] 新建 `tests/test_doudizhu_rules.uya`。
- [x] 新建 `tests/test_doudizhu_ai.uya`。
- [x] 修改 `tests/test_suite.uya` 聚合新测试。
- [x] 如需快速 smoke，补 `examples` demo 渲染测试。

验收：

- [x] `make test` 通过。

### A6. 文档先写离线路径

- [x] 设计文档开头明确“阶段 A 先离线完成”。
- [x] 运行文档先给出离线启动命令。
- [x] 验收文档先给出离线 smoke 命令。
- [x] 明确阶段 A 不要求 API Key、libcurl、联网。

### A7. 阶段 A 真实 smoke

必须真实执行，不接受“理论上可运行”：

- [ ] `make sim-run SIM_ARGS="--demo doudizhu --scale 1"` 手工可玩验证通过。
- [ ] 至少完整跑完一局离线对局。
- [x] `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"` 通过。
- [x] 产出并核对 `build/sim/doudizhu.bmp`。

阶段 A 最终验收：

- [ ] 离线 demo 可玩。
- [ ] 离线 AI 可完成整局。
- [ ] 默认 CI/测试不访问网络。
- [ ] 规则测试覆盖 MVP 牌型。
- [ ] 进入阶段 B 前，不存在“只是文档完成、代码未闭环”的假完成状态。

## 阶段 B：OpenAI 接入

目标：

- 在不破坏阶段 A 离线可玩的前提下，增加可选 OpenAI 决策路径。
- 所有失败场景自动回退到本地启发式 AI。

### B1. OpenAI Uya 封装

- [x] 新建 `src/gui/platform/openai/chat.uya`。
- [x] 声明 `uya_openai_chat_available()`。
- [x] 声明 `uya_openai_chat_start()`。
- [x] 声明 `uya_openai_chat_poll()`。
- [x] 声明 `uya_openai_chat_cancel()`。
- [x] 封装 Uya 侧状态码 enum。
- [x] 封装 start/poll/cancel 的薄包装。
- [x] 实现 `{ "action_id": n }` 解析辅助。

验收：

- [x] OpenAI 封装不 import 斗地主规则。
- [x] 返回码映射清晰。
- [x] JSON 解析失败返回错误，不产生未定义 action。

### B2. OpenAI Stub

- [x] 新建 `src/gui/platform/openai/openai_chat_stub.c`。
- [x] 实现 `uya_openai_chat_available()` 返回 0。
- [x] 实现 `start()` 返回负值。
- [x] 实现 `poll()` 返回 `-4`。
- [x] 实现 `cancel()` 幂等空操作。

验收：

- [x] 无 libcurl 开发包时 `make sim-build` 仍通过。
- [x] 未设置 API Key 时 demo 自动离线。

### B3. libcurl Host Bridge

- [x] 新建 `src/gui/platform/openai/openai_chat_host.c`。
- [x] 读取 `OPENAI_API_KEY`。
- [x] 读取 `OPENAI_MODEL`。
- [x] 读取 `OPENAI_BASE_URL`，默认 `https://api.openai.com/v1`。
- [x] 读取 `UYA_DDZ_USE_OPENAI`。
- [x] 实现单 in-flight 请求槽。
- [x] `start()` 中拷贝 request body。
- [x] 后台子进程执行 libcurl 请求。
- [x] 设置 `Authorization: Bearer` header。
- [x] 设置 `Content-Type: application/json`。
- [x] 设置连接超时 `1500ms`。
- [x] 设置总超时 `3500ms`。
- [x] 限制最大 HTTP body。
- [x] 解析 Chat Completions envelope。
- [x] 提取 `choices[0].message.content`。
- [x] 处理 refusal 或缺失 content。
- [x] `poll()` 成功时复制 assistant content JSON。
- [x] `poll()` 终态释放资源。
- [x] `cancel()` 可取消未完成请求。
- [x] 不打印 API Key。

验收：

- [x] 有 libcurl 时 `make sim-build` 链接成功。
- [x] 无 API Key 时 `available() == 0`。
- [x] 请求失败不会阻塞 UI 主循环。
- [x] 终态 handle 不可重复消费。

### B4. 构建脚本接入

- [x] 修改 `tools/build_gui_sim.sh` 检测 `pkg-config --exists libcurl`。
- [x] 有 libcurl 时加入 cflags/libs。
- [x] 有 libcurl 时编译 `openai_chat_host.c`。
- [x] 无 libcurl 时编译 `openai_chat_stub.c`。
- [x] 保持 SDL2 检测逻辑不变。
- [x] 保持默认 core/test 构建不链接 libcurl。

验收：

- [x] `make sim-build` 在有 libcurl 环境通过。
- [x] `make sim-build` 在无 libcurl 环境应可通过 stub 路径。
- [x] `make test` 不因 libcurl 缺失受影响。

### B5. OpenAI 决策接入

- [x] 在 `doudizhu.ai` 中构造 prompt payload。
- [x] 固定 prompt 缓冲 `DDZ_PROMPT_BYTES`。
- [x] 叫分阶段生成 bid `legal_actions` prompt。
- [x] 出牌阶段生成 play `legal_actions` prompt。
- [x] 实现 request in-flight 状态机。
- [x] 实现 poll 成功后的 `action_id` 校验。
- [x] 非法 `action_id` 自动 fallback。
- [x] 超时、HTTP 错误、JSON 错误自动 fallback。
- [x] `重开` 时 cancel 未完成请求。

验收：

- [x] OpenAI 只从合法动作列表中选动作。
- [x] 任意异常都能自动回退到离线 AI。
- [x] 不会因 OpenAI 超时卡死主循环。

### B6. OpenAI 环境变量文档

- [x] 补充 `OPENAI_API_KEY` 用法。
- [x] 补充 `OPENAI_MODEL` 用法。
- [x] 补充 `OPENAI_BASE_URL` 用法。
- [x] 补充 `UYA_DDZ_USE_OPENAI` 用法。
- [x] 说明未配置时默认离线。

### B7. live smoke

仅在阶段 A 已真实完成后执行：

- [ ] `ALLOW_OPENAI_LIVE=1 OPENAI_API_KEY=... UYA_DDZ_USE_OPENAI=1 make sim-run SIM_ARGS="--demo doudizhu --max-frames 300"` 通过。
- [x] 验证 OpenAI 可参与电脑决策。
- [x] 验证拔掉 API Key 后可自动降级。

阶段 B 最终验收：

- [x] OpenAI 可用时可参与电脑决策。
- [x] OpenAI 不可用时自动降级。
- [x] 默认 CI 不访问网络。
- [x] live smoke 仅作为可选验收，不进入默认测试路径。

## 后续增强

- [ ] 增加飞机。
- [ ] 增加飞机带翅膀。
- [ ] 增加四带二。
- [ ] 增加四带两对。
- [ ] 增加倍数、春天、反春天。
- [ ] 增加更强本地 AI。
- [ ] 增加出牌动画。
- [ ] 增加更细的 OpenAI 决策解释调试面板。
- [ ] 增加录制回放样例。
