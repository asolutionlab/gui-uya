# UyaGUI 与 LVGL 文字渲染效果对比

> 日期: 2026-04-26  
> 范围: 文字渲染“效果层面对比”，不是完整性能 benchmark  
> 结论基线: UyaGUI 侧结论来自当前仓库代码与 `make text-compare` 样张；LVGL 侧结论对照 2026-04-26 查询到的 LVGL 官方文档

---

## 1. 如何复现本仓库样张

运行：

```bash
make text-compare
```

会生成：

```text
build/text_compare/uya_text_render_samples.png
```

样张包含 4 组对照面：

- `Builtin 5x7 / CJK 8x8`
- `Vector stroke`
- `TTF hinted 11px`
- `TTF plain 11px`

这 4 组基本覆盖了当前仓库实际存在的文字路径：内置位图字、简化矢量字、纯 Uya TTF 轮廓栅格化，以及 hinting 开关对小字号的影响。

如需生成一张可直接对照的 LVGL 样张，可运行：

```bash
make lvgl-text-compare
```

会生成：

```text
build/text_compare/lvgl_text_render_samples.bmp
```

当前样张已经优化为：

- 每个面板上方显示 `1x` 原始尺寸
- 每个面板下方显示 `x3` 放大视图
- 放大视图叠加轻量网格，便于观察边缘软化、hinting 和像素对齐
- `make test` 里的 render suite 会做一次 PNG 生成 smoke 检查

---

## 2. 当前 UyaGUI 的文字渲染现状

结合 [src/gui/render/font.uya](../src/gui/render/font.uya) 和现有测试，可以确认当前实现具备这些能力：

- 内置 ASCII 位图字：默认 `5x7`
- 内置中文点阵：`U+4E00..U+9FFF` 映射到 `8x8` 点阵
- 灰度/彩色 framebuffer 上，会对字形边缘做轻量 alpha 抗锯齿
- 单色 `I1` framebuffer 保持硬边渲染
- 内置 kerning：当前只覆盖少量硬编码字偶，例如 `AV`
- TTF 路径：支持纯 Uya 轮廓解析、复合字形、glyph cache、小字号 hinting 开关，以及 `TTC` 集合字体
- GUI 层默认 widget 字体现在会先按 `font_size` + DPI 解析系统 UI/CJK TTF/TTC，不可用时再回退到内嵌可缩放 UI TTF
- 当前已经有一条实用 fallback 链：`UI/TTF -> 系统 CJK -> 内置位图`；但还没有 RTL / BiDi / 阿拉伯文 shaping / emoji 等更完整文本栈

从样张能直接看出的现象是：

- 内置位图字的边缘已经不是“纯硬锯齿”，但本质仍是小尺寸点阵字放大后的效果
- `TTF hinted 11px` 比 `TTF plain 11px` 更收敛，竖线和斜线更稳
- 核心保底中文仍保留 `8x8` 点阵，但默认控件和缺字 TTF 路径已经能优先切到可缩放中文字形

---

## 3. LVGL 官方字体机制对照面

以下结论只引用 LVGL 官方文档：

- LVGL 字体总览明确支持 UTF-8、kerning、font fallback，并区分 built-in / Tiny TTF / FreeType 等字体引擎  
  来源: https://docs.lvgl.io/master/main-modules/fonts/index.html
- LVGL 的内置/转换字体支持多种位图 `bpp`，常见为 `1/2/4/8`，更高 `bpp` 会带来更平滑边缘，但占用更多内存  
  来源: https://docs.lvgl.io/8/overview/font.html  
  补充: https://docs.lvgl.io/9.5/main-modules/fonts/built_in_fonts.html
- LVGL 的 Tiny TTF 引擎可直接从内存或文件创建 TTF/OTF 字体，支持 cache size 和 kerning 配置  
  来源: https://docs.lvgl.io/master/libs/font_support/tiny_ttf.html
- LVGL 的 FreeType 引擎支持运行时位图化、glyph cache、kerning、彩色 bitmap glyph（例如 emoji），以及 outline/vector 路径  
  来源: https://docs.lvgl.io/master/libs/font_support/freetype.html
- LVGL 的双向文字与 Arabic/Persian shaping 是有独立能力入口的，需要在配置里显式开启  
  来源: https://docs.lvgl.io/master/main-modules/fonts/rtl.html

---

## 4. 效果对比结论

### 4.1 小字号英文

如果拿 UyaGUI 当前默认内置字和 LVGL 的常规位图字体相比：

- UyaGUI 当前默认字更接近“超轻量嵌入式点阵字 + 轻量边缘软化”
- LVGL 在 `4bpp/8bpp` 位图字体路径下，理论上可得到更完整的灰度边缘层次
- 所以在小字号英文字体观感上，当前 UyaGUI 默认路径大致接近 LVGL 的轻量 bitmap 路径，但通常还达不到 LVGL 高 `bpp` 字体的平滑度上限

