# Campaign Launch Test

## Overview

This document records the findings from automated testing of the Vega Strike campaign launch flow: start the game, launch a new campaign, launch into space, then exit. The goal was to verify the game exits cleanly (return code 0) after a full campaign launch cycle.

## Environment

- **Display:** `DISPLAY=:1` (X11 via Xwayland on Wayland)
- **Game binary:** `./bin/vegastrike-engine`
- **Assets:** `/home/stephengt/Projects/Assets-Production/`
- **Debug level:** `--debug=2`

## Step-by-Step Flow

### Step 1: Start the game

```bash
./bin/vegastrike-engine --target /home/stephengt/Projects/Assets-Production/ --debug=2
```

Wait for the "Vega Strike" window to appear (detected via `xdotool search --name "Vega Strike"`).

### Step 2: Click "Campaign" button on Main Menu

The Campaign (New Game) button is defined in `bases/main_menu.py`:

```python
sprite_loc = GUI.GUIRect(50, 220, 177, 43, "pixel", (1280, 1024))
```

- **Position:** (50, 220) in design coordinates (1280×1024)
- **Size:** 177 × 43 pixels
- **Text offset:** ~90% into button width (same as Quit button)

Screen position calculation:
```python
sx = WW / 1280
sy = WH / 1024
screen_x = WX + (50 + 177 * 0.9) * sx
screen_y = WY + (220 + 43 / 2) * sy
```

### Step 3: Click "Continue" on Intro Monologue screen

After clicking Campaign, the Intro Monologue screen appears. It uses design resolution (1024×768).

The Continue button is defined in `bases/main_menu.py`:

```python
sprite_loc = GUI.GUIRect(8, 697, 420, 47, "pixel", (1024, 768))
```

- **Position:** (8, 697) in design coordinates (1024×768)
- **Size:** 420 × 47 pixels
- **Text offset:** Centered (50% into button width) — unlike the main menu buttons

Screen position calculation:
```python
sx = WW / 1024
sy = WH / 768
screen_x = WX + (8 + 420 * 0.5) * sx
screen_y = WY + (697 + 47 / 2) * sy
```

### Step 4: Click "Launch" hotspot in the Hangar view

The starting location is **Atlantis** in the **Cephid_17** system, which uses the **ocean** base type (`bases/ocean_lib.py`).

The Launch hotspot is defined in `bases/ocean_lib.py`:

```python
Base.LaunchPython(room0, 'my_launch_id', 'bases/launch_music.py',
                  -0.609375, -0.502604, 0.712891, 0.65625, 'Launch')
```

The coordinates are **normalized** (-1 to +1), where:
- X: -1 (left edge) to +1 (right edge)
- Y: -1 (bottom edge) to +1 (top edge)

The engine converts screen pixels to normalized coords in `engine/src/cmd/base_interface.cpp`:
```cpp
nx = 2.0f * (mx - vx) / vw - 1.0f;
ny = 1.0f - 2.0f * (my - vy) / vh;
```

Reversing this to get screen position from normalized coords:
```python
# Hotspot center in normalized coords
center_x = -0.609375 + 0.712891 / 2  # = -0.2529295
center_y = -0.502604 + 0.65625 / 2   # = -0.174479

# Convert to screen (assuming base fills the viewport)
screen_x = WX + (center_x + 1) / 2 * WW  # = WX + 0.3735 * WW
screen_y = WY + (1 - center_y) / 2 * WH   # = WY + 0.5872 * WH
```

### Step 5: Wait for launch, then press Esc then Q

After clicking Launch, the starship launches into space. Wait a few seconds for the launch animation, then:

1. Press `Escape` to open the pause/menu
2. Press `Q` to quit

### Step 6: Verify exit code 0

The game should exit cleanly with return code 0.

## Coordinate System Summary

| Screen | Resolution | Coordinate System | Example |
|--------|------------|-------------------|---------|
| Main Menu | 1280×1024 | `GUI.GUIRect` pixels | Campaign: (50, 220) |
| Intro Monologue | 1024×768 | `GUI.GUIRect` pixels | Continue: (8, 697) |
| Hangar/Base | Full screen | Normalized -1 to +1 | Launch: center (-0.25, -0.17) |

## Input Method Notes

| Method | Result | Notes |
|--------|--------|-------|
| `xdotool mousemove --window` + `click` | **Worked** for Campaign and Continue | Window-relative coordinates are reliable for menu buttons |
| PyAutoGUI | **Worked** for Launch hotspot and keyboard | Used for Launch click and Esc/Q keystrokes |
| `wtype` | Failed | Compositor doesn't support virtual keyboard protocol |

Both `xdotool` (with `--window` flag for window-relative coords) and PyAutoGUI can click buttons successfully. PyAutoGUI was used for the Launch hotspot and for keyboard input (Esc, Q). The key is ensuring the window is focused before sending input.

## Key Findings

1. **Main menu button text is at 90% offset** — clicking the geometric center misses the text.
2. **Intro Monologue button text is centered** — unlike the main menu buttons.
3. **Base hotspots use normalized coordinates** — the engine converts screen pixels to -1..1 range.
4. **The starting base is Atlantis (ocean type)** — uses `bases/ocean_lib.py` for hotspot definitions.
5. **Esc then Q quits from space** — the standard quit sequence from the pause menu.
6. **Window movement breaks absolute coordinates** — always re-query window geometry with `xdotool getwindowgeometry` before clicking, as the user may have moved the window.
7. **Launch hotspot verification** — the blue dot on the Launch hotspot dims/brightens with mouse proximity; the custom cursor turns orange when hovering a valid hotspot, and the hotspot name appears in orange near the bottom-left of the screen.
8. **Game exit code** — the game process exits after Esc+Q, but the wrapper bash script may return non-zero. Check the actual vegastrike-engine process rather than the wrapper.
9. **OCR limitations** — Tesseract OCR reliably detects large text ("Landing Center", "Campaign") but may miss small or dim text like "Launch" hotspot labels. Visual pixel analysis (brightest pixel search, blue dot detection) is more reliable for hotspot localization.

## Notes

- Window detection uses `xdotool search --name "Vega Strike"`
- The game may take 15-30 seconds to load the campaign and reach the hangar
- "fatal local force" log messages are normal simulation warnings
- The base viewport may be letterboxed depending on `config.json` settings
- **Always re-query window geometry** before clicking — the user may have moved the window between steps
- The Launch hotspot can be found by moving the mouse to ~25px left of center, then verifying the orange cursor and "Launch" label appear
- The game binary is `vegastrike-engine`; older scripts may reference `vegastrike`
- Missing video textures (preintro.ogv, intro.ogv) produce harmless warnings in the log
