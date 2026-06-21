# UyaGUI Phase 5 报告

> 日期: 2026-04-29  
> 范围: 当前仓库内可直接实现与验证的 `Phase 5` 性能基准闭环  
> 备注: 目标板 / 显示屏 / 真实 DMA 或硬件 GPU 驱动实测不在本次本地交付范围内

## 本次完成

- `src/gui/benchmarks/core_bench.uya`
  - 补齐 `empty frame` 与 `fullscreen` 两条基准，覆盖“空渲染帧时间 / 全屏刷新时间 / 脏区 / 分配 / 布局 / 事件 / 渲染 / cache / startup / 动画”整套基线
- `src/gui/benchmarks/phase5_bench_baseline.json`
  - 固化仓库内 benchmark snapshot 与回归阈值
- `tools/check_gui_bench.py`
  - 新增 benchmark 文本解析、JSON 导出和阈值校验
- `Makefile`
  - 新增 `make bench-json`、`make bench-snapshot`、`make bench-verify`
  - 新增 `make uya-dashboard-compare`、`make lvgl-dashboard-compare-build`、`make dashboard-compare-report`
- `src/gui/sim/dashboard_compare.uya` + `apps/dashboard_compare_main.uya`
  - 新增最小 Uya dashboard compare runner，固定 `640x480` 输出，并打印启动时间与帧耗时
- `tools/lvgl_compare/dashboard.c`
  - 新增 `startup` 输出与 `LVGL_DASHBOARD_CAPTURE` 开关，便于统一统计
- `tools/dashboard_compare_report.py`
  - 自动汇总 UyaGUI / LVGL 在同场景下的帧耗时、启动时间、最大 RSS 和二进制体积，生成 `build/dashboard_compare/dashboard_compare_report.{md,json}`
- `.github/workflows/gui-phase0.yml`
  - CI 默认改为 `make bench-verify`
  - nightly / manual 额外生成 dashboard compare report

## 验证结果

- `make bench-verify`
  - 通过，当前 `23` 项 benchmark 均落在 `src/gui/benchmarks/phase5_bench_baseline.json` 阈值内
- `make bench-json`
  - 成功生成 `build/phase5_bench.json`
- `make dashboard-compare-report DASHBOARD_COMPARE_FRAMES=10`
  - 成功生成 `build/dashboard_compare/dashboard_compare_report.md`
  - 成功生成 `build/dashboard_compare/uya_dashboard.bmp`
  - 成功生成 `build/dashboard_compare/lvgl_dashboard.bmp`

## 最新基准

以下数据来自 `make bench-report`（`-O3`，5000 iterations）：

| 项目 | 数值 |
|------|------|
| `DirtyRegion` 结构体大小 | `208 bytes` |
| `DrawBatch` 结构体大小 | `4624 bytes` |
| `RenderCtx` 结构体大小 | `416 bytes` |
| `ZeroCopyCtx` 结构体大小 | `160 bytes` |
| `ImageCache` 结构体大小 | `800 bytes` |
| `FB 480x320 RGB565` | `307200 bytes` |
| `FB 320x240 RGB565` | `153600 bytes` |
| `obj_pool` | `1 ms` |
| `allocator` | `0 ms` |
| `layout+dirty` | `0 ms` |
| `dispatcher` | `4 ms` |
| `render prim` | `104 ms` |
| `render fill` | `106 ms` |
| `render alpha` | `1360 ms` |
| `empty frame` | `32668 us` |
| `fullscreen` | `224 ms` |
| `batch raw` | `44 ms` |
| `batch opt` | `7 ms` |
| `image+batch` | `20 ms` |
| `gpu batch` | `13 ms` |
| `cache churn` | `1 ms` |
| `startup init` | `506 us` |
| `32 anims` | `13 ms` |

## Dashboard 对照

以下数据来自 `make dashboard-compare-report DASHBOARD_COMPARE_FRAMES=10`：

| 指标 | UyaGUI | LVGL |
|------|--------|------|
| `frame avg` | `11 ms` | `4 ms` |
| `frame max` | `11 ms` | `4 ms` |
| `startup` | `28700 us` | `39214 us` |
| `max rss` | `12824 KB` | `30280 KB` |
| `elf file size` | `2913024 B` | `1145752 B` |

完整明细见：

- `build/dashboard_compare/dashboard_compare_report.md`
- `build/dashboard_compare/dashboard_compare_report.json`

## 仍待后续

- 更大范围的 LVGL 场景对照（不止 `dashboard`）
- 真实 DMA 后端
- 真实硬件 GPU 后端
- STM32 / ESP32 / RP2040 / RISC-V 目标板实测
- 多分辨率显示屏实机适配验证
