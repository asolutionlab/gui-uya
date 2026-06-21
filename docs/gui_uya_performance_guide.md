# UyaGUI 性能优化指南

## 先看哪里

当前性能关键点主要集中在：

- `core/dirty_region.uya`
- `core/color.uya`
- `platform/disp.uya`
- `render/ctx.uya`
- `render/batch.uya`
- `render/gpu.uya`
- `render/img.uya`
- `render/zerocopy.uya`
- `res/cache.uya`

## 已有优化

### 脏区

- 包含关系去重
- 触碰区域自动合并
- 溢出时优先近似压缩，而不是直接全屏刷新
- `coverage_percent()` / `merge_ops` 可观测

### 批处理

- 同色填充矩形横向/纵向合并
- 水平线段拼接
- `gpu_execute_batch()` 按段下发批量命令

### 像素热路径

- `fill_rect` / `draw_line` / `draw_circle` 减少逐像素 dirty bookkeeping
- `ARGB8888` / `RGB565` / `RGB888` / `L8` / `A8` 走连续行写入快路径
- `draw_image*` 复用单次源 framebuffer 视图，按可见区域直接写目标缓冲

### LUT / SIMD

- Alpha `scale/unscale` 统一走查表
- 弧形绘制使用角度 trig LUT
- 行填充与同格式 blit 使用 `@vector.load/store` 16-byte 块路径

### 零拷贝

- 多传输区域队列
- 自动合并相邻区域
- 支持 `present_dirty()`

### 资源缓存

- 命中率、压力百分比、剩余预算
- LRU 风格淘汰

## 如何量化

推荐命令：

```bash
make bench
make bench-report
make bench-json
make bench-verify
make dashboard-compare-report
```

产物：

- [phase5_bench.txt](/home/winger/gui-uya/build/phase5_bench.txt:1)
- [phase5_bench.json](/home/winger/gui-uya/build/phase5_bench.json:1)
- [gui_uya_phase5_report.md](./gui_uya_phase5_report.md)
- `build/dashboard_compare/dashboard_compare_report.md`
- `src/gui/benchmarks/phase5_bench_baseline.json`

说明：

- `bench-verify` 会把 `build/phase5_bench.txt` 和仓库内阈值基线做比对，适合 CI 或本机回归检查。
- `dashboard-compare-report` 会在同一 `640x480` dashboard 场景下输出 UyaGUI / LVGL 的帧耗时、启动时间、最大 RSS 和二进制体积对照。

## 优化顺序建议

1. 先看是否真的需要更多像素填充
2. 再看是否能减小 dirty region 数量
3. 再看 batch 是否被充分合并
4. 再看缓存预算是否合适
5. 最后再决定是否需要真实 DMA / 硬件 GPU 后端
