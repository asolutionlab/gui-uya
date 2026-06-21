# UyaGUI 主题定制指南

## 主题系统位置

- 主题定义与管理: [theme.uya](/home/winger/uya/gui-uya/src/gui/style/theme.uya:1)
- 样式结构: [style.uya](/home/winger/uya/gui-uya/src/gui/style/style.uya:1)
- 样式属性枚举: [prop.uya](/home/winger/uya/gui-uya/src/gui/style/prop.uya:1)

## 内置主题

当前仓库提供三套基础主题：

1. `material_light_theme()`
2. `material_dark_theme()`
3. `compact_theme()`

它们覆盖了按钮、面板、文本和紧凑组件等常见类型。

## 使用方式

```uya
var manager: ThemeManager = theme_manager_new();
manager.register_theme(material_dark_theme());
manager.switch_theme(ThemeVariant.Dark);
manager.apply_to_tree(page.obj());
```

`apply_to_tree()` 会根据 `type_tag` 和基础 flags，把主题样式递归分发到对象树。

## 调整建议

- 要统一按钮视觉，先改 `button_style`
- 要统一容器视觉，先改 `panel_style`
- 要兼顾小屏设备，优先从 `compact_style` 分支扩展
- 只改单个组件时，直接对组件 `base.style` 覆写即可

## 示例

[demo_dashboard.uya](/home/winger/uya/gui-uya/examples/demo_dashboard.uya:1) 演示了页面树构建完成后再统一应用 `Dark` 主题的做法。
