UYA_BUNDLED ?= ./uya/bin/uya
UYA_AUTO := $(shell if command -v uya >/dev/null 2>&1; then command -v uya; elif [ -x "$(UYA_BUNDLED)" ]; then printf '%s\n' "$(UYA_BUNDLED)"; else printf '%s\n' "$(UYA_BUNDLED)"; fi)
UYA ?= $(UYA_AUTO)
export UYA
BUILD_DIR ?= build
UYA_STAGE_DIR ?= $(BUILD_DIR)/uya_stage
UYA_STAGE_EXAMPLE_ALIAS ?= demo
UYA_STAGE_TEST_ALIAS ?= suite
UYA_STAGE_BENCH_ALIAS ?= bench
stage_path = $(abspath $(UYA_STAGE_DIR)/$(patsubst examples/%,$(UYA_STAGE_EXAMPLE_ALIAS)/%,$(patsubst tests/%,$(UYA_STAGE_TEST_ALIAS)/%,$(1))))
TEST_DIR ?= tests
BENCH_DIR ?= src/gui/benchmarks
EXAMPLE_DIR ?= examples
SMOKE_APP ?= apps/phase6_smoke.uya
BENCH_APP ?= gui/bench_suite.uya
TEST_ENTRY ?= tests/test_suite.uya
TEST_EXTRA_ENTRY ?= tests/test_suite_extra.uya
RICHTEXT_TEST_ENTRY ?= tests/richtext_test_suite.uya
WEB_PRESENT_TEST_ENTRY ?= tests/web_present_plan_suite.uya
RENDER_TEST_ENTRY ?= tests/render_test_suite.uya
TEST_STACK_SIZE ?= 65536
BENCH_STACK_SIZE ?= $(TEST_STACK_SIZE)
TEXT_COMPARE_APP ?= apps/text_render_compare.uya
LVGL_COMPARE_DIR ?= tools/lvgl_compare
LVGL_COMPARE_BUILD_DIR ?= $(BUILD_DIR)/lvgl_compare
MODE ?= debug
UYA_OPT := $(if $(filter release,$(MODE)),-O3,-O0)
DASHBOARD_COMPARE_FRAMES ?= 120
DASHBOARD_COMPARE_MODE ?= release
LVGL_DASHBOARD_FRAMES ?= $(DASHBOARD_COMPARE_FRAMES)
LVGL_DASHBOARD_REBUILD ?= 0
FONT_BACKEND_COMPARE_FRAMES ?= 120
BENCH_JSON ?= $(BUILD_DIR)/phase5_bench.json
BENCH_BASELINE ?= src/gui/benchmarks/phase5_bench_baseline.json
UYA_DASHBOARD_COMPARE_DIR ?= $(BUILD_DIR)/dashboard_compare
UYA_DASHBOARD_COMPARE_BIN ?= $(UYA_DASHBOARD_COMPARE_DIR)/uya_dashboard_compare
DASHBOARD_COMPARE_REPORT ?= $(BUILD_DIR)/dashboard_compare/dashboard_compare_report.md

.PHONY: check-uya uya-version stage-uya build test bench bench-report bench-json bench-snapshot bench-verify docs-api ci release release-artifacts clean hooks build-arm build-riscv build-esp32 sim-build sim-run sim-debug sim-headless sim-web-build sim-web-run sim-web-serve sim-web-smoke sim-web-richtext-smoke sim-web-pages text-compare lvgl-text-compare uya-dashboard-compare-build uya-dashboard-compare lvgl-dashboard-compare-build lvgl-dashboard-compare dashboard-compare dashboard-compare-report font-backend-compare demo-font-atlas

