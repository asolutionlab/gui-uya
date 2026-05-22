const DEFAULT_UPSTREAM_BASE_URL = "https://gpt001.iotalking.top/v1";
const DEFAULT_UPSTREAM_MODEL = "gpt-5.4-mini";
const DEFAULT_MAX_REQUEST_BYTES = 24 * 1024;
const DEFAULT_MAX_OUTPUT_TOKENS = 64;
const DEFAULT_UPSTREAM_TIMEOUT_MS = 12_000;
const DEFAULT_CORS_MAX_AGE = 86_400;
const DDZ_ENDPOINT = "/ddz/decision";
const DDZ_SYSTEM_PROMPT = [
  "你是一个很强的斗地主 AI。",
  "目标是最大化长期胜率，而不是只看眼前这一手的短期收益。",
  "你必须且只能从 legal_actions[].id 中选择一个 id，并且只返回 JSON，例如 {\"action_id\":1}。",
  "不要虚构牌、叫分、历史、队友信息或规则。",
  "你需要先在脑中完成四步：看清当前牌权和敌我关系；结合 history 与公开未见牌做保守猜牌；区分 legal_actions 里的赢牌、阻断、配合、试探和高成本动作；最后再选唯一 action_id。",
  "看牌局时不要只判断这一手能不能压住，还要判断压住之后牌权会落到谁手里、谁最可能被放走。",
  "猜牌只能基于公开信息做保守推断，不要把猜测当成已知事实。",
  "比较接近的候选动作时，要重点参考 legal_actions[].cards 和 legal_actions[].remaining_hand_count，不要只看 label。",
  "主动出牌时，如果没有立刻收官或强制阻断的需要，优先考虑能试探外面反应、出弱留强、尽量不暴露控制牌的低成本线路。",
  "如果你是农民，要和队友做隐式配合；除非你能立刻赢牌，或者必须阻止地主，否则不要用高牌抢走队友的节奏。",
  "如果队友已经拿到牌权，且危险对手没有马上走牌的风险，通常不要或最克制的配合线会优于强行接管。",
  "如果对手快要走完，优先用最小且安全的赢牌动作去卡住对手。",
  "除非能直接赢牌、必须阻止对手跑牌，或者没有可接受的非炸弹线路，否则尽量保留炸弹和火箭。",
  "如果多个方案战略价值接近，优先更小的安全非炸弹、更低的带牌，以及更低的顺子或连对，尽量保留强控牌。"
].join(" ");
const DDZ_USER_ALLOWED_KEYS = new Set([
  "game",
  "phase",
  "seat",
  "role",
  "landlord",
  "current_player",
  "highest_bid",
  "highest_bidder",
  "hand_count",
  "hand",
  "card_counts",
  "legal_actions_truncated",
  "history_summary",
  "history",
  "legal_actions",
]);
const DDZ_HISTORY_SUMMARY_KEYS = new Set([
  "history_count",
  "bomb_played",
  "rocket_played",
  "consecutive_passes",
  "last_non_pass_player",
  "last_non_pass_play",
  "players_reported_single",
]);
const DDZ_HISTORY_ENTRY_KEYS = new Set([
  "step",
  "phase",
  "player",
  "action",
  "hand_counts",
  "current_player",
  "landlord",
  "highest_bid",
  "highest_bid_player",
  "pass_count",
]);
const DDZ_ACTION_KEYS = new Set([
  "id",
  "kind",
  "label",
  "bid",
  "card_count",
  "main_rank",
  "cards",
]);
const DDZ_LEGAL_ACTION_KEYS = new Set([
  "id",
  "label",
  "kind",
  "bid",
  "card_count",
  "main_rank",
  "cards",
  "remaining_hand_count",
  "is_bomb_like",
  "ends_hand",
]);
const DDZ_ALLOWED_PHASES = new Set(["bid", "play"]);
const DDZ_ALLOWED_ROLES = new Set(["unknown", "landlord", "peasant"]);
const DDZ_ALLOWED_ACTION_KINDS = new Set([
  "pass",
  "single",
  "pair",
  "triple",
  "triple_single",
  "triple_pair",
  "straight",
  "straight_pairs",
  "bomb",
  "rocket",
  "invalid",
]);
const DDZ_RANK_ORDER = ["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "BJ", "RJ"];
const DDZ_ALLOWED_RANKS = new Set(DDZ_RANK_ORDER);
const DDZ_HIGH_CONTROL_RANKS = ["K", "A", "2", "BJ", "RJ"];
const DDZ_PROBE_KINDS = new Set(["single", "pair", "straight", "straight_pairs"]);
const DDZ_RESPONSE_FORMAT = {
  type: "json_schema",
  json_schema: {
    name: "doudizhu_action",
    strict: true,
    schema: {
      type: "object",
      properties: {
        action_id: {
          type: "integer",
        },
      },
      required: ["action_id"],
      additionalProperties: false,
    },
  },
};

export default {
  async fetch(request, env) {
    const url = new URL(request.url);

    if (request.method === "OPTIONS") {
      return handlePreflight(request, env, url.pathname);
    }

    if (url.pathname === "/healthz") {
      return withCors(
        request,
        env,
        jsonResponse({
          ok: true,
          endpoint: DDZ_ENDPOINT,
          upstream_base_url: readString(env.UPSTREAM_BASE_URL) || DEFAULT_UPSTREAM_BASE_URL,
        })
      );
    }

    if (url.pathname !== DDZ_ENDPOINT) {
      return withCors(request, env, errorResponse("NOT_FOUND", `Only ${DDZ_ENDPOINT} is exposed.`, 404));
    }

    if (request.method !== "POST") {
      return withCors(request, env, errorResponse("METHOD_NOT_ALLOWED", "Use POST.", 405));
    }

    const origin = request.headers.get("Origin");
    if (!isOriginAllowed(origin, env)) {
      return errorResponse("ORIGIN_FORBIDDEN", "Origin is not allowed.", 403);
    }

    const authCheck = checkClientAuthorization(request, env);
    if (!authCheck.ok) {
      return withCors(request, env, errorResponse(authCheck.code, authCheck.message, authCheck.status));
    }

    const contentType = (request.headers.get("Content-Type") || "").toLowerCase();
    if (!contentType.includes("application/json")) {
      return withCors(request, env, errorResponse("UNSUPPORTED_MEDIA_TYPE", "Content-Type must be application/json.", 415));
    }

    const rawBody = await request.text();
    const bodyBytes = new TextEncoder().encode(rawBody).byteLength;
    const maxRequestBytes = readPositiveInt(env.MAX_REQUEST_BYTES, DEFAULT_MAX_REQUEST_BYTES);
    if (bodyBytes <= 0) {
      return withCors(request, env, errorResponse("EMPTY_BODY", "Request body is empty.", 400));
    }
    if (bodyBytes > maxRequestBytes) {
      return withCors(
        request,
        env,
        errorResponse("BODY_TOO_LARGE", `Request body exceeds ${maxRequestBytes} bytes.`, 413)
      );
    }

    let payload;
    try {
      payload = JSON.parse(rawBody);
    } catch {
      return withCors(request, env, errorResponse("INVALID_JSON", "Request body must be valid JSON.", 400));
    }

    const ddzPayload = sanitizeDdzUserPayload(payload);
    if (!ddzPayload.ok) {
      return withCors(request, env, errorResponse(ddzPayload.code, ddzPayload.message, ddzPayload.status));
    }

    const rateLimitResult = await enforceRateLimit(env, request, authCheck.clientKey);
    if (!rateLimitResult.ok) {
      return withCors(request, env, errorResponse(rateLimitResult.code, rateLimitResult.message, rateLimitResult.status));
    }

    if (!readString(env.UPSTREAM_OPENAI_API_KEY)) {
      return withCors(request, env, errorResponse("SERVER_MISCONFIGURED", "Missing UPSTREAM_OPENAI_API_KEY.", 500));
    }

    const model = resolveUpstreamModel(env);
    if (!model.ok) {
      return withCors(request, env, errorResponse(model.code, model.message, model.status));
    }

    const upstreamUrl = buildUpstreamUrl(env);
    const tacticalContext = buildTacticalContext(ddzPayload.value);
    const upstreamMessages = [
      {
        role: "system",
        content: DDZ_SYSTEM_PROMPT,
      },
    ];
    if (tacticalContext) {
      upstreamMessages.push({
        role: "system",
        content: tacticalContext,
      });
    }
    upstreamMessages.push({
      role: "user",
      content: JSON.stringify(ddzPayload.value),
    });

    const upstreamBody = {
      model: model.value,
      messages: upstreamMessages,
      response_format: DDZ_RESPONSE_FORMAT,
      max_tokens: readPositiveInt(env.MAX_OUTPUT_TOKENS, DEFAULT_MAX_OUTPUT_TOKENS),
      temperature: 0,
      n: 1,
      stream: false,
    };

    const upstreamHeaders = new Headers();
    upstreamHeaders.set("Authorization", `Bearer ${env.UPSTREAM_OPENAI_API_KEY}`);
    upstreamHeaders.set("Content-Type", "application/json");
    upstreamHeaders.set("Accept", "application/json");
    upstreamHeaders.set("User-Agent", "uya-ddz-cf-worker/1.0");

    const timeoutMs = readPositiveInt(env.UPSTREAM_TIMEOUT_MS, DEFAULT_UPSTREAM_TIMEOUT_MS);
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort("upstream timeout"), timeoutMs);

    let upstreamResponse;
    try {
      upstreamResponse = await fetch(upstreamUrl, {
        method: "POST",
        headers: upstreamHeaders,
        body: JSON.stringify(upstreamBody),
        signal: controller.signal,
      });
    } catch (error) {
      const message = error && error.name === "AbortError"
        ? `Upstream timeout after ${timeoutMs}ms.`
        : "Failed to reach upstream service.";
      return withCors(request, env, errorResponse("UPSTREAM_ERROR", message, 502));
    } finally {
      clearTimeout(timeoutId);
    }

    const upstreamText = await upstreamResponse.text();
    const upstreamRequestId = upstreamResponse.headers.get("x-request-id") || "";
    if (upstreamResponse.status < 200 || upstreamResponse.status >= 300) {
      const response = errorResponse("UPSTREAM_HTTP_ERROR", `Upstream returned HTTP ${upstreamResponse.status}.`, 502);
      return withCors(request, env, withOptionalUpstreamRequestId(response, upstreamRequestId));
    }

    const actionResult = extractActionDecision(upstreamText, ddzPayload.value.legal_actions);
    if (!actionResult.ok) {
      const response = errorResponse(actionResult.code, actionResult.message, actionResult.status);
      return withCors(request, env, withOptionalUpstreamRequestId(response, upstreamRequestId));
    }

    const response = jsonResponse({ action_id: actionResult.actionId });
    return withCors(request, env, withOptionalUpstreamRequestId(response, upstreamRequestId));
  },
};

