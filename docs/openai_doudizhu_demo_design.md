# OpenAI Chat 人机斗地主 Demo 详细设计文档

> 状态：阶段 A 主体实现、默认测试、headless smoke 与 `sim-run` 启动链路验证已完成；仍缺手工完整对局收口  
> 推荐交付顺序：先完成阶段 A 离线 MVP，再进入阶段 B OpenAI 接入。  
> 核心要求：阶段 A 必须真实可玩、可测、可 smoke，不能靠文档或占位代码假装完成。

## 当前实现状态

- 已落地阶段 A 主体代码：`rules.uya`、`ai.uya`、`demo_doudizhu.uya`、sim 接入和基础测试文件均已创建并接线。
- 当前仍以离线路径为唯一完成口径；OpenAI 相关文件尚未开始。
- 已真实通过默认测试链路：`make test`。
- 已真实通过 headless smoke，并产出/核对 `build/sim/doudizhu.bmp` 截图。
- 已修复 simulator 窗口路径下 `--max-frames` 对静态 retained 页面不退出的问题，并真实通过 `make sim-run SIM_ARGS="--demo doudizhu --max-frames 3 --screenshot build/sim/doudizhu_simrun_check.bmp"`。
- 文档中仍保留未完成的只有手工可玩与完整打一局相关项。

## 1. 项目背景

当前仓库是 UyaGUI 嵌入式 GUI/模拟器工程，已有 `2048`、`novel`、`widgets` 等 retained-state demo。斗地主 demo 应沿用现有 demo 结构：示例逻辑放在 `examples/*`，模拟器入口由 `src/gui/sim/config.uya` 和 `src/gui/sim/app.uya` 接入，运行通过 `make sim-run` / `make sim-headless` 验证。

本项目最终目标包含 OpenAI 决策路径，但开发上不建议一开始把规则、UI、模拟器接入、网络桥接混做。推荐拆成两个严格串行的阶段：

- 阶段 A：离线 MVP
- 阶段 B：OpenAI 接入

这样做的原因：

- 规则正确性必须先独立成立，否则后续所有 AI 决策都不可信。
- UI 交互和 simulator 接入必须先形成完整闭环，否则很难判断问题来自规则、输入还是网络层。
- OpenAI 只应该是“可选电脑决策源”，不应该成为 MVP 可玩的前置条件。
- 默认测试和 CI 必须先稳定在离线路径上，避免把联网调用引入基础回归链路。

## 2. 阶段划分与 gate

### 2.1 阶段 A：离线 MVP

阶段 A 必须一次性完成以下闭环：

- 规则层
- 本地启发式 AI
- UI demo
- simulator 接入
- tests
- 文档先写离线路径

阶段 A 的真实完成标准：

- 本地可完整玩完一局。
- 默认测试不访问网络。
- `make sim-run SIM_ARGS="--demo doudizhu --scale 1"` 可打开并手工验证。
- `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"` 可产出截图。
- 单测和 smoke 结果有真实命令验证，不靠“理论上应该可以”。

### 2.2 阶段 B：OpenAI 接入

只有阶段 A 全部完成后才进入阶段 B。阶段 B 包含：

- `chat.uya`
- `stub.c`
- `host.c + libcurl`
- prompt / poll / fallback / cancel
- OpenAI 环境变量文档
- live smoke

阶段 B 的目标不是替代阶段 A，而是在不破坏离线闭环的前提下增加可选网络决策路径。

## 3. 设计目标

1. 阶段 A 先交付一个可玩的 UyaGUI 斗地主 demo：`make sim-run SIM_ARGS="--demo doudizhu --scale 1"`。
2. 支持单局完整流程：发牌、叫地主、发底牌、出牌、不要、轮转、胜负判定、重开。
3. 电脑玩家在阶段 A 使用本地启发式 AI，在阶段 B 可选接入 OpenAI。
4. OpenAI 只允许从本地生成的 `legal_actions` 中选择动作，不能绕过规则。
5. 任何 OpenAI 失败场景都必须自动回退到本地启发式 AI。
6. 默认测试与 CI 不依赖网络，不依赖真实 OpenAI Key。
7. 保持 UyaGUI 当前风格：固定容量数组、明确生命周期、无默认动态 GC、可 headless 截图验证。

