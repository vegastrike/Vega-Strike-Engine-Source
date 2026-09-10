# vega_draw — developer API reference

`libraries/vega_draw` is the unified 2D drawing / text / widget library. It is
split into a **renderer-agnostic core** (coordinates, markup, layout, text box,
picker, scroller, widgets) and a thin **ImGui adapter** that measures and draws.
Only `imgui_backend.*` depends on ImGui; the core is pure and unit-tested
headlessly.

* Include headers as `#include "vega_draw/<name>.h"`.
* Everything lives in `namespace vega_draw`.
* CMake target: `vegastrike_vega_draw` (links `vegastrike_imgui`).

---

## 1. Coordinate & units model (`units.h`)

One authoring and placement space: a **1000×1000 grid**, top-left origin, y
increasing downward. Position, region, margins, indent, line advance and font
size are authored on this grid. The grid is **never a render resolution** — it is
converted to pixels only at the render boundary against a `Viewport`.

```cpp
struct Viewport { float width_px, height_px; };

float GridToPixelX(float grid_x, const Viewport&);   // x across the width
float GridToPixelY(float grid_y, const Viewport&);   // y down the height
float GridToPixelW(float grid_w, const Viewport&);
float GridToPixelH(float grid_h, const Viewport&);
float FontGridToPixel(float font_grid, const Viewport&); // rounded to a whole pixel
```

**Font size is a fraction of the space height, authored per-1000** (2% → `20`).
It rounds to a whole pixel so the glyph atlas is used at native size (crisp).

### Named font heights (`FontHeight`)

Because the grid uses integer per-1000 sizes, the standard tiers are a real enum:

```cpp
enum class FontHeight : int { ExtraSmall = 40, Small = 45, Medium = 50, Large = 55, ExtraLarge = 60 };
float ToFontGrid(FontHeight); // -> grid value
```

### Geometry (`geometry.h`)

```cpp
struct Point { float x, y; };
struct Size  { float width, height; };
struct Rect  { Point origin; Size size; };
// Rect: left/right/top/bottom (y-down: top() == origin.y), center(), inside() [half-open],
//       inset(Size) (mutating), copyAndInset(Size), translated(dx,dy)
Rect  GridToPixelRect(const Rect&, const Viewport&);
Point GridToPixelPoint(const Point&, const Viewport&);
```

---

## 2. Markup (`markup.h`)

A self-rolled **HTML subset** (no full HTML renderer). `ParseMarkup` produces
styled lines.

```cpp
struct Color { std::uint8_t r, g, b, a; bool set = false; };
struct Style { float weight = kWeightNormal; Color color; };   // weight is continuous
struct Run   { std::string text; Style style; };
struct Line  { std::vector<Run> runs; bool manual_break = false; };
using  TextLines = std::vector<Line>;
TextLines ParseMarkup(const std::string& source);

// Weight presets: kWeightLight=0.6, kWeightNormal=1.0, kWeightBold=1.5; kBoldThreshold=1.25
bool IsBoldWeight(float weight);
```

| Markup | Meaning |
|---|---|
| `<color=RRGGBB>` … `</color>` | colour span (8-digit `RRGGBBAA` also accepted) |
| `<b>` … `</b>` | bold span (sets weight to `kWeightBold`) |
| `<br>` | hard line break |
| `&lt;` `&gt;` `&amp;` | escaped literal `<` `>` `&` |

Spans nest and are **block-scoped with explicit close tags**. Anything that is
not a recognised tag is literal text (a lone `<` renders literally).

---

## 3. Text layout (`text_layout.h`)

Turns parsed lines into positioned runs (pixels). Measurement is injected.

```cpp
struct TextMetrics { float width, height; };
class TextMeasurer { virtual TextMetrics Measure(const std::string&, float font_px) const = 0; };

enum class Justification { Left, Center, Right };
struct TextStyle {
    float font_px = 16.0f;
    bool  wrap = false;            // opt-in; never reflows a manual <br>
    float wrap_width_px = 0.0f;
    float region_width_px = 0.0f;  // used for justification
    float line_spacing = 0.0f;     // extra, as a fraction of line height
    Justification justification = Justification::Left;
};
struct LaidOutRun  { std::string text; Style style; float x, width; };
struct LaidOutLine { std::vector<LaidOutRun> runs; float width, height, y; };
struct TextLayout  { std::vector<LaidOutLine> lines; float width, height; };

TextLayout LayoutText(const TextLines&, const TextStyle&, const TextMeasurer&);

// Truncate to max_width, appending an ellipsis (UTF-8 aware).
void TruncateLineWithEllipsis(LaidOutLine&, float max_width, float font_px,
                              const TextMeasurer&, const std::string& ellipsis = "...");
```

---

## 4. Text box (`text_box.h`)

A region that lays out text. Orthogonal behaviours: word wrap, overflow
clip/scroll, autofit.