function buildUpstreamUrl(env) {
  const rawBaseUrl = readString(env.UPSTREAM_BASE_URL) || DEFAULT_UPSTREAM_BASE_URL;
  const normalizedBaseUrl = rawBaseUrl.endsWith("/") ? rawBaseUrl : `${rawBaseUrl}/`;
  return new URL("chat/completions", normalizedBaseUrl).toString();
}

function resolveUpstreamModel(env) {
  const model = readString(env.OPENAI_MODEL) || DEFAULT_UPSTREAM_MODEL;
  const allowedModels = splitCsv(readString(env.ALLOWED_MODELS));
  if (allowedModels.length > 0 && !allowedModels.includes(model)) {
    return fail("MODEL_FORBIDDEN", `Configured model ${model} is not allowed.`, 500);
  }
  return {
    ok: true,
    value: model,
  };
}

function buildTacticalContext(payload) {
  if (!isPlainObject(payload)) {
    return "";
  }
  const notes = payload.phase === "bid"
    ? buildBidTacticalNotes(payload)
    : buildPlayTacticalNotes(payload);
  if (notes.length <= 0) {
    return "";
  }
  return `根据当前牌桌状态补充的战术提示：\n${notes.map((note) => `- ${note}`).join("\n")}`;
}

function buildBidTacticalNotes(payload) {
  const notes = [];
  const controlCards = countHighControlCards(payload.hand);
  const twoCount = countRankCopies(payload.hand, "2");
  const jokerCount = countRankCopies(payload.hand, "BJ") + countRankCopies(payload.hand, "RJ");

  if (jokerCount >= 2 || twoCount >= 2 || controlCards >= 5) {
    notes.push("这手牌高张控制力比较明显，只有在优势足够明确时才值得继续抬高叫分。");
  } else {
    notes.push("这手牌的高张优势并不明显，叫分不要勉强。");
  }
  if (payload.highest_bid >= 2) {
    notes.push(`当前最高叫分已经是 ${payload.highest_bid}，只有在控制力明显更强时才考虑继续压上去。`);
  }
  if (payload.legal_actions.some((item) => item.bid === 0)) {
    notes.push("边缘手牌宁可不叫，也不要硬抬分。");
  }
  return notes;
}

