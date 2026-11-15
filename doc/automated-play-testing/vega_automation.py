#!/usr/bin/env python3
"""
Shared utilities for automated Vega Strike play testing on KDE/Wayland.

Provides helpers for window management (kdotool), screenshots (spectacle),
OCR-based button finding (Tesseract/pytesseract), and input simulation (dotool).
"""
import re
import subprocess
import time
import os
from collections import namedtuple
from re import Pattern, Match
from typing import SupportsInt, Any

import pytesseract
from PIL import Image
from PIL.ImageFile import ImageFile

POSITION_RE: Pattern[str] = re.compile(r'^ {2}Position: (\d+)(?:\.\d+)?,(\d+)(?:\.\d+)?$')
GEOMETRY_RE: Pattern[str] = re.compile(r'^ {2}Geometry: (\d+)(?:\.\d+)?x(\d+)(?:\.\d+)?$')
MODE_RE: Pattern[str] = re.compile(r'^[ \t]+mode:\r?\n[ \t]+width: (\d+) px, height: (\d+) px, refresh: \d+(?:\.\d+)? Hz,\r?\n[ \t]+flags: current$')


def run_cmd(cmd: str, shell: bool = True) -> tuple[str, int]:
    """Run a shell command and return (stdout, returncode).

    :rtype: tuple[str, int]
    :param cmd: The command to run
    :param shell: Whether to execute the specified command through the shell
    :return: A tuple containing the stdout and return code
    """
    env = os.environ.copy()
    result = subprocess.run(cmd, shell=shell, capture_output=True, text=True, env=env)
    return result.stdout.strip(), result.returncode


def take_screenshot(path: str, active_window_only: bool, verbose: bool = False) -> bool:
    """
    Take a screenshot using spectacle (Wayland-compatible).

    :return: A bool indicating whether the operation was successful
    :rtype: bool
    :param verbose: Print debug info
    :param active_window_only: Take the screenshot of the active window only? (True) Or of the entire screen? (False)
    :param path: Output file path for the screenshot
    """
    if active_window_only:
        command_string = f'spectacle --activewindow --background --nonotify --output {path} 2>/dev/null'
    else:
        command_string = f'spectacle --fullscreen --background (background, no GUI) --nonotify --output {path} 2>/dev/null'
    out, rc = run_cmd(command_string)
    if rc != 0:
        if verbose:
            print(f"spectacle failed: {out}")
    return rc == 0


def find_window_by_class(klass: str, verbose: bool = False) -> str:
    """
    Find the window ID of the window with the specified class.

    :rtype: str
    :param klass: The window class to look for
    :param verbose: Print debug info
    :return: The window ID. If more than one is found, return the first one
    """
    out, rc = run_cmd(f'kdotool search --class {klass}')
    if rc != 0:
        if verbose:
            print(f"kdotool failed: {out}")
        return ""

    output_lines: list[str] = out.splitlines()
    if len(output_lines) < 1:
        if verbose:
            print("kdotool failed: no output")
    first_output_line: str = output_lines[0]
    return first_output_line.strip()


def find_vega_strike_window(verbose: bool = False) -> str:
    """
    Find the window ID of the Vega Strike window
    :rtype: str
    :param verbose:  Print debug info
    :return: The window ID. If more than one is found, return the first one
    """
    return find_window_by_class('vegastrike-engine', verbose=verbose)


def activate_window(window_id: str, verbose: bool = False) -> bool:
    """
    Activate the window with the specified ID.

    :rtype: bool
    :param window_id: The window ID to activate
    :param verbose: Print debug info
    :return: A bool indicating whether the operation was successful
    """
    out, rc = run_cmd(f'kdotool windowactivate {window_id}')
    if rc != 0:
        if verbose:
            print(f"kdotool failed: {out}")
    return rc == 0


def get_window_geometry(window_id: str, verbose: bool = False) -> tuple[int, int, int, int] | None:
    """
    Get the window geometry of the window with the specified ID.

    :rtype: tuple[int, int, int, int] | None
    :param window_id: The window ID whose geometry to get
    :param verbose: Print debug info
    :return: The window geometry(x, y, width, height) or None on failure
    """
    out, rc = run_cmd(f'kdotool getwindowgeometry {window_id}')
    if rc != 0:
        if verbose:
            print(f"kdotool failed: {out}")
        return None
    output_lines = out.splitlines()
    if len(output_lines) < 3:
        if verbose:
            print("kdotool failed: too few lines")
        return None
    if output_lines[0] != f'Window {window_id}':
        if verbose:
            print(f"kdotool failed: First line of output, '{output_lines[0]}', did not match expected value 'Window {window_id}'")
        return None
    position_match: Match[str] | None = re.fullmatch(POSITION_RE, output_lines[1])
    if position_match is None:
        if verbose:
            print(f"kdotool failed: Second line of output, '{output_lines[1]}', did not match expected pattern, '{POSITION_RE}'")
        return None
    window_x, window_y = position_match.groups()
    geometry_match: Match[str] | None = re.fullmatch(GEOMETRY_RE, output_lines[2])
    if geometry_match is None:
        if verbose:
            print(f"kdotool failed: Third line of output, '{output_lines[2]}', did not match expected pattern, '{GEOMETRY_RE}'")
        return None
    window_width, window_height = geometry_match.groups()
    if verbose:
        print(f"kdotool successfully parsed window '{window_id}'. Window X: {window_x}, Window Y: {window_y}, Window Width: {window_width}, Window Height: {window_height}")
    return int(window_x), int(window_y), int(window_width), int(window_height)


def ocr_text_via_command_lina(screenshot_path: str, verbose: bool = False) -> str:
    """Run Tesseract OCR on a screenshot and return the detected text.

    Returns the detected text as a string, or empty string on failure.

    :rtype: str
    :param screenshot_path: The path to the screenshot file to read
    :param verbose: Print debug info
    :return: The detected text as a string, or empty string on failure
    """
    try:
        cmd = f'tesseract {screenshot_path} stdout 2>/dev/null'
        text, rc = run_cmd(cmd)
        if rc == 0 and text:
            if verbose:
                print(f"OCR detected text: {text[:200]}")
            return text
        else:
            if verbose:
                print(f"Warning: OCR failed or returned empty text")
            return ""
    except Exception as e:
        if verbose:
            print(f"Warning: OCR error: {e}")
        return ""


