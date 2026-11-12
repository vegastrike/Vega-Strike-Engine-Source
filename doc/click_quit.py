#!/usr/bin/env python3
"""
Start Vega Strike, wait for the main menu, click the Quit button, and verify clean exit.

Usage:
    python3 doc/click_quit.py [options]

Options:
    --target DIR        Path to Assets-Production directory (default: auto-detect)
    --binary PATH       Path to vegastrike-engine binary (default: ./bin/vegastrike-engine)
    --debug LEVEL       Debug level 0-3 (default: 2)
    --display NUM       X11 display number (default: 1)
    --timeout SECS      Max seconds to wait for game exit (default: 15)
    --verbose           Print detailed progress

Dependencies:
    - pyautogui (install: pip install pyautogui)
    - xdotool (install: sudo apt install xdotool)
    - vegastrike-engine built binary
    - Assets-Production data directory

See doc/quit-button-test.md for details on the coordinate system and why PyAutoGUI
is required instead of xdotool for clicking.
"""

import subprocess
import time
import sys
import os
import argparse

# Block mouseinfo import to suppress tkinter warning
class BlockImporter:
    def find_module(self, name, path=None):
        if name == 'mouseinfo' or name.startswith('mouseinfo.'):
            return self
    def load_module(self, name):
        import types
        m = types.ModuleType(name)
        sys.modules[name] = m
        return m
sys.meta_path.insert(0, BlockImporter())

import pyautogui
pyautogui.FAILSAFE = False


# Quit button definition from bases/main_menu.py:
# GUI.GUIRect(50, 580, 177, 43, "pixel", (1280, 1024))
# The "Quit" text image is at ~90% into the button width, not centered.
BTN_X = 50
BTN_Y = 580
BTN_W = 177
BTN_H = 43
DESIGN_W = 1280
DESIGN_H = 1024
TEXT_OFFSET_X = 0.9  # 90% into button width hits the "Quit" text


def run_cmd(cmd, shell=True):
    """Run a shell command and return (stdout, returncode)."""
    result = subprocess.run(cmd, shell=shell, capture_output=True, text=True)
    return result.stdout.strip(), result.returncode


def find_vega_strike_window(verbose=False):
    """
    Wait for the Vega Strike window to appear and return its window ID.

    Returns:
        window_id string or None if not found
    """
    if verbose:
        print("Waiting for Vega Strike window...")

    for attempt in range(60):
        time.sleep(1)
        out, _ = run_cmd('xdotool search --name "Vega Strike" 2>/dev/null')
        if out:
            for wid in out.split('\n'):
                wid = wid.strip()
                if wid:
                    name, _ = run_cmd(f'xdotool getwindowname {wid} 2>/dev/null')
                    if 'Vega' in name:
                        if verbose:
                            print(f"Found window: {wid}")
                        return wid
            # If search returned something, use the first valid ID
            first_wid = out.split('\n')[0].strip()
            if first_wid:
                if verbose:
                    print(f"Found window (fallback): {first_wid}")
                return first_wid

        # Fallback: try getactivewindow
        out, _ = run_cmd('xdotool getactivewindow 2>/dev/null')
        if out:
            wid = out.strip()
            if wid:
                name, _ = run_cmd(f'xdotool getwindowname {wid} 2>/dev/null')
                if 'Vega' in name:
                    if verbose:
                        print(f"Found window (active): {wid}")
                    return wid

    return None


def get_window_geometry(window_id, verbose=False):
    """
    Get window position and size using xdotool.

    Returns:
        Tuple of (x, y, width, height) or None on failure
    """
    out, _ = run_cmd(f'xdotool getwindowgeometry --shell {window_id} 2>/dev/null')
    if not out:
        return None

    wx = wy = ww = wh = 0
    for line in out.split('\n'):
        if line.startswith('X='):
            wx = int(line.split('=')[1])
        elif line.startswith('Y='):
            wy = int(line.split('=')[1])
        elif line.startswith('WIDTH='):
            ww = int(line.split('=')[1])
        elif line.startswith('HEIGHT='):
            wh = int(line.split('=')[1])

    if verbose:
        print(f"Window: ({wx}, {wy}), {ww}x{wh}")

    return wx, wy, ww, wh