function buildPlayTacticalNotes(payload) {
  const notes = [];
  const liveSeat = payload.history_summary.last_non_pass_player;
  const liveRelation = relationToSeat(payload, liveSeat);
  const liveAction = findLiveAction(payload);
  const canPass = payload.legal_actions.some((item) => item.kind === "pass");
  const passAction = payload.legal_actions.find((item) => item.kind === "pass") || null;
  const immediateWins = payload.legal_actions.filter((item) => item.ends_hand);
  const bombOptions = payload.legal_actions.filter((item) => item.is_bomb_like);
  const normalPlayOptions = payload.legal_actions.filter((item) => item.kind !== "pass" && !item.is_bomb_like);
  const dangerousOpponents = findSeatsByRelationWithMaxCards(payload, "opponent", 2);
  const opponentSingles = dangerousOpponents.filter((seat) => payload.card_counts[seat] === 1);
  const opponentPairs = dangerousOpponents.filter((seat) => payload.card_counts[seat] === 2);
  const teammateSeat = findTeammateSeat(payload);
  const unseenCounts = buildUnseenRankCounts(payload);
  const unseenHighSummary = summarizeRankCounts(unseenCounts, ["RJ", "BJ", "2", "A", "K"]);
  const unseenHighTotal = sumRankCounts(unseenCounts, ["RJ", "BJ", "2", "A", "K"]);
  const unseenBombThreat = estimateUnseenBombThreat(payload, unseenCounts);
  const cheapBlocks = liveAction ? selectCheapestResponses(normalPlayOptions, liveAction) : [];
  const probeActions = selectProbeActions(normalPlayOptions);
  const expensiveActions = selectExpensiveActions(payload.legal_actions, payload.hand_count);

  if (liveSeat < 0) {
    notes.push("这一轮由你起手，优先考虑能出弱留强、便于继续观察外面反应的低成本线路。");
  } else if (liveRelation === "teammate") {
    notes.push(`座位 ${liveSeat} 是你的队友，而且当前牌权在他手里；通常应当保住队友牌权，而不是轻易反超。`);
  } else if (liveRelation === "opponent") {
    notes.push(`座位 ${liveSeat} 是对手，而且当前牌权在他手里；如果要接，优先选择最小且安全的非炸弹压制。`);
  }
  if (liveAction && liveSeat >= 0) {
    notes.push(`当前桌面的有效牌是座位 ${liveSeat} 打出的 ${liveAction.label}。`);
  }
  if (payload.legal_actions_truncated) {
    notes.push("legal_actions 已截断显示；更要优先看 ends_hand、最小安全压制和非炸弹线路。");
  }
  if (unseenHighTotal > 0) {
    notes.push(`公开未见的高控制牌大致还有 ${unseenHighSummary}；猜牌时应把这些潜在控制牌仍在外面的风险算进去。`);
  } else {
    notes.push("公开信息里几乎没有未见的高控制牌了，后续争牌权可以比平时更积极。");
  }
  if (unseenBombThreat > 0) {
    notes.push(`公开信息仍允许存在 ${unseenBombThreat} 组未见炸弹或火箭候选，因此高价值长链和强控牌更适合先试探再重投。`);
  }
  if (liveAction && liveRelation === "opponent") {
    const higherSameKindThreats = countHigherSameKindThreats(unseenCounts, liveAction);
    if (higherSameKindThreats > 0) {
      notes.push(`按公开信息推断，外面仍可能保留 ${higherSameKindThreats} 档比当前 ${liveAction.label} 更大的同类牌；如果接手后未必守得住，不要为抢一手过早交重牌。`);
    } else if (isComparablePressureKind(liveAction.kind)) {
      notes.push(`按公开信息推断，继续压过当前 ${liveAction.label} 的同类空间已经不大；若能用较小动作接住，这手的控权价值会更高。`);
    }
  }

  if (dangerousOpponents.length > 0) {
    notes.push(`危险对手提醒：座位 ${dangerousOpponents.join(", ")} 的对手手里只剩 2 张或更少。`);
  }
  if (opponentSingles.length > 0) {
    notes.push(`座位 ${opponentSingles.join(", ")} 的对手已经报单，当前应优先考虑卡住他，尤其要重视单张和对子这一类应对。`);
  }
  if (opponentPairs.length > 0) {
    notes.push(`座位 ${opponentPairs.join(", ")} 的对手只剩 2 张，当前要额外警惕对子、连对或炸弹式收尾。`);
  }
  if (teammateSeat >= 0 && payload.card_counts[teammateSeat] <= 2) {
    notes.push(`你的队友座位 ${teammateSeat} 牌已经不多了，应尽量配合他走牌，不要无谓消耗高控制牌。`);
  }
  if (immediateWins.length > 0) {
    notes.push(`动作 ${describeActionRefs(immediateWins)} 可以直接出完手牌，应当被高度优先考虑。`);
  }
  if (liveRelation === "teammate" && passAction) {
    notes.push(`当前可以不要，对应动作是 id ${passAction.id}；如果你不能直接赢牌，也不是在挡危险对手，通常先不要更像正确配合。`);
  }
  if (liveRelation === "opponent" && cheapBlocks.length > 0) {
    notes.push(`较小的阻断线路可以先检查 ${describeActionRefs(cheapBlocks)}，不要一上来就交最高控制牌。`);
  }
  if (!canPass && probeActions.length > 0) {
    notes.push(`可用于试探外面反应的起手动作有 ${describeActionRefs(probeActions)}；这类动作更适合先摸牌，再决定要不要升高强度。`);
  }
  if (expensiveActions.length > 0) {
    notes.push(`高成本动作提醒：${describeActionRefs(expensiveActions)} 会较早消耗炸弹、王、2 或其他高控制牌，除非为了直接赢牌或强制卡敌，否则应后置。`);
  }
  if (bombOptions.length > 0 && normalPlayOptions.length > 0) {
    notes.push("当前存在非炸弹选择，因此除非能马上赢牌或必须阻止对手跑牌，否则应尽量保留炸弹和火箭。");
  }
  if (canPass && liveRelation === "teammate") {
    notes.push("当前可以不要，而且如果队友已经拿到牌权，通常不要会比强行接管更合理。");
  } else if (canPass && dangerousOpponents.length === 0) {
    notes.push("如果对手暂时没有立刻跑牌的风险，那么不要往往比浪费控制牌更好。");
  }
  notes.push("若仍有多个接近选择，请同时比较 legal_actions.cards、remaining_hand_count 和是否会暴露控制牌，优先更低成本的安全线路。");
  return notes;
}

