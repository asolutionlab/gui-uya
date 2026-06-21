# UyaGUI 富文本输入框详细设计

> 版本: v0.1.0  
> 日期: 2026-05-23  
> 状态: W0 方案收口完成，尚未实现

> 说明: 本文设计一个参考 Quill 的 `RichTextInput` 控件，目标是轻量、易用、可扩展，适合聊天回复、评论、工单描述、后台 CMS 文本域、知识库条目等大多数应用场景。控件应在 UyaGUI 现有 `Widget` / `RenderCtx` / `platform/*` 架构上增量落地，而不是引入 DOM/CSS 依赖或重量级文档编辑器。

## 0. 当前基线与设计结论

当前仓库已经具备几块关键基础：

- `src/gui/widget/lbl.uya`
  - 已有单行 `TextInput`。
  - 已支持 UTF-8 commit 输入、左右移动、退格、点击定位、焦点态绘制。
- `src/gui/core/event.uya`
  - 已有 `TextInput` / `KeyDown` / `KeyUp` / `FocusIn` / `FocusOut` / `Drag*` 事件。
- `src/gui/platform/indev.uya`
  - 已有 `TouchDriver` / `KeyDriver` / `TextDriver` 抽象。
- `src/gui/render/ctx.uya` 与 `src/gui/render/font.uya`
  - 已有裁剪、文字绘制、字体度量和动态字体加载能力。
- `src/gui/platform/web/*`
  - 已有 Web 事件桥接和页面 overlay 容器，可作为隐藏输入层、剪贴板层、IME 层的宿主。

当前明显缺口：

- 只有单行输入，没有多段落、富文本标记、选区、撤销重做。
- 没有统一的剪贴板、粘贴清洗、HTML/Delta 序列化抽象。
- 没有富文本布局缓存、命中测试、选区绘制、段落样式系统。
- Web 侧当前只有文本 commit，没有成体系的富文本宿主桥。

因此本方案的核心结论是：

- 新增控件名定为 `RichTextInput`。
- 核心模型采用“文档树 + 规范化 Delta”双层结构。
- 控件本体保持跨后端一致，平台差异收敛到宿主桥。
- 首版以内建常用格式为主，不追求 Word/Docs 级能力。

---

## 1. 目标

- 轻量
  - 面向 1 KB 到 200 KB 的常见业务文本。
  - 不引入浏览器 DOM 作为主渲染面。
  - 不把 HTML 当作内部真源。
- 易用
  - 默认 API 简单，直接可替换大多数多行文本域。
  - 内建常见快捷键、工具栏命令、占位符、只读态。
  - 支持纯文本、Delta、受限 HTML 三种主流接入方式。
- 可扩展
  - 采用 Quill 风格的 Delta 思想表达编辑操作。
  - 提供格式注册表，允许后续扩展 mention、tag、image、divider 等格式。
  - 平台桥可按后端逐步增强，不阻塞核心控件落地。
- 跨后端一致
  - SDL2 / Framebuffer / Web 使用同一套文档、布局、命令、渲染逻辑。
  - Web 只额外负责系统输入法、剪贴板、移动端键盘等宿主特性。
- 适合大多数应用场景
  - 评论框
  - 工单描述
  - 私信/聊天长输入
  - 后台公告与说明
  - 轻量知识库编辑

## 2. 非目标

- 不做 Word、Google Docs、飞书文档级排版系统。
- 不在首版支持表格、跨页分页、页眉页脚、脚注、批注。
- 不在首版支持多人协同 OT/CRDT。
- 不把任意 HTML/CSS 无损导入作为目标。
- 不在首版支持任意嵌入对象的视频、公式、任意自定义组件树。
- 不在首版保证完整 IME 预编辑 UI 在所有后端都完全一致。
- 不在首版追求超大文档虚拟化编辑器。

## 3. 设计原则

### 3.1 内部模型优先