## 4. 非目标

1. 不实现联网多人对战。
2. 不把 OpenAI Key 写入代码、日志、截图或 UI 文本。
3. MVP 不做完整积分体系、倍数、春天、反春天、音效和复杂动画。
4. MVP 不覆盖全部斗地主边角牌型，先覆盖能稳定完成对局的核心牌型。
5. 不把 OpenAI HTTPS 调用放入裸机目标路径；首版只在 Linux SDL2 模拟器中启用。
6. 不追求 AI 强度，首版重点是稳定、合法、可解释、可回退。

## 5. 用户体验

### 5.1 阶段 A：离线运行

```bash
make sim-run SIM_ARGS="--demo doudizhu --scale 1"
```

离线模式要求：

- 不设置 `OPENAI_API_KEY` 也能完整玩一局。
- 不要求 libcurl。
- 不要求任何联网依赖。

### 5.2 阶段 B：启用 OpenAI

```bash
OPENAI_API_KEY=... \
OPENAI_MODEL=gpt-5.4-mini \
UYA_DDZ_USE_OPENAI=1 \
make sim-run SIM_ARGS="--demo doudizhu --scale 1"
```

用户看到的行为：

- 进入 demo 后自动发牌。
- 人类玩家在叫分阶段点击 `不叫`、`1分`、`2分`、`3分`。
- 出牌阶段点击手牌选中或取消，选中牌上移。
- 点击 `提示` 时只选择推荐牌，不自动提交。
- 点击 `出牌` 提交选中牌。
- 点击 `不要` 在规则允许时跳过。
- 点击 `重开` 重新开始；若阶段 B 正在请求 OpenAI，则同时 cancel 请求。
- AI 回合显示简短状态，例如 `Local`、`OpenAI`、`Fallback`、`Timeout`。

## 6. MVP 规则范围

牌组：

- 54 张牌：`3 4 5 6 7 8 9 10 J Q K A 2 BJ RJ`。
- 花色只用于显示和区分实体牌，比较时只看 rank。
- 三家各 17 张，底牌 3 张。
- 地主拿底牌后最大手牌数为 20。

叫地主：

- 简化为一轮叫分，每家一次机会。
- 动作为 `bid_0`、`bid_1`、`bid_2`、`bid_3`。
- 最高分者为地主；无人叫分则自动重新发牌。
- 若多人同分，先叫到最高分者保持领先。
- 地主拿底牌后进入出牌阶段，地主先出。

出牌牌型：

- `Pass`
- 单张
- 对子
- 三张
- 三带一
- 三带一对
- 顺子，至少 5 张，不含 `2` 和大小王
- 连对，至少 3 对，不含 `2` 和大小王
- 炸弹
- 火箭

暂不纳入 MVP 的牌型：

- 飞机
- 飞机带翅膀
- 四带二
- 四带两对

## 7. 总体架构

### 7.1 阶段 A 架构

```text
SDL2 input / keyboard
  -> sim.app
    -> demo_doudizhu retained page
      -> doudizhu.rules    本地规则、牌型、合法动作
      -> doudizhu.ai       本地启发式决策
```

### 7.2 阶段 B 增量架构

```text
SDL2 input / keyboard
  -> sim.app
    -> demo_doudizhu retained page
      -> doudizhu.rules
      -> doudizhu.ai
        -> platform.openai.chat.uya
          -> openai_chat_host.c / openai_chat_stub.c
            -> libcurl
            -> POST /v1/chat/completions
```

核心原则：

- 规则层不依赖 UI、不依赖 OpenAI，必须可单元测试。
- AI 层只拿规则层生成的 `DdzActionList`，不能手写绕过规则的出牌。
- UI 层只负责展示、输入、状态同步，不复制牌型判断。
- OpenAI host bridge 不知道斗地主规则，只负责 HTTPS 调用、外层响应解析、生命周期管理。

## 8. 文件与模块

### 8.1 阶段 A 新增文件

- `examples/demo_doudizhu.uya`
- `examples/doudizhu/rules.uya`
- `examples/doudizhu/ai.uya`
- `tests/test_doudizhu_rules.uya`
- `tests/test_doudizhu_ai.uya`