function findLiveAction(payload) {
  if (!isPlainObject(payload) || !Array.isArray(payload.history)) {
    return null;
  }
  const liveSeat = payload.history_summary.last_non_pass_player;
  for (let index = payload.history.length - 1; index >= 0; index -= 1) {
    const entry = payload.history[index];
    if (!entry || entry.phase !== "play" || !isPlainObject(entry.action)) {
      continue;
    }
    if (entry.action.kind === "pass") {
      continue;
    }
    if (liveSeat >= 0 && entry.player !== liveSeat) {
      continue;
    }
    return entry.action;
  }
  return null;
}

function buildUnseenRankCounts(payload) {
  const visibleCounts = Object.fromEntries(DDZ_RANK_ORDER.map((rank) => [rank, 0]));
  addRanksToCount(visibleCounts, payload.hand);
  if (Array.isArray(payload.history)) {
    for (const entry of payload.history) {
      if (!entry || !isPlainObject(entry.action)) {
        continue;
      }
      addRanksToCount(visibleCounts, entry.action.cards);
    }
  }

  const unseenCounts = {};
  for (const rank of DDZ_RANK_ORDER) {
    unseenCounts[rank] = Math.max(totalCopiesForRank(rank) - (visibleCounts[rank] || 0), 0);
  }
  return unseenCounts;
}

function addRanksToCount(counts, ranks) {
  if (!counts || !Array.isArray(ranks)) {
    return;
  }
  for (const rank of ranks) {
    if (typeof rank === "string" && Object.prototype.hasOwnProperty.call(counts, rank)) {
      counts[rank] += 1;
    }
  }
}

function totalCopiesForRank(rank) {
  return rank === "BJ" || rank === "RJ" ? 1 : 4;
}

function summarizeRankCounts(counts, ranks) {
  const parts = [];
  for (const rank of ranks) {
    const total = counts && counts[rank];
    if (total > 0) {
      parts.push(`${rank}x${total}`);
    }
  }
  return parts.length > 0 ? parts.join("、") : "无";
}

function sumRankCounts(counts, ranks) {
  let total = 0;
  for (const rank of ranks) {
    total += counts && Number.isInteger(counts[rank]) ? counts[rank] : 0;
  }
  return total;
}

function estimateUnseenBombThreat(payload, unseenCounts) {
  const maxBombSlots = countPossibleBombSlotsOutsideHand(payload);
  let total = 0;
  for (const rank of DDZ_RANK_ORDER) {
    if (rank === "BJ" || rank === "RJ") {
      continue;
    }
    if ((unseenCounts[rank] || 0) >= 4) {
      total += 1;
    }
  }
  const bombThreat = Math.min(total, maxBombSlots);
  const rocketThreat = hasPossibleOutsideRocket(payload, unseenCounts) ? 1 : 0;
  return bombThreat + rocketThreat;
}

function countPossibleBombSlotsOutsideHand(payload) {
  if (!isPlainObject(payload) || !Array.isArray(payload.card_counts)) {
    return 0;
  }
  let total = 0;
  for (let seat = 0; seat < payload.card_counts.length; seat += 1) {
    if (seat === payload.seat) {
      continue;
    }
    total += Math.floor(payload.card_counts[seat] / 4);
  }
  return total;
}

function hasPossibleOutsideRocket(payload, unseenCounts) {
  if (!isPlainObject(payload) || !Array.isArray(payload.card_counts)) {
    return false;
  }
  if ((unseenCounts.BJ || 0) <= 0 || (unseenCounts.RJ || 0) <= 0) {
    return false;
  }
  for (let seat = 0; seat < payload.card_counts.length; seat += 1) {
    if (seat !== payload.seat && payload.card_counts[seat] >= 2) {
      return true;
    }
  }
  return false;
}

function minimumCopiesForPressureKind(kind) {
  if (kind === "single") {
    return 1;
  }
  if (kind === "pair") {
    return 2;
  }
  if (kind === "triple" || kind === "triple_single" || kind === "triple_pair") {
    return 3;
  }
  if (kind === "bomb") {
    return 4;
  }
  return 0;
}

function isComparablePressureKind(kind) {
  return minimumCopiesForPressureKind(kind) > 0;
}

function countHigherSameKindThreats(unseenCounts, action) {
  const minCopies = minimumCopiesForPressureKind(action && action.kind);
  if (minCopies <= 0 || !action || !Number.isInteger(action.main_rank)) {
    return 0;
  }
  let total = 0;
  for (let index = action.main_rank + 1; index < DDZ_RANK_ORDER.length; index += 1) {
    const rank = DDZ_RANK_ORDER[index];
    if ((unseenCounts[rank] || 0) >= minCopies) {
      total += 1;
    }
  }
  return total;
}

function selectCheapestResponses(actions, liveAction, limit = 3) {
  if (!Array.isArray(actions) || actions.length <= 0 || !liveAction) {
    return [];
  }
  const sameKind = actions.filter((item) => item.kind === liveAction.kind);
  const pool = sameKind.length > 0 ? sameKind : actions;
  return sortActionsByEconomy(pool).slice(0, limit);
}

function selectProbeActions(actions, limit = 4) {
  if (!Array.isArray(actions) || actions.length <= 0) {
    return [];
  }
  const maxProbeRank = DDZ_RANK_ORDER.indexOf("10");
  const probeActions = actions.filter((action) => {
    if (!action || !DDZ_PROBE_KINDS.has(action.kind) || action.ends_hand || action.is_bomb_like) {
      return false;
    }
    if (!Number.isInteger(action.main_rank) || action.main_rank < 0 || action.main_rank > maxProbeRank) {
      return false;
    }
    if (countControlCardsInAction(action) > 0) {
      return false;
    }
    if (action.card_count >= 6 || action.remaining_hand_count <= 0) {
      return false;
    }
    return true;
  });
  return sortActionsByEconomy(probeActions).slice(0, limit);
}