def calculate_quit_position(wx, wy, ww, wh, verbose=False):
    """
    Calculate the screen position of the Quit button text.

    The Quit button is defined in design coordinates (1280x1024) and must be
    scaled to the actual window size. The text is at 90% into the button width.
    """
    sx = ww / DESIGN_W
    sy = wh / DESIGN_H

    text_x = BTN_X + BTN_W * TEXT_OFFSET_X
    text_y = BTN_Y + BTN_H / 2

    screen_x = int(wx + text_x * sx)
    screen_y = int(wy + text_y * sy)

    if verbose:
        print(f"Scale factors: sx={sx:.4f}, sy={sy:.4f}")
        print(f"Design text pos: ({text_x:.1f}, {text_y:.1f})")
        print(f"Screen click pos: ({screen_x}, {screen_y})")

    return screen_x, screen_y


def click_quit(screen_x, screen_y, verbose=False):
    """Move mouse to position and click using PyAutoGUI."""
    if verbose:
        print(f"Moving mouse to ({screen_x}, {screen_y})...")

    pyautogui.moveTo(screen_x, screen_y, duration=0.3)
    time.sleep(0.5)

    if verbose:
        pos = pyautogui.position()
        print(f"Mouse at: {pos}")

    pyautogui.click()
    if verbose:
        print("Clicked!")


def main():
    parser = argparse.ArgumentParser(
        description='Start Vega Strike, click Quit, and verify clean exit.'
    )
    parser.add_argument('--target', default=None,
                        help='Path to Assets-Production directory')
    parser.add_argument('--binary', default='./bin/vegastrike-engine',
                        help='Path to vegastrike-engine binary')
    parser.add_argument('--debug', type=int, default=2,
                        help='Debug level 0-3 (default: 2)')
    parser.add_argument('--display', type=int, default=1,
                        help='X11 display number (default: 1)')
    parser.add_argument('--timeout', type=int, default=15,
                        help='Max seconds to wait for game exit (default: 15)')
    parser.add_argument('--verbose', action='store_true',
                        help='Print detailed progress')
    args = parser.parse_args()

    os.environ['DISPLAY'] = f':{args.display}'

    # Default target path if not specified
    if args.target is None:
        # Try common locations
        candidates = [
            os.path.expand('../Assets-Production/'),
            os.path.expand('~/Projects/Assets-Production/'),
        ]
        for candidate in candidates:
            if os.path.isdir(candidate):
                args.target = candidate
                break
        else:
            print("ERROR: --target not specified and Assets-Production not found")
            print("Please specify --target /path/to/Assets-Production/")
            return 1

    if not os.path.isfile(args.binary):
        print(f"ERROR: Binary not found: {args.binary}")
        return 1

    if args.verbose:
        print(f"Binary: {args.binary}")
        print(f"Target: {args.target}")
        print(f"Display: :{args.display}")

    # Start the game
    if args.verbose:
        print("Starting Vega Strike...")

    game_proc = subprocess.Popen(
        [args.binary, '--target', args.target, f'--debug={args.debug}'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Wait for window to appear
    window_id = find_vega_strike_window(args.verbose)
    if not window_id:
        print("ERROR: Could not find Vega Strike window")
        game_proc.kill()
        game_proc.wait()
        return 1

    # Get window geometry
    geom = get_window_geometry(window_id, args.verbose)
    if not geom:
        print("ERROR: Could not get window geometry")
        game_proc.kill()
        game_proc.wait()
        return 1

    wx, wy, ww, wh = geom

    # Focus the window
    if args.verbose:
        print(f"Focusing window {window_id}...")
    run_cmd(f'xdotool windowfocus --sync {window_id} 2>/dev/null || true')
    time.sleep(1)

    # Calculate and click Quit button
    screen_x, screen_y = calculate_quit_position(wx, wy, ww, wh, args.verbose)
    click_quit(screen_x, screen_y, args.verbose)

    # Wait for game to exit
    if args.verbose:
        print(f"Waiting for game to exit (timeout: {args.timeout}s)...")

    try:
        returncode = game_proc.wait(timeout=args.timeout)
        print(f"SUCCESS: Game exited with code: {returncode}")
        return returncode
    except subprocess.TimeoutExpired:
        print(f"ERROR: Game did not exit within {args.timeout} seconds")
        game_proc.kill()
        game_proc.wait()
        return 1


if __name__ == '__main__':
    sys.exit(main())