SIM_BUILD_DIR ?= $(BUILD_DIR)/sim
SIM_BIN ?= $(SIM_BUILD_DIR)/gui_uya_sim
SIM_ARGS ?=
SIM_HEADLESS_ARGS ?= --max-frames 3 --screenshot $(SIM_BUILD_DIR)/headless.bmp
SIM_FB_ARGS ?= --backend fb --max-frames 60
SIM_WEB_BUILD_DIR ?= $(BUILD_DIR)/web
SIM_WEB_PAGES_DIR ?= $(BUILD_DIR)/pages
SIM_WEB_ARGS ?= --backend web --demo dashboard --max-frames 3
SIM_WEB_PORT ?= 8000
WQY_DEMO_FONT_DIR ?= src/gui/render/generated
WQY_DEMO_FONT_TOOL ?= $(BUILD_DIR)/tools/gen_wqy_demo_bitmap_font
WQY_DEMO_FONT_SRC ?= third_party/fonts/wqy/wqy-microhei.ttc

BENCH_REPORT ?= $(BUILD_DIR)/phase5_bench.txt
RELEASE_VERSION ?= $(shell sed -n 's/^version = "\(.*\)"/\1/p' uya.toml)
RELEASE_DIR ?= $(BUILD_DIR)/release/uyagui-$(RELEASE_VERSION)
RELEASE_TARBALL ?= $(BUILD_DIR)/release/uyagui-$(RELEASE_VERSION).tar.gz

check-uya:
	@if ! command -v "$(UYA)" >/dev/null 2>&1; then \
		echo "error: Uya compiler not found: $(UYA)" >&2; \
		echo "hint : install 'uya' on PATH or run 'make UYA=/path/to/uya <target>'." >&2; \
		exit 127; \
	fi

uya-version: check-uya
	$(UYA) --version

stage-uya:
	python3 tools/stage_uya_sources.py --root '$(CURDIR)' --stage '$(abspath $(UYA_STAGE_DIR))' --examples-alias '$(UYA_STAGE_EXAMPLE_ALIAS)' --tests-alias '$(UYA_STAGE_TEST_ALIAS)' --benchmarks-alias '$(UYA_STAGE_BENCH_ALIAS)'

build: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)
	$(UYA) build $(call stage_path,$(SMOKE_APP)) $(UYA_OPT) -o $(BUILD_DIR)/phase6_smoke

test: check-uya stage-uya
	$(UYA) test $(call stage_path,$(TEST_ENTRY)) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)
	$(UYA) test $(call stage_path,$(TEST_EXTRA_ENTRY)) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)
	$(UYA) test $(call stage_path,$(RICHTEXT_TEST_ENTRY)) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)
	$(UYA) test $(call stage_path,$(WEB_PRESENT_TEST_ENTRY)) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)
	$(UYA) test $(call stage_path,$(RENDER_TEST_ENTRY)) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)

bench: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)
	$(UYA) run $(call stage_path,$(BENCH_APP)) $(UYA_OPT) --stack-size $(BENCH_STACK_SIZE)

bench-report: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)
	@raw_report="$(BENCH_REPORT).raw"; \
	if ! $(UYA) run $(call stage_path,$(BENCH_APP)) -O3 --stack-size $(BENCH_STACK_SIZE) > "$$raw_report" 2>&1; then \
		cat "$$raw_report"; \
		exit 1; \
	fi; \
	sed -n '/^Phase5 benchmark\/report/,$$p' "$$raw_report" > $(BENCH_REPORT)
	@sed -n '1,240p' $(BENCH_REPORT)

bench-json: bench-report
	python3 tools/check_gui_bench.py --report $(BENCH_REPORT) --json-out $(BENCH_JSON)

bench-snapshot: bench-json

bench-verify: bench-report
	python3 tools/check_gui_bench.py --report $(BENCH_REPORT) --baseline $(BENCH_BASELINE) --json-out $(BENCH_JSON) --verify

text-compare: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)/text_compare
	$(UYA) run $(call stage_path,$(TEXT_COMPARE_APP)) $(UYA_OPT)