function selectExpensiveActions(actions, handCount, limit = 4) {
  if (!Array.isArray(actions) || actions.length <= 0) {
    return [];
  }
  const expensiveActions = actions.filter((action) => {
    if (!action || action.kind === "pass") {
      return false;
    }
    const controlCount = countControlCardsInAction(action);
    return action.is_bomb_like
      || countRankCopies(action.cards, "BJ") > 0
      || countRankCopies(action.cards, "RJ") > 0
      || countRankCopies(action.cards, "2") >= Math.min(2, action.card_count)
      || controlCount >= 2
      || (action.card_count >= Math.min(Math.max(5, handCount - 1), 6) && action.remaining_hand_count > 0);
  });
  return sortActionsByExpense(expensiveActions).slice(0, limit);
}

function sortActionsByEconomy(actions) {
  return [...actions].sort((left, right) => {
    const mainRankDiff = normalizeMainRank(left.main_rank) - normalizeMainRank(right.main_rank);
    if (mainRankDiff !== 0) {
      return mainRankDiff;
    }
    const controlDiff = countControlCardsInAction(left) - countControlCardsInAction(right);
    if (controlDiff !== 0) {
      return controlDiff;
    }
    const cardCountDiff = left.card_count - right.card_count;
    if (cardCountDiff !== 0) {
      return cardCountDiff;
    }
    const remainingDiff = left.remaining_hand_count - right.remaining_hand_count;
    if (remainingDiff !== 0) {
      return remainingDiff;
    }
    return left.id - right.id;
  });
}

function sortActionsByExpense(actions) {
  return [...actions].sort((left, right) => {
    const bombDiff = Number(right.is_bomb_like) - Number(left.is_bomb_like);
    if (bombDiff !== 0) {
      return bombDiff;
    }
    const controlDiff = countControlCardsInAction(right) - countControlCardsInAction(left);
    if (controlDiff !== 0) {
      return controlDiff;
    }
    const cardCountDiff = right.card_count - left.card_count;
    if (cardCountDiff !== 0) {
      return cardCountDiff;
    }
    const mainRankDiff = normalizeMainRank(right.main_rank) - normalizeMainRank(left.main_rank);
    if (mainRankDiff !== 0) {
      return mainRankDiff;
    }
    return left.id - right.id;
  });
}

function normalizeMainRank(mainRank) {
  return Number.isInteger(mainRank) && mainRank >= 0 ? mainRank : 99;
}

function countControlCardsInAction(action) {
  return action && Array.isArray(action.cards)
    ? countSpecificRanks(action.cards, DDZ_HIGH_CONTROL_RANKS)
    : 0;
}

function countSpecificRanks(ranks, targets) {
  if (!Array.isArray(ranks) || !Array.isArray(targets)) {
    return 0;
  }
  let total = 0;
  for (const rank of ranks) {
    if (targets.includes(rank)) {
      total += 1;
    }
  }
  return total;
}

function describeActionRefs(actions, limit = 4) {
  if (!Array.isArray(actions) || actions.length <= 0) {
    return "";
  }
  const refs = actions
    .slice(0, limit)
    .map((item) => `id ${item.id}(${item.label})`);
  return actions.length > limit
    ? `${refs.join("、")} 等`
    : refs.join("、");
}

function relationToSeat(payload, seat) {
  if (!Number.isInteger(seat) || seat < 0 || seat > 2) {
    return "none";
  }
  if (seat === payload.seat) {
    return "self";
  }
  if (payload.role === "landlord") {
    return "opponent";
  }
  if (payload.role === "peasant") {
    return seat === payload.landlord ? "opponent" : "teammate";
  }
  return "other";
}

function findTeammateSeat(payload) {
  if (payload.role !== "peasant") {
    return -1;
  }
  for (let seat = 0; seat < 3; seat += 1) {
    if (seat !== payload.seat && seat !== payload.landlord) {
      return seat;
    }
  }
  return -1;
}

function findSeatsByRelationWithMaxCards(payload, relation, maxCards) {
  const seats = [];
  for (let seat = 0; seat < 3; seat += 1) {
    if (seat === payload.seat) {
      continue;
    }
    if (relationToSeat(payload, seat) !== relation) {
      continue;
    }
    if (payload.card_counts[seat] <= maxCards) {
      seats.push(seat);
    }
  }
  return seats;
}

function countHighControlCards(hand) {
  if (!Array.isArray(hand)) {
    return 0;
  }
  let total = 0;
  for (const rank of hand) {
    if (rank === "A" || rank === "2" || rank === "BJ" || rank === "RJ" || rank === "K") {
      total += 1;
    }
  }
  return total;
}

function countRankCopies(hand, targetRank) {
  if (!Array.isArray(hand)) {
    return 0;
  }
  let total = 0;
  for (const rank of hand) {
    if (rank === targetRank) {
      total += 1;
    }
  }
  return total;
}

function extractActionDecision(upstreamText, legalActions) {
  let upstreamJson;
  try {
    upstreamJson = JSON.parse(upstreamText);
  } catch {
    return fail("UPSTREAM_BAD_JSON", "Upstream response is not valid JSON.", 502);
  }

  const content = upstreamJson &&
    upstreamJson.choices &&
    upstreamJson.choices[0] &&
    upstreamJson.choices[0].message &&
    upstreamJson.choices[0].message.content;
  if (typeof content !== "string") {
    return fail("UPSTREAM_NO_CONTENT", "Upstream response did not contain message.content.", 502);
  }

  let decision;
  try {
    decision = JSON.parse(content);
  } catch {
    return fail("UPSTREAM_BAD_DECISION", "Assistant content is not valid JSON.", 502);
  }

  if (!isPlainObject(decision) || !Number.isInteger(decision.action_id)) {
    return fail("UPSTREAM_BAD_ACTION", "Assistant content did not contain an integer action_id.", 502);
  }

  const isLegalAction = legalActions.some((item) => item.id === decision.action_id);
  if (!isLegalAction) {
    return fail("UPSTREAM_ILLEGAL_ACTION", "Assistant returned an action_id outside legal_actions.", 502);
  }

  return {
    ok: true,
    actionId: decision.action_id,
  };
}

