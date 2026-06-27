# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0-dev] - 2026-05-03

### Added

- **SDL GLES2 direct rendering and batch pipeline** (`gui/platform/sdl2/gpu_sdl.uya`).
  - New `SdlGles2GpuCtx` implementing `IGpuCtx` for hardware-accelerated fill rect,
    stroke rect, line, and image batch rendering via OpenGL ES 2.
  - CPU fallback backend (`gui/render/cpu_backend.uya`) for non-GPU batch execution.
  - GPU software reference implementation (`gui/render/gpu_software.uya`) for testing.
  - Render scheduler integration (`gui/render/scheduler.uya`) supporting seed/full/dirty
    present strategies.
- **Animation demo** (`gui/examples/demo_animation.uya`) with retained state,
  play/pause, replay, and section navigation.
- **Expanded clock demo** (`gui/examples/demo_clock.uya`) with retained state and
  canvas-based face rendering.
- **Release build pipeline** (`Makefile`).
  - `make release` runs full CI then packages cross-platform artifacts:
    x86_64 simulator and ARM Cortex-M C99 handoff output.
  - Generates `RELEASE_MANIFEST.txt`, tarball, and SHA-256 checksum.
- **Public API documentation comments** across the entire `gui/` tree;
  `docs/gui_uya_api_reference.md` is now auto-generated with line-number anchors.
- **Integration tests** for phase 5 runtime, render pipeline, and sim app touch
  replay (`gui/tests/test_phase5_runtime.uya`, `gui/tests/test_sim_app.uya`).
- **Benchmark baselines and dashboard compare tooling**
  (`gui/benchmarks/phase5_bench_baseline.json`, `tools/lvgl_compare/`).

### Changed

- **RenderCtx** extended to support GPU batch flushing (`batch`, `gpu` fields) and
  dirty-only rendering mode.
- **Batch system** (`gui/render/batch.uya`) now tracks state-switch counts before
  and after optimization for easier pipeline tuning.
- **Image rendering** (`gui/render/img.uya`) gained bilinear sampling, fast blit,
  rotation, and scaled drawing helpers.
- **Platform display abstraction** (`gui/platform/disp.uya`) refactored to expose
  `DisplayDriverKind` and driver-profile queries for easier porting.
- **Simulator configuration** (`gui/sim/config.uya`) now supports runtime backend
  selection (SDL2/GLES2 vs software) and headless screenshot mode.
- **Dashboard demo layout** now uses `current_demo_origin_x/y()` instead of
  hard-coded offsets for better multi-resolution support.
- README updated with `make release` instructions.

### Fixed

- **GLES2 black screen** on dirty-render mode when no widgets changed.
- **GLES2 gray background** caused by unintended blending during seed/full present.
- **Dashboard button hit testing** now correctly resolves local coordinates across
  nested retained demo pages.
- **Benchmark thresholds** relaxed for `render prim` and `image+batch` to reduce
  host-jitter false alarms in CI.
- **Test robustness**: `test_sim_app.uya` dashboard click test now renders first
  and queries widget screen area instead of hard-coding pixel coordinates.

## [Earlier commits]

See `git log` for history prior to the introduction of this changelog.
