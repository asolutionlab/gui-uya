UYA ?= ./uya/bin/uya
BUILD_DIR ?= build
TEST_DIR ?= gui/tests
BENCH_DIR ?= gui/benchmarks
EXAMPLE_DIR ?= gui/examples
SMOKE_APP ?= gui/phase6_smoke.uya
BENCH_APP ?= gui/bench_suite.uya
TEST_ENTRY ?= gui/test_suite.uya
RENDER_TEST_ENTRY ?= gui/render_test_suite.uya
TEST_STACK_SIZE ?= 65536
TEXT_COMPARE_APP ?= gui/text_render_compare.uya
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
BENCH_BASELINE ?= gui/benchmarks/phase5_bench_baseline.json
UYA_DASHBOARD_COMPARE_DIR ?= $(BUILD_DIR)/dashboard_compare
UYA_DASHBOARD_COMPARE_BIN ?= $(UYA_DASHBOARD_COMPARE_DIR)/uya_dashboard_compare
DASHBOARD_COMPARE_REPORT ?= $(BUILD_DIR)/dashboard_compare/dashboard_compare_report.md

.PHONY: build test bench bench-report bench-json bench-snapshot bench-verify docs-api ci release release-artifacts clean hooks build-arm build-riscv build-esp32 sim-build sim-run sim-debug sim-headless text-compare lvgl-text-compare uya-dashboard-compare-build uya-dashboard-compare lvgl-dashboard-compare-build lvgl-dashboard-compare dashboard-compare dashboard-compare-report font-backend-compare

SIM_BUILD_DIR ?= $(BUILD_DIR)/sim
SIM_BIN ?= $(SIM_BUILD_DIR)/gui_uya_sim
SIM_ARGS ?=
SIM_HEADLESS_ARGS ?= --max-frames 3 --screenshot $(SIM_BUILD_DIR)/headless.bmp
SIM_FB_ARGS ?= --backend fb --max-frames 60

BENCH_REPORT ?= $(BUILD_DIR)/phase5_bench.txt
RELEASE_VERSION ?= $(shell sed -n 's/^version = "\(.*\)"/\1/p' uya.toml)
RELEASE_DIR ?= $(BUILD_DIR)/release/uyagui-$(RELEASE_VERSION)
RELEASE_TARBALL ?= $(BUILD_DIR)/release/uyagui-$(RELEASE_VERSION).tar.gz

build:
	@mkdir -p $(BUILD_DIR)
	$(UYA) build $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/phase6_smoke

test:
	$(UYA) test $(TEST_ENTRY) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)
	$(UYA) test $(RENDER_TEST_ENTRY) $(UYA_OPT) --stack-size $(TEST_STACK_SIZE)

bench:
	@mkdir -p $(BUILD_DIR)
	$(UYA) run $(BENCH_APP) $(UYA_OPT)

bench-report:
	@mkdir -p $(BUILD_DIR)
	$(UYA) run $(BENCH_APP) -O3 2>&1 | sed -n '/^Phase5 benchmark\/report/,$$p' > $(BENCH_REPORT)
	@sed -n '1,240p' $(BENCH_REPORT)

bench-json: bench-report
	python3 tools/check_gui_bench.py --report $(BENCH_REPORT) --json-out $(BENCH_JSON)

bench-snapshot: bench-json

bench-verify: bench-report
	python3 tools/check_gui_bench.py --report $(BENCH_REPORT) --baseline $(BENCH_BASELINE) --json-out $(BENCH_JSON) --verify

text-compare:
	@mkdir -p $(BUILD_DIR)/text_compare
	$(UYA) run $(TEXT_COMPARE_APP) $(UYA_OPT)

lvgl-text-compare:
	@mkdir -p $(LVGL_COMPARE_BUILD_DIR)
	cmake -S $(LVGL_COMPARE_DIR) -B $(LVGL_COMPARE_BUILD_DIR)
	cmake --build $(LVGL_COMPARE_BUILD_DIR) -j
	SDL_VIDEODRIVER=dummy $(LVGL_COMPARE_BUILD_DIR)/lvgl_text_compare

uya-dashboard-compare-build:
	@mkdir -p $(UYA_DASHBOARD_COMPARE_DIR)
	@APP=$(abspath gui/dashboard_compare_main.uya) OUT_NAME=uya_dashboard_compare MODE=$(DASHBOARD_COMPARE_MODE) BUILD_DIR=$(abspath $(UYA_DASHBOARD_COMPARE_DIR)) bash tools/build_gui_sim.sh

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
build-arm:
	@mkdir -p $(BUILD_DIR)/c99
	TARGET_ARCH=arm TARGET_OS=none $(UYA) build --c99 $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/c99/phase6_smoke_arm.c

build-riscv:
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile rv32_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase6_smoke_rv32.pobj

build-esp32:
	@mkdir -p $(BUILD_DIR)/microapp
	$(UYA) build --app microapp --microapp-profile xtensa_baremetal_softvm $(SMOKE_APP) $(UYA_OPT) -o $(BUILD_DIR)/microapp/phase6_smoke_xtensa.pobj

sim-build:
	@mkdir -p $(SIM_BUILD_DIR)
	@MODE=$(MODE) BUILD_DIR=$(abspath $(SIM_BUILD_DIR)) bash tools/build_gui_sim.sh

sim-run: sim-build
	$(SIM_BIN) $(SIM_ARGS)

sim-debug:
	@mkdir -p $(SIM_BUILD_DIR)
	@MODE=debug BUILD_DIR=$(abspath $(SIM_BUILD_DIR)) bash tools/build_gui_sim.sh >/dev/null
	$(SIM_BIN) --hud --profile-every 60 $(SIM_ARGS)

sim-headless: sim-build
	SDL_VIDEODRIVER=dummy $(SIM_BIN) $(SIM_HEADLESS_ARGS)

sim-fb-run: sim-build
	$(SIM_BIN) $(SIM_FB_ARGS)
