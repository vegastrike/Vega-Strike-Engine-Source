# Campaign Launch Test

## Overview

This document records the findings from automated testing of the Vega Strike campaign launch flow: start the game, launch a new campaign, launch into space, then exit. The goal is to verify the game exits cleanly (return code 0) after a full campaign launch cycle.

The automation script is `campaign_launch_kde_wayland.py` in this directory, which relies on shared helpers in `vega_automation.py`.

## Environment

- **Display:** Native Wayland (KDE Plasma) — the game runs as a native Wayland window.
- **Game binary:** `./bin/vegastrike-engine`
- **Assets:** auto-detected from `~/Projects/Assets-Production/`, or specified via `--target`
- **Debug level:** `--debug=2` (default)

## Dependencies

- `dotool` — for mouse movement and clicking (`sudo dnf install dotool`; may require user in `input` group)
- `kdotool` — for window management on Wayland (`sudo dnf install kdotool`)
- `spectacle` — KDE screenshot tool for Wayland screenshots
- Tesseract OCR (`sudo dnf install tesseract`) — for finding button text
- `pytesseract` (`pip install pytesseract`) — Python Tesseract wrapper
- `Pillow` (`pip install Pillow`) — image processing
- `vegastrike-engine` built binary
- Assets-Production data directory

## Step-by-Step Flow

### Step 1: Start the game

The script launches `vegastrike-engine` with `--target` pointing to the Assets-Production directory and `--debug=2`. After a 20-second startup delay (5s initial + 15s load), the game window is detected.

### Step 2: Find and activate the game window

Window detection uses `kdotool` to search for a window with class `vegastrike-engine`. The window is then activated (brought to focus) via `kdotool activate`, and its geometry (position and size) is queried.

### Step 3: Click "Campaign" button on Main Menu

A screenshot is taken using `spectacle` (active window only). The HiDPI scale factor is read from the KDE configuration (`kwinrc`) via `get_display_scale()`, and the native display resolution is obtained from `wayland-info` or `xdotool` via `get_display_bounds()`. Together these produce the logical display size through `get_display_bounds_logical()`.

The script first verifies the Main Menu is displayed by running OCR and checking for "Campaign" text using Levenshtein fuzzy matching (threshold ≤2). Then it attempts to find the Campaign button via OCR in the upper-left portion of the letterboxed area (the Main Menu is letterboxed to 1.25:1, derived from the design resolution 1280×1024). The Y-coordinate of the found text is adjusted by the `MAIN_MENU_BUTTON_Y_ADJUSTMENT` factor (0.95) to account for the button text not being vertically centered within the button sprite. If OCR fails, it falls back to calculating the position from the design coordinates defined in `bases/main_menu.py`:

```python
# Design resolution: 1280x1024
GUI.GUIRect(50, 220, 177, 43, "pixel", (1280, 1024))
```

The `calculate_main_menu_button_position()` helper scales these design coordinates to the current window size.

### Step 4: Click "Continue" on Intro Monologue screen

After clicking Campaign, the Intro Monologue screen appears. The script waits 5 seconds, refreshes the window geometry, and takes another screenshot.

Verification is done by checking for the title text. Tesseract reads "---Intro Monologue---" as separate fragments, so the script searches for any of `{"---Intro", "--Intro", "~-Intro"}` using the `verify_screen()` function (which runs OCR once and checks each target via Levenshtein distance). The "Continue" button text is also verified as a secondary confirmation.

The Continue button is found via OCR in the bottom 10% of the letterboxed area, left half. The Intro Monologue screen is letterboxed to 1.33:1 (4:3), derived from the design resolution 1024×768. The Continue button does NOT use the Main Menu Y-coordinate adjustment — only Main Menu buttons (Campaign, Quit) apply the `MAIN_MENU_BUTTON_Y_ADJUSTMENT` factor. If OCR fails, the fallback uses design coordinates from `bases/main_menu.py`:

```python
# Design resolution: 1024x768
GUI.GUIRect(8, 697, 420, 47, "pixel", (1024, 768))
```

After clicking Continue, the script polls every 2 seconds (up to 30s) to confirm the Intro Monologue screen has disappeared. The Continue button uses the standard Y-coordinate calculation (no Main Menu adjustment), as the `main_menu` flag is not set for non-main-menu screens.

### Step 5: Wait for Hangar view, click "Launch" hotspot

The starting location is **Atlantis** in the **Cephid_17** system, which uses the **ocean** base type (`bases/ocean_lib.py`).

The script waits for the Hangar view to load by repeatedly taking screenshots and checking for any of `{"Landing Center", "Launch", "Main Concourse"}` via OCR. This is necessary because the loading splash screen contains many blue pixels that would confuse the hotspot detector.

Once the Hangar is loaded, the Launch hotspot is found using a three-tier strategy:

