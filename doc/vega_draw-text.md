# vega_draw text utilities

Developer reference for the `libraries/vega_draw` text utilities: the text
primitive, the text box, and the picker.

`vega_draw` is the unified 2D drawing/text library. It is split into a
**renderer-agnostic core** (markup parser, coordinate model, text layout, text
box, picker) and a thin **ImGui adapter** (measurement + drawing). Only the
adapter depends on ImGui; the core is pure and unit-tested headlessly.

* Include headers as `#include "vega_draw/<name>.h"`.
* Everything lives in `namespace vega_draw`.
* Sources: `libraries/vega_draw/`. CMake target: `vegastrike_vega_draw`.

---

## Coordinate, size & font model

There is a single authoring and placement space: a **1000×1000 grid**, top-left
origin `(0,0)`, y increasing downward. Position, region, margins, indent, line
advance and font size are all authored on this grid.

The grid is **never a render resolution**. It is converted to pixels only at the
render boundary, against a `Viewport` (`units.h`):

```cpp
struct Viewport { float width_px; float height_px; };

float GridToPixelX(float grid_x, const Viewport&);  // x maps across the width
float GridToPixelY(float grid_y, const Viewport&);  // y maps down the height
float GridToPixelW(float grid_w, const Viewport&);
float GridToPixelH(float grid_h, const Viewport&);
```

**Font size is a fraction of the space height, authored per-1000** (2% → `20`,
7% → `70`). It is converted to a **whole pixel** height so the glyph atlas is
used at its native size (baked scale 1.0) and text stays crisp:

```cpp
float FontGridToPixel(float font_grid, const Viewport&);  // rounds to a whole pixel
```

---

## Markup

`markup.h` is a self-rolled **HTML subset** (there is no full HTML renderer).
`ParseMarkup` turns a string into styled lines:

```cpp
namespace vega_draw {

struct Color  { std::uint8_t r, g, b, a; bool set = false; };
struct Style  { bool bold = false; Color color; };
struct Run    { std::string text; Style style; };
struct Line   { std::vector<Run> runs; bool manual_break = false; };
using  TextLines = std::vector<Line>;

TextLines ParseMarkup(const std::string& source);
}
```

Grammar:

| Markup | Meaning |
|---|---|
| `<color=RRGGBB>` … `</color>` | colour span (`RRGGBB`, or `RRGGBBAA` for alpha) |
| `<b>` … `</b>` | bold span |
| `<br>` | hard line break |
| `&lt;` `&gt;` `&amp;` | escaped literal `<`, `>`, `&` |

Behaviour:

* Spans nest and are **block-scoped with explicit close tags**. A colour applies
  until its matching `</color>` (or the end of the text). An explicit close is why
  the old line-scoped colour behaviour is not needed.
* Anything that is **not** a recognised tag is literal text — a `<` that does not
  begin a known tag renders literally, so `Ship <foo> 4` is safe. Literal `<`,
  `>`, `&` should still be written as entities.
* `Line::manual_break` is true when the line ended on a `<br>`, so a wrapping box
  knows not to reflow a hand-authored break.
* `Run::style.color.set == false` means "no explicit colour" — the box applies its
  own default.

---

## Layout (`text_layout.h`)

Turns parsed lines into positioned runs, in pixels. Measurement is injected so
the core is renderer-agnostic:

```cpp
struct TextMetrics { float width; float height; };

class TextMeasurer {
public:
    virtual ~TextMeasurer() = default;
    // Measure("").height is the line height.
    virtual TextMetrics Measure(const std::string& text, float font_px) const = 0;
};

enum class Justification { Left, Center, Right };

struct TextStyle {
    float font_px = 16.0f;
    bool  wrap = false;            // opt-in; never reflows a manual <br>
    float wrap_width_px = 0.0f;
    float region_width_px = 0.0f;  // used for justification; 0 -> no shift
    float line_spacing = 0.0f;     // extra spacing between lines (fraction of line height)
    Justification justification = Justification::Left;
};

struct LaidOutRun  { std::string text; Style style; float x; float width; };
struct LaidOutLine { std::vector<LaidOutRun> runs; float width; float height; float y; };
struct TextLayout  { std::vector<LaidOutLine> lines; float width; float height; };

TextLayout LayoutText(const TextLines& parsed, const TextStyle&, const TextMeasurer&);
```