### 4.2 Kerning

- UyaGUI 当前内置 kerning 是少量硬编码字偶，已经能改善 `AV` 这类组合
- LVGL 官方字体链路的 kerning 能力更完整，尤其在 Tiny TTF / FreeType 路径下覆盖面更大

结论：

- 对少量英文标题字偶，当前 UyaGUI 已经不“生硬”
- 但在通用字体排版质量上，LVGL 现成能力更成熟

### 4.3 小字号 TTF 效果

从本仓库样张里的 `TTF hinted 11px` 和 `TTF plain 11px` 可以很明确地看到：

- hinting 对小字号是有效的
- 当前纯 Uya TTF 栈已经不只是“能显示”，而是已经进入“可优化文字观感”的阶段

如果对照 LVGL：

- 这个路径更接近 LVGL Tiny TTF 或 LVGL + FreeType 的定位
- 但 LVGL 在现成生态、字体来源、cache 调参、fallback 和复杂排版能力上仍更强

结论：

- 就“ASCII/拉丁文小字号可读性”而言，UyaGUI 的 TTF hinted 路径已经是当前仓库里最接近 LVGL 现代字体链路的实现
- 它明显好于当前默认内置位图字路径

### 4.4 中文效果

这是两边差距最明显的部分。

当前 UyaGUI：

- 核心保底中文仍来自固定 `8x8` 点阵
- 默认 widget 路径现在会先走系统 UI/CJK TTF/TTC；在系统字体不可用时，才回退到仓库内可缓存的可缩放 UI TTF
- 普通 TTF 路径在缺少中文字形时，也不再只剩 `8x8` 点阵保底，而是会先尝试系统 CJK fallback
- 优点是英文/数字和中文都能在 GUI 层拿到更稳定的小字号观感，同时保底路径仍占用可控
- 缺点是高质量 CJK 仍依赖宿主环境字体，不是完全自带的中文字体资产方案

LVGL：

- 自带字体未必天然覆盖完整 CJK
- 但官方明确提供 font fallback、Tiny TTF、FreeType 等路径，可以更自然地接入可缩放中文字体

结论：

- 如果只看“当前仓库自带保底路径”，LVGL 接入外部字体引擎后的上限仍明显更高
- 如果看当前 GUI 默认字体链路，UyaGUI 已经从“纯 8x8 保底”迈到“系统 UI/CJK TTF + 内嵌 UI fallback + 位图兜底”的阶段
- 但 UyaGUI 还缺少自带中文字体资产、复杂排版能力，以及更广覆盖的 fallback 链，距离 LVGL 的成熟字体生态仍有明显差距

### 4.5 复杂脚本

当前仓库没有看到 RTL / BiDi / Arabic shaping 的实现，而 LVGL 官方文档明确提供了对应能力入口。

结论：

- 在阿拉伯文、波斯文、希伯来文、混排方向控制这些场景里，当前 UyaGUI 还不能和 LVGL 正面比较
- 这部分 LVGL 明显领先

---

## 5. 总结判断

如果只比较“当前仓库已经落地的真实效果”，可以归纳成一句话：

> UyaGUI 现在已经具备可用的英文字体渲染基线，TTF hinted 路径也已经摸到接近现代 GUI 字体链路的门槛；默认 widget 与缺字 TTF 中文都已经能接入可缩放 CJK fallback，但在自带中文字体资产、复杂脚本和更广文本生态上，和 LVGL 的成熟字体生态相比还有明显差距。

更细一点：

- 默认英文观感：`UyaGUI < LVGL 高 bpp 位图/TTF 路径`
- 当前 TTF 小字号：`UyaGUI 已接近“可认真打磨”的阶段`
- 中文默认路径：`UyaGUI 已明显好于纯 8x8 保底，但仍弱于 LVGL + 完整外部字体引擎`
- 复杂文本能力：`UyaGUI 目前缺项，LVGL 更完整`

---

## 6. 建议的下一步

如果目标是把 UyaGUI 的文字效果继续往 LVGL 靠，优先级建议如下：

1. 把当前 `UI/TTF -> 系统 CJK -> 位图` 的 fallback 链继续扩展到 emoji / 更多脚本，而不是只覆盖 Latin/CJK 主路径
2. 把当前“依赖系统 CJK TTC”的默认中文路径，升级成仓库自带、可缓存的 TTF/bitmap atlas 方案
3. 扩大 kerning 覆盖，至少不要只停留在少量硬编码字偶
4. 增加 RTL / BiDi / Arabic shaping 能力
5. 再补“真实 LVGL 渲染截图 + 同文案同字号同背景”的并排样张

当前这份文档完成的是第 0 步：先把“效果对比”从口头判断，变成仓库内可复现的样张和结论。