demo-font-atlas:
	@mkdir -p $(BUILD_DIR)/tools $(WQY_DEMO_FONT_DIR)
	cc -std=c99 -O2 tools/gen_wqy_demo_bitmap_font.c -o $(WQY_DEMO_FONT_TOOL) $$(pkg-config --cflags --libs freetype2) -lm
	python3 tools/gen_wqy_demo_bitmap_assets.py --generator $(WQY_DEMO_FONT_TOOL) --font $(WQY_DEMO_FONT_SRC) --out-dir $(WQY_DEMO_FONT_DIR)

lvgl-text-compare:
	@mkdir -p $(LVGL_COMPARE_BUILD_DIR)
	cmake -S $(LVGL_COMPARE_DIR) -B $(LVGL_COMPARE_BUILD_DIR)
	cmake --build $(LVGL_COMPARE_BUILD_DIR) -j
	SDL_VIDEODRIVER=dummy $(LVGL_COMPARE_BUILD_DIR)/lvgl_text_compare

uya-dashboard-compare-build: check-uya stage-uya
	@mkdir -p $(UYA_DASHBOARD_COMPARE_DIR)
	@APP=$(call stage_path,apps/dashboard_compare_main.uya) OUT_NAME=uya_dashboard_compare MODE=$(DASHBOARD_COMPARE_MODE) BUILD_DIR=$(abspath $(UYA_DASHBOARD_COMPARE_DIR)) bash tools/build_gui_sim.sh

uya-dashboard-compare:
	@mkdir -p $(BUILD_DIR)/dashboard_compare
	$(MAKE) uya-dashboard-compare-build MODE=$(DASHBOARD_COMPARE_MODE)
	SDL_VIDEODRIVER=dummy UYA_DASHBOARD_FRAMES=$(DASHBOARD_COMPARE_FRAMES) UYA_DASHBOARD_OUT=$(BUILD_DIR)/dashboard_compare/uya_dashboard.bmp $(UYA_DASHBOARD_COMPARE_BIN)

lvgl-dashboard-compare-build:
	@mkdir -p $(LVGL_COMPARE_BUILD_DIR) $(BUILD_DIR)/dashboard_compare
	cmake -S $(LVGL_COMPARE_DIR) -B $(LVGL_COMPARE_BUILD_DIR)
	cmake --build $(LVGL_COMPARE_BUILD_DIR) -j --target lvgl_dashboard_compare

lvgl-dashboard-compare:
	@mkdir -p $(LVGL_COMPARE_BUILD_DIR) $(BUILD_DIR)/dashboard_compare
	$(MAKE) lvgl-dashboard-compare-build
	SDL_VIDEODRIVER=dummy LVGL_DASHBOARD_FRAMES=$(LVGL_DASHBOARD_FRAMES) LVGL_DASHBOARD_REBUILD=$(LVGL_DASHBOARD_REBUILD) $(LVGL_COMPARE_BUILD_DIR)/lvgl_dashboard_compare

dashboard-compare:
	@mkdir -p $(BUILD_DIR)/dashboard_compare
	$(MAKE) uya-dashboard-compare MODE=$(DASHBOARD_COMPARE_MODE) DASHBOARD_COMPARE_FRAMES=$(DASHBOARD_COMPARE_FRAMES)
	$(MAKE) lvgl-dashboard-compare LVGL_DASHBOARD_FRAMES=$(DASHBOARD_COMPARE_FRAMES) LVGL_DASHBOARD_REBUILD=$(LVGL_DASHBOARD_REBUILD)

dashboard-compare-report:
	@mkdir -p $(BUILD_DIR)/dashboard_compare
	$(MAKE) uya-dashboard-compare-build MODE=$(DASHBOARD_COMPARE_MODE)
	$(MAKE) lvgl-dashboard-compare-build
	python3 tools/dashboard_compare_report.py --uya-bin $(UYA_DASHBOARD_COMPARE_BIN) --lvgl-bin $(LVGL_COMPARE_BUILD_DIR)/lvgl_dashboard_compare --out-dir $(BUILD_DIR)/dashboard_compare --frames $(DASHBOARD_COMPARE_FRAMES) --lvgl-rebuild $(LVGL_DASHBOARD_REBUILD)

