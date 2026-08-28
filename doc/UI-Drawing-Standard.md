# Vega Strike — Text, Text Boxes & Coordinate Systems (Developer Standard)

**This is the standard way to place text, define text boxes, and specify rectangles in Vega Strike.**

It exists so developers use ONE approach instead of inventing their own per-screen systems — the way
the base computer historically did, which is why we accumulated `ImGuiText` vs `TextPlane` divergence,
three coordinate interpretations, and a duplicated format parser.

> **Rule:** before writing a new screen, label, text box, hotspot, or rect, read this. Use the
> standard API. If it doesn't cover your case, **extend the standard** — don't fork it. This is a
> **living reference**.

## TL;DR — the rules

1. **ONE text box class: `ImGuiText`.** Do not use `TextPlane` for new work; never create a new
   text-box class.
2. **ONE format-string parser: `ImGuiText`'s `#` format language.** Do not hand-parse `#c`/`#n`/
   `#l`-style strings in your screen.
3. **ONE coordinate space: normalized `-1..1`**, top-left `(-1,+1)`, bottom-right `(+1,-1)`
   (Y-inverted). Use `Coordinates::` (engine) / `GUIRect` (Assets) to convert.
4. **Font size from `font_point`**, glyph + line spacing computed fresh from rendered metrics — never
   a per-screen literal.
5. **Line spacing from the rendered glyph height**, never `font.size()` or a hardcoded value.
## The two layers

| Layer | Where | What it does |
|---|---|---|
| Engine C++ | `engine/src/vsdraw/` | The actual text box / renderer / coordinate converter. |
| Assets Python | `modules/GUI.py` (Assets-Production) | The authoring API (`GUIRect`, `GUIRoom`, `GUIStaticText`, ...) that builds the engine objects. |

---

## 1. Text box — `ImGuiText`

The single text box class. Use it for any formatted, multi-line text.

```cpp
ImGuiText textBox;
textBox.setRect(rect);                 // Rect in normalized -1..1
textBox.setText("Some #c1:.5:.5#red#-c text");
textBox.setFont(font);                 // Font — size from font_point (see #4)
textBox.setColor(color);
textBox.setJustification(CENTER_JUSTIFY);
textBox.setMultiLine(true);
textBox.draw();                        // draws with the current ImGui draw list
```

**Public API** (`engine/src/vsdraw/imguitext.h`):

| Method | Purpose |
|---|---|
| `setRect(const Rect&)` / `text()` | Set / get the rect (normalized `-1..1`). |
| `setText(const std::string&)` / `text()` | Set / get the text (with `#` format codes). |
| `setFont(const Font&)` / `font()` | Set / get the font. |
| `setColor(const GFXColor&)` / `color()` | Text color. |
| `setJustification(Justification)` / `justification()` | LEFT / CENTER / RIGHT justify. |
| `setMultiLine(bool)` / `multiLine()` | Single vs wrapped multi-line. |
| `setTextScale(float)` / `textScale()` | Per-text render scale (1.0 = normal). |
| `lineCount()` / `layoutVersion()` / `visibleLineCountStartingWith(...)` | Layout introspection (scrolling). |
| `draw(int firstLineToDraw=0)` | Render with the current ImGui draw list. |