def find_button_via_ocr(screenshot_path: str, button_texts: list[str], window_x: int = 0, window_y: int = 0, window_width: int = 0, window_height: int = 0, crop_x_starting_percent: float | int = 0, crop_y_starting_percent: float | int = 0, crop_x_ending_percent: float | int = 100, crop_y_ending_percent: float | int = 100, verbose: bool = False) -> tuple[int, int] | None:
    """
    Find a button with any of the specified text strings in the specified area of the screenshot and return its absolute screen coordinates.
    Searches for each text string in the order provided; returns the first match found.

    On Wayland with HiDPI scaling, the screenshot may be at a different resolution than
    the logical window size.  This function calculates the scale factor and uses it to
    convert screenshot-space OCR positions to logical screen coordinates.

    :rtype: tuple[int, int] | None
    :param screenshot_path: The path to the screenshot file to read
    :param button_texts: List of possible button captions to search for (first match wins)
    :param window_x: The window X coordinate on screen (logical pixels)
    :param window_y: The window Y coordinate on screen (logical pixels)
    :param window_width: The logical window width from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param window_height: The logical window height from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param crop_x_starting_percent: The starting horizontal percentage of the screenshot to crop to
    :param crop_y_starting_percent: The starting vertical percentage of the screenshot to crop to
    :param crop_x_ending_percent: The ending horizontal percentage of the screenshot to crop to
    :param crop_y_ending_percent: The ending vertical percentage of the screenshot to crop to
    :param verbose: Print debug info
    :return: The absolute coordinates of the center of the button text (logical screen pixels), or None on failure
    """
    img: ImageFile = Image.open(screenshot_path)
    img_w, img_h = img.size
    if verbose:
        print(f"Screenshot dimensions: {img_w}x{img_h}")
        if window_width > 0 and window_height > 0:
            print(f"Window geometry (logical): {window_width}x{window_height}")

    # Calculate HiDPI scaling factor if window geometry is provided.
    # On Wayland, spectacle captures at native pixels while kdotool reports logical pixels.
    # The scale factor converts screenshot-space positions to logical screen coordinates.
    scale_x: float = 1.0
    scale_y: float = 1.0
    if window_width > 0 and window_height > 0:
        scale_x, scale_y = calculate_scale_factor(img_w, img_h, window_width, window_height, verbose)

    # Crop to specified percentages of the actual screenshot image
    crop_x_start: int = int(img_w * crop_x_starting_percent / 100)
    crop_y_start: int = int(img_h * crop_y_starting_percent / 100)
    crop_x_ending: int = int(img_w * crop_x_ending_percent / 100)
    crop_y_ending: int = int(img_h * crop_y_ending_percent / 100)

    if verbose:
        print(f"crop_x_start: {crop_x_start}")
        print(f"crop_y_start: {crop_y_start}")
        print(f"crop_x_ending: {crop_x_ending}")
        print(f"crop_y_ending: {crop_y_ending}")
    cropped: Image.Image = img.crop((crop_x_start, crop_y_start, crop_x_ending, crop_y_ending))

    # Convert to grayscale for more reliable OCR detection.
    # Tesseract via pytesseract struggles with RGB images from screenshots,
    # often returning empty strings. Grayscale conversion fixes this.
    cropped = cropped.convert('L')

    # Use Tesseract to find word-level bounding boxes
    data: bytes | str | dict[Any, Any] | Any = pytesseract.image_to_data(cropped, output_type=pytesseract.Output.DICT)

    # Debug: show all detected words
    if verbose:
        all_words: list[Any] = []
        for i, text in enumerate(data['text']):
            t = text.strip()
            if t:
                all_words.append(f"{t}@({data['left'][i]},{data['top'][i]})")
        print(f"OCR words in cropped region: {all_words}")

    # Build a set of lowercased target texts for quick lookup
    target_texts: set[str] = {t.lower() for t in button_texts}

    # Look for any of the specified button texts in the detected text
    for i, text in enumerate(data['text']):
        text_stripped: str = text.strip()
        if text_stripped.lower() in target_texts:
            word_x: int = int(data['left'][i])
            word_y: int = int(data['top'][i])
            word_w: int = int(data['width'][i])
            word_h: int = int(data['height'][i])

            # Map crop-relative OCR position to screenshot-space pixel coordinates
            button_center_x_in_screenshot: int = crop_x_start + word_x + word_w // 2
            button_center_y_in_screenshot: int = crop_y_start + word_y + word_h // 2

            # Convert screenshot-space pixels to logical screen coordinates.
            # Divide by the HiDPI scale factor to get logical window-relative coords,
            # then add the window's screen position.
            screen_x: int = int(window_x + button_center_x_in_screenshot / scale_x)
            screen_y: int = int(window_y + button_center_y_in_screenshot / scale_y)

            if verbose:
                print(f"Found '{text_stripped}' at: ({word_x}, {word_y}), size: {word_w}x{word_h}")
                print(f"Position in screenshot: ({button_center_x_in_screenshot}, {button_center_y_in_screenshot})")
                print(f"Position on screen (logical): ({screen_x}, {screen_y})")

            return screen_x, screen_y

    if verbose:
        print(f"Warning: None of the button texts found in screenshot: {button_texts}")
    return None


def find_button_via_ocr_single(screenshot_path: str, button_text: str, window_x: int = 0, window_y: int = 0, window_width: int = 0, window_height: int = 0, crop_x_starting_percent: float | int = 0, crop_y_starting_percent: float | int = 0, crop_x_ending_percent: float | int = 100, crop_y_ending_percent: float | int = 100, verbose: bool = False) -> tuple[int, int] | None:
    """
    Find the button with the specified text in the specified area of the screenshot and return its absolute screen coordinates.
    Thin wrapper around the list-based ``find_button_via_ocr``.

    :rtype: tuple[int, int] | None
    :param screenshot_path: The path to the screenshot file to read
    :param button_text: The button text to find
    :param window_x: The window X coordinate on screen (logical pixels)
    :param window_y: The window Y coordinate on screen (logical pixels)
    :param window_width: The logical window width from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param window_height: The logical window height from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param crop_x_starting_percent: The starting horizontal percentage of the screenshot to crop to
    :param crop_y_starting_percent: The starting vertical percentage of the screenshot to crop to
    :param crop_x_ending_percent: The ending horizontal percentage of the screenshot to crop to
    :param crop_y_ending_percent: The ending vertical percentage of the screenshot to crop to
    :param verbose: Print debug info
    :return: The absolute coordinates of the center of the button text (logical screen pixels), or None on failure
    """
    return find_button_via_ocr(
        screenshot_path=screenshot_path,
        button_texts=[button_text],
        window_x=window_x,
        window_y=window_y,
        window_width=window_width,
        window_height=window_height,
        crop_x_starting_percent=crop_x_starting_percent,
        crop_y_starting_percent=crop_y_starting_percent,
        crop_x_ending_percent=crop_x_ending_percent,
        crop_y_ending_percent=crop_y_ending_percent,
        verbose=verbose
    )