内部真源必须是平台无关的富文本文档模型，而不是 HTML 字符串。这样：

- `RenderCtx` 可直接渲染。
- SDL2 / FB / Web 不需要三套编辑逻辑。
- 剪贴板和序列化可以做受控转换。

### 3.2 常用能力内建，复杂能力分层

首版内建：

- 粗体、斜体、下划线、删除线
- 行内代码
- 链接
- 段落、标题、引用、无序列表、有序列表、代码块
- 撤销重做
- 纯文本粘贴
- 受限 HTML 导出

扩展能力后置：

- mention / hashtag
- 图片
- 自定义 embed
- Markdown import/export
- 复杂 IME preedit UI

### 3.3 块级与行内分离

- 块级样式决定段落语义、缩进、列表、引用、标题。
- 行内样式决定文字表现。
- 这与 Quill 的“行属性”和“行内属性”职责划分一致，但在内部实现上不依赖 DOM 节点。

### 3.4 编辑命令统一入口

无论来自：

- 键盘快捷键
- 工具栏按钮
- 程序化 API
- 剪贴板粘贴

都统一落到 `RichEditorCommand` 层，避免多套逻辑。

---

## 4. 功能范围

### 4.1 MVP 内建格式

| 类别 | 能力 | MVP 状态 | 说明 |
|------|------|----------|------|
| 行内 | bold | 支持 | 常用强调 |
| 行内 | italic | 支持 | 常用强调 |
| 行内 | underline | 支持 | 常用强调 |
| 行内 | strike | 支持 | 常用强调 |
| 行内 | inline code | 支持 | 开发文案常用 |
| 行内 | link | 支持 | 受限 URL 校验 |
| 块级 | paragraph | 支持 | 默认块 |
| 块级 | heading 1/2/3 | 支持 | 不开放任意字号 |
| 块级 | blockquote | 支持 | 常见内容引用 |
| 块级 | bullet list | 支持 | 常见列表 |
| 块级 | ordered list | 支持 | 常见列表 |
| 块级 | code block | 支持 | 轻量代码展示 |
| 编辑 | undo/redo | 支持 | 合并连续输入 |
| 编辑 | selection | 支持 | 鼠标与键盘选区 |
| 导出 | plain text | 支持 | 搜索/提交常用 |
| 导出 | Delta | 支持 | 内部存储/网络提交 |
| 导出 | HTML | 支持 | 受限标签集合 |

### 4.2 首版不内建但预留扩展点

- 任意颜色 / 任意字号
- 背景高亮
- 图片
- mention/tag
- checklist
- divider
- Markdown 双向导入导出
- 粘贴保留样式

## 5. 总体架构

```text
App / Form / Toolbar
  -> widget.rich_text_input.RichTextInput
      -> richtext.document.RichDocument
      -> richtext.delta.RichDelta
      -> richtext.selection.RichSelection
      -> richtext.history.RichHistory
      -> richtext.layout.RichLayoutCache
      -> richtext.render.RichTextRenderer
      -> platform.text_host.IRichTextHostBridge (optional)

backend differences:
  SDL2 / FB
    -> committed text + key events + optional clipboard bridge
  Web
    -> hidden textarea + clipboard + composition + virtual keyboard
```

分层原则：

- `widget/rich_text_input.uya`
  - 暴露控件 API，承接焦点、样式、滚动、事件回调。
- `richtext/*`
  - 只处理文档、布局、编辑、序列化，不依赖具体后端。
- `platform/*`
  - 只处理系统输入法、剪贴板、焦点同步等宿主能力。

## 6. 目录与文件设计

W0 定稿后的首版目录布局如下：

