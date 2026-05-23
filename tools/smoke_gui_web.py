#!/usr/bin/env python3

import argparse
import asyncio
import json
import sys

from pyppeteer import launch
from pyppeteer.errors import TimeoutError


PNG_SIGNATURE = [137, 80, 78, 71, 13, 10, 26, 10]
RICHTEXT_INPUT_TOKEN = "[[SMOKE_INPUT]]"
RICHTEXT_PASTE_TOKEN = "\n[[SMOKE_PASTE]]"
RICHTEXT_IME_TOKEN = "中文IME"


def parse_expect_sizes(raw_values: list[str]) -> list[int]:
    sizes: list[int] = []
    for raw in raw_values:
        for part in raw.split(","):
            part = part.strip()
            if not part:
                continue
            sizes.append(int(part))
    return sizes


async def read_richtext_export(page, copy_fn_name: str) -> str | None:
    return await page.evaluate(
        """(copyFnName) => {
            if (!window.Module || !Module[copyFnName] || !Module._malloc || !Module._free || typeof TextDecoder === 'undefined') {
              return null;
            }
            var cap = 16384;
            var ptr = Module._malloc(cap);
            try {
              var len = Module[copyFnName](ptr, cap) | 0;
              if (len <= 0) {
                return '';
              }
              var bytes = Module.HEAPU8.slice(ptr, ptr + len);
              return new TextDecoder('utf-8').decode(bytes);
            } finally {
              Module._free(ptr);
            }
        }""",
        copy_fn_name,
    )


async def wait_for_richtext_export_contains(page, copy_fn_name: str, token: str, timeout_ms: int) -> str | None:
    deadline = asyncio.get_running_loop().time() + timeout_ms / 1000.0
    last_value = None
    while asyncio.get_running_loop().time() < deadline:
        last_value = await read_richtext_export(page, copy_fn_name)
        if isinstance(last_value, str) and token in last_value:
            return last_value
        await asyncio.sleep(0.05)
    return last_value


async def wait_for_richtext_export_equals(page, copy_fn_name: str, expected: str, timeout_ms: int) -> str | None:
    deadline = asyncio.get_running_loop().time() + timeout_ms / 1000.0
    last_value = None
    while asyncio.get_running_loop().time() < deadline:
        last_value = await read_richtext_export(page, copy_fn_name)
        if last_value == expected:
            return last_value
        await asyncio.sleep(0.05)
    return last_value


async def click_canvas_point(page, canvas_x: int, canvas_y: int) -> None:
    point = await page.evaluate(
        """(canvasX, canvasY) => {
            var canvas = document.getElementById('uya-gui-canvas');
            if (!canvas) {
              return null;
            }
            var rect = canvas.getBoundingClientRect();
            var width = canvas.width || rect.width || 1;
            var height = canvas.height || rect.height || 1;
            return {
              x: rect.left + (canvasX / width) * rect.width,
              y: rect.top + (canvasY / height) * rect.height,
            };
        }""",
        canvas_x,
        canvas_y,
    )
    if not point:
        raise RuntimeError("canvas_unavailable")
    await page.mouse.click(point["x"], point["y"])


async def touch_canvas_point(page, canvas_x: int, canvas_y: int) -> dict:
    return await page.evaluate(
        """(canvasX, canvasY) => {
            var canvas = document.getElementById('uya-gui-canvas');
            if (!canvas) {
              return { ok: false, error: 'canvas_unavailable' };
            }
            var rect = canvas.getBoundingClientRect();
            var width = canvas.width || rect.width || 1;
            var height = canvas.height || rect.height || 1;
            var clientX = rect.left + (canvasX / width) * rect.width;
            var clientY = rect.top + (canvasY / height) * rect.height;
            var touchInit = {
              identifier: 1,
              target: canvas,
              clientX: clientX,
              clientY: clientY,
              screenX: clientX,
              screenY: clientY,
              pageX: clientX,
              pageY: clientY,
            };
            var touch = typeof Touch === 'function' ? new Touch(touchInit) : touchInit;
            function touchEvent(name, active) {
              if (typeof TouchEvent === 'function') {
                return new TouchEvent(name, {
                  bubbles: true,
                  cancelable: true,
                  touches: active ? [touch] : [],
                  targetTouches: active ? [touch] : [],
                  changedTouches: [touch],
                });
              }
              var evt = new Event(name, { bubbles: true, cancelable: true });
              Object.defineProperty(evt, 'touches', { value: active ? [touch] : [] });
              Object.defineProperty(evt, 'targetTouches', { value: active ? [touch] : [] });
              Object.defineProperty(evt, 'changedTouches', { value: [touch] });
              return evt;
            }
            canvas.dispatchEvent(touchEvent('touchstart', true));
            canvas.dispatchEvent(touchEvent('touchend', false));
            if (window.Module && Module.uyaGuiSyncRichTextIme) {
              Module.uyaGuiSyncRichTextIme();
            }
            return { ok: true };
        }""",
        canvas_x,
        canvas_y,
    )