```cpp
enum class Overflow { Clip, Scroll };

struct TextBox {
    float region_x, region_y, region_w, region_h;  // 1000-grid
    float margin_x, margin_y;                      // 1000-grid
    float font_grid = 20.0f;
    bool  multiline = true;
    bool  wrap = false;
    float line_spacing = 0.0f;
    Justification justification = Justification::Left;
    Overflow overflow = Overflow::Clip;
    int   scroll_start_line = 0;
    bool  autofit = false;
    float autofit_min_grid = 5.0f, autofit_max_grid = 120.0f;
};

struct BoxRect { float x, y, width, height; };
struct TextBoxLayout { TextLayout layout; float font_px; };

BoxRect TextBoxInnerRectPx(const TextBox&, const Viewport&);
TextBoxLayout LayoutTextBox(const TextBox&, const std::string& markup,
                            const Viewport&, const TextMeasurer&);
int TextBoxVisibleLineCount(const TextBox&, const TextLayout&, const Viewport&, int start_line);

// Caches the layout, recomputing only when inputs change; version() is a change signal.
class TextBoxLayoutCache {
    const TextBoxLayout& Update(const TextBox&, const std::string&, const Viewport&, const TextMeasurer&);
    int  version() const;
    bool hasLayout() const;
};
```

---

## 5. Picker (`picker.h`)

A scrollable, selectable list of rows. Rows carry markup, an indent level
(grouped/hierarchical lists — the caller supplies an already-flattened list),
an optional colour and a caller id; rows grow to fit wrapped text.

```cpp
struct PickerRow   { std::string text; int level = 0; Color text_color; std::uint64_t id = 0; };
struct PickerColors{ Color text, background, selection_background, selection_text,
                           highlight_background, highlight_text; };
struct PickerStyle {
    float region_x, region_y, region_w, region_h, margin_x, margin_y;
    float font_grid = 20.0f, indent_grid = 20.0f, row_padding_grid = 4.0f,
          extra_row_height_grid = 0.0f, line_spacing = 0.0f;
    bool  wrap_rows = false;
};
struct PickerRowLayout { int index; std::uint64_t id; float y, height, indent_px, padding_px;
                         Color text_color; TextLayout text; };
struct PickerLayout { std::vector<PickerRowLayout> visible_rows; std::vector<float> row_tops;
                      float viewport_x, viewport_y, viewport_width, viewport_height;
                      float content_height, scroll_px, max_scroll_px; };

PickerLayout LayoutPicker(const PickerStyle&, const std::vector<PickerRow>&,
                          const Viewport&, const TextMeasurer&, float scroll_px = 0.0f);
int   PickerRowAt(const PickerLayout&, float y_px);                 // row under a drawn y
float PickerScrollToRow(const PickerStyle&, const std::vector<PickerRow>&,
                        const Viewport&, const TextMeasurer&, int row_index);
```

---

## 6. Scroller (`scroller.h`)

A scroll-bar model + thumb geometry (grid). Extents are in the content's unit
(e.g. pixels); the track is on the grid.

```cpp
enum class ScrollOrientation { Vertical, Horizontal };
struct ScrollerModel { float content_extent, viewport_extent, position; };
struct ScrollerStyle { Rect track; ScrollOrientation orientation = Vertical; float min_thumb_grid = 20.0f; };

float ScrollerMaxPosition(const ScrollerModel&);
float ScrollerClampPosition(const ScrollerModel&, float position);
Rect  ScrollerThumbRect(const ScrollerStyle&, const ScrollerModel&);
float ScrollerPositionForThumbCentre(const ScrollerStyle&, const ScrollerModel&, float track_point_grid);
```

---

## 7. Widget framework (`widget.h` + widget headers)

A 1000-grid UI tree with grid input events. Layout/hit-test/event routing is
renderer-agnostic; drawing is implemented by the adapter against `ImDrawList`.

```cpp
enum class InputType { MouseDown, MouseUp, MouseMove, MouseDrag, KeyDown, KeyUp };
struct InputEvent { InputType type; Point loc; unsigned int code, mask; };
using EventCommand = std::string;

class Widget {
    Rect rect() const; void setRect(const Rect&);
    const std::string& id() const; void setId(const std::string&);
    bool hidden() const; void setHidden(bool = true);
    bool hitTest(const Point&) const;                    // half-open, respects hidden
    Widget* parent() const;

    virtual void draw(ImDrawList*, const Viewport&) = 0;
    virtual bool onMouseDown/onMouseUp/onMouseMove/onMouseDrag(const InputEvent&);
    virtual bool onKeyDown/onKeyUp(const InputEvent&);
    virtual bool onCommand(const EventCommand&, Widget*);
};

class Container : public Widget {           // owns children, routes events
    void addChild(std::unique_ptr<Widget>);
    Widget* findById(const std::string&) const;   // recursive
    Widget* mouseCapture() const;                  // capture during a drag
};
```

**Widgets**