```text
gui/
  richtext/
    document.uya
    delta.uya
    selection.uya
    history.uya
    layout.uya
    render.uya
  widget/
    rich_text_input.uya
  platform/
    text_host.uya
    web/
      web_host.c                  # RichText 首版桥接直接并入现有宿主
  tests/
    test_richtext_document.uya
    test_richtext_delta.uya
    test_richtext_layout.uya
    test_richtext_selection.uya
    test_richtext_html.uya
    test_richtext_widget.uya
docs/
  gui_uya_rich_text_input_design.md
  gui_uya_rich_text_input_todo.md
```

W0 模块边界说明：

- `src/gui/richtext/document.uya`
  - 承载 `RichBlockType`、`RichInlineMarks`、`RichSpan`、`RichBlock`、`RichDocument`、normalize、plain text 导出和 `RichPos <-> linear_offset` 公共 helper。
- `src/gui/richtext/delta.uya`
  - 承载 snapshot / patch 两类 `RichDelta` 规范、`apply_delta()` 和 `document_to_delta()`。
- `src/gui/richtext/selection.uya`
  - 承载 `RichPos`、`RichRange`、选区归一化与光标移动辅助逻辑。
- `src/gui/richtext/history.uya`
  - 承载 undo / redo 栈和连续输入合并策略。
- `src/gui/richtext/layout.uya`
  - 承载 block 测量、visual line、命中测试和 block 级增量重排。
- `src/gui/richtext/render.uya`
  - 承载基于 layout cache 的绘制逻辑，不直接持有文档真源。
- `src/gui/widget/rich_text_input.uya`
  - 暴露 Widget API，承接焦点、滚动、输入事件和 `on_change` 回调。
- `src/gui/platform/text_host.uya`
  - 只暴露跨后端 `IRichTextHostBridge` 抽象，不放任何 Web / SDL2 / FB 专有实现细节。

延后拆分说明：

- `command`、`format`、`serialize_*`、`normalize`、`clipboard` 首版不作为额外顶层文件；它们先作为上述模块的内部类型或 helper 收敛，等实现压力真实出现后再拆分，避免在 W1 前把模块面做得过宽。
- `rich_toolbar.uya` 是 W8 的可选配套，不是 `RichTextInput` 的首版硬依赖。
- Web 富文本宿主桥首版明确并入现有 `src/gui/platform/web/web_host.c`，不新增 `rich_text_web_host.c`；这样可以复用现有 canvas / overlay 生命周期、减少一套并行宿主初始化逻辑。

---

## 7. 数据模型设计

### 7.1 核心对象

内部真源采用“块 + span”结构，Delta 作为变更集与序列化层。

```uya
export enum RichBlockType {
    Paragraph,
    Heading1,
    Heading2,
    Heading3,
    Quote,
    BulletItem,
    OrderedItem,
    CodeBlock,
}

export struct RichInlineMarks {
    bits: u16,
    link_ref: u16,
}

export struct RichSpan {
    text_ptr: &byte,
    text_len: usize,
    marks: RichInlineMarks,
}

export struct RichBlock {
    block_type: RichBlockType,
    indent: u8,
    spans: &RichSpan,
    span_count: u16,
}

export struct RichDocument {
    blocks: &RichBlock,
    block_count: u32,
    revision: u32,
}
```

其中：

- `RichBlock`
  - 对应一个段落级单元。
- `RichSpan`
  - 对应相同 mark 的连续 UTF-8 文本片段。
- `RichInlineMarks.bits`
  - 用 bit flag 表示 `bold` / `italic` / `underline` / `strike` / `code`。
- `link_ref`
  - 指向链接池，避免在每个 span 内直接持有可变长 URL。

### 7.2 为什么不用 HTML 作为真源

- HTML 导入容易，HTML 编辑正确难。
- HTML 属性过于开放，不适合 framebuffer/canvas 受限渲染链。
- 多后端共享时，树状语义模型比 HTML 字符串更稳定。

### 7.3 为什么不是“只有 Delta，没有文档树”

只用 Delta 做真源会让这些能力变复杂：

- 光标命中测试
- 增量排版
- 块级样式归并
- 列表编号
- 选区高亮和滚动定位