An unwrapped line preserves the parser's runs 1:1. A wrapped line is split at
spaces into word/space runs; trailing spaces are trimmed and a wrapped line never
starts with a space. Line advance = line height × `(1 + line_spacing)`.

---

## Text box (`text_box.h`)

A rectangular region that lays out text. Per-box behaviours are orthogonal: word
wrap (line breaking), overflow clip/scroll (vertical), and autofit (size solving).

```cpp
enum class Overflow { Clip, Scroll };   // Clip is the default

struct TextBox {
    // region and margins, on the 1000-grid
    float region_x, region_y, region_w, region_h;
    float margin_x, margin_y;

    float font_grid = 20.0f;        // per-1000 font size
    bool  multiline = true;
    bool  wrap = false;             // wraps at the inner region width
    float line_spacing = 0.0f;
    Justification justification = Justification::Left;

    Overflow overflow = Overflow::Clip;
    int   scroll_start_line = 0;    // first line drawn when overflow == Scroll

    bool  autofit = false;          // solve the largest font that fits the region height
    float autofit_min_grid = 5.0f;
    float autofit_max_grid = 120.0f;
};

struct BoxRect { float x, y, width, height; };
struct TextBoxLayout { TextLayout layout; float font_px; };

BoxRect TextBoxInnerRectPx(const TextBox&, const Viewport&);

TextBoxLayout LayoutTextBox(const TextBox&, const std::string& markup,
                            const Viewport&, const TextMeasurer&);

int TextBoxVisibleLineCount(const TextBox&, const TextLayout&,
                            const Viewport&, int start_line);
```

* `TextBoxInnerRectPx` returns the region inset by the margins, in pixels.
* `LayoutTextBox` applies autofit (if on), lays the text out, and reports the
  pixel font size it used (needed to draw). A non-multiline box shows only the
  first line.
* `TextBoxVisibleLineCount` counts how many lines from `start_line` fully fit in
  the inner height — the value an owning scroller needs.

---

## Picker (`picker.h`)

A scrollable, selectable list of rows over the text path. Rows may be indented by
level (grouped/hierarchical lists) and may grow to fit wrapped text.