def find_quit_button_via_ocr(screenshot_path: str, window_x: int = 0, window_y: int = 0,
                              window_width: int = 0, window_height: int = 0,
                              verbose: bool = False) -> tuple[int, int] | None:
    """
    Use OCR on a screenshot of the active window to find the Quit button.

    The main menu buttons are in the lower portion of the game window.
    The Quit button is the lowest button. We crop to the bottom portion
    to avoid matching "Quit" in the game logo at the top.

    :rtype: tuple[int, int] | None
    :param screenshot_path: Path to the screenshot image
    :param window_x: The window's X position on the screen (logical pixels)
    :param window_y: The window's Y position on the screen (logical pixels)
    :param window_width: Logical window width from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param window_height: Logical window height from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param verbose:  Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the Quit button (logical pixels), or None
    """
    # Crop to the bottom 55% (starting at 45% from the top) to focus on the
    # button area and avoid matching "Quit" in the game logo at the top.
    return find_button_via_ocr(
        screenshot_path=screenshot_path,
        button_texts=["Quit"],
        window_x=window_x,
        window_y=window_y,
        window_width=window_width,
        window_height=window_height,
        crop_x_starting_percent=0,
        crop_y_starting_percent=45,
        crop_x_ending_percent=100,
        crop_y_ending_percent=100,
        verbose=verbose
    )


def find_confirmation_button_via_ocr(screenshot_path: str, window_x: int = 0, window_y: int = 0,
                                     window_width: int = 0, window_height: int = 0,
                                     verbose: bool = False) -> tuple[int, int] | None:
    """
    Use OCR on a screenshot to find a confirmation dialog button.

    After clicking Quit, the game may show a confirmation dialog.
    Searches for "Yes", "OK", or similar confirmation text.
    Confirmation dialogs appear near the center of the screen, so we crop
    to the center 60% region to avoid false matches elsewhere.

    :rtype: tuple[int, int] | None
    :param screenshot_path: Path to the screenshot image
    :param window_x: The window's X position on the screen (logical pixels)
    :param window_y: The window's Y position on the screen (logical pixels)
    :param window_width: Logical window width from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param window_height: Logical window height from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param verbose: Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the confirmation button (logical pixels), or None
    """
    # Crop to the center 60% (20% margins on all sides) to focus on the
    # confirmation dialog area and avoid false matches elsewhere.
    return find_button_via_ocr(
        screenshot_path=screenshot_path,
        button_texts=["yes", "ok", "confirm"],
        window_x=window_x,
        window_y=window_y,
        window_width=window_width,
        window_height=window_height,
        crop_x_starting_percent=20,
        crop_y_starting_percent=20,
        crop_x_ending_percent=80,
        crop_y_ending_percent=80,
        verbose=verbose
    )


def get_display_bounds(verbose: bool = False) -> tuple[int, int]:
    """
    Get the total display resolution in native pixels.

    Tools like xdotool and wayland-info report the native pixel resolution,
    not the logical (scaled) resolution.  For example, a 3840x2160 physical
    display reports 3840x2160 even with 1.5x scaling applied.

    :rtype: tuple[int, int]
    :param verbose: Print debug info
    :return: Tuple of (display_width, display_height) in native pixels
    """
    # Try xdotool first (works on X11 fallback)
    out, _ = run_cmd('xdotool getdisplaygeometry 2>/dev/null')
    if out:
        parts = out.split()
        if len(parts) >= 2:
            dw, dh = int(parts[0]), int(parts[1])
            if verbose:
                print(f"Display geometry: {dw}x{dh} (found using xdotool)")
            return dw, dh

    out, _ = run_cmd('wayland-info 2>/dev/null | grep -A 10 "interface: \'wl_output\'" | grep "mode:" -A 3')
    if out:
        mode_match = MODE_RE.fullmatch(out)
        if mode_match:
            dw, dh = int(mode_match.group(1)), int(mode_match.group(2))
            if verbose:
                print(f"Display geometry: {dw}x{dh} (found using wayland-info)")
            return dw, dh
    if verbose:
        print("No display geometry found; falling back to default geometry")
    return 3840, 2160


def calculate_scale_factor(screenshot_w: int, screenshot_h: int,
                           window_w: int, window_h: int,
                           verbose: bool = False) -> tuple[float, float]:
    """
    Calculate the HiDPI scaling factor between a screenshot and the window geometry.

    On Wayland with HiDPI scaling, spectacle captures at a higher resolution than
    the logical window size reported by kdotool.  For example, a 1920x1080 logical
    window may produce a 2973x1796 screenshot on a 1.5x scaled display.

    The scaling factor is computed as screenshot_size / window_size.  A value of 1.0
    means no scaling; 1.5 means the screenshot is 1.5x the logical size.

    :param screenshot_w: Width of the captured screenshot in pixels
    :param screenshot_h: Height of the captured screenshot in pixels
    :param window_w: Logical window width from kdotool
    :param window_h: Logical window height from kdotool
    :param verbose: Print debug info
    :return: Tuple of (scale_x, scale_y)
    """
    scale_x = screenshot_w / window_w if window_w > 0 else 1.0
    scale_y = screenshot_h / window_h if window_h > 0 else 1.0
    if verbose:
        print(f"HiDPI scale factor: {scale_x:.3f}x{scale_y:.3f} "
              f"(screenshot {screenshot_w}x{screenshot_h}, window {window_w}x{window_h})")
    return scale_x, scale_y


def move_mouse_to_position(screen_x: SupportsInt, screen_y: SupportsInt, screen_width: SupportsInt, screen_height: SupportsInt,
                           verbose: bool = False) -> None:
    """
    Move the mouse to the specified screen position using dotool (without clicking).

    dotool uses the Linux uinput device to simulate input events at the
    kernel level, bypassing Wayland's restrictions on cross-process input.
    dotool mouseto uses screen percentages (0.0 to 1.0).

    :param screen_x: Target position X coordinate in screen pixel coordinates
    :param screen_y: Target position Y coordinate in screen pixel coordinates
    :param screen_width: The width of the screen in pixels
    :param screen_height: The height of the screen in pixels
    :param verbose: Print debug info
    """
    screen_x_fraction = screen_x / screen_width
    screen_y_fraction = screen_y / screen_height

    if verbose:
        print(f"Moving mouse to screen percentage ({screen_x_fraction}, {screen_y_fraction})...")

    out, rc = run_cmd(
        f'echo "mouseto {screen_x_fraction} {screen_y_fraction}" | dotool 2>/dev/null')
    if rc != 0 and verbose:
        print(f"dotool mousemove failed: {out}")
    time.sleep(0.3)


def click_at_position(screen_x: SupportsInt, screen_y: SupportsInt, screen_width: SupportsInt, screen_height: SupportsInt,
                      verbose: bool = False) -> None:
    """
    Click at the specified screen position using dotool.

    dotool uses the Linux uinput device to simulate input events at the
    kernel level, bypassing Wayland's restrictions on cross-process input.
    dotool mouseto uses screen pixel coordinates.

    :param screen_x: Target click position X coordinate in screen pixel coordinates
    :param screen_y: Target click position Y coordinate in screen pixel coordinates
    :param screen_width: The width of the screen in pixels
    :param screen_height: The height of the screen in pixels
    :param verbose: Print debug info
    """
    move_mouse_to_position(screen_x, screen_y, screen_width, screen_height, verbose)

    # Click left button
    if verbose:
        print("Clicking...")
    out, rc = run_cmd('echo "click left" | dotool 2>/dev/null')
    if rc != 0 and verbose:
        print(f"dotool click failed: {out}")

    if verbose:
        print("Click sent!")