因此这里采用 Quill 的思想但不照搬实现：

- 文档树是运行时真源。
- Delta 是操作、历史、存储、网络交换格式。

### 7.4 标准化规则

每次编辑后执行轻量 normalize，保持文档结构稳定：

- 文档至少有一个 block。
- block 至少有一个 span；空段落用零长度 span 表示。
- 相邻且 marks 相同的 span 自动合并。
- 空 span 在非必要时移除。
- 不允许 block 直接嵌套 block。
- 不支持的格式在导入时被丢弃，不进入内部模型。

### 7.5 线性文档投影

为了让选区、Delta、撤销重做、序列化共享同一套计数语义，文档树之外再定义一个规范化的“线性投影”：

- 每个 Unicode codepoint 记为一个逻辑原子。
- 每个 block 结束时追加一个逻辑 `BlockBreak` 原子。
- 文档始终带有最后一个 `BlockBreak`，即使只有一个空段落也成立。
- block 级样式绑定在该 block 结束处的 `BlockBreak` 原子上。
- 跨段删除、跨段 retain、历史回放全部按这套线性原子计数。

这相当于把运行时树结构投影成：

```text
[text atoms...] + [block break attrs]
[text atoms...] + [block break attrs]
...
```

约束：

- `RichPos`
  - 仍是运行时编辑器内部的定位结构。
- `linear_offset`
  - 作为 Delta / 历史 / 序列化层的统一偏移语义。
- 任何 `RichPos <-> linear_offset` 转换都必须经过统一 helper，而不是让各模块各自推导。

这样可以明确：

- block 样式挂在哪里
- 跨 block 删除如何计数
- 最后一个段落为什么也需要终止符
- `document_to_delta()` 怎样输出稳定快照

## 8. Delta 变更模型

### 8.1 设计目标

- 让撤销重做和程序化编辑复用同一套语义。
- 让网络存储与回放足够简单。
- 保持 Quill 用户心智一致。
- 让序列化结果稳定、可比较、可规范化。

### 8.2 变更操作

```uya
export struct RichBlockAttrs {
    block_type: RichBlockType,
    indent: u8,
}

export struct RichDeltaAttrs {
    inline_marks: RichInlineMarks,
    block: RichBlockAttrs,
}

export enum RichOpKind {
    Retain,
    InsertText,
    InsertBreak,
    Delete,
}

export struct RichDeltaOp {
    kind: RichOpKind,
    count: u32,
    text_ptr: &const byte,
    text_len: usize,
    attrs: RichDeltaAttrs,
}

export struct RichDelta {
    ops: &RichDeltaOp,
    op_count: u32,
}
```

约束：

- `Retain`
  - 跳过线性文档中的逻辑原子数。
  - 在 patch delta 中可携带 attrs，表示对现有内容施加格式变更。
- `InsertText`
  - 插入 UTF-8 文本和对应 marks。
- `InsertBreak`
  - 插入一个逻辑 `BlockBreak`。
  - block 级样式附着在该 break 的 attrs 上。
- `Delete`
  - 删除线性文档中的逻辑原子范围。

说明：

- `ToggleBold` / `ToggleItalic` / `SetHeading1` / `ToggleBulletList` 这类语义只存在于命令层，不进入持久化 Delta。
- 命令执行后，最终必须展开成规范化的 delta patch，而不是把“toggle”本身存进历史或网络格式。
- 与 Quill 一样，块级样式在序列化时投影到换行边界；内部运行时仍保留 block 结构。

进一步区分两种常用 Delta 形态：

- snapshot delta
  - 用于持久化、快照比较、测试 golden。
  - `document_to_delta()` 输出的规范结果。
  - 只包含 `InsertText` 与 `InsertBreak`。
- patch delta
  - 用于历史记录、编辑回放、程序化变更。
  - 可包含 `Retain` / `Insert*` / `Delete`。
  - 仍然不允许出现 toggle 型 op。