async def wait_for_richtext_ime_state(page, timeout_ms: int) -> dict:
    await page.waitForFunction(
        """() => {
            var sink = document.getElementById('uya-gui-ime-sink');
            return !!(sink
              && window.Module
              && Module._uya_gui_web_host_richtext_active_session
              && (Module._uya_gui_web_host_richtext_active_session() | 0) > 0
              && document.activeElement === sink
              && sink.dataset.uyaRichtextActive === '1'
              && sink.dataset.uyaImeMode === 'richtext');
        }""",
        {"timeout": timeout_ms},
    )
    return await read_richtext_ime_state(page)


async def read_richtext_ime_state(page) -> dict:
    return await page.evaluate(
        """() => {
            var sink = document.getElementById('uya-gui-ime-sink');
            var style = sink ? window.getComputedStyle(sink) : null;
            return {
              activeSession: window.Module && Module._uya_gui_web_host_richtext_active_session
                ? (Module._uya_gui_web_host_richtext_active_session() | 0)
                : 0,
              sinkFocused: !!(sink && document.activeElement === sink),
              sinkMode: sink ? (sink.dataset.uyaImeMode || '') : '',
              sinkActive: sink ? (sink.dataset.uyaRichtextActive || '') : '',
              sinkValue: sink ? sink.value : '',
              inputMode: sink ? (sink.getAttribute('inputmode') || '') : '',
              enterKeyHint: sink ? (sink.getAttribute('enterkeyhint') || '') : '',
              fontSize: style ? style.fontSize : '',
            };
        }"""
    )


async def dispatch_richtext_input(page, text: str) -> bool:
    return await page.evaluate(
        """(text) => {
            var sink = document.getElementById('uya-gui-ime-sink');
            if (!sink) {
              return false;
            }
            sink.value = text;
            var evt;
            try {
              evt = new InputEvent('input', { data: text, inputType: 'insertText', bubbles: true, cancelable: true });
            } catch (err) {
              evt = new Event('input', { bubbles: true, cancelable: true });
              Object.defineProperty(evt, 'data', { value: text });
              Object.defineProperty(evt, 'inputType', { value: 'insertText' });
            }
            sink.dispatchEvent(evt);
            return true;
        }""",
        text,
    )


async def dispatch_richtext_composition(page, text: str) -> bool:
    return await page.evaluate(
        """(text) => {
            var sink = document.getElementById('uya-gui-ime-sink');
            if (!sink) {
              return false;
            }
            function composition(type, data) {
              try {
                return new CompositionEvent(type, { data: data, bubbles: true, cancelable: true });
              } catch (err) {
                var evt = new Event(type, { bubbles: true, cancelable: true });
                Object.defineProperty(evt, 'data', { value: data });
                return evt;
              }
            }
            sink.dispatchEvent(composition('compositionstart', ''));
            sink.dispatchEvent(composition('compositionupdate', text.slice(0, 1)));
            sink.dispatchEvent(composition('compositionend', text));
            return true;
        }""",
        text,
    )


async def dispatch_richtext_beforeinput_paste(page) -> bool:
    return await page.evaluate(
        """() => {
            var sink = document.getElementById('uya-gui-ime-sink');
            if (!sink) {
              return false;
            }
            var evt;
            try {
              evt = new InputEvent('beforeinput', { inputType: 'insertFromPaste', bubbles: true, cancelable: true });
            } catch (err) {
              evt = new Event('beforeinput', { bubbles: true, cancelable: true });
              Object.defineProperty(evt, 'inputType', { value: 'insertFromPaste' });
            }
            var allowed = sink.dispatchEvent(evt);
            return evt.defaultPrevented || !allowed;
        }"""
    )


async def dispatch_richtext_paste(page, text: str) -> bool:
    return await page.evaluate(
        """(text) => {
            var sink = document.getElementById('uya-gui-ime-sink');
            if (!sink) {
              return false;
            }
            var evt = new Event('paste', { bubbles: true, cancelable: true });
            Object.defineProperty(evt, 'clipboardData', {
              value: {
                getData: function(type) { return type === 'text/plain' ? text : ''; }
              }
            });
            var allowed = sink.dispatchEvent(evt);
            return evt.defaultPrevented || !allowed;
        }""",
        text,
    )