# === Letterbox calculation ===
# The Main Menu, Intro Monologue, and base/hangar views are letterboxed to a 1.33:1
# aspect ratio within the window.  When the window is wider (e.g. 16:9), black bars
# appear on the left and right.  When the window is taller, black bars appear on
# the top and bottom.  Button design coordinates are relative to the letterboxed
# area, not the full window.
LETTERBOX_ASPECT_RATIO = 4.0 / 3.0  # 1.33:1


def calculate_letterbox_region(wx: int, wy: int, ww: int, wh: int) -> tuple[int, int, int, int]:
    """
    Calculate the letterboxed region within the window.

    The game renders at a 1.33:1 aspect ratio.  When the window aspect ratio
    differs, black bars are added.  This function returns the position and size
    of the actual rendered area within the window, fitting the 4:3 content
    while maintaining the aspect ratio.

    :param wx: Window X position on screen
    :param wy: Window Y position on screen
    :param ww: Window width in pixels
    :param wh: Window height in pixels
    :return: Tuple of (letterbox_x, letterbox_y, letterbox_w, letterbox_h) in screen coords
    """
    window_aspect = ww / wh

    if window_aspect > LETTERBOX_ASPECT_RATIO:
        # Window is wider than 4:3 — letterbox horizontally (black bars left/right).
        # The letterbox fills the window height, and width is derived from the aspect.
        lb_h = wh
        lb_w = int(wh * LETTERBOX_ASPECT_RATIO)
        if lb_w > ww:
            # Defensive: letterbox width exceeds window width, fill width instead.
            lb_w = ww
            lb_h = int(ww / LETTERBOX_ASPECT_RATIO)
            lb_x = wx
            lb_y = wy + (wh - lb_h) // 2
        else:
            lb_x = wx + (ww - lb_w) // 2
            lb_y = wy
    else:
        # Window is taller (or equal) than 4:3 — letterbox vertically (black bars top/bottom).
        # The letterbox fills the window width, and height is derived from the aspect.
        lb_w = ww
        lb_h = int(ww / LETTERBOX_ASPECT_RATIO)
        if lb_h > wh:
            # Defensive: letterbox height exceeds window height, fill height instead.
            lb_h = wh
            lb_w = int(wh * LETTERBOX_ASPECT_RATIO)
            lb_x = wx + (ww - lb_w) // 2
            lb_y = wy
        else:
            lb_x = wx
            lb_y = wy + (wh - lb_h) // 2

    return lb_x, lb_y, lb_w, lb_h


# === Button position calculation helpers ===
# Main menu buttons use design resolution 1280x1024
# Button text is typically at 90% into the button width (not centered).
MAIN_MENU_DESIGN_W = 1280
MAIN_MENU_DESIGN_H = 1024
MAIN_MENU_TEXT_OFFSET_X = 0.9

# Intro monologue buttons use design resolution 1024x768
# Button text is centered (50% offset).
INTRO_DESIGN_W = 1024
INTRO_DESIGN_H = 768
INTRO_TEXT_OFFSET_X = 0.5


def calculate_button_position(wx: int, wy: int, ww: int, wh: int,
                              btn_x: int, btn_y: int, btn_w: int, btn_h: int,
                              text_offset_x: float = MAIN_MENU_TEXT_OFFSET_X,
                              design_w: int = MAIN_MENU_DESIGN_W,
                              design_h: int = MAIN_MENU_DESIGN_H,
                              letterboxed: bool = True,
                              verbose: bool = False) -> tuple[int, int]:
    """
    Calculate the screen position of a button's text based on design-resolution coordinates.

    Converts design-space button coordinates to actual screen pixel coordinates
    by scaling for the letterboxed region size relative to the design resolution,
    then offsetting to the text position within the button.

    :param wx: Window X position on screen
    :param wy: Window Y position on screen
    :param ww: Window width in pixels
    :param wh: Window height in pixels
    :param btn_x: Button X in design coordinates
    :param btn_y: Button Y in design coordinates
    :param btn_w: Button width in design coordinates
    :param btn_h: Button height in design coordinates
    :param text_offset_x: Horizontal offset fraction (0-1) into button width where text sits
    :param design_w: Design resolution width
    :param design_h: Design resolution height
    :param letterboxed: If True, account for the letterboxed region (default True for menus)
    :param verbose: Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the button text
    """
    if letterboxed:
        lb_x, lb_y, lb_w, lb_h = calculate_letterbox_region(wx, wy, ww, wh)
        if verbose:
            print(f"Letterbox region: ({lb_x}, {lb_y}), size: {lb_w}x{lb_h}")
        sx = lb_w / design_w
        sy = lb_h / design_h
    else:
        lb_x = wx
        lb_y = wy
        sx = ww / design_w
        sy = wh / design_h

    text_x = btn_x + btn_w * text_offset_x
    text_y = btn_y + btn_h / 2

    screen_x = int(lb_x + text_x * sx)
    screen_y = int(lb_y + text_y * sy)

    if verbose:
        print(f"Scale: sx={sx:.4f}, sy={sy:.4f}")
        print(f"Design text pos: ({text_x:.1f}, {text_y:.1f})")
        print(f"Screen click pos: ({screen_x}, {screen_y})")

    return screen_x, screen_y


def calculate_main_menu_button_position(wx: int, wy: int, ww: int, wh: int,
                                        btn_x: int, btn_y: int, btn_w: int, btn_h: int,
                                        verbose: bool = False) -> tuple[int, int]:
    """
    Calculate the screen position of a main menu button's text.

    Main menu buttons use design resolution 1280x1024 with text at 90% into button width.
    The main menu is letterboxed to 1.33:1.

    :return: Tuple of (screen_x, screen_y) — center of the button text
    :param wx: Window X position on screen
    :param wy: Window Y position on screen
    :param ww: Window width in pixels
    :param wh: Window height in pixels
    :param btn_x: Button X in design coordinates
    :param btn_y: Button Y in design coordinates
    :param btn_w: Button width in design coordinates
    :param btn_h: Button height in design coordinates
    :param verbose: Print debug info
    """
    return calculate_button_position(
        wx, wy, ww, wh, btn_x, btn_y, btn_w, btn_h,
        text_offset_x=MAIN_MENU_TEXT_OFFSET_X,
        design_w=MAIN_MENU_DESIGN_W,
        design_h=MAIN_MENU_DESIGN_H,
        letterboxed=True,
        verbose=verbose
    )


