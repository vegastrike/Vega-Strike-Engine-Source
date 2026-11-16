# Quit Button Click Test

## Overview

This document records the findings from automated testing of the Vega Strike main menu Quit button. The goal is to verify the game exits cleanly (return code 0) when Quit is clicked.

The automation script is `click_quit_kde_wayland.py` in this directory, which relies on shared helpers in `vega_automation.py`.

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

## How It Works

The script uses OCR (Tesseract) as the primary method to locate the Quit button, rather than relying on hardcoded coordinates. This approach is more robust across different window sizes and HiDPI displays.

### Step-by-step flow

1. **Start the game** — `vegastrike-engine` is launched with `--target` pointing to the Assets-Production directory. After a 20-second startup delay (5s initial + 15s load), the game window is detected.

2. **Find and activate the window** — `kdotool` searches for a window with class `vegastrike-engine`. The window is activated (brought to focus) and its geometry (position and size) is queried.

3. **Take a screenshot** — `spectacle` captures the active window. The HiDPI scale factor is read from the KDE configuration (`kwinrc`) via `get_display_scale()`, and the native display resolution is obtained from `wayland-info` or `xdotool` via `get_display_bounds()`. Together these produce the logical display size through `get_display_bounds_logical()`.

4. **Find the Quit button via OCR** — `find_quit_button_via_ocr()` searches the screenshot for "Quit" text. The Quit button is defined in `bases/main_menu.py`:

```python
# Design resolution: 1280x1024
GUI.GUIRect(50, 580, 177, 43, "pixel", (1280, 1024))
```

The OCR function searches in the lower portion of the letterboxed area where the Quit button appears. The function returns the center screen coordinates of the found text.

5. **Click the Quit button** — `click_at_position()` translates logical coordinates to the physical screen and invokes `dotool` to perform the click.

6. **Handle confirmation dialog** — after clicking Quit, the game may show a confirmation dialog. The script waits 2 seconds, checks if the game is still running, and if so takes another screenshot to look for "Yes" or "Ok" buttons via OCR using `find_confirmation_button_via_ocr()`. If found, the confirmation button is clicked. (Note: the current version of the game does not display a confirmation dialog when quitting, but this check is left in place in case it is added in a future release.)

7. **Verify exit code** — the script waits for the game process to exit (default timeout: 120 seconds) and prints the return code.

## The Coordinate System (Background)

Vega Strike uses a design-resolution coordinate system that scales to the actual window size. Menu button positions are defined in `bases/main_menu.py` using `GUI.GUIRect` with a design resolution of `(1280, 1024)`.

The Quit button sprite is 177px wide, but the "Quit" text image is **not centered** within the button — it's positioned at approximately **90% into the button width**. This is why clicking at the geometric center of the button fails to register.

| Offset | Result |
|--------|--------|
| 50% (center) | Misses the text |
| 70% | Edge of text |
| **90%** | **Hits the text** |

OCR-based finding avoids this problem entirely by locating the actual text rather than computing the click position from design coordinates.

## Input Method

All input is handled by `dotool` (mouse movement and clicking) and `kdotool` (window management). The `click_at_position()` function in `vega_automation.py` translates logical coordinates to the physical screen and invokes `dotool`.

Previous approaches that were abandoned:
- `xdotool` — works on X11 but not on native Wayland
- PyAutoGUI — works at the X11 input level but requires extra setup (blocking `mouseinfo` import, disabling failsafe, setting `DISPLAY=:1`)
- `wtype` (Wayland virtual keyboard) — the compositor doesn't support the virtual keyboard protocol

## Key Findings

1. **OCR is the primary button-finding method** — Tesseract reliably detects the "Quit" button label in the letterboxed menu area. This is more robust than hardcoded coordinates across different window sizes and HiDPI displays.

2. **Confirmation dialog after quit** — the current version of the game does not display a confirmation dialog when quitting, but the script includes a check for "Yes" or "Ok" buttons in case this behavior is added in a future release.

3. **HiDPI scaling** — the HiDPI scale factor is read from the KDE configuration (`kwinrc`) via `get_display_scale()`. The native display resolution is obtained from `wayland-info` or `xdotool` via `get_display_bounds()`. Together these produce the logical display size through `get_display_bounds_logical()`. This scale factor ensures clicks land correctly on HiDPI displays.

4. **Window geometry must be queried** — the window position and size are obtained via `kdotool` before each operation, as the user may have moved or resized the window.

## Usage

```bash
python3 doc/automated-play-testing/click_quit_kde_wayland.py [options]
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

- The game window title is `"Vega Strike"`.
- The game may take 8-12 seconds to fully load and display the main menu.
- "fatal local force" log messages are normal simulation warnings, not errors.
- The custom mouse pointer is rendered by the game, not the OS.
- The game binary is `vegastrike-engine`; older scripts may reference `vegastrike`.