### 8.3 历史记录策略

`RichHistory` 维护 undo / redo 栈，记录 patch delta：

- 连续输入合并
  - 500 ms 到 1000 ms 窗口内、相邻插入位置的文本输入合并成一个历史项。
- 结构编辑独立成项
  - 回车拆段
  - 列表切换
  - 粘贴
  - 删除大段文本
- 组合输入合并
  - IME composition 在 `start -> update -> end` 期间只生成一个历史项。

---

## 9. 选择区与光标模型

### 9.1 逻辑位置

```uya
export struct RichPos {
    block_index: u32,
    span_index: u16,
    utf8_offset: u32,
    affinity: u8,
}

export struct RichRange {
    anchor: RichPos,
    focus: RichPos,
}
```

规则：

- `anchor == focus` 表示插入点。
- 选区始终允许反向，渲染时再做归一化。
- `utf8_offset` 指 span 内 UTF-8 字节偏移，但所有对外长度接口优先暴露“逻辑字符数”或 plain text 长度，避免业务层直接依赖字节位置。
- `RichPos` 只作为编辑器内部游标结构存在；跨模块的可比较偏移一律转换成 `linear_offset`。

### 9.2 选区行为

支持以下基础交互：

- 单击
  - 定位光标。
- 拖拽
  - 扩展选区。
- Shift + 方向键
  - 键盘扩展选区。
- 双击
  - 选中词。
- 三击
  - 选中段落。

其中：

- 双击/三击可作为首版增强项，不阻塞 MVP 核心。
- 词边界首版按“空白 / 标点 / CJK 单字边界”处理即可。

### 9.3 自动滚动

控件内部维护：

```uya
scroll_y: i32
preferred_x: i16
```

行为：

- 光标上下移动时保留 `preferred_x`。
- 光标移出视口时自动滚动到可见区域。
- 拖拽选区超出边界时触发自动滚动。

## 10. 布局与渲染设计

### 10.1 布局缓存

为了避免每帧全量重排，布局应拆成两层缓存：

- `RichBlockLayout`
  - 对应一个 block 的测量和软换行结果。
- `RichLayoutCache`
  - 记录视口宽度、字体版本、主题版本、文档 revision。

```uya
export struct RichVisualLine {
    block_index: u32,
    start: RichPos,
    end: RichPos,
    x: i16,
    y: i16,
    w: u16,
    h: u16,
    baseline: i16,
}
```

缓存失效条件：

- 文档 revision 改变
- 控件宽度改变
- 主题字体或字号改变
- block 级样式改变

增量原则：

- 只重排受影响 block。
- 段落高度变化后向后更新累计 y。
- 光标闪烁和选区颜色变化不触发布局，只触发重绘。

### 10.2 换行规则

首版目标是“正确且可接受”，不追求完整 Unicode line breaking：

- 显式 `\n` 一定断行。
- Latin 文本优先按空格和常见标点断行。
- CJK 允许字符级断行。
- 超长 token 允许字符级回退换行。
- code block 首版默认也可软换行，后续再考虑可选横向滚动。

### 10.3 样式解析

最终绘制样式由三层叠加：

1. 控件基础主题
2. block 级样式
3. inline mark 样式

示例：

- `Heading1`
  - 使用更高字号和更大段前段后距。
- `Quote`
  - 使用左侧色条和内缩。
- `InlineCode`
  - 使用浅底纹、等宽字体、微小圆角。
- `Link`
  - 使用主题链接色和下划线。

### 10.4 绘制内容

渲染顺序：

1. 背景与边框
2. 占位符或正文文本
3. 选区高亮
4. 光标
5. 焦点边框
6. 可选滚动条 / 调试 overlay

选区绘制策略：

- 基于 visual line 切分成矩形片段。
- 用半透明主题色填充。
- 不改变文本本身绘制顺序，避免复杂反色逻辑。

