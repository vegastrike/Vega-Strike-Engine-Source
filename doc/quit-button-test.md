# Quit Button Click Test

## Overview

This document records the findings from automated testing of the Vega Strike main menu Quit button. The goal was to verify the game exits cleanly (return code 0) when Quit is clicked.

## Environment

- **Display:** `DISPLAY=:1` (X11 via Xwayland on Wayland)
- **Game binary:** `./bin/vegastrike-engine`
- **Assets:** `/home/stephengt/Projects/Assets-Production/`
- **Debug level:** `--debug=2`

## The Coordinate System Challenge

Vega Strike uses a design-resolution coordinate system that scales to the actual window size. Menu button positions are defined in `bases/main_menu.py` using `GUI.GUIRect` with a design resolution of `(1280, 1024)`.

### Quit Button Definition

From `bases/main_menu.py`:

```python
GUI.GUIRect(50, 580, 177, 43, "pixel", (1280, 1024))
```

This means:
- **Position:** (50, 580) in design coordinates
- **Size:** 177 × 43 pixels in design coordinates
- **Design resolution:** 1280 × 1024

### The Key Finding: Text Is Not Centered

The Quit button sprite is 177px wide, but the "Quit" text image is **not centered** within the button. It's positioned at approximately **90% into the button width**.

| Offset | Screen X | Result |
|--------|----------|--------|
| 50% (center) | ~478 | Misses the text |
| 70% | ~543 | Edge of text |
| **90%** | **~591** | **Hits the text** |

This is why clicking at the geometric center of the button fails to register.

### Scaling to Screen Coordinates

Given a window at screen position `(WX, WY)` with size `WW × WH`:

```python
sx = WW / 1280  # X scale factor
sy = WH / 1024  # Y scale factor

# Quit button text center (at 90% offset)
screen_x = WX + (50 + 177 * 0.9) * sx
screen_y = WY + (580 + 43 / 2) * sy
```

## Why Other Methods Failed

| Method | Result | Reason |
|--------|--------|--------|
| `xdotool click` | Failed | xdotool cannot reliably interact with SDL windows under Xwayland |
| `xdotool key` (Escape/Return/Down) | Failed | Keyboard navigation doesn't trigger Quit from main menu |
| `wtype` (Wayland virtual keyboard) | Failed | Compositor doesn't support virtual keyboard protocol |
| **PyAutoGUI** | **Success** | Works at the X11 input level, bypassing window focus issues |

## PyAutoGUI Setup

PyAutoGUI works but requires some setup to avoid warnings:

1. **Block the `mouseinfo` import** to suppress a tkinter warning
2. **Disable failsafe** (`pyautogui.FAILSAFE = False`) to prevent corner-click interruptions
3. **Set `DISPLAY=:1`** environment variable

## Verification

The test was verified by:
1. Starting the game process
2. Waiting for the "Vega Strike" window to appear (using xdotool to find it)
3. Calculating the Quit button screen position
4. Moving the mouse and clicking with PyAutoGUI
5. Confirming the game exits with return code 0

## Notes

- The game window title is `"Vega Strike"`
- Window detection uses `xdotool search --name "Vega Strike"`
- The game may take 8-12 seconds to fully load and display the main menu
- "fatal local force" log messages are normal simulation warnings, not errors
- The custom mouse pointer is rendered by the game, not the OS