function sanitizeDdzUserPayload(payload) {
  if (!isPlainObject(payload)) {
    return fail("INVALID_DDZ_PAYLOAD", "Dou Dizhu payload must be an object.", 400);
  }
  if (!hasOnlyKeys(payload, DDZ_USER_ALLOWED_KEYS)) {
    return fail("DDZ_ONLY_PAYLOAD", "Payload contains fields outside the Dou Dizhu schema.", 403);
  }
  if (payload.game !== "doudizhu") {
    return fail("INVALID_GAME", "game must be doudizhu.", 403);
  }
  if (!DDZ_ALLOWED_PHASES.has(payload.phase)) {
    return fail("INVALID_PHASE", "phase must be bid or play.", 400);
  }
  if (!DDZ_ALLOWED_ROLES.has(payload.role)) {
    return fail("INVALID_ROLE", "role must be unknown, landlord, or peasant.", 400);
  }

  const seat = sanitizeInt(payload.seat, 0, 2, "seat");
  if (!seat.ok) {
    return seat;
  }
  const landlord = sanitizeInt(payload.landlord, -1, 2, "landlord");
  if (!landlord.ok) {
    return landlord;
  }
  const currentPlayer = sanitizeInt(payload.current_player, 0, 2, "current_player");
  if (!currentPlayer.ok) {
    return currentPlayer;
  }
  const highestBid = sanitizeInt(payload.highest_bid, 0, 3, "highest_bid");
  if (!highestBid.ok) {
    return highestBid;
  }
  const highestBidder = sanitizeInt(payload.highest_bidder, -1, 2, "highest_bidder");
  if (!highestBidder.ok) {
    return highestBidder;
  }
  const handCount = sanitizeInt(payload.hand_count, 0, 20, "hand_count");
  if (!handCount.ok) {
    return handCount;
  }
  const hand = sanitizeRankArray(payload.hand, handCount.value, "hand");
  if (!hand.ok) {
    return hand;
  }
  const cardCounts = sanitizeFixedIntArray(payload.card_counts, 3, 0, 20, "card_counts");
  if (!cardCounts.ok) {
    return cardCounts;
  }
  if (typeof payload.legal_actions_truncated !== "boolean") {
    return fail("INVALID_LEGAL_ACTIONS_TRUNCATED", "legal_actions_truncated must be boolean.", 400);
  }

  const historySummary = sanitizeHistorySummary(payload.history_summary);
  if (!historySummary.ok) {
    return historySummary;
  }
  const history = sanitizeHistory(payload.history);
  if (!history.ok) {
    return history;
  }
  const legalActions = sanitizeLegalActions(payload.legal_actions, handCount.value);
  if (!legalActions.ok) {
    return legalActions;
  }

  return {
    ok: true,
    value: {
      game: "doudizhu",
      phase: payload.phase,
      seat: seat.value,
      role: payload.role,
      landlord: landlord.value,
      current_player: currentPlayer.value,
      highest_bid: highestBid.value,
      highest_bidder: highestBidder.value,
      hand_count: handCount.value,
      hand: hand.value,
      card_counts: cardCounts.value,
      legal_actions_truncated: payload.legal_actions_truncated,
      history_summary: historySummary.value,
      history: history.value,
      legal_actions: legalActions.value,
    },
  };
}

function sanitizeHistorySummary(payload) {
  if (!isPlainObject(payload) || !hasOnlyKeys(payload, DDZ_HISTORY_SUMMARY_KEYS)) {
    return fail("INVALID_HISTORY_SUMMARY", "history_summary does not match the Dou Dizhu schema.", 400);
  }

  const historyCount = sanitizeInt(payload.history_count, 0, 512, "history_summary.history_count");
  if (!historyCount.ok) {
    return historyCount;
  }
  const consecutivePasses = sanitizeInt(payload.consecutive_passes, 0, 2, "history_summary.consecutive_passes");
  if (!consecutivePasses.ok) {
    return consecutivePasses;
  }
  const lastNonPassPlayer = sanitizeInt(payload.last_non_pass_player, -1, 2, "history_summary.last_non_pass_player");
  if (!lastNonPassPlayer.ok) {
    return lastNonPassPlayer;
  }
  const singles = sanitizeFixedIntArray(
    payload.players_reported_single,
    undefined,
    0,
    2,
    "history_summary.players_reported_single",
    3
  );
  if (!singles.ok) {
    return singles;
  }
  if (typeof payload.bomb_played !== "boolean" || typeof payload.rocket_played !== "boolean") {
    return fail("INVALID_HISTORY_FLAGS", "history_summary flags must be boolean.", 400);
  }
  if (typeof payload.last_non_pass_play !== "string" || payload.last_non_pass_play.length > 96) {
    return fail("INVALID_LAST_PLAY", "history_summary.last_non_pass_play must be a short string.", 400);
  }

  return {
    ok: true,
    value: {
      history_count: historyCount.value,
      bomb_played: payload.bomb_played,
      rocket_played: payload.rocket_played,
      consecutive_passes: consecutivePasses.value,
      last_non_pass_player: lastNonPassPlayer.value,
      last_non_pass_play: payload.last_non_pass_play,
      players_reported_single: singles.value,
    },
  };
}