光标绘制策略：

- 使用当前主题强调色。
- 基于时间戳闪烁。
- 失焦后隐藏。

### 10.5 Placeholder

当文档逻辑内容为空时：

- 使用单独 placeholder 文本绘制。
- placeholder 不进入文档模型。
- 一旦用户输入，placeholder 消失。

---

## 11. 编辑命令系统

### 11.1 统一命令枚举

```uya
export enum RichCommandKind {
    InsertText,
    InsertParagraphBreak,
    Backspace,
    DeleteForward,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    SelectAll,
    ToggleBold,
    ToggleItalic,
    ToggleUnderline,
    ToggleStrike,
    ToggleInlineCode,
    SetParagraph,
    SetHeading1,
    SetHeading2,
    SetHeading3,
    ToggleBulletList,
    ToggleOrderedList,
    ToggleQuote,
    ToggleCodeBlock,
    Undo,
    Redo,
    Copy,
    Cut,
    PastePlain,
}
```

所有输入入口最终都转换成上述命令。

说明：

- 命令层允许使用 `Toggle*` 语义，因为这对工具栏和快捷键最自然。
- Delta 层不保留 `Toggle*`；命令执行后必须下沉为规范化 patch。

### 11.2 核心编辑规则

- 插入文本
  - 有选区时先删选区，再插入。
- 退格
  - 无选区时删除前一个逻辑字符；在段首时与前段合并。
- 前删
  - 删除后一个逻辑字符；在段尾时与后段合并。
- 回车
  - 普通段落下拆分当前 block。
  - 列表项下保留列表语义。
  - 空列表项回车时退出列表。
- 切换块类型
  - 对当前 block 或选中 block 批量应用。
- 切换行内样式
  - 无选区时更新“待输入 marks”。
  - 有选区时改写对应 span，并做 normalize。

### 11.3 快捷键建议

桌面与 Web 尽量统一：

- `Ctrl/Cmd + B`
  - 粗体
- `Ctrl/Cmd + I`
  - 斜体
- `Ctrl/Cmd + U`
  - 下划线
- `Ctrl/Cmd + Z`
  - 撤销
- `Ctrl/Cmd + Shift + Z`
  - 重做
- `Ctrl/Cmd + A`
  - 全选
- `Ctrl/Cmd + C`
  - 复制
- `Ctrl/Cmd + X`
  - 剪切
- `Ctrl/Cmd + V`
  - 粘贴纯文本

是否支持保留样式粘贴：

- 首版默认否。
- 后续可通过 HTML 粘贴清洗器增量开放。

## 12. Widget 对外 API

建议控件 API 保持和现有 `TextInput` 风格一致，但适配富文本能力：

```uya
export struct RichTextInput {
    widget: Widget,
    editor: RichEditorCore,
    placeholder: &const byte,
    readonly: bool,
    scroll_y: i32,
}

RichTextInput {
    fn new(placeholder: &const byte) RichTextInput
    fn obj(self: &Self) &GuiObj
    fn set_plain_text(self: &Self, text: &const byte) !&Self
    fn plain_text(self: &Self, out: &byte, cap: usize) usize
    fn set_delta(self: &Self, delta: &RichDelta) !&Self
    fn export_delta(self: &Self, out: &RichDeltaBuilder) !void
    fn export_html(self: &Self, out: &RichStringBuilder) !void
    fn set_readonly(self: &Self, readonly: bool) &Self
    fn selection(self: &Self) RichRange
    fn exec(self: &Self, cmd: RichCommand) bool
    fn on_change(self: &Self, callback: &IWidgetValueCallback, user_data: &void) &Self
}
```

这里约定：

- `set_delta()`
  - 读取的是规范化 snapshot delta，而不是命令式 patch。
- `export_delta()`
  - 导出的是稳定、可比较的 snapshot delta。
- `exec()`
  - 走命令层；命令执行后再转成 patch delta 进入历史系统。