async def dispatch_richtext_clipboard(page, event_name: str) -> dict:
    return await page.evaluate(
        """(eventName) => {
            var sink = document.getElementById('uya-gui-ime-sink');
            if (!sink) {
              return { ok: false, error: 'sink_unavailable' };
            }
            var values = {};
            var evt = new Event(eventName, { bubbles: true, cancelable: true });
            Object.defineProperty(evt, 'clipboardData', {
              value: {
                setData: function(type, value) { values[type] = value; },
                getData: function(type) { return values[type] || ''; }
              }
            });
            var allowed = sink.dispatchEvent(evt);
            return {
              ok: true,
              defaultPrevented: evt.defaultPrevented || !allowed,
              text: values['text/plain'] || ''
            };
        }""",
        event_name,
    )


async def run_richtext_smoke(page, timeout_ms: int) -> dict:
    await page.waitForFunction(
        "window.Module && (Module.uyaGuiLastPresentAt || 0) > 0",
        {"timeout": timeout_ms},
    )
    touch_result = await touch_canvas_point(page, 180, 500)
    if not touch_result.get("ok"):
        return {"ok": False, "error": "touch_focus_dispatch_failed", "touch": touch_result}
    focus_ok = await page.evaluate(
        """() => !!(window.Module
          && Module._uya_gui_web_host_richtext_focus_editor
          && Module._uya_gui_web_host_richtext_focus_editor())"""
    )
    if not focus_ok:
        return {"ok": False, "error": "focus_editor_unavailable"}
    focus_state = await wait_for_richtext_ime_state(page, timeout_ms)
    if focus_state.get("fontSize") != "16px" or focus_state.get("inputMode") != "text" or focus_state.get("enterKeyHint") != "done":
        return {"ok": False, "error": "mobile_ime_sink_attributes_invalid", "state": focus_state}

    await page.setViewport({"width": 414, "height": 736, "isMobile": True, "hasTouch": True, "deviceScaleFactor": 2})
    await page.evaluate("window.dispatchEvent(new Event('resize'))")
    resize_state = await wait_for_richtext_ime_state(page, timeout_ms)
    if not resize_state.get("sinkFocused") or resize_state.get("sinkMode") != "richtext" or resize_state.get("sinkActive") != "1":
        return {"ok": False, "error": "resize_ime_state_lost", "state": resize_state}

    input_ok = await dispatch_richtext_input(page, RICHTEXT_INPUT_TOKEN)
    if not input_ok:
        return {"ok": False, "error": "input_event_unavailable", "state": focus_state}

    plain_after_input = await wait_for_richtext_export_contains(
        page,
        "_uya_gui_web_host_richtext_plain_text_copy",
        RICHTEXT_INPUT_TOKEN,
        timeout_ms,
    )
    if not isinstance(plain_after_input, str) or RICHTEXT_INPUT_TOKEN not in plain_after_input:
        return {
            "ok": False,
            "error": "input_token_missing",
            "plain": plain_after_input,
        }

    beforeinput_prevented = await dispatch_richtext_beforeinput_paste(page)
    if not beforeinput_prevented:
        return {"ok": False, "error": "beforeinput_paste_not_prevented"}

    paste_ok = await dispatch_richtext_paste(page, RICHTEXT_PASTE_TOKEN)
    if not paste_ok:
        return {"ok": False, "error": "paste_event_unavailable"}

    final_plain = await wait_for_richtext_export_contains(
        page,
        "_uya_gui_web_host_richtext_plain_text_copy",
        "[[SMOKE_PASTE]]",
        timeout_ms,
    )
    if not isinstance(final_plain, str) or "[[SMOKE_PASTE]]" not in final_plain:
        return {
            "ok": False,
            "error": "paste_token_missing",
            "plain": final_plain,
        }

    composition_ok = await dispatch_richtext_composition(page, RICHTEXT_IME_TOKEN)
    if not composition_ok:
        return {"ok": False, "error": "composition_event_unavailable"}
    ime_plain = await wait_for_richtext_export_contains(
        page,
        "_uya_gui_web_host_richtext_plain_text_copy",
        RICHTEXT_IME_TOKEN,
        timeout_ms,
    )
    if not isinstance(ime_plain, str) or RICHTEXT_IME_TOKEN not in ime_plain:
        return {
            "ok": False,
            "error": "composition_token_missing",
            "plain": ime_plain,
        }

    final_html = await read_richtext_export(page, "_uya_gui_web_host_richtext_html_copy")
    if not isinstance(final_html, str):
        return {"ok": False, "error": "html_export_unavailable"}
    if (
        RICHTEXT_INPUT_TOKEN not in final_html
        or "[[SMOKE_PASTE]]" not in final_html
        or RICHTEXT_IME_TOKEN not in final_html
        or "<p>" not in final_html
    ):
        return {
            "ok": False,
            "error": "html_export_missing_tokens",
            "html": final_html,
        }

    select_ok = await page.evaluate(
        """() => !!(window.Module
          && Module._uya_gui_web_host_richtext_select_all
          && Module._uya_gui_web_host_richtext_select_all())"""
    )
    if not select_ok:
        return {"ok": False, "error": "select_all_unavailable"}
    copy_result = await dispatch_richtext_clipboard(page, "copy")
    if not copy_result.get("defaultPrevented") or RICHTEXT_INPUT_TOKEN not in copy_result.get("text", ""):
        return {"ok": False, "error": "copy_event_failed", "copy": copy_result}

    select_ok = await page.evaluate(
        """() => !!(window.Module
          && Module._uya_gui_web_host_richtext_select_all
          && Module._uya_gui_web_host_richtext_select_all())"""
    )
    if not select_ok:
        return {"ok": False, "error": "select_all_before_cut_unavailable"}
    cut_result = await dispatch_richtext_clipboard(page, "cut")
    if not cut_result.get("defaultPrevented") or RICHTEXT_PASTE_TOKEN.strip() not in cut_result.get("text", ""):
        return {"ok": False, "error": "cut_event_failed", "cut": cut_result}
    plain_after_cut = await wait_for_richtext_export_equals(page, "_uya_gui_web_host_richtext_plain_text_copy", "", timeout_ms)
    if plain_after_cut != "":
        return {"ok": False, "error": "cut_did_not_clear_selection", "plain": plain_after_cut}

    window_blur_ok = await page.evaluate(
        """() => {
            window.dispatchEvent(new Event('blur'));
            return true;
        }"""
    )
    await asyncio.sleep(0.1)
    blur_state = await read_richtext_ime_state(page)
    if not window_blur_ok or blur_state.get("sinkFocused") or blur_state.get("sinkMode") != "idle" or blur_state.get("sinkActive") != "0" or blur_state.get("sinkValue") != "":
        return {"ok": False, "error": "blur_state_not_clean", "state": blur_state}

    richtext_state = await read_richtext_ime_state(page)

    return {
        "ok": True,
        "plain": ime_plain,
        "html": final_html,
        "copyTextBytes": len(copy_result.get("text", "").encode("utf-8")),
        "cutTextBytes": len(cut_result.get("text", "").encode("utf-8")),
        "focusState": focus_state,
        "resizeState": resize_state,
        "blurState": blur_state,
        "state": richtext_state,
    }