function sanitizeHistory(payload) {
  if (!Array.isArray(payload) || payload.length > 256) {
    return fail("INVALID_HISTORY", "history must be an array with at most 256 entries.", 400);
  }

  const out = [];
  for (let index = 0; index < payload.length; index += 1) {
    const item = payload[index];
    if (!isPlainObject(item) || !hasOnlyKeys(item, DDZ_HISTORY_ENTRY_KEYS)) {
      return fail("INVALID_HISTORY_ENTRY", `history[${index}] is invalid.`, 400);
    }
    if (!DDZ_ALLOWED_PHASES.has(item.phase)) {
      return fail("INVALID_HISTORY_PHASE", `history[${index}].phase is invalid.`, 400);
    }

    const step = sanitizeInt(item.step, 0, 1024, `history[${index}].step`);
    if (!step.ok) {
      return step;
    }
    const player = sanitizeInt(item.player, 0, 2, `history[${index}].player`);
    if (!player.ok) {
      return player;
    }
    const currentPlayer = sanitizeInt(item.current_player, 0, 2, `history[${index}].current_player`);
    if (!currentPlayer.ok) {
      return currentPlayer;
    }
    const landlord = sanitizeInt(item.landlord, -1, 2, `history[${index}].landlord`);
    if (!landlord.ok) {
      return landlord;
    }
    const highestBid = sanitizeInt(item.highest_bid, 0, 3, `history[${index}].highest_bid`);
    if (!highestBid.ok) {
      return highestBid;
    }
    const highestBidPlayer = sanitizeInt(item.highest_bid_player, -1, 2, `history[${index}].highest_bid_player`);
    if (!highestBidPlayer.ok) {
      return highestBidPlayer;
    }
    const passCount = sanitizeInt(item.pass_count, 0, 2, `history[${index}].pass_count`);
    if (!passCount.ok) {
      return passCount;
    }
    const handCounts = sanitizeFixedIntArray(item.hand_counts, 3, 0, 20, `history[${index}].hand_counts`);
    if (!handCounts.ok) {
      return handCounts;
    }
    const action = sanitizeAction(item.action, `history[${index}].action`);
    if (!action.ok) {
      return action;
    }

    out.push({
      step: step.value,
      phase: item.phase,
      player: player.value,
      action: action.value,
      hand_counts: handCounts.value,
      current_player: currentPlayer.value,
      landlord: landlord.value,
      highest_bid: highestBid.value,
      highest_bid_player: highestBidPlayer.value,
      pass_count: passCount.value,
    });
  }

  return {
    ok: true,
    value: out,
  };
}

function sanitizeAction(payload, fieldName) {
  if (!isPlainObject(payload) || !hasOnlyKeys(payload, DDZ_ACTION_KEYS)) {
    return fail("INVALID_ACTION", `${fieldName} is invalid.`, 400);
  }
  if (!DDZ_ALLOWED_ACTION_KINDS.has(payload.kind)) {
    return fail("INVALID_ACTION_KIND", `${fieldName}.kind is invalid.`, 400);
  }
  if (typeof payload.label !== "string" || payload.label.length > 128) {
    return fail("INVALID_ACTION_LABEL", `${fieldName}.label is invalid.`, 400);
  }

  const id = sanitizeInt(payload.id, 0, 1024, `${fieldName}.id`);
  if (!id.ok) {
    return id;
  }
  const bid = sanitizeInt(payload.bid, 0, 3, `${fieldName}.bid`);
  if (!bid.ok) {
    return bid;
  }
  const cardCount = sanitizeInt(payload.card_count, 0, 20, `${fieldName}.card_count`);
  if (!cardCount.ok) {
    return cardCount;
  }
  const mainRank = sanitizeInt(payload.main_rank, -1, 14, `${fieldName}.main_rank`);
  if (!mainRank.ok) {
    return mainRank;
  }
  const cards = sanitizeRankArray(payload.cards, cardCount.value, `${fieldName}.cards`, 20);
  if (!cards.ok) {
    return cards;
  }

  return {
    ok: true,
    value: {
      id: id.value,
      kind: payload.kind,
      label: payload.label,
      bid: bid.value,
      card_count: cardCount.value,
      main_rank: mainRank.value,
      cards: cards.value,
    },
  };
}

function sanitizeLegalActions(payload, handCount) {
  if (!Array.isArray(payload) || payload.length <= 0 || payload.length > 128) {
    return fail("INVALID_LEGAL_ACTIONS", "legal_actions must be a non-empty array with at most 128 items.", 400);
  }

  const out = [];
  for (let index = 0; index < payload.length; index += 1) {
    const item = payload[index];
    if (!isPlainObject(item) || !hasOnlyKeys(item, DDZ_LEGAL_ACTION_KEYS)) {
      return fail("INVALID_LEGAL_ACTION", `legal_actions[${index}] is invalid.`, 400);
    }
    if (!DDZ_ALLOWED_ACTION_KINDS.has(item.kind)) {
      return fail("INVALID_LEGAL_ACTION_KIND", `legal_actions[${index}].kind is invalid.`, 400);
    }
    if (typeof item.label !== "string" || item.label.length > 128) {
      return fail("INVALID_LEGAL_ACTION_LABEL", `legal_actions[${index}].label is invalid.`, 400);
    }
    if (typeof item.is_bomb_like !== "boolean" || typeof item.ends_hand !== "boolean") {
      return fail("INVALID_LEGAL_ACTION_FLAGS", `legal_actions[${index}] flags are invalid.`, 400);
    }

    const id = sanitizeInt(item.id, 0, 1024, `legal_actions[${index}].id`);
    if (!id.ok) {
      return id;
    }
    const bid = sanitizeInt(item.bid, 0, 3, `legal_actions[${index}].bid`);
    if (!bid.ok) {
      return bid;
    }
    const cardCount = sanitizeInt(item.card_count, 0, 20, `legal_actions[${index}].card_count`);
    if (!cardCount.ok) {
      return cardCount;
    }
    const mainRank = sanitizeInt(item.main_rank, -1, 14, `legal_actions[${index}].main_rank`);
    if (!mainRank.ok) {
      return mainRank;
    }
    const cards = sanitizeRankArray(item.cards, cardCount.value, `legal_actions[${index}].cards`, 20);
    if (!cards.ok) {
      return cards;
    }
    const remainingHandCount = sanitizeInt(item.remaining_hand_count, 0, 20, `legal_actions[${index}].remaining_hand_count`);
    if (!remainingHandCount.ok) {
      return remainingHandCount;
    }
    if (remainingHandCount.value !== handCount - cardCount.value) {
      return fail(
        "INVALID_REMAINING_HAND_COUNT",
        `legal_actions[${index}].remaining_hand_count does not match hand_count - card_count.`,
        400
      );
    }

    out.push({
      id: id.value,
      label: item.label,
      kind: item.kind,
      bid: bid.value,
      card_count: cardCount.value,
      main_rank: mainRank.value,
      cards: cards.value,
      remaining_hand_count: remainingHandCount.value,
      is_bomb_like: item.is_bomb_like,
      ends_hand: item.ends_hand,
    });
  }

  return {
    ok: true,
    value: out,
  };
}