def calculate_intro_button_position(wx: int, wy: int, ww: int, wh: int,
                                    btn_x: int, btn_y: int, btn_w: int, btn_h: int,
                                    verbose: bool = False) -> tuple[int, int]:
    """
    Calculate the screen position of an intro monologue button's text.

    Intro monologue uses design resolution 1024x768 with centered text (50% offset).
    The intro monologue is letterboxed to 1.33:1, just like the main menu.

    :return: Tuple of (screen_x, screen_y) — center of the button text
    :param wx: Window X position on screen
    :param wy: Window Y position on screen
    :param ww: Window width in pixels
    :param wh: Window height in pixels
    :param btn_x: Button X in design coordinates
    :param btn_y: Button Y in design coordinates
    :param btn_w: Button width in design coordinates
    :param btn_h: Button height in design coordinates
    :param verbose: Print debug info
    """
    return calculate_button_position(
        wx, wy, ww, wh, btn_x, btn_y, btn_w, btn_h,
        text_offset_x=INTRO_TEXT_OFFSET_X,
        design_w=INTRO_DESIGN_W,
        design_h=INTRO_DESIGN_H,
        letterboxed=True,
        verbose=verbose
    )


def refresh_window_geometry(window_id: str, verbose: bool = False) -> tuple[int, int, int, int] | None:
    """
    Re-query the window geometry in case the user moved the window.

    :rtype: tuple[int, int, int, int] | None
    :param window_id: The window ID whose geometry to refresh
    :param verbose: Print debug info
    :return: The window geometry (x, y, width, height) or None on failure
    """
    geom = get_window_geometry(window_id, verbose)
    if verbose and geom:
        print(f"Refreshed window geometry: {geom}")
    return geom


def press_key(key: str, verbose: bool = False) -> None:
    """
    Press a key using dotool.

    :param key: The key name to press (e.g. 'escape', 'q')
    :param verbose: Print debug info
    """
    if verbose:
        print(f"Pressing '{key}'...")
    out, rc = run_cmd(f'echo "key {key}" | dotool 2>/dev/null')
    if rc != 0 and verbose:
        print(f"dotool key press failed: {out}")
    time.sleep(0.5)


def verify_screen(screenshot_path: str, expected_text: str, step_label: str = "", verbose: bool = False) -> bool:
    """
    Run OCR on a screenshot and verify expected text is visible.

    :param screenshot_path: Path to the screenshot image file
    :param expected_text: Text phrase to look for in the OCR output
    :param step_label: Human-readable label for log messages
    :param verbose: Print detailed progress
    :return: True if expected text was found, False otherwise
    """
    if verbose:
        print(f"Running OCR to verify {step_label}...")

    text = ocr_text_via_command_lina(screenshot_path, verbose=verbose)
    if expected_text.lower() in text.lower():
        if verbose:
            print(f"Verified '{step_label}': found '{expected_text}'")
        return True
    else:
        if verbose:
            print(f"Verification failed for '{step_label}': expected '{expected_text}' not found")
        return False


def find_text_in_region_via_ocr(screenshot_path: str,
                                text: str,
                                window_x: int,
                                window_y: int,
                                window_width: int = 0,
                                window_height: int = 0,
                                region_x_percent: float = 0.0,
                                region_y_percent: float = 0.0,
                                region_w_percent: float = 100.0,
                                region_h_percent: float = 100.0,
                                verbose: bool = False) -> tuple[int, int] | None:
    """
    Find text (such as a button label or hotspot caption) in a designated region of a screenshot
    using OCR and return the center screen coordinates.

    General purpose wrapper around ``find_button_via_ocr_single`` for finding any labeled
    clickable region — buttons, hotspots, etc.

    :param screenshot_path: Path to the screenshot image file
    :param text: The text/caption to find
    :param window_x: Window X position on screen (logical pixels)
    :param window_y: Window Y position on screen (logical pixels)
    :param window_width: Logical window width from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param window_height: Logical window height from kdotool (for HiDPI scale calculation).
        If 0, no scaling is applied.
    :param region_x_percent: Starting X of search region as a percentage (0-100)
    :param region_y_percent: Starting Y of search region as a percentage (0-100)
    :param region_w_percent: Width of search region as a percentage (0-100)
    :param region_h_percent: Height of search region as a percentage (0-100)
    :param verbose: Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the found text (logical pixels), or None
    """
    crop_x_end = region_x_percent + region_w_percent
    crop_y_end = region_y_percent + region_h_percent
    return find_button_via_ocr_single(
        screenshot_path=screenshot_path,
        button_text=text,
        window_x=window_x,
        window_y=window_y,
        window_width=window_width,
        window_height=window_height,
        crop_x_starting_percent=region_x_percent,
        crop_y_starting_percent=region_y_percent,
        crop_x_ending_percent=crop_x_end,
        crop_y_ending_percent=crop_y_end,
        verbose=verbose
    )