font-backend-compare:
	@mkdir -p $(BUILD_DIR)/dashboard_compare
	$(MAKE) sim-build MODE=release
	SDL_VIDEODRIVER=dummy $(SIM_BIN) --demo dashboard --max-frames $(FONT_BACKEND_COMPARE_FRAMES) --screenshot $(BUILD_DIR)/dashboard_compare/uya_font_backend_uya.bmp
	SDL_VIDEODRIVER=dummy UYA_FONT_BACKEND=c $(SIM_BIN) --demo dashboard --max-frames $(FONT_BACKEND_COMPARE_FRAMES) --screenshot $(BUILD_DIR)/dashboard_compare/uya_font_backend_c.bmp

docs-api:
	bash tools/gen_gui_api_docs.sh

ci:
	$(MAKE) build
	$(MAKE) test
	$(MAKE) bench-verify
	$(MAKE) docs-api

release: ci release-artifacts

release-artifacts:
	@rm -rf $(RELEASE_DIR) $(RELEASE_TARBALL) $(RELEASE_TARBALL).sha256
	@mkdir -p $(RELEASE_DIR)/bin $(RELEASE_DIR)/c99 $(RELEASE_DIR)/microapp $(RELEASE_DIR)/docs
	$(MAKE) build MODE=release BUILD_DIR=$(BUILD_DIR)/release-build
	$(MAKE) sim-build MODE=release SIM_BUILD_DIR=$(BUILD_DIR)/release-sim
	$(MAKE) build-arm MODE=release BUILD_DIR=$(BUILD_DIR)/release-build
	-$(MAKE) build-riscv MODE=release BUILD_DIR=$(BUILD_DIR)/release-build
	-$(MAKE) build-esp32 MODE=release BUILD_DIR=$(BUILD_DIR)/release-build
	cp $(BUILD_DIR)/release-build/phase6_smoke $(RELEASE_DIR)/bin/
	cp $(BUILD_DIR)/release-sim/gui_uya_sim $(RELEASE_DIR)/bin/
	cp $(BUILD_DIR)/release-build/c99/phase6_smoke_arm.c $(RELEASE_DIR)/c99/
	@if [ -f $(BUILD_DIR)/release-build/microapp/phase6_smoke_rv32.pobj ]; then cp $(BUILD_DIR)/release-build/microapp/phase6_smoke_rv32.pobj $(RELEASE_DIR)/microapp/; else echo "phase6_smoke_rv32.pobj: not built; phase6 demo imports host APIs not allowed by microapp mode" > $(RELEASE_DIR)/microapp/README.txt; fi
	@if [ -f $(BUILD_DIR)/release-build/microapp/phase6_smoke_xtensa.pobj ]; then cp $(BUILD_DIR)/release-build/microapp/phase6_smoke_xtensa.pobj $(RELEASE_DIR)/microapp/; else echo "phase6_smoke_xtensa.pobj: not built; phase6 demo imports host APIs not allowed by microapp mode" >> $(RELEASE_DIR)/microapp/README.txt; fi
	cp README.md uya.toml $(RELEASE_DIR)/
	cp docs/gui_uya_release_plan.md docs/gui_uya_quickstart.md docs/gui_uya_api_reference.md $(RELEASE_DIR)/docs/
	@printf 'UyaGUI %s\nBuilt at: %s\nUya compiler: %s\n' '$(RELEASE_VERSION)' "$$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$$($(UYA) --version)" > $(RELEASE_DIR)/RELEASE_MANIFEST.txt
	@find $(RELEASE_DIR) -type f | sort | sed 's#^$(RELEASE_DIR)/##' >> $(RELEASE_DIR)/RELEASE_MANIFEST.txt
	@tar -C $(BUILD_DIR)/release -czf $(RELEASE_TARBALL) uyagui-$(RELEASE_VERSION)
	@sha256sum $(RELEASE_TARBALL) > $(RELEASE_TARBALL).sha256
	@echo "Release artifact: $(RELEASE_TARBALL)"