function sanitizeRankArray(payload, exactLength, fieldName, maxLength = 20) {
  if (!Array.isArray(payload)) {
    return fail("INVALID_RANK_ARRAY", `${fieldName} must be an array.`, 400);
  }
  if (exactLength !== undefined && payload.length !== exactLength) {
    return fail("INVALID_RANK_ARRAY_LENGTH", `${fieldName} length is invalid.`, 400);
  }
  if (payload.length > maxLength) {
    return fail("RANK_ARRAY_TOO_LONG", `${fieldName} is too large.`, 400);
  }

  const out = [];
  for (let index = 0; index < payload.length; index += 1) {
    const item = payload[index];
    if (typeof item !== "string" || !DDZ_ALLOWED_RANKS.has(item)) {
      return fail("INVALID_RANK", `${fieldName}[${index}] is invalid.`, 400);
    }
    out.push(item);
  }

  return {
    ok: true,
    value: out,
  };
}

function sanitizeFixedIntArray(payload, exactLength, min, max, fieldName, maxLength = exactLength) {
  if (!Array.isArray(payload)) {
    return fail("INVALID_INT_ARRAY", `${fieldName} must be an array.`, 400);
  }
  if (exactLength !== undefined && payload.length !== exactLength) {
    return fail("INVALID_INT_ARRAY_LENGTH", `${fieldName} length is invalid.`, 400);
  }
  if (maxLength !== undefined && payload.length > maxLength) {
    return fail("INT_ARRAY_TOO_LONG", `${fieldName} is too large.`, 400);
  }

  const out = [];
  for (let index = 0; index < payload.length; index += 1) {
    const value = sanitizeInt(payload[index], min, max, `${fieldName}[${index}]`);
    if (!value.ok) {
      return value;
    }
    out.push(value.value);
  }

  return {
    ok: true,
    value: out,
  };
}

function sanitizeInt(value, min, max, fieldName) {
  if (!Number.isInteger(value) || value < min || value > max) {
    return fail("INVALID_INTEGER", `${fieldName} must be an integer in [${min}, ${max}].`, 400);
  }
  return {
    ok: true,
    value,
  };
}

async function enforceRateLimit(env, request, clientKey) {
  if (!env.OPENAI_PROXY_LIMITER || typeof env.OPENAI_PROXY_LIMITER.limit !== "function") {
    return { ok: true };
  }

  const ip = readString(request.headers.get("CF-Connecting-IP")) || "unknown";
  const key = clientKey
    ? `token:${clientKey}`
    : `ip:${ip}`;

  const result = await env.OPENAI_PROXY_LIMITER.limit({ key });
  if (result && result.success === false) {
    return fail("RATE_LIMITED", "Too many requests, please retry later.", 429);
  }

  return { ok: true };
}

function checkClientAuthorization(request, env) {
  const expectedToken = readString(env.CLIENT_BEARER_TOKEN);
  if (!expectedToken) {
    return { ok: true, clientKey: "" };
  }

  const auth = request.headers.get("Authorization") || "";
  const prefix = "Bearer ";
  if (!auth.startsWith(prefix)) {
    return fail("UNAUTHORIZED", "Missing Authorization: Bearer <token>.", 401);
  }

  const token = auth.slice(prefix.length).trim();
  if (!token || token !== expectedToken) {
    return fail("FORBIDDEN", "Invalid client token.", 403);
  }

  return { ok: true, clientKey: token };
}

function handlePreflight(request, env, pathname) {
  if (pathname !== DDZ_ENDPOINT) {
    return new Response(null, { status: 404 });
  }

  const origin = request.headers.get("Origin");
  if (!isOriginAllowed(origin, env)) {
    return new Response(null, { status: 403 });
  }

  const headers = new Headers();
  applyCorsHeaders(headers, request, env);
  headers.set("Access-Control-Allow-Methods", "POST, OPTIONS");
  headers.set("Access-Control-Allow-Headers", "Authorization, Content-Type");
  headers.set("Access-Control-Max-Age", String(readPositiveInt(env.CORS_MAX_AGE, DEFAULT_CORS_MAX_AGE)));
  headers.set("Content-Length", "0");
  return new Response(null, { status: 204, headers });
}

function withCors(request, env, response) {
  const headers = new Headers(response.headers);
  applyCorsHeaders(headers, request, env);
  return new Response(response.body, {
    status: response.status,
    statusText: response.statusText,
    headers,
  });
}

function withOptionalUpstreamRequestId(response, requestId) {
  if (!requestId) {
    return response;
  }
  const headers = new Headers(response.headers);
  headers.set("X-Upstream-Request-Id", requestId);
  return new Response(response.body, {
    status: response.status,
    statusText: response.statusText,
    headers,
  });
}

function applyCorsHeaders(headers, request, env) {
  const origin = request.headers.get("Origin");
  if (!origin || !isOriginAllowed(origin, env)) {
    return;
  }

  headers.set("Access-Control-Allow-Origin", origin);
  headers.set("Vary", "Origin");
}

function isOriginAllowed(origin, env) {
  const allowedOrigins = splitCsv(readString(env.ALLOWED_ORIGINS));
  if (allowedOrigins.length === 0) {
    return true;
  }
  if (!origin) {
    return false;
  }
  return allowedOrigins.includes("*") || allowedOrigins.includes(origin);
}

function splitCsv(value) {
  if (!value) {
    return [];
  }
  return value
    .split(/[,\n]/)
    .map((item) => item.trim())
    .filter(Boolean);
}

function readString(value) {
  return typeof value === "string" ? value.trim() : "";
}

function readPositiveInt(value, fallback) {
  const num = Number(value);
  return Number.isFinite(num) && num > 0 ? Math.floor(num) : fallback;
}

function isPlainObject(value) {
  return value !== null && typeof value === "object" && !Array.isArray(value);
}

function hasOnlyKeys(value, allowedKeys) {
  if (!isPlainObject(value)) {
    return false;
  }
  const keys = Object.keys(value);
  return keys.every((key) => allowedKeys.has(key));
}

function jsonResponse(data, status = 200) {
  return new Response(JSON.stringify(data, null, 2), {
    status,
    headers: {
      "Content-Type": "application/json; charset=utf-8",
      "Cache-Control": "no-store",
    },
  });
}

function errorResponse(code, message, status) {
  return jsonResponse(
    {
      error: {
        code,
        message,
      },
    },
    status
  );
}

function fail(code, message, status) {
  return {
    ok: false,
    code,
    message,
    status,
  };
}