建议补充的变更通知：

- 文档内容变更
- 选区变更
- 格式态变更
- 滚动变更

其中 `on_change` 保持已有 `Widget` 风格即可，其余高级回调可后续扩展。

## 13. 工具栏设计

工具栏不与编辑器强绑定，但建议提供轻量配套控件：

```text
RichToolbar
  -> bold
  -> italic
  -> underline
  -> strike
  -> heading
  -> bullet list
  -> ordered list
  -> quote
  -> code block
```

原则：

- 工具栏只是命令发送器。
- 当前选区的激活态从 `RichEditorCore` 查询。
- 不让工具栏持有文档副本。

---

## 14. 平台宿主桥设计

### 14.1 新增抽象

建议新增统一宿主桥：

```uya
export interface IRichTextHostBridge {
    fn attach(self: &Self, session_id: u32) bool;
    fn detach(self: &Self, session_id: u32) void;
    fn set_caret_rect(self: &Self, rect: Rect) void;
    fn set_selection_text(self: &Self, text: &const byte, len: usize) void;
    fn request_copy(self: &Self) bool;
    fn request_cut(self: &Self) bool;
    fn request_paste(self: &Self) bool;
    fn begin_composition(self: &Self) bool;
    fn update_composition(self: &Self, text: &const byte, len: usize) bool;
    fn end_composition(self: &Self, text: &const byte, len: usize) bool;
}
```

首版可以把它做得更简单，但职责边界要先定下来：

- 核心编辑器不直接依赖 DOM、SDL 窗口、系统 API。
- 所有宿主输入增强都走 bridge。

### 14.2 Web 方案

Web 侧推荐方案：

- 在 `src/gui/platform/web/shell.html` 的 `#uya-gui-overlay` 内挂一个隐藏 `textarea`。
- 当 `RichTextInput` 获得焦点时：
  - 同步 caret rect
  - 激活隐藏 `textarea`
  - 接收 `beforeinput` / `input` / `composition*` / `paste` / `copy` / `cut`
- 普通的短文本 commit 可以继续复用现有 `TextInput` 路径。
- 粘贴、剪切板读写、IME composition 不应走当前 `WEB_EVT_TEXT_INPUT` / `EventType.TextInput` 主路径。
  - 当前事件载荷上限只有 `96` 字节。
  - 当前事件模型也没有 composition 生命周期语义。
- 这类富文本宿主能力应通过 `IRichTextHostBridge` 直达编辑器，或走单独的 side buffer / host callback 协议。

Web 侧这样做的收益：

- 系统剪贴板支持更自然。
- 中文、日文、韩文 IME 更可控。
- 移动端虚拟键盘可直接弹出。

### 14.3 SDL2 / Linux 模拟器方案

桌面端可以分阶段做：

- 阶段 1
  - 继续依赖现有 `TextInput` commit 与 `KeyDown`。
  - 支持基本编辑、撤销重做、快捷键。
- 阶段 2
  - 增加 SDL 剪贴板桥。
  - 增加 IME 候选位置同步。

### 14.4 Framebuffer 方案

Framebuffer 不强求系统级剪贴板和 IME：

- 最低保证 committed text 编辑可用。
- 平台功能缺失时，API 行为应降级但不崩溃。
- 可通过外部虚拟键盘或宿主注入文本事件。

## 15. 序列化与导入导出

### 15.1 Plain Text

用途：

- 搜索索引
- 表单提交 fallback
- 占位统计
- 简单 diff

规则：

- block 之间用 `\n` 拼接。
- 不导出格式。

### 15.2 Delta

用途：

- 内部历史
- 网络提交
- 本地持久化
- 测试快照

要求：

- 稳定、可比较、可回放。
- 作为最主要的结构化交换格式。
- 快照导出必须是规范化结果：
  - 包含尾部 `BlockBreak`
  - block attrs 只挂在 `InsertBreak`
  - 不包含 `Toggle*` 一类命令语义