def find_text_in_letterbox_region_via_ocr(screenshot_path: str,
                                          text: str,
                                          window_x: int,
                                          window_y: int,
                                          window_width: int,
                                          window_height: int,
                                          region_x_percent: float = 0.0,
                                          region_y_percent: float = 0.0,
                                          region_w_percent: float = 100.0,
                                          region_h_percent: float = 100.0,
                                          active_window_only: bool = True,
                                          verbose: bool = False) -> tuple[int, int] | None:
    """
    Find text in a designated region of the letterboxed area of a screenshot using OCR.

    Unlike ``find_text_in_region_via_ocr``, the region percentages are relative to the
    letterboxed (rendered) area, not the full window.  This function calculates the
    letterbox region first, then maps the relative percentages to screenshot-relative
    crop percentages for the OCR search.

    On Wayland with HiDPI scaling, the screenshot may be at a higher resolution than
    the logical window size.  The letterbox calculation uses the actual screenshot
    dimensions, and the HiDPI scale factor is applied when converting OCR positions
    to logical screen coordinates.

    :param screenshot_path: Path to the screenshot image file
    :param text: The text/caption to find
    :param window_x: Window X position on screen (logical pixels)
    :param window_y: Window Y position on screen (logical pixels)
    :param window_width: Logical window width from kdotool
    :param window_height: Logical window height from kdotool
    :param region_x_percent: Starting X within letterbox as a percentage (0-100)
    :param region_y_percent: Starting Y within letterbox as a percentage (0-100)
    :param region_w_percent: Width of search region within letterbox as a percentage (0-100)
    :param region_h_percent: Height of search region within letterbox as a percentage (0-100)
    :param active_window_only: If True, the screenshot is window-only (no desktop background).
        The letterbox calculation uses the actual screenshot dimensions. Default True.
    :param verbose: Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the found text (logical pixels), or None
    """
    # Read actual screenshot dimensions — these are what matter for cropping.
    # On Wayland with HiDPI, the screenshot is at native pixel resolution while
    # window_width/window_height are in logical pixels.
    img: ImageFile = Image.open(screenshot_path)
    img_w, img_h = img.size
    img.close()

    if active_window_only:
        # Use the actual screenshot dimensions for the letterbox calculation.
        # The screenshot IS the client area, so the letterbox is relative to it.
        # We treat the screenshot origin as (0, 0) for letterbox purposes, then
        # add the window offset when returning screen coordinates.
        lb_x, lb_y, lb_w, lb_h = calculate_letterbox_region(0, 0, img_w, img_h)

        # lb_x and lb_y are now relative to the screenshot origin (0, 0)
        lb_rel_x = lb_x  # Letterbox X relative to screenshot
        lb_rel_y = lb_y  # Letterbox Y relative to screenshot

        # Convert to screenshot-relative percentages
        crop_x_start = (lb_rel_x + lb_w * region_x_percent / 100) / img_w * 100
        crop_y_start = (lb_rel_y + lb_h * region_y_percent / 100) / img_h * 100
        crop_x_end = (lb_rel_x + lb_w * (region_x_percent + region_w_percent) / 100) / img_w * 100
        crop_y_end = (lb_rel_y + lb_h * (region_y_percent + region_h_percent) / 100) / img_h * 100

        if verbose:
            print(f"Screenshot dimensions: {img_w}x{img_h}")
            print(f"Window geometry (logical): {window_width}x{window_height}")
            print(f"Letterbox: ({lb_x}, {lb_y}), size {lb_w}x{lb_h}")
            print(f"Letterbox-relative region: ({region_x_percent}%, {region_y_percent}%), size {region_w_percent}%x{region_h_percent}%")
            print(f"Screenshot-relative crop: ({crop_x_start:.1f}%, {crop_y_start:.1f}%) to ({crop_x_end:.1f}%, {crop_y_end:.1f}%)")

        # Pass the logical window dimensions so find_button_via_ocr_single can
        # calculate the HiDPI scale factor and convert screenshot-space OCR
        # positions to logical screen coordinates.
        return find_button_via_ocr_single(
            screenshot_path=screenshot_path,
            button_text=text,
            window_x=window_x,
            window_y=window_y,
            window_width=window_width,
            window_height=window_height,
            crop_x_starting_percent=crop_x_start,
            crop_y_starting_percent=crop_y_start,
            crop_x_ending_percent=crop_x_end,
            crop_y_ending_percent=crop_y_end,
            verbose=verbose
        )
    else:
        # Full-screen screenshot: use window geometry for letterbox calculation.
        lb_x, lb_y, lb_w, lb_h = calculate_letterbox_region(window_x, window_y, window_width, window_height)
        lb_rel_x = lb_x - window_x
        lb_rel_y = lb_y - window_y
        crop_x_start = (lb_rel_x + lb_w * region_x_percent / 100) / window_width * 100
        crop_y_start = (lb_rel_y + lb_h * region_y_percent / 100) / window_height * 100
        crop_x_end = (lb_rel_x + lb_w * (region_x_percent + region_w_percent) / 100) / window_width * 100
        crop_y_end = (lb_rel_y + lb_h * (region_y_percent + region_h_percent) / 100) / window_height * 100

        return find_button_via_ocr_single(
            screenshot_path=screenshot_path,
            button_text=text,
            window_x=window_x,
            window_y=window_y,
            window_width=window_width,
            window_height=window_height,
            crop_x_starting_percent=crop_x_start,
            crop_y_starting_percent=crop_y_start,
            crop_x_ending_percent=crop_x_end,
            crop_y_ending_percent=crop_y_end,
            verbose=verbose
        )


# === Hotspot pixel-based detection ===
# Hotspots in the hangar/base view are marked by bright blue dots on the screen.
# When hovering, the cursor turns orange and the hotspot name appears in orange
# near the bottom-left of the screen. OCR alone often misses small hotspot labels,
# so we search for the blue indicator first, then verify the caption via OCR.

# === Hotspot geometry per base type ===
# Different base types (ocean, desert, space station, etc.) display their clickable
# hotspots at different positions on the screen.  The geometry is stored as
# percentages of the letterboxed base area (1.33:1 aspect ratio):
#   (horizontal_min%, horizontal_max%, vertical_min%, vertical_max%)
# The center of that rectangle guides the pixel search to the correct blue cluster.


HotspotGeometry = namedtuple('HotspotGeometry', ['h_min', 'h_max', 'v_min', 'v_max'])


def _hotspot_center(geometry: HotspotGeometry) -> tuple[float, float]:
    """
    Calculate the center of a hotspot geometry as percentages.

    :param geometry: HotspotGeometry namedtuple
    :return: Tuple of (center_x_percent, center_y_percent)
    """
    cx = (geometry.h_min + geometry.h_max) / 2
    cy = (geometry.v_min + geometry.v_max) / 2
    return (cx, cy)


# Known hotspot geometries, keyed by (base_type, hotspot_name).
# Base types follow the naming used in the game's base definition files.
# Hotspot names are the caption text shown when hovering (e.g. "Launch").
HOTSPOT_GEOMETRY: dict[tuple[str, str], HotspotGeometry] = {
    # Ocean-type bases (e.g. Atlantis) — the starting base.
    # Design geometry relative to the letterboxed base area (1.33:1 aspect ratio).
    ("ocean", "Launch"): HotspotGeometry(
        h_min=19.53125, h_max=85.64455,
        v_min=24.8698, v_max=82.8125
    ),
}


def get_hotspot_hint_position(
    base_type: str,
    hotspot_name: str,
    window_width: int,
    window_height: int,
) -> tuple[float, float] | None:
    """
    Calculate the hint position for a hotspot on a given base type.

    Looks up the design geometry for the (base_type, hotspot_name) pair and
    returns the center as window coordinates.  The hotspot geometry percentages
    are relative to the letterboxed (4:3) rendering area, so the letterbox
    region is calculated first.

    Returns None if the combination is not known.

    :param base_type: Base type string (e.g. "ocean", "desert", "space_station")
    :param hotspot_name: Hotspot caption text (e.g. "Launch")
    :param window_width: Current window width in pixels
    :param window_height: Current window height in pixels
    :return: Tuple of (hint_x, hint_y) in window coordinates, or None
    """
    geometry = HOTSPOT_GEOMETRY.get((base_type, hotspot_name))
    if geometry is None:
        return None
    cx_pct, cy_pct = _hotspot_center(geometry)
    # Hotspot geometry is relative to the letterboxed area, not the full window.
    lb_x, lb_y, lb_w, lb_h = calculate_letterbox_region(0, 0, window_width, window_height)
    return (lb_x + lb_w * cx_pct / 100, lb_y + lb_h * cy_pct / 100)


def _is_bright_blue_pixel(r: int, g: int, b: int, blue_threshold: int = 128, red_max: int = 100, green_max: int = 150) -> bool:
    """
    Check whether a pixel is a bright blue color (hotspot indicator).

    Hotspot markers are bright blue dots. The blue channel dominates,
    while red and green are relatively low.

    :param r: Red channel value (0-255)
    :param g: Green channel value (0-255)
    :param b: Blue channel value (0-255)
    :param blue_threshold: Minimum blue channel value to consider the pixel blue
    :param red_max: Maximum red channel value for a blue pixel
    :param green_max: Maximum green channel value for a blue pixel
    :return: True if the pixel is bright blue
    """
    return b >= blue_threshold and r <= red_max and g <= green_max