### 8.2 阶段 A 修改文件

- `src/gui/sim/config.uya`
- `src/gui/sim/app.uya`
- `tests/test_suite.uya`
- `docs/openai_doudizhu_demo_todo.md`

### 8.3 阶段 B 新增文件

- `src/gui/platform/openai/chat.uya`
- `src/gui/platform/openai/openai_chat_host.c`
- `src/gui/platform/openai/openai_chat_stub.c`

### 8.4 阶段 B 修改文件

- `tools/build_gui_sim.sh`
- `README.md`
- `examples/doudizhu/ai.uya`

## 9. 常量与数据结构

固定容量常量：

```text
DDZ_PLAYER_COUNT = 3
DDZ_DECK_COUNT = 54
DDZ_BOTTOM_COUNT = 3
DDZ_INITIAL_HAND_COUNT = 17
DDZ_MAX_CARDS_PER_HAND = 20
DDZ_RANK_COUNT = 15
DDZ_MAX_COMBO_CARDS = 20
DDZ_MAX_ACTIONS = 128
DDZ_ACTION_LABEL_BYTES = 48
DDZ_PROMPT_BYTES = 8192
DDZ_OPENAI_RESPONSE_BYTES = 1024
```

Rank 映射：

```text
0  -> 3
1  -> 4
2  -> 5
3  -> 6
4  -> 7
5  -> 8
6  -> 9
7  -> 10
8  -> J
9  -> Q
10 -> K
11 -> A
12 -> 2
13 -> BJ
14 -> RJ
```

核心结构草案：

```text
DdzCard
  id: i32
  rank: i32
  suit: i32

DdzHand
  cards: [DdzCard; DDZ_MAX_CARDS_PER_HAND]
  count: i32
  rank_counts: [i32; DDZ_RANK_COUNT]

DdzComboKind
  Invalid | Pass | Single | Pair | Triple | TripleSingle | TriplePair |
  Straight | PairStraight | Bomb | Rocket

DdzCombo
  kind: DdzComboKind
  main_rank: i32
  sequence_len: i32
  cards: [DdzCard; DDZ_MAX_COMBO_CARDS]
  count: i32

DdzAction
  id: i32
  combo: DdzCombo
  label: [byte; DDZ_ACTION_LABEL_BYTES]

DdzActionList
  actions: [DdzAction; DDZ_MAX_ACTIONS]
  count: i32
  truncated: bool

DdzGamePhase
  Deal | Bid | RevealBottom | Play | GameOver

DdzGame
  hands: [DdzHand; 3]
  bottom: [DdzCard; 3]
  landlord: i32
  current_player: i32
  last_combo: DdzCombo
  last_player: i32
  pass_count: i32
  bid_values: [i32; 3]
  highest_bid: i32
  highest_bidder: i32
  bid_turn: i32
  phase: DdzGamePhase
  winner: i32
  rng_state: u32
```

UI 状态结构：

```text
DdzPageRetained
  game: DdzGame
  ai: DdzAiController
  selected_card_ids: [i32; DDZ_MAX_CARDS_PER_HAND]
  selected_count: i32
  last_play_label: [byte; 96]
  status_label: [byte; 128]
  ai_status_label: [byte; 96]
  canvas + buttons + retained widget tree
```

## 10. 阶段 A 详细设计

### 10.1 规则层

发牌流程：

1. `ddz_deck_init(deck)` 生成 54 张唯一牌。
2. `ddz_deck_shuffle(deck, rng_state)` 使用轻量可重复随机。
3. 前 51 张按 `player = index % 3` 分发。
4. 最后 3 张存入 `bottom`。
5. 每个手牌调用 `ddz_hand_sort()`。
6. 更新每手牌的 `rank_counts`。

`ddz_combo_detect(cards, count)` 识别顺序：

