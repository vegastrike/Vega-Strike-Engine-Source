#!/usr/bin/env python3
"""
Shared utilities for automated Vega Strike play testing on KDE/Wayland.

Provides helpers for window management (kdotool), screenshots (spectacle),
OCR-based button finding (Tesseract/pytesseract), and input simulation (dotool).
"""
import re
import subprocess
import time
from re import Pattern, Match
from typing import SupportsInt, Any

import pytesseract
from PIL import Image
from PIL.ImageFile import ImageFile

SCREEN_X_ADJUST = 30

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
    result = subprocess.run(cmd, shell=shell, capture_output=True, text=True)
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


def find_button_via_ocr(screenshot_path: str, button_texts: list[str], window_x: int = 0, window_y: int = 0, crop_x_starting_percent: float | int = 0, crop_y_starting_percent: float | int = 0, crop_x_ending_percent: float | int = 100, crop_y_ending_percent: float | int = 100, verbose: bool = False) -> tuple[int, int] | None:
    """
    Find a button with any of the specified text strings in the specified area of the screenshot and return its absolute screen coordinates.
    Searches for each text string in the order provided; returns the first match found.

    :rtype: tuple[int, int] | None
    :param screenshot_path: The path to the screenshot file to read
    :param button_texts: List of possible button captions to search for (first match wins)
    :param window_x: The window X coordinate
    :param window_y: The window Y coordinate
    :param crop_x_starting_percent: The starting horizontal percentage of the screenshot to crop to
    :param crop_y_starting_percent: The starting vertical percentage of the screenshot to crop to
    :param crop_x_ending_percent: The ending horizontal percentage of the screenshot to crop to
    :param crop_y_ending_percent: The ending vertical percentage of the screenshot to crop to
    :param verbose: Print debug info
    :return: The absolute coordinates of the center of the button text, or None on failure
    """
    img: ImageFile = Image.open(screenshot_path)
    img_w, img_h = img.size
    if verbose:
        print(f"Starting screenshot dimensions: {img_w}x{img_h}")

    # Crop to specified percentages
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

            button_center_x_inside_window: int = crop_x_start + word_x + word_w // 2
            button_center_y_inside_window: int = crop_y_start + word_y + word_h // 2

            screen_x: int = window_x + button_center_x_inside_window
            screen_y: int = window_y + button_center_y_inside_window

            if verbose:
                print(f"Found '{text_stripped}' at: ({word_x}, {word_y}), size: {word_w}x{word_h}")
                print(f"Position in full window: ({button_center_x_inside_window}, {button_center_y_inside_window})")
                print(f"Position on screen: ({screen_x}, {screen_y})")

            return screen_x, screen_y

    if verbose:
        print(f"Warning: None of the button texts found in screenshot: {button_texts}")
    return None


def find_button_via_ocr_single(screenshot_path: str, button_text: str, window_x: int = 0, window_y: int = 0, crop_x_starting_percent: float | int = 0, crop_y_starting_percent: float | int = 0, crop_x_ending_percent: float | int = 100, crop_y_ending_percent: float | int = 100, verbose: bool = False) -> tuple[int, int] | None:
    """
    Find the button with the specified text in the specified area of the screenshot and return its absolute screen coordinates.
    Thin wrapper around the list-based ``find_button_via_ocr``.

    :rtype: tuple[int, int] | None
    :param screenshot_path: The path to the screenshot file to read
    :param button_text: The button text to find
    :param window_x: The window X coordinate
    :param window_y: The window Y coordinate
    :param crop_x_starting_percent: The starting horizontal percentage of the screenshot to crop to
    :param crop_y_starting_percent: The starting vertical percentage of the screenshot to crop to
    :param crop_x_ending_percent: The ending horizontal percentage of the screenshot to crop to
    :param crop_y_ending_percent: The ending vertical percentage of the screenshot to crop to
    :param verbose: Print debug info
    :return: The absolute coordinates of the center of the button text, or None on failure
    """
    return find_button_via_ocr(
        screenshot_path=screenshot_path,
        button_texts=[button_text],
        window_x=window_x,
        window_y=window_y,
        crop_x_starting_percent=crop_x_starting_percent,
        crop_y_starting_percent=crop_y_starting_percent,
        crop_x_ending_percent=crop_x_ending_percent,
        crop_y_ending_percent=crop_y_ending_percent,
        verbose=verbose
    )