### 15.3 HTML

用途：

- 服务端预览
- 邮件/网页展示
- 与外部系统对接

受限标签建议：

- `p`
- `h1` / `h2` / `h3`
- `blockquote`
- `ul` / `ol` / `li`
- `pre` / `code`
- `strong`
- `em`
- `u`
- `s`
- `a`
- `br`

安全策略：

- 不保留内联样式。
- 不保留脚本、事件属性、未知标签。
- 链接仅接受白名单协议，如 `http`、`https`、`mailto`。

## 16. 样式系统建议

新增一组专用于富文本的 style token：

- `RichBgColor`
- `RichBorderColor`
- `RichSelectionColor`
- `RichCaretColor`
- `RichPlaceholderColor`
- `RichQuoteBarColor`
- `RichCodeBgColor`
- `RichLinkColor`
- `RichHeadingScale1`
- `RichHeadingScale2`
- `RichHeadingScale3`
- `RichParagraphSpacing`

这样做的原因：

- 不污染普通 `Label` / `TextInput` 默认样式语义。
- 方便主题统一调色。

---

## 17. 性能设计

目标范围：

- 5,000 行内字符以内保持流畅编辑。
- 常规评论/工单场景基本无感知卡顿。

策略：

- block 级增量重排
- span normalize 合并，减少碎片
- 光标与选区单独 dirty
- 历史项按输入节流合并
- 链接池和文本缓冲复用，减少频繁 malloc/free

首版不做：

- 可视区虚拟化
- 后台线程排版
- GPU 专用富文本批处理

## 18. 测试设计

至少需要以下测试层次：

- 文档层
  - 插入、删除、拆段、合段、normalize
- Delta 层
  - apply、invert、history merge
- 选择区层
  - 光标移动、词选择、跨段选择
- 布局层
  - 中英文混排换行
  - 列表缩进
  - 标题/引用高度
- 序列化层
  - plain text / Delta / HTML 输出
  - HTML 导入清洗
- Widget 层
  - 焦点
  - placeholder
  - 滚动跟随
  - 快捷键
- Web bridge 层
  - hidden textarea
  - composition
  - paste/copy/cut

## 19. 风险与权衡

### 19.1 UTF-8 与逻辑字符定位

当前基础控件多处直接按 UTF-8 字节偏移处理。富文本要避免业务层直接接触字节偏移，否则会出现：

- 光标落在多字节中间
- 删除半个字符
- 命中测试和序列化不一致

解决方向：

- 内部允许保留 UTF-8 偏移以减少内存，但所有编辑入口都必须经过 codepoint 校正。

### 19.2 Web IME 是真实复杂度来源

如果没有隐藏 `textarea`：

- 中文输入体验会明显不足。
- 移动端键盘和剪贴板很难稳定。
- 当前事件主路径还额外受 `96` 字节载荷上限约束，不适合承接长粘贴和 composition 中间态。

因此 Web bridge 应视为“增强项里最高优先级的一项”，不是可有可无的 UI 优化。

### 19.3 HTML 导入容易失控

建议首版只做受限 HTML 子集：

- 先保证稳定清洗和可预测输出。
- 不追求“复制任何网页都保真”。

## 20. 推荐实施范围

为了平衡“像 Quill”与“保持轻量”，推荐把首版范围收口到：

- 多段落编辑
- 常用 inline marks
- 段落 / 标题 / 引用 / 列表 / 代码块
- 选区、撤销重做、快捷键
- plain text / Delta / 受限 HTML
- Web hidden textarea + paste/copy/cut + composition

不建议首版同时引入：

- 图片
- Markdown 双向转换
- 任意颜色/字号
- 表格
- 协同编辑

这样可以保证：

- 架构足够像 Quill
- 能力足够覆盖多数场景
- 实现复杂度仍然可控