def _cluster_blue_pixels(pixels_list: list[tuple[int, int]], cluster_distance: int = 30) -> list[list[tuple[int, int]]]:
    """
    Group blue pixels into clusters based on proximity.

    Pixels within `cluster_distance` of each other belong to the same cluster.
    Uses a simple single-pass greedy clustering algorithm.

    :param pixels_list: List of (x, y) tuples of blue pixels
    :param cluster_distance: Maximum distance between pixels to be in the same cluster
    :return: List of clusters, each cluster is a list of (x, y) tuples
    """
    if not pixels_list:
        return []

    clusters: list[list[tuple[int, int]]] = []
    cluster_centers: list[tuple[float, float]] = []
    dist_sq = cluster_distance * cluster_distance

    for px, py in pixels_list:
        best_cluster = -1
        best_dist_sq = dist_sq

        for i, (cx, cy) in enumerate(cluster_centers):
            d_sq = (px - cx) ** 2 + (py - cy) ** 2
            if d_sq < best_dist_sq:
                best_dist_sq = d_sq
                best_cluster = i

        if best_cluster >= 0:
            clusters[best_cluster].append((px, py))
            # Update cluster center incrementally
            cluster = clusters[best_cluster]
            n = len(cluster)
            new_cx = (cluster_centers[best_cluster][0] * (n - 1) + px) / n
            new_cy = (cluster_centers[best_cluster][1] * (n - 1) + py) / n
            cluster_centers[best_cluster] = (new_cx, new_cy)
        else:
            clusters.append([(px, py)])
            cluster_centers.append((float(px), float(py)))

    return clusters


def _cluster_center(cluster: list[tuple[int, int]]) -> tuple[float, float]:
    """
    Calculate the center of a pixel cluster.

    :param cluster: List of (x, y) tuples
    :return: Tuple of (center_x, center_y)
    """
    if not cluster:
        return (0.0, 0.0)
    cx = sum(p[0] for p in cluster) / len(cluster)
    cy = sum(p[1] for p in cluster) / len(cluster)
    return (cx, cy)


def _distance_to_hint(cx: float, cy: float, hint_x: float, hint_y: float) -> float:
    """
    Calculate the Euclidean distance from a cluster center to a hint point.

    :param cx: Cluster center X
    :param cy: Cluster center Y
    :param hint_x: Hint point X
    :param hint_y: Hint point Y
    :return: Euclidean distance
    """
    return ((cx - hint_x) ** 2 + (cy - hint_y) ** 2) ** 0.5


def _is_orange_pixel(r: int, g: int, b: int,
                      red_min: int = 150, green_min: int = 80, green_max: int = 200,
                      blue_max: int = 80) -> bool:
    """
    Check whether an RGB pixel looks orange (warm color with dominant red).

    :param r: Red channel value (0-255)
    :param g: Green channel value (0-255)
    :param b: Blue channel value (0-255)
    :param red_min: Minimum red channel value for orange
    :param green_min: Minimum green channel value for orange
    :param green_max: Maximum green channel value for orange
    :param blue_max: Maximum blue channel value for orange
    :return: True if the pixel is orange
    """
    return r >= red_min and green_min <= g <= green_max and b <= blue_max


def _verify_caption_via_ocr(screenshot_path: str, caption_text: str, verbose: bool = False) -> bool:
    """
    Verify that the expected caption text appears in the bottom-left portion of the screenshot
    using OCR.  Hotspot captions appear in orange near the bottom-left of the screen when
    the mouse is hovering over the hotspot.

    The function first checks for orange-colored pixels in the bottom-left caption region,
    then runs OCR on that region to find the expected text.

    :param screenshot_path: Path to the screenshot image file
    :param caption_text: Expected caption text (e.g. "Launch")
    :param verbose: Print debug info
    :return: True if the caption text was found via OCR
    """
    try:
        img: ImageFile = Image.open(screenshot_path)
        img_w, img_h = img.size
        pixels = img.load()

        # The caption area is the bottom-left: left 40% of width, bottom 15% of height
        caption_x2 = int(img_w * 0.40)
        caption_y1 = int(img_h * 0.85)

        # Check for orange pixels in the caption region to confirm a caption is visible
        orange_count = 0
        for y in range(caption_y1, img_h):
            for x in range(0, caption_x2):
                pixel = pixels[x, y]
                if len(pixel) >= 3:
                    r, g, b = pixel[0], pixel[1], pixel[2]
                    if _is_orange_pixel(r, g, b):
                        orange_count += 1

        if verbose:
            print(f"Orange pixels in caption region: {orange_count}")

        # Crop to the bottom-left caption area and convert to grayscale for OCR
        cropped = img.crop((0, caption_y1, caption_x2, img_h))
        cropped = cropped.convert('L')

        # Use Tesseract to find word-level bounding boxes
        data = pytesseract.image_to_data(cropped, output_type=pytesseract.Output.DICT)

        if verbose:
            all_words = []
            for i, text in enumerate(data['text']):
                t = text.strip()
                if t:
                    all_words.append(t)
            print(f"OCR words in caption region (bottom-left): {all_words}")

        target = caption_text.lower()
        for i, text in enumerate(data['text']):
            if text.strip().lower() == target:
                if verbose:
                    print(f"Caption verification passed: found '{target}' in caption region")
                return True

        if verbose:
            print(f"Caption verification failed: '{target}' not found in caption region")
        return False
    except Exception as e:
        if verbose:
            print(f"Caption OCR error: {e}")
        return False