def find_quit_button_via_ocr(screenshot_path: str, window_x: int = 0, window_y: int = 0, verbose: bool = False) -> tuple[int, int] | None:
    """
    Use OCR on a screenshot of the active window to find the Quit button.

    The main menu buttons are in the lower portion of the game window.
    The Quit button is the lowest button. We crop to the bottom portion
    to avoid matching "Quit" in the game logo at the top.

    :rtype: tuple[int, int] | None
    :param screenshot_path: Path to the screenshot image
    :param window_x: The window's X position on the screen
    :param window_y: The window's Y position on the screen
    :param verbose:  Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the Quit button, or None
    """
    # Crop to the bottom 55% (starting at 45% from the top) to focus on the
    # button area and avoid matching "Quit" in the game logo at the top.
    return find_button_via_ocr(
        screenshot_path=screenshot_path,
        button_texts=["Quit"],
        window_x=window_x,
        window_y=window_y,
        crop_x_starting_percent=0,
        crop_y_starting_percent=45,
        crop_x_ending_percent=100,
        crop_y_ending_percent=100,
        verbose=verbose
    )


def find_confirmation_button_via_ocr(screenshot_path: str, window_x: int = 0, window_y: int = 0, verbose: bool = False) -> tuple[int, int] | None:
    """
    Use OCR on a screenshot to find a confirmation dialog button.

    After clicking Quit, the game may show a confirmation dialog.
    Searches for "Yes", "OK", or similar confirmation text.
    Confirmation dialogs appear near the center of the screen, so we crop
    to the center 60% region to avoid false matches elsewhere.

    :rtype: tuple[int, int] | None
    :param screenshot_path: Path to the screenshot image
    :param window_x: The window's X position on the screen
    :param window_y: The window's Y position on the screen
    :param verbose: Print debug info
    :return: Tuple of (screen_x, screen_y) — center of the confirmation button, or None
    """
    # Crop to the center 60% (20% margins on all sides) to focus on the
    # confirmation dialog area and avoid false matches elsewhere.
    return find_button_via_ocr(
        screenshot_path=screenshot_path,
        button_texts=["yes", "ok", "confirm"],
        window_x=window_x,
        window_y=window_y,
        crop_x_starting_percent=20,
        crop_y_starting_percent=20,
        crop_x_ending_percent=80,
        crop_y_ending_percent=80,
        verbose=verbose
    )


def get_display_bounds(verbose: bool = False) -> tuple[int, int]:
    """
    Get the total display resolution.

    :rtype: tuple[int, int]
    :param verbose: Print debug info
    :return: Tuple of (display_width, display_height)
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


def click_at_position(screen_x: SupportsInt, screen_y: SupportsInt, screen_width: SupportsInt, screen_height: SupportsInt,
                      verbose: bool = False) -> None:
    """
    Click at the specified screen position using ydotool.

    ydotool uses the Linux uinput device to simulate input events at the
    kernel level, bypassing Wayland's restrictions on cross-process input.
    ydotool mousemove --absolute uses actual screen pixel coordinates.

    :param screen_width: The width of the screen in pixels
    :param screen_height: The height of the screen in pixels
    :rtype: None
    :param screen_x: Target click position X coordinate in screen pixel coordinates
    :param screen_y: Target click position Y coordinate in screen pixel coordinates
    :param verbose: Print debug info
    """
    screen_x_fraction = screen_x / screen_width
    screen_y_fraction = screen_y / screen_height

    if verbose:
        print(f"Moving mouse to screen percentage ({screen_x_fraction}, {screen_y_fraction})...")

    # Move mouse
    out, rc = run_cmd(
        f'echo "mouseto {screen_x_fraction} {screen_y_fraction}" | dotool 2>/dev/null')
    if rc != 0 and verbose:
        print(f"dotool mousemove failed: {out}")
    time.sleep(0.3)

    # Click left button
    if verbose:
        print("Clicking...")
    out, rc = run_cmd('echo "click left" | dotool 2>/dev/null')
    if rc != 0 and verbose:
        print(f"dotool click failed: {out}")

    if verbose:
        print("Click sent!")