1. **Pixel-based search** (`find_hotspot_via_pixel_search`) — hotspots are marked by blue-purple starburst shapes with a bright white/gray center. The detector enforces the visual structure: a bright center (≥3 pixels, all RGB channels ≥ 180, within 40 of each other) surrounded by a blue/purple ring (≥5 pixels, blue dominant). A hint position from the per-base-type geometry table (for "ocean" type, the Launch hotspot's expected window position) narrows the search region to ±25% around the hint within the letterboxed area. This avoids scanning the splash-screen blue bars. The caption text "Launch" is verified via OCR in the bottom caption area using Levenshtein fuzzy matching (threshold ≤2), so misreads like "Paunch" are handled automatically.

2. **OCR fallback** — if pixel search fails, the script tries `find_text_in_region_via_ocr()` for "Launch" across the full screen. Levenshtein fuzzy matching handles any OCR misreads.

3. **Error** — if neither method finds the hotspot, the script aborts.

The Launch hotspot definition in `bases/ocean_lib.py` (for reference):
```python
Base.LaunchPython(room0, 'my_launch_id', 'bases/launch_music.py',
                  -0.609375, -0.502604, 0.712891, 0.65625, 'Launch')
```

### Step 6: Wait for launch, press Esc then Q

After clicking Launch, the script waits 5 seconds for the launch animation, takes a screenshot to confirm the ship has launched into space, then presses Escape followed by Q to quit.

After pressing Q, the game may show a confirmation dialog. The script checks if the game is still running, takes a screenshot, and looks for any of `{"Yes", "Ok", "Confirm"}` buttons via OCR in the center region using Levenshtein fuzzy matching. If found, the confirmation button is clicked. (Note: the current version of the game does not display a confirmation dialog when quitting, but this check is left in place in case it is added in a future release.)

### Step 7: Verify exit code 0

The script waits for the game process to exit (default timeout: 120 seconds) and prints the return code.

## Coordinate Systems

| Screen | Resolution | Coordinate System | Button Finding |
|--------|------------|-------------------|----------------|
| Main Menu | 1280×1024 | `GUI.GUIRect` pixels | OCR primary, calculated fallback |
| Intro Monologue | 1024×768 | `GUI.GUIRect` pixels | OCR primary, calculated fallback |
| Hangar/Base | Full screen | Normalized -1 to +1 | Pixel search primary, OCR fallback |

### HiDPI scaling

The HiDPI scale factor is read from the KDE configuration (`kwinrc`) via `get_display_scale()`. The native display resolution is obtained from `wayland-info` or `xdotool` via `get_display_bounds()`. Together these produce the logical display size through `get_display_bounds_logical()`. This scale factor is used by `click_at_position()` to ensure clicks land correctly on HiDPI displays.

### Base hotspot normalized coordinates

Base hotspots use normalized coordinates (-1 to +1). The engine converts screen pixels to normalized coords in `engine/src/cmd/base_interface.cpp`:
```cpp
nx = 2.0f * (mx - vx) / vw - 1.0f;
ny = 1.0f - 2.0f * (my - vy) / vh;
```

Reversing this for the Launch hotspot on the ocean base:
```python
center_x = -0.609375 + 0.712891 / 2  # ≈ -0.253
center_y = -0.502604 + 0.65625 / 2   # ≈ -0.174
screen_x = WX + (center_x + 1) / 2 * WW  # ≈ WX + 0.374 * WW
screen_y = WY + (1 - center_y) / 2 * WH   # ≈ WY + 0.587 * WH
```

## Input Method

All input is handled by `dotool` (mouse movement and clicking) and `kdotool` (window management). The `click_at_position()` function in `vega_automation.py` translates logical coordinates to the physical screen and invokes `dotool`. Keyboard input (Esc, Q) is sent via `dotool` as well.

## Key Findings

1. **OCR is the primary button-finding method** — Tesseract reliably detects button labels ("Campaign", "Continue", "Quit") in the letterboxed menu area. Calculated design coordinates serve as a fallback.

2. **Levenshtein fuzzy matching for OCR** — instead of exact string comparison, all OCR-based text matching uses Levenshtein (edit) distance with a threshold of ≤2 (`OCR_LEVENSHTEIN_THRESHOLD`). This means OCR misreads like "Launch" → "Paunch" or "---Intro" → "--Intro" are handled automatically without needing explicit fallback strings. The `levenshtein_distance` package (available in the project virtualenv) provides the `Levenshtein(target, actual).distance()` call.

3. **Multi-string OCR search** — functions like `find_button_via_ocr()` and `verify_screen()` accept a `set[str]` of target texts. Each target is checked independently via Levenshtein distance, so semantically different words (e.g., "Yes" vs "Ok" vs "Confirm" in confirmation dialogs) and context-dependent phrases (e.g., "Landing Center" vs "Launch" vs "Main Concourse" in the hangar) are all matched in one OCR pass.

4. **Multi-word phrase matching via sliding window** — `verify_screen()` handles multi-word targets (e.g., "Main Concourse") by splitting both the target and OCR output into words, then sliding a window over consecutive OCR words. Each word pair is compared via Levenshtein distance, so "main" matches "main" and "concourse" matches "concourse" as a consecutive pair. Single-word targets are checked against each OCR word individually.

5. **Main Menu button Y-coordinate adjustment** — the Main Menu buttons (Campaign, Quit, etc.) need a vertical offset correction because the button text is not vertically centered within the button sprite. The constant `MAIN_MENU_BUTTON_Y_ADJUSTMENT = 0.95` scales the Y position of detected text to land on the clickable area. This adjustment is applied only to Main Menu buttons (via the `main_menu=True` parameter) and does not affect buttons on other screens (Intro Monologue "Continue", Hangar hotspots, confirmation dialogs). Both the OCR path (`find_button_via_ocr()`) and the calculated fallback path (`calculate_button_position()`) respect this flag.

6. **Pixel-based hotspot detection with structure validation** — hotspot markers are blue-purple starburst shapes with a bright white/gray center. The detector enforces the visual structure observed in reference screenshots:
   - **Bright center**: at least 3 pixels where all RGB channels are ≥ 180 and within 40 of each other (white/gray, not colored). Center diameter is between 4–19 pixels (±25% tolerance around the 5–15 pixel reference size).
   - **Blue/purple ring**: at least 5 pixels where blue dominates over green, minimum blue value of 60, and bright neutral pixels (the center) are excluded.
   - **Spatial validation**: center pixels are near the cluster center, and ring pixels surround the center on average.
   Clusters that don't meet these criteria are rejected, reducing false positives from the loading splash screen or other blue elements.

7. **Tesseract misreads "Launch" as "Paunch"** — handled automatically by Levenshtein fuzzy matching (finding "Launch" matches "Paunch" at distance 1). No explicit dual-string fallback is needed.

8. **Intro Monologue title varies** — Tesseract may parse "---Intro Monologue---" as "---Intro", "--Intro", or "~-Intro". The `verify_screen()` function checks all variants in one OCR pass using a set of target strings.

9. **Hangar verification with green caption support** — the bottom-left caption may say "Landing Center" (green, no hotspot hovered), "Launch" (orange, mouse over Launch), or "Main Concourse" (orange, mouse over Main Concourse). The caption OCR function (`_verify_caption_via_ocr()`) recognizes both orange AND green text when tracing the black-background bounding box for OCR cropping. Green is detected as dominant green channel (G ≥ 120, R ≤ 100, B ≤ 100).

10. **Confirmation dialog after quit** — the current version of the game does not display a confirmation dialog when quitting, but the script includes a check for any of `{"Yes", "Ok", "Confirm"}` buttons in case this behavior is added in a future release.

11. **Window geometry must be refreshed** — the window may be moved or resized between steps. `refresh_window_geometry()` is called before each screenshot.

12. **Loading splash screen interferes** — the splash screen's blue pixels confuse the hotspot detector, so the Hangar must be verified via OCR before searching for hotspots.

13. **Letterboxed viewport** — the Main Menu is letterboxed to 1.25:1 (1280/1024), and the Intro Monologue screen is letterboxed to 1.33:1 (1024/768). OCR search regions are defined as percentages of the letterboxed area.

14. **Game exit code** — the game process exits after Esc+Q, but a wrapper bash script may return non-zero. Check the actual `vegastrike-engine` process.

## Usage

```bash
python3 doc/automated-play-testing/campaign_launch_kde_wayland.py [options]
```

Options:
- `--target DIR` — path to Assets-Production (auto-detected if omitted)
- `--binary PATH` — path to vegastrike-engine (default: `./bin/vegastrike-engine`)
- `--debug LEVEL` — debug level 0-3 (default: 2)
- `--display NUM` — X11 display number (default: 0)
- `--timeout SECS` — max seconds to wait for game exit (default: 120)
- `--verbose` — print detailed progress
- `--kdotool-path PATH` — path to folder containing kdotool, if not system-installed
- `--x-adjust N` — pixel adjustment for button X coordinates (default: 0)
- `--y-adjust N` — pixel adjustment for button Y coordinates (default: 0)

## Notes

- The game may take 15-30 seconds to load the campaign and reach the hangar.
- "fatal local force" log messages are normal simulation warnings.
- The base viewport may be letterboxed depending on `config.json` settings.
- Missing video textures (preintro.ogv, intro.ogv) produce harmless warnings in the log.
- The game binary is `vegastrike-engine`; older scripts may reference `vegastrike`.