async def run_smoke(
    url: str,
    screenshot_path: str,
    timeout_ms: int,
    expect_bitmap_ready_sizes: list[int],
    expect_bitmap_ready_at_least: int,
    expect_bitmap_requested_at_most: int | None,
    scenario: str,
) -> int:
    browser = await launch(
        headless=True,
        args=[
            "--no-sandbox",
            "--disable-setuid-sandbox",
            "--disable-dev-shm-usage",
        ],
    )
    page = await browser.newPage()
    if scenario == "richtext":
        await page.setViewport({"width": 390, "height": 844, "isMobile": True, "hasTouch": True, "deviceScaleFactor": 2})
    page.on("console", lambda msg: print(f"[browser:{msg.type}] {msg.text}"))
    page.on("pageerror", lambda err: print(f"[pageerror] {err}"))
    try:
        await page.goto(url, waitUntil="networkidle0", timeout=timeout_ms)
        scenario_result = None
        if scenario == "richtext":
            scenario_result = await run_richtext_smoke(page, timeout_ms)
            if scenario_result.get("ok") is not True:
                print(json.dumps({"richtext": scenario_result}, ensure_ascii=True))
                return 1
        try:
            await page.waitForFunction(
                "window.Module && Module.uyaGuiCompleted === true",
                {"timeout": timeout_ms},
            )
        except TimeoutError:
            diag = await page.evaluate(
                """(path) => {
                    const mod = window.Module || null;
                    const out = {
                        completed: !!(mod && mod.uyaGuiCompleted),
                        lastPresentAt: mod ? (mod.uyaGuiLastPresentAt || 0) : 0,
                        completion: mod ? (mod.uyaGuiCompletion || null) : null,
                        args: mod ? (mod.arguments || []) : [],
                        status: document.getElementById('uya-gui-status') ? document.getElementById('uya-gui-status').textContent : '',
                        log: document.getElementById('uya-gui-log') ? document.getElementById('uya-gui-log').textContent : '',
                        screenshotExists: false,
                    };
                    if (typeof FS !== 'undefined' && path) {
                        try {
                            out.screenshotExists = FS.analyzePath(path).exists;
                        } catch (err) {
                            out.fsError = String(err);
                        }
                    }
                    return out;
                }""",
                screenshot_path,
            )
            print(json.dumps({"timeout": diag, "scenario": scenario_result}, ensure_ascii=True))
            return 1
        result = await page.evaluate(
            """(path) => {
                const bitmapStates = (window.Module && Module.uyaGuiBitmapFontStates) ? Module.uyaGuiBitmapFontStates : {};
                const bitmapRequested = Object.keys(bitmapStates).map((key) => Number(key)).sort((a, b) => a - b);
                const bitmapReady = bitmapRequested.filter((size) => bitmapStates[size] === 'ready');
                const out = {
                    completed: !!(window.Module && Module.uyaGuiCompleted),
                    lastPresentAt: window.Module ? (Module.uyaGuiLastPresentAt || 0) : 0,
                    completion: window.Module ? (Module.uyaGuiCompletion || null) : null,
                    screenshotExists: false,
                    screenshotBytes: 0,
                    screenshotSig: [],
                    files: window.Module ? (Module.uyaGuiFiles || {}) : {},
                    bitmapFontStates: bitmapStates,
                    bitmapFontRequested: bitmapRequested,
                    bitmapFontReady: bitmapReady,
                };
                if (typeof FS !== 'undefined' && path) {
                    try {
                        if (FS.analyzePath(path).exists) {
                            const bytes = FS.readFile(path);
                            out.screenshotExists = true;
                            out.screenshotBytes = bytes.length;
                            out.screenshotSig = Array.from(bytes.slice(0, 8));
                        }
                    } catch (err) {
                        out.fsError = String(err);
                    }
                }
                return out;
            }""",
            screenshot_path,
        )
        result["scenario"] = scenario_result
        print(json.dumps(result, ensure_ascii=True))
        if not result["completed"]:
            print("smoke failed: app did not complete", file=sys.stderr)
            return 1
        if result["lastPresentAt"] <= 0:
            print("smoke failed: no frame was presented", file=sys.stderr)
            return 1
        if not result["screenshotExists"] or result["screenshotBytes"] <= 0:
            print("smoke failed: screenshot file missing or empty", file=sys.stderr)
            return 1
        if result["screenshotSig"] != PNG_SIGNATURE:
            print("smoke failed: screenshot is not a PNG payload", file=sys.stderr)
            return 1
        if len(result["bitmapFontReady"]) < expect_bitmap_ready_at_least:
            print(
                "smoke failed: too few ready bitmap fonts "
                f"({len(result['bitmapFontReady'])} < {expect_bitmap_ready_at_least})",
                file=sys.stderr,
            )
            return 1
        for size in expect_bitmap_ready_sizes:
            if size not in result["bitmapFontReady"]:
                print(f"smoke failed: expected ready bitmap font size {size}", file=sys.stderr)
                return 1
        if (
            expect_bitmap_requested_at_most is not None
            and len(result["bitmapFontRequested"]) > expect_bitmap_requested_at_most
        ):
            print(
                "smoke failed: too many bitmap font sizes requested "
                f"({len(result['bitmapFontRequested'])} > {expect_bitmap_requested_at_most})",
                file=sys.stderr,
            )
            return 1
        return 0
    finally:
        await browser.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--url", required=True)
    parser.add_argument("--screenshot-path", default="/tmp/last_frame.png")
    parser.add_argument("--timeout-ms", type=int, default=30000)
    parser.add_argument("--expect-bitmap-ready-size", action="append", default=[])
    parser.add_argument("--expect-bitmap-ready-at-least", type=int, default=0)
    parser.add_argument("--expect-bitmap-requested-at-most", type=int, default=None)
    parser.add_argument("--scenario", choices=["basic", "richtext"], default="basic")
    args = parser.parse_args()
    return asyncio.run(
        run_smoke(
            args.url,
            args.screenshot_path,
            args.timeout_ms,
            parse_expect_sizes(args.expect_bitmap_ready_size),
            args.expect_bitmap_ready_at_least,
            args.expect_bitmap_requested_at_most,
            args.scenario,
        )
    )


if __name__ == "__main__":
    raise SystemExit(main())