def find_hotspot_via_pixel_search(screenshot_path: str,
                                   window_x: int,
                                   window_y: int,
                                   window_width: int,
                                   window_height: int,
                                   caption_text: str,
                                   hint_window_x: float | None = None,
                                   hint_window_y: float | None = None,
                                   search_x_percent: float = 0.0,
                                   search_y_percent: float = 0.0,
                                   search_w_percent: float = 100.0,
                                   search_h_percent: float = 100.0,
                                   screen_width: int = 3840,
                                   screen_height: int = 2160,
                                   verbose: bool = False) -> tuple[int, int] | None:
    """
    Find a clickable hotspot by moving the mouse to the expected design position,
    then verifying the caption text appears in orange at the bottom-left.

    Hotspot blue pixels dim when the mouse is far away and brighten as it moves closer.
    Instead of scanning the entire window for dim blue pixels, this function uses the
    calculated design coordinates (hint position) to move the mouse close first, then
    confirms the correct hotspot by checking the orange caption text.

    On Wayland with HiDPI scaling, the screenshot is at native pixel resolution while
    window coordinates are logical.  The scale factor is computed from the screenshot
    and window dimensions, and applied when converting pixel positions to screen coords.

    If no hint position is provided, falls back to scanning for bright blue pixel clusters
    and trying each one until the caption matches.

    The algorithm (with hint):
    1. Move the mouse to the hint position to brighten the blue pixels and reveal the caption
    2. Wait briefly for the caption to appear
    3. Take a screenshot
    4. Verify the expected caption text appears in orange at the bottom-left via OCR
    5. Return the hint position as screen coordinates if verified

    The algorithm (without hint, fallback):
    1. Scan the search region for bright blue pixels
    2. Cluster nearby blue pixels together
    3. For each cluster: hover, screenshot, verify caption
    4. Return the first matching cluster's center, or None

    :param screenshot_path: Path to the screenshot image file (used for fallback scanning)
    :param window_x: Window X position on screen (logical pixels)
    :param window_y: Window Y position on screen (logical pixels)
    :param window_width: Logical window width from kdotool
    :param window_height: Logical window height from kdotool
    :param caption_text: Expected hotspot caption (e.g. "Launch") — mandatory for verification
    :param hint_window_x: Hint X in logical window coordinates (from design coords)
    :param hint_window_y: Hint Y in logical window coordinates (from design coords)
    :param search_x_percent: Starting X of search region as a percentage (0-100)
    :param search_y_percent: Starting Y of search region as a percentage (0-100)
    :param search_w_percent: Width of search region as a percentage (0-100)
    :param search_h_percent: Height of search region as a percentage (0-100)
    :param screen_width: Screen width in logical pixels (for mouse movement)
    :param screen_height: Screen height in logical pixels (for mouse movement)
    :param verbose: Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the hotspot (logical pixels), or None
    """
    import tempfile

    # Read screenshot dimensions to compute HiDPI scale factor.
    img: ImageFile = Image.open(screenshot_path)
    img_w, img_h = img.size
    scale_x, scale_y = calculate_scale_factor(img_w, img_h, window_width, window_height, verbose)

    # Primary strategy: use the hint position to move the mouse close, then verify caption
    if hint_window_x is not None and hint_window_y is not None:
        hint_screen_x = int(window_x + hint_window_x)
        hint_screen_y = int(window_y + hint_window_y)

        if verbose:
            print(f"Moving mouse to hint position ({hint_screen_x}, {hint_screen_y}) to brighten hotspot...")

        # Move mouse to the hint position to trigger the hotspot caption
        move_mouse_to_position(hint_screen_x, hint_screen_y, screen_width, screen_height, verbose)
        time.sleep(1.0)  # Wait for caption to appear and blue pixels to brighten

        # Take a screenshot to capture the caption
        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            hover_screenshot_path = f.name

        try:
            if not take_screenshot(hover_screenshot_path, active_window_only=True, verbose=verbose):
                if verbose:
                    print("Failed to take hover screenshot")
            else:
                # Verify the caption text via OCR in the bottom-left area
                if _verify_caption_via_ocr(hover_screenshot_path, caption_text, verbose=verbose):
                    if verbose:
                        print(f"Found hotspot at hint position ({hint_screen_x}, {hint_screen_y})")
                    return hint_screen_x, hint_screen_y
                else:
                    if verbose:
                        print(f"Caption verification failed at hint position, trying fallback...")
        finally:
            if os.path.exists(hover_screenshot_path):
                os.unlink(hover_screenshot_path)

    # Fallback strategy: scan for blue pixel clusters and try each one
    if verbose:
        print("Scanning for blue pixel clusters (fallback)...")

    pixels = img.load()

    # Calculate search region (in screenshot pixel space)
    crop_x1 = int(img_w * search_x_percent / 100)
    crop_y1 = int(img_h * search_y_percent / 100)
    crop_x2 = int(img_w * (search_x_percent + search_w_percent) / 100)
    crop_y2 = int(img_h * (search_y_percent + search_h_percent) / 100)

    if verbose:
        print(f"Searching for blue hotspot pixels in region: ({crop_x1}, {crop_y1}) to ({crop_x2}, {crop_y2})")

    # Find all bright blue pixels in the search region
    blue_pixels: list[tuple[int, int]] = []
    for y in range(crop_y1, crop_y2):
        for x in range(crop_x1, crop_x2):
            pixel = pixels[x, y]
            if len(pixel) >= 3:
                r, g, b = pixel[0], pixel[1], pixel[2]
                if _is_bright_blue_pixel(r, g, b):
                    blue_pixels.append((x, y))

    if not blue_pixels:
        if verbose:
            print("No bright blue pixels found in search region")
        return None

    if verbose:
        print(f"Found {len(blue_pixels)} bright blue pixels")

    # Cluster blue pixels by proximity
    clusters = _cluster_blue_pixels(blue_pixels)
    if verbose:
        print(f"Found {len(clusters)} blue pixel cluster(s)")
        for i, cluster in enumerate(clusters):
            cx, cy = _cluster_center(cluster)
            print(f"  Cluster {i}: {len(cluster)} pixels, center ({cx:.1f}, {cy:.1f})")

    # Convert hint to screenshot-space for distance comparison (if provided).
    # The hint is in logical window coords; multiply by scale to get screenshot coords.
    hint_screenshot_x = hint_window_x * scale_x if hint_window_x is not None else None
    hint_screenshot_y = hint_window_y * scale_y if hint_window_y is not None else None

    # Sort clusters: by distance to hint (if hint provided) or by size (largest first)
    if hint_screenshot_x is not None and hint_screenshot_y is not None:
        clusters_with_dist = []
        for cluster in clusters:
            cx, cy = _cluster_center(cluster)
            dist = _distance_to_hint(cx, cy, hint_screenshot_x, hint_screenshot_y)
            clusters_with_dist.append((dist, cluster))
            if verbose:
                print(f"  Cluster center ({cx:.1f}, {cy:.1f}) distance to hint: {dist:.1f}")
        clusters_with_dist.sort(key=lambda x: x[0])
        sorted_clusters = [cluster for _, cluster in clusters_with_dist]
    else:
        sorted_clusters = sorted(clusters, key=len, reverse=True)

    if not sorted_clusters:
        if verbose:
            print("No suitable cluster found")
        return None

    # Try each cluster: hover, screenshot, verify caption
    for i, cluster in enumerate(sorted_clusters):
        avg_x, avg_y = _cluster_center(cluster)
        # Convert screenshot-space pixel position to logical screen coordinates
        screen_x = int(window_x + avg_x / scale_x)
        screen_y = int(window_y + avg_y / scale_y)

        if verbose:
            print(f"Trying cluster {i} at ({screen_x}, {screen_y})...")

        move_mouse_to_position(screen_x, screen_y, screen_width, screen_height, verbose)
        time.sleep(0.5)  # Wait for caption to appear

        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            hover_screenshot_path = f.name

        try:
            if not take_screenshot(hover_screenshot_path, active_window_only=True, verbose=verbose):
                if verbose:
                    print("Failed to take hover screenshot")
                continue

            if _verify_caption_via_ocr(hover_screenshot_path, caption_text, verbose=verbose):
                if verbose:
                    print(f"Found hotspot via cluster {i} at ({screen_x}, {screen_y})")
                return screen_x, screen_y
            else:
                if verbose:
                    print(f"Caption verification failed for cluster {i}, trying next...")
        finally:
            if os.path.exists(hover_screenshot_path):
                os.unlink(hover_screenshot_path)

    if verbose:
        print(f"Could not find hotspot with caption '{caption_text}'")
    return None