> **Hierarchy:** the caller supplies an **already-flattened** row list (expanding
> or collapsing children is the caller's tree walk), with `level` carrying the
> indent. This matches how the base computer builds its display list.

```cpp
struct PickerRow {
    std::string text;    // markup
    int   level = 0;     // indent level (0 = none)
    Color text_color;    // optional explicit row colour
};

struct PickerStyle {
    float region_x, region_y, region_w, region_h;   // 1000-grid
    float margin_x, margin_y;
    float font_grid = 20.0f;
    float indent_grid = 20.0f;      // indent per level
    float row_padding_grid = 4.0f;  // added to each row (top and bottom)
    float line_spacing = 0.0f;
    bool  wrap_rows = false;        // rows grow to fit wrapped text
};

struct PickerRowLayout {
    int   index;          // index into the supplied rows
    float y;              // content-space top (before scrolling)
    float height;
    float indent_px;
    float padding_px;
    Color text_color;
    TextLayout text;
};

struct PickerLayout {
    std::vector<PickerRowLayout> visible_rows;
    float viewport_x, viewport_y, viewport_width, viewport_height;
    float content_height;
    float scroll_px;      // clamped scroll actually applied
    float max_scroll_px;
};

PickerLayout LayoutPicker(const PickerStyle&, const std::vector<PickerRow>&,
                          const Viewport&, const TextMeasurer&, float scroll_px = 0.0f);
```

`LayoutPicker` measures and stacks every row, clamps the requested scroll to the
content, and returns only the rows intersecting the viewport.

---

## ImGui adapter (`imgui_backend.h`)

The only ImGui-dependent part. All functions require an **active ImGui context
with a loaded font** (call them inside a frame).

```cpp
class ImGuiTextMeasurer : public TextMeasurer { /* uses ImGui::GetFont() */ };

ImU32 ToImU32(const Color&, ImU32 fallback);   // fallback when colour.set == false

// Draw a layout at a pixel origin; first_line skips lines (scrolling).
void DrawTextLayout(ImDrawList*, const ImVec2& origin, const TextLayout&,
                    float font_px, ImU32 default_color,
                    const ImVec4* clip_rect = nullptr, int first_line = 0);

// Text primitive: parse markup, draw at a 1000-grid position with a per-1000 font
// size. Returns the drawn height in pixels. <br> makes multiple lines; no wrap.
float DrawText(ImDrawList*, const Viewport&, float grid_x, float grid_y,
               float font_grid, const std::string& markup, ImU32 default_color,
               const ImVec4* clip_rect = nullptr);

// Text box: lay out and draw, clipped to the inner region and starting at the
// box's scroll line. Draws no background (the caller owns any panel background).
void DrawTextBox(ImDrawList*, const TextBox&, const std::string& markup,
                 const Viewport&, ImU32 default_color);

// Picker colours (zero-alpha backgrounds are not painted; selection beats highlight).
struct PickerColors {
    ImU32 text, background, selection_background, selection_text,
          highlight_background, highlight_text;
};

void DrawPicker(ImDrawList*, const PickerStyle&, const std::vector<PickerRow>&,
                const Viewport&, const PickerColors&,
                int selected_index, int highlighted_index, float scroll_px = 0.0f);
```

### Example

```cpp
#include "vega_draw/imgui_backend.h"

void DrawHudMessage(ImDrawList* dl, float screen_w, float screen_h) {
    const vega_draw::Viewport viewport{screen_w, screen_h};

    // A simple coloured, wrapped line.
    vega_draw::TextBox box;
    box.region_x = 20.0f; box.region_y = 800.0f;
    box.region_w = 600.0f; box.region_h = 160.0f;
    box.font_grid = 20.0f;          // 2% of height
    box.multiline = true;
    box.wrap = true;
    box.overflow = vega_draw::Overflow::Clip;

    vega_draw::DrawTextBox(dl, box,
                           "Message from <color=55ffff>Control</color>: "
                           "docking clearance granted.<br>Welcome back.",
                           viewport, IM_COL32(255, 255, 255, 255));
}
```

---

## Unit tests

The renderer-agnostic core is unit-tested with **GoogleTest**, so most behaviour
is verified without a running window or game.

* Tests live in `libraries/vega_draw/tests/`, one file per module:
  * `markup_tests.cpp` — grammar, colour values, nesting, `<br>`, entities,
    unrecognised/invalid tags.
  * `units_tests.cpp` — grid→pixel mapping and whole-pixel font rounding.
  * `text_layout_tests.cpp` — run positioning, hard breaks, wrap, justification.
  * `text_box_tests.cpp` — margins, wrap width, visible-line count, line spacing,
    justification, autofit, single-line behaviour.
  * `picker_tests.cpp` — row stacking, indent, padding, viewport/scroll culling,
    scroll clamping, variable-height wrap.
* Measurement is replaced by a deterministic `FakeMeasurer` (defined in each test
  file), so expectations are independent of ImGui/font metrics.
* The test executable is `Vega_Strike_vega_draw_tests`, wired into the existing
  `USE_GTEST` build and registered with CTest.

Build and run (requires the CMake option `USE_GTEST=ON`, the default here):

```sh
# from a configured build directory
ninja Vega_Strike_vega_draw_tests
./Vega_Strike_vega_draw_tests            # or:
ctest -R "ParseMarkup|TextLayout|TextBox|Picker|Units" --output-on-failure
```

The ImGui adapter is **not** unit-tested — it needs a live ImGui context. It is
verified in-engine (see the migration proving ground) rather than headlessly.

### Adding a test

Add a test file under `tests/`, list it in `libraries/vega_draw/CMakeLists.txt`
in the `ADD_EXECUTABLE` for `${PROJECT_NAME}_vega_draw_tests`, and use a local
deterministic `TextMeasurer` (as the existing tests do).

---

## Current limitations

* **Single-line ellipsis** on overflow is not implemented; a non-multiline box
  currently clips.
* **Bold** is carried in the run style but not yet rendered differently (there is
  a single weight in the atlas; faking bold with an offset shadow is deliberately
  avoided pending the `<b>` face/weight decision).
* The adapter requires an active ImGui font; there is no headless drawing path.
