const DEFAULT_UPSTREAM_BASE_URL = "https://gpt001.iotalking.top/v1";
const DEFAULT_UPSTREAM_MODEL = "gpt-5.4-mini";
const DEFAULT_MAX_REQUEST_BYTES = 24 * 1024;
const DEFAULT_MAX_OUTPUT_TOKENS = 64;
const DEFAULT_UPSTREAM_TIMEOUT_MS = 12_000;
const DEFAULT_CORS_MAX_AGE = 86_400;
const DDZ_ENDPOINT = "/ddz/decision";
const DDZ_SYSTEM_PROMPT = "You are a strong Dou Dizhu AI. Use the full bidding and play history to maximize win probability. If you are a peasant, coordinate implicitly with your teammate instead of selfish short-term play. Choose exactly one legal action id from legal_actions, never invent cards, history, bids, or rules, and return JSON only like {\"action_id\":1}.";
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
const DDZ_ALLOWED_RANKS = new Set(["3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2", "BJ", "RJ"]);
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
    const upstreamBody = {
      model: model.value,
      messages: [
        {
          role: "system",
          content: DDZ_SYSTEM_PROMPT,
        },
        {
          role: "user",
          content: JSON.stringify(ddzPayload.value),
        },
      ],
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
  const legalActions = sanitizeLegalActions(payload.legal_actions);
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
  const cards = sanitizeRankArray(payload.cards, undefined, `${fieldName}.cards`, 20);
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

function sanitizeLegalActions(payload) {
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

    out.push({
      id: id.value,
      label: item.label,
      kind: item.kind,
      bid: bid.value,
      card_count: cardCount.value,
      main_rank: mainRank.value,
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
    return true;
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