| Class | Header | Notes |
|---|---|---|
| `Panel` | `panel.h` | `Container` + background/border (`PanelStyle`) |
| `PushButton` | `button.h` | `ButtonStyle` (face/down/highlight/border/text/shadows, `font_grid`, `weight`); centred mark-up label; fires a command to a target, else bubbles to the parent |
| `TextDisplay` | `text_display.h` | shows mark-up via the text box; its rect drives the box region |
| `ListPicker` | `list_picker.h` | selection list wrapping the picker; `rowAt(grid, viewport, measurer)`, `selectedIndex`, `scroll` |
| `ScrollBar` | `scroll_bar.h` | wraps the scroller; rect is the track; press/drag positions the thumb |
| `Slider` | `slider.h` | value in `[min,max]`; `thumbRect()` |
| `TextInput` | `text_input.h` | single-line input; key codes as characters (8 = backspace), `disallowed`, `max_length` |
| `EventManager` | `event_manager.h` | `dispatch(InputEvent)` into a root widget, with a modal taking precedence |

---

## 8. ImGui adapter (`imgui_backend.h`)

The only ImGui-dependent part. Most functions require an active ImGui context;
drawing uses `ImDrawList`.

```cpp
Viewport DisplayViewport();                       // current ImGui display size
class ImGuiTextMeasurer : public TextMeasurer;    // uses ImGui::GetFont()
ImU32 ToImU32(const Color&, ImU32 fallback);      // fallback when colour.set == false

void DrawTextLayout(ImDrawList*, const ImVec2& origin, const TextLayout&, float font_px,
                    ImU32 default_color, const ImVec4* clip_rect = nullptr, int first_line = 0);
float DrawText(ImDrawList*, const Viewport&, float grid_x, float grid_y, float font_grid,
               const std::string& markup, ImU32 default_color, const ImVec4* clip = nullptr);
void DrawTextBox(ImDrawList*, const TextBox&, const std::string& markup, const Viewport&, ImU32 default_color);
void DrawPicker(ImDrawList*, const PickerStyle&, const std::vector<PickerRow>&, const Viewport&,
                const PickerColors&, int selected_index, int highlighted_index, float scroll_px = 0.0f);
void DrawScroller(ImDrawList*, const ScrollerStyle&, const ScrollerModel&, const Viewport&,
                  const ScrollerColors&);

void DrawRectFill(ImDrawList*, const Rect& grid_rect, const Viewport&, ImU32 color);
void DrawRectOutline(ImDrawList*, const Rect& grid_rect, const Viewport&, ImU32 color, float thickness_px = 1.0f);
void DrawUpLeftShadow(ImDrawList*, const Rect&, const Viewport&, ImU32, float thickness_px = 1.0f);
void DrawLowRightShadow(ImDrawList*, const Rect&, const Viewport&, ImU32, float thickness_px = 1.0f);
```

---

## 9. Composing widgets

```cpp
#include "vega_draw/panel.h"
#include "vega_draw/button.h"
#include "vega_draw/text_display.h"
#include "vega_draw/imgui_backend.h"

vega_draw::Panel root;
root.setRect({100, 100, 800, 600});
root.style().background = {0, 0, 0, 200, true};

auto title = std::make_unique<vega_draw::TextDisplay>();
title->setRect({140, 140, 720, 80});
title->box().font_grid = vega_draw::ToFontGrid(vega_draw::FontHeight::Large);
title->box().multiline = false;
title->setText("Base <color=55ffff>Concourse</color>");
root.addChild(std::move(title));

auto ok = std::make_unique<vega_draw::PushButton>();
ok->setRect({400, 600, 200, 60});
ok->setLabel("OK");
ok->setCommand("ok");
root.addChild(std::move(ok));

// Per frame (inside an ImGui frame):
const vega_draw::Viewport vp = vega_draw::DisplayViewport();
root.draw(ImGui::GetWindowDrawList(), vp);
```

---

## 10. Unit tests

The renderer-agnostic core is unit-tested with GoogleTest, so most behaviour is
verified without a window. Tests live in `libraries/vega_draw/tests/`, one file
per module (markup, units, geometry, text layout, text box, picker, scroller,
widget/container, button, panel/event manager, text display/scroll bar, list
picker, slider, text input). Measurement is replaced by a deterministic fake
`TextMeasurer`. Build/run:

```sh
ninja Vega_Strike_vega_draw_tests           # requires USE_GTEST=ON
./Vega_Strike_vega_draw_tests               # or:
ctest -R "ParseMarkup|TextLayout|TextBox|Picker|Scroller|Widget|PushButton|Slider|TextInput|..." --output-on-failure
```

The ImGui adapter and widget `draw()` are **not** unit-tested (they need a live
ImGui context) — they are verified in-engine.

---

## 11. Current limitations

* **Single-line ellipsis** is implemented for text boxes (UTF-8 aware); the picker
  clips rather than ellipsising rows.
* **Stroke weight** is carried in the run style but not yet rendered differently
  (single-weight atlas); faking bold with an offset shadow is deliberately avoided.
* **Textures/sprites** are not yet part of the library — panels are solid-colour
  (a sprite primitive is a later inventory iteration).
* The adapter requires an active ImGui font; there is no headless drawing path.