clean:
	rm -rf $(BUILD_DIR) .uyacache

hooks:
	git config core.hooksPath .githooks

# ARM Cortex-M 当前以 C99 代码生成为交叉编译交接点。
build-arm: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)/c99
	TARGET_ARCH=arm TARGET_OS=none $(UYA) build --c99 $(call stage_path,$(SMOKE_APP)) $(UYA_OPT) -o $(BUILD_DIR)/c99/phase6_smoke_arm.c

build-riscv: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile rv32_baremetal_softvm $(call stage_path,$(SMOKE_APP)) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase6_smoke_rv32.pobj

build-esp32: check-uya stage-uya
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile xtensa_baremetal_softvm $(call stage_path,$(SMOKE_APP)) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase6_smoke_xtensa.pobj

sim-build: check-uya stage-uya
	@mkdir -p $(SIM_BUILD_DIR)
	@APP=$(call stage_path,apps/sim_main.uya) MODE=$(MODE) BUILD_DIR=$(abspath $(SIM_BUILD_DIR)) bash tools/build_gui_sim.sh

sim-run: sim-build
	$(SIM_BIN) $(SIM_ARGS)

sim-debug: check-uya stage-uya
	@mkdir -p $(SIM_BUILD_DIR)
	@APP=$(call stage_path,apps/sim_main.uya) MODE=debug BUILD_DIR=$(abspath $(SIM_BUILD_DIR)) bash tools/build_gui_sim.sh >/dev/null
	$(SIM_BIN) --hud --profile-every 60 $(SIM_ARGS)

sim-headless: sim-build
	SDL_VIDEODRIVER=dummy $(SIM_BIN) $(SIM_HEADLESS_ARGS)

sim-fb-run: sim-build
	$(SIM_BIN) $(SIM_FB_ARGS)

sim-web-build: check-uya stage-uya
	@mkdir -p $(SIM_WEB_BUILD_DIR)
	@APP=$(call stage_path,apps/sim_web_main.uya) MODE=$(MODE) BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR)) bash tools/build_gui_web.sh

sim-web-run:
	@$(MAKE) sim-web-build MODE=$(MODE) SIM_WEB_BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR))
	@echo "Open http://127.0.0.1:$(SIM_WEB_PORT)/index.html?args=$(SIM_WEB_ARGS)"
	@PORT=$(SIM_WEB_PORT) BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR)) bash tools/serve_gui_web.sh

sim-web-serve:
	@$(MAKE) sim-web-build MODE=$(MODE) SIM_WEB_BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR))
	@PORT=$(SIM_WEB_PORT) BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR)) bash tools/serve_gui_web.sh

sim-web-smoke: sim-web-build
	@BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR)) SMOKE_EXPECT_BITMAP_READY_AT_LEAST=0 SMOKE_EXPECT_BITMAP_REQUESTED_AT_MOST=0 bash tools/smoke_gui_web.sh

sim-web-richtext-smoke: sim-web-build
	@BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR)) \
	SMOKE_SCENARIO=richtext \
	SMOKE_ARGS="--backend web --demo richtext --max-frames 240 --screenshot /tmp/last_frame.png" \
	SMOKE_EXPECT_BITMAP_READY_AT_LEAST=0 \
	SMOKE_EXPECT_BITMAP_REQUESTED_AT_MOST=0 \
	SMOKE_TIMEOUT_MS=45000 \
	bash tools/smoke_gui_web.sh

sim-web-pages: sim-web-build
	@BUILD_DIR=$(abspath $(SIM_WEB_BUILD_DIR)) PAGES_DIR=$(abspath $(SIM_WEB_PAGES_DIR)) bash tools/stage_gui_web_pages.sh
