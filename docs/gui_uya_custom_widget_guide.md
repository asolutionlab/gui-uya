# UyaGUI 自定义组件教程

## 设计原则

当前仓库里的自定义组件主要沿用两种方式：

1. 基于 `Widget` 组合一个带默认样式和回调绑定的组件
2. 基于 `Canvas` 或直接 `RenderCtx` 做更自由的绘制

示例可参考：

- [gauge.uya](/home/winger/uya/gui-uya/examples/custom/gauge.uya:1)
- [keyboard.uya](/home/winger/uya/gui-uya/examples/custom/keyboard.uya:1)
- [canvas.uya](/home/winger/uya/gui-uya/src/gui/widget/canvas.uya:1)

## 路径一：组合现有 Widget

如果你只是想做一个“特殊布局 + 特殊渲染”的组件，最稳的做法是：

1. 用 `widget_new()` 创建基础 `Widget`
2. 绑定 `IGuiRenderCallback`
3. 在 `render()` 中组合现有绘制 API

仓库里的 `Panel`、`Page`、`Chart` 都是这种思路。

## 路径二：用 Canvas 做自定义绘制

`Canvas` 很适合这几类场景：

- 小游戏棋盘
- 波形/示波器
- 自定义图标
- 迷你仪表盘

典型流程：

```uya
var canvas: Canvas = Canvas.with_buffer(&pixels[0], 16, 16, 64, PixelFormat.ARGB8888);
canvas.begin_draw().fill_rect(Rect{ x: 0, y: 0, w: 16, h: 16 }, bg);
canvas.end_draw();
canvas.render(&ctx);
```

## 建议

- 优先复用 `widget_draw_surface()` 和 `widget_inner_rect()` 维持统一内边距语义
- 有交互时，优先复用 `Widget.handle_input()` 的状态机
- 有滚动或复杂容器行为时，优先参考 `Page` / `ListView` / `GridView`
- 要进入组件树时，始终通过 `obj()` 暴露 `GuiObj`

## 最小检查清单

- 是否定义了默认尺寸
- 是否定义了默认样式
- 是否绑定了 render/input callback
- 是否在内部状态变化后调用 `invalidate()`
- 是否为示例或测试补了一个最小回归入口