**`ImGuiText` format language** (the ONE parser — reuse it, don't reinvent):

- `#c<color>` push color, `#-` pop, `#!` reset.
- `#n` newline, `#l<spacing>` line spacing (per-line or permanent).
- `#b<width>` bold / stroke width.

**Color notations** — the parser accepts **both**, so authors use whichever fits:

- `#RRGGBB` — industry-standard hex (the primary authoring form; artists know it).
  `#000000` (black) is a **reset to the default color**, not literal black.
- `#cR:G:B#` / `#cR:G:B:A#` — float 0..1 channels, for colors that need alpha or
  fractional channels.

> `#c` and `#b` are reserved prefixes for the float-color / stroke codes, so a hex
> color must not begin with `b` or `c` (e.g. write `#bb0000`, not `#b0000`).

### `TextPlane` (legacy — migrating)
`TextPlane` (`engine/src/gfx/hud.cpp`) is the base/HUD text box. It is being migrated onto
`ImGuiText`. **New code must use `ImGuiText`.** Its separate `#c`-only parser will be removed.

### `PaintText` (dead)
`PaintText` is superseded by `ImGuiText`. Do not use or reference it.

---

## 2. Coordinates — one space, one converter

**Canonical space: normalized `-1..1`**, top-left `(-1,+1)`, bottom-right `(+1,-1)`, **Y-inverted**
(screen Y is top-down; base Y is bottom-up). This is what `GUIRect.getNormalXYWH()` returns and what
the engine consumes.

### Engine: `Coordinates::` (`engine/src/vsdraw/guidefs.h`)

Convert canonical normalized → display pixels:

```cpp
float px = Coordinates::normToPixelX(nx);   // [-1,1] -> [0, DisplaySize.x]
float py = Coordinates::normToPixelY(ny);   // [-1,1] (top=+1) -> [0, DisplaySize.y]
float pw = Coordinates::normToPixelW(nw);   // width [0,2] -> pixels
float ph = Coordinates::normToPixelH(nh);   // height [0,2] -> pixels
float ps = Coordinates::normToPixelFontSize(ns);  // font size [0,2] -> pixels
```

Convert display pixels → canonical normalized `-1..1` (inverse, for laying out a *pixel* size
against a normalized rect — e.g. the picker uses this for cell height):

```cpp
float nh = Coordinates::pixelToNormH(pix);  // pixels -> [0,2] height (2.0 == DisplaySize.y)
```

> **Rule:** never mix pixel and normalized units in one rect/layout math. If you start from a
> pixel quantity (like a `font_point`-relative `Font::size()`), convert it with `pixelToNormH`
> before using it against a normalized rect — this is what broke the base-computer picker cells
> (fixed in `932827d08`).

### Assets: `GUIRect` (`modules/GUI.py`)

Author a rect; it converts into the canonical space.

```python
Rect = GUI.GUIRect(x, y, wid, hei, mode="pixel", ref=None)
Rect = GUI.GUIRect(0, 0, 1, 1, "normalized")               # whole screen
Rect = GUI.GUIRect(408, 8, 300, 50, "pixel", (1024, 768))  # px in a 1024x768 basis
```

**Coordinate modes** (targeting reduction from 5 → 2):

- `"pixel"` — integer pixels relative to a reference resolution `ref=(W,H)` (authoring convenience).
- `"normalized"` — top-left (0,0)→bottom-right(1,1), converted to canonical.

The redundant `normalized_biased`, `normalized_biased_scaled`, and `normalized_scaled` modes are
being removed — **don't use them**.

### The ONE origin

`GUIRect` currently has three interpretations (the divergence):

- `getHotRect()` — bottom-left origin (Base hotspots).
- `getSpriteRect()` — center origin (Base sprites).
- `getTextRect()` — top-left origin (Base textboxes).

**Target:** ONE interpretation (canonical top-left `(-1,+1)`). Where an element genuinely needs a
different origin (e.g. sprite center-point), expose it as an explicit `origin` flag on a single
method — not three divergent helpers.

---

## 3. Font size & line spacing

- **Glyph size from `font_point`** (`configuration().graphics.font_point_flt`). Author a
  *relative scale* on `font_point` (default 1.0); the engine renders it as a pixel glyph height
  (`font_point * scale`). Do **not** hardcode a per-control font size, and do **not** do
  resolution-relative math — the engine's standard font metrics handle resolution.
- **Line spacing from the RENDERED glyph height.** `ImGuiText` measures each line with ImGui's
  rendered metrics (`CalcTextSizeA` / `dimensions.y`), never `font.size()` or a literal — so a
  future config font-height change propagates automatically.
- The base computer sets font from `controls.json`'s per-control `font`, which is now a *relative
  scale* (`font_point * scale`), not an override.
- The engine's `Font` class is a thin `{size, strokeWeight}` carrier feeding `ImGuiText`. It does
  **not** implement its own glyph metrics/renderer; the glyphs come from ImGui's font atlas.
  (`Font::verticalScaling()` and the old metrics subsystem were removed as dead code.)

### Special text scaling (per-text-box `textScale`)

For UI where one text box must render *larger* than another without a different config font size
(e.g. the base-room word-by-word dialogue vs. the surrounding menu text), use the per-box
`ImGuiText::setTextScale()` (default 1.0). The render path measures AND draws with the same scaled
size, so pen advance and line height stay consistent. This is how the base-room streaming dialogue
(`othtext`, the fixer/campaign text typed word-by-word via `BaseTalk`) renders at 1.4x while
`curtext` and the rest of the base text stay at normal size.

> Note: the base-room streaming text's background is drawn at each word's own left edge (never
> offset behind the previous word), so consecutive word backgrounds do not overlap and clip glyphs.

---

## 4. Theming / controls (data-driven)

The base computer reads `controls.json` (via `controls_factory`), applying a style vocabulary. This
is the reusable, data-driven theme layer:

`type`, `text`, `label`, `command`, `texture`, `rect`, `justification`, `font`, `textMargins`,
`multiline`, `color`, `textColor`, `downColor`, `downTextColor`, `highlightColor`,
`highlightTextColor`, `borderColor`, `endBorderColor`, `buttonColor`, `outlineColor`,
`selectionColor`, `shadowWidth`, `parent`, `scroller`, `cycleTime`, `id`.

Control types: `button`, `staticDisplay`, `picker`, `scroller`, `groupControl`, `staticImageDisplay`,
`textInputDisplay`.

If bases are unified onto the `Control`/`ImGuiText` world, they inherit this theming for free — the
"one themed layer" goal.

---

## Anti-patterns (don't do these)

- Don't create a new text-box class — use `ImGuiText`.
- Don't hand-parse `#` color/line codes in your screen — use `ImGuiText`'s parser.
- Don't use `TextPlane` for new work.
- Don't hardcode a font size / line spacing per screen or per control — derive from `font_point` +
  rendered metrics.
- Don't add a new `GUIRect` mode or a new coordinate-origin helper.
- Don't mix `pixel`-basis and `normalized`-basis ad hoc — author in one, via `GUIRect`.
- In engine C++, don't use a *pixel* quantity (e.g. `Font::size()`, now a `font_point`-relative
  pixel height) directly against a *normalized* rect — convert it first with
  `Coordinates::pixelToNormH`. (The base-computer picker cells broke this exact way; see `932827d08`.)
- Prefer fixing rendering behavior **at the call site** (e.g. choose the right `automatte`/background
  flag when you draw). If a genuinely new capability is needed in the shared drawing library, add it
  as an **opt-in** flag, defaulting to the current behavior, so other screens are unaffected.

## See also

- Engine: `engine/src/vsdraw/imguitext.{h,cpp}`, `engine/src/vsdraw/guidefs.h`.
- Assets: `modules/GUI.py`, `controls.json`.
