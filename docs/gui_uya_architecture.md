# UyaGUI 架构图

## 系统架构图

```text
apps/* / examples/*
  -> Widget Layer
    -> Core / Layout / Anim / Style
      -> Render
        -> Platform / Resource
          -> FrameBuffer / Input / Tick / FS
```

## 数据流图

```text
InputDev
  -> EventQueue
    -> EventDispatcher
      -> GuiObj tree
        -> invalidate()
          -> DirtyRegion
            -> DrawBatch / RenderCtx
              -> ZeroCopy / DisplayCtx
```

## 模块依赖图

```text
examples/* -> src/gui/widget/*, src/gui/core/*, src/gui/render/*
src/gui/widget/*  -> src/gui/core/*, src/gui/render/*, src/gui/style/*
src/gui/layout/*  -> src/gui/core/obj, src/gui/core/rect, src/gui/style/prop
src/gui/anim/*    -> src/gui/core/obj, src/gui/style/prop
src/gui/render/*  -> src/gui/core/{rect,color,dirty_region}, src/gui/platform/disp
src/gui/res/*     -> src/gui/render/img, std async/libc
src/gui/platform/*-> src/gui/core/event, src/gui/core/point, src/gui/core/rect
```

## 内存布局图

```text
Static / Stack
  - GuiObj / Widget / Panel / Page / Chart
  - DrawBatch
  - DirtyRegion
  - ThemeManager
  - Example framebuffers

Managed Pools / Cache
  - MemPool / PoolManager
  - ObjPool<T>
  - ImageCache entries

Display Buffers
  - front framebuffer
  - back framebuffer
  - optional canvas buffers
```

## 相关阅读

- [gui_uya_design.md](./gui_uya_design.md)
- [gui_uya_phase5_report.md](./gui_uya_phase5_report.md)
- [gui_uya_performance_guide.md](./gui_uya_performance_guide.md)