1. `count == 0` -> `Pass`
2. `count == 1` -> `Single`
3. `count == 2` 且双王 -> `Rocket`
4. `count == 2` 且同 rank -> `Pair`
5. `count == 3` 且同 rank -> `Triple`
6. `count == 4` 且四张同 rank -> `Bomb`
7. `count == 4` 且存在三张 -> `TripleSingle`
8. `count == 5` 且存在三张和一对 -> `TriplePair`
9. `count >= 5` 且连续单 rank、每 rank 一张、不含 rank >= 12 -> `Straight`
10. `count >= 6` 且偶数、连续 rank、每 rank 两张、不含 rank >= 12 -> `PairStraight`
11. 否则 `Invalid`

`ddz_combo_can_beat(candidate, last)`：

- `last.kind == Pass` 时，任何非 Pass 合法牌都可出。
- `candidate.kind == Rocket` 时永远可压。
- `last.kind == Rocket` 时不可压。
- `candidate.kind == Bomb` 且 `last.kind != Bomb` 时可压。
- 双方都是 Bomb 时比较 `main_rank`。
- 普通牌必须 kind 相同、count 相同、`sequence_len` 相同，且 `candidate.main_rank > last.main_rank`。

`ddz_generate_legal_actions(game, player, out_actions)`：

1. 清空 action list。
2. 如果当前不是主动出牌，先加入 `pass`。
3. 根据手牌 `rank_counts` 枚举 MVP 牌型。
4. 对每个候选调用 `ddz_combo_can_beat()`。
5. 通过则加入 action list。
6. 按保守优先排序。
7. 超过 `DDZ_MAX_ACTIONS` 时丢弃尾部动作并置 `truncated = true`。

`ddz_game_apply_action(game, player, action)`：

- 校验 `player == current_player`。
- 校验 action 在当前合法列表中。
- Pass 仅允许非主动出牌。
- 非 Pass 时从手牌移除 action.cards。
- 手牌为空则进入 `GameOver`。
- 正常轮转到下一位玩家。

### 10.2 本地启发式 AI

阶段 A 的 AI 只做本地启发式，不依赖网络。

叫分评分：

- 火箭 +5
- 每个炸弹 +4
- 每个王 +2
- 每张 `2` +1
- A/K 较多 +1
- `score >= 8` 叫 3，`>= 5` 叫 2，`>= 3` 叫 1，否则不叫

出牌策略：

- 主动出牌：优先出最小非炸弹动作。
- 跟牌：选择能压过上家的最小动作。
- 默认不使用炸弹/火箭压普通牌。
- 自己剩余牌数少时允许炸。
- 地主将要走完时允许更积极压制。

### 10.3 UI demo

阶段 A UI 必须覆盖：

- 桌面背景
- 玩家区域
- 底牌区
- 上一手牌显示
- 人类手牌显示
- 选牌上移反馈
- 叫分按钮
- `提示` / `出牌` / `不要` / `重开`
- 人类点击交互
- AI 自动行动
- 胜负状态和当前玩家提示

### 10.4 simulator 接入

阶段 A 需要把 `doudizhu` 作为正式 demo 接入：

- `SimDemoKind.Doudizhu`
- `--demo doudizhu`
- `sim_demo_name()` 返回 `斗地主`
- `src/gui/sim/app.uya` 接入 render/update/input
- 热键 `Z`

### 10.5 tests 与 smoke

阶段 A 需要：

- 规则单测
- AI 单测
- `tests/test_suite.uya` 聚合
- `make test`
- `sim-run` 手工对局验证
- `sim-headless` 截图 smoke

## 11. 阶段 B 详细设计

### 11.1 OpenAI API 说明

用户明确要求使用 OpenAI 的 chat 接口，因此阶段 B 使用：

- `POST /v1/chat/completions`

参考资料：

- Chat Completions API: `https://platform.openai.com/docs/api-reference/chat/create`
- Structured Outputs: `https://platform.openai.com/docs/guides/structured-outputs`
- Latest model guidance: `https://developers.openai.com/api/docs/guides/latest-model`

### 11.2 OpenAI 决策输入

OpenAI 接收的是压缩后的局面摘要，不接收 API Key 或无关日志。

play 阶段 payload 草案：

```json
{
  "phase": "play",
  "seat": 1,
  "role": "peasant",
  "hand": "3 3 4 7 K A 2",
  "hand_count": 7,
  "landlord": 0,
  "current_player": 1,
  "last_player": 0,
  "last_play": "pair 9",
  "card_counts": [20, 7, 11],
  "legal_actions_truncated": false,
  "legal_actions": [
    { "id": 0, "label": "pass" },
    { "id": 1, "label": "pair K" }
  ]
}
```

bid 阶段 payload 草案：

```json
{
  "phase": "bid",
  "seat": 2,
  "role": "unknown",
  "hand": "3 4 4 7 8 9 J Q K A 2 BJ",
  "highest_bid": 1,
  "highest_bidder": 0,
  "legal_actions": [
    { "id": 0, "label": "bid_0" },
    { "id": 2, "label": "bid_2" },
    { "id": 3, "label": "bid_3" }
  ]
}
```

### 11.3 Chat Completions 请求

环境变量：

```bash
OPENAI_API_KEY=...
OPENAI_MODEL=gpt-5.4-mini
OPENAI_BASE_URL=https://api.openai.com/v1
UYA_DDZ_USE_OPENAI=1
```

请求体：

```json
{
  "model": "gpt-5.4-mini",
  "messages": [
    {
      "role": "system",
      "content": "You are a Dou Dizhu AI. Choose exactly one legal action id from the user-provided legal_actions. Do not invent cards. Return JSON only."
    },
    {
      "role": "user",
      "content": "{\"phase\":\"play\",\"seat\":1,\"role\":\"peasant\",\"hand\":\"3 3 4 7 K A 2\",\"last_play\":\"pair 9\",\"legal_actions\":[{\"id\":0,\"label\":\"pass\"},{\"id\":1,\"label\":\"pair K\"}]}"
    }
  ],
  "response_format": {
    "type": "json_schema",
    "json_schema": {
      "name": "doudizhu_action",
      "strict": true,
      "schema": {
        "type": "object",
        "properties": {
          "action_id": { "type": "integer" }
        },
        "required": ["action_id"],
        "additionalProperties": false
      }
    }
  }
}
```

期望 assistant content：

```json
{ "action_id": 1 }
```

### 11.4 返回校验与 fallback

OpenAI 返回动作必须经过三层校验：

1. C bridge 校验 HTTP 和 Chat envelope。
2. `platform.openai.chat` 校验返回 JSON 长度与格式。
3. `doudizhu.ai` 校验 `action_id` 是否存在于当前 `DdzActionList`。

任意一层失败都进入本地启发式兜底。

### 11.5 host bridge

阶段 B host bridge 负责：

- 读取 `OPENAI_API_KEY`
- 读取 `OPENAI_MODEL`
- 读取 `OPENAI_BASE_URL`
- 读取 `UYA_DDZ_USE_OPENAI`
- 单 in-flight 请求槽
- libcurl 后台请求
- timeout 控制
- envelope 解析
- cancel

bridge 不知道斗地主规则本身。

## 12. 验收策略

### 12.1 阶段 A 验收

必须全部满足：

- `make test`
- `make sim-run SIM_ARGS="--demo doudizhu --scale 1"`
- 至少手工完整跑完一局
- `make sim-headless SIM_HEADLESS_ARGS="--demo doudizhu --max-frames 5 --screenshot build/sim/doudizhu.bmp"`

### 12.2 阶段 B 验收

在阶段 A 已通过后，再追加：

```bash
ALLOW_OPENAI_LIVE=1 OPENAI_API_KEY=... UYA_DDZ_USE_OPENAI=1 \
make sim-run SIM_ARGS="--demo doudizhu --max-frames 300"
```

要求：

- OpenAI 可用时可参与电脑决策。
- OpenAI 不可用时自动降级。
- 默认 CI 不访问网络。
- live smoke 仅作为可选人工验收，不进入默认测试链路。

## 13. 当前结论

推荐就按下面这个顺序推进，不要交叉偷跑：

1. 阶段 A：规则层、本地启发式 AI、UI demo、sim 接入、tests、离线路径文档、真实 smoke。
2. 阶段 B：`chat.uya`、`stub.c`、`host.c + libcurl`、prompt / poll / fallback / cancel、OpenAI 环境变量文档、live smoke。

如果阶段 A 还不能真实玩完一局，就不应该声称“斗地主 demo 已基本完成”。
