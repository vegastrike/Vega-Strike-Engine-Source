#!/usr/bin/env python3
"""
Start Vega Strike, wait for the main menu, click the Quit button, and verify
clean exit.

Takes a fullscreen screenshot using spectacle (Wayland-compatible), uses OCR
(Tesseract) to find the "Quit" button text, then clicks using ydotool.
The game runs as a native Wayland window.

Usage:
    python3 doc/click_quit.bak.py [options]

Options:
    --target DIR        Path to Assets-Production directory (default: auto-detect)
    --binary PATH       Path to vegastrike-engine binary (default: ./bin/vegastrike-engine)
    --debug LEVEL       Debug level 0-3 (default: 2)
    --timeout SECS      Max seconds to wait for game exit (default: 120)
    --verbose           Print detailed progress

Dependencies:
    - dotool (install: sudo dnf install dotool) — for mouse movement and clicking
      (may require user in input group)
    - kdotool (install: sudo dnf install kdotool) —
    - spectacle (KDE screenshot tool) — for screenshots on Wayland
    - Tesseract OCR (install: sudo dnf install tesseract) — for finding button text
    - pytesseract (install: pip install pytesseract) — Python Tesseract wrapper
    - Pillow (install: pip install Pillow) — image processing
    - vegastrike-engine built binary
    - Assets-Production data directory
"""
import subprocess
import time
import sys
import os
import argparse
import tempfile

from vega_automation import (
    SCREEN_X_ADJUST,
    take_screenshot,
    find_vega_strike_window,
    activate_window,
    get_window_geometry,
    find_quit_button_via_ocr,
    find_confirmation_button_via_ocr,
    get_display_bounds,
    click_at_position,
)


def main():
    parser = argparse.ArgumentParser(
        description='Start Vega Strike, click Quit via ydotool, and verify clean exit.'
    )
    parser.add_argument('--target', default=None,
                        help='Path to Assets-Production directory')
    parser.add_argument('--binary', default='./bin/vegastrike-engine',
                        help='Path to vegastrike-engine binary')
    parser.add_argument('--debug', type=int, default=2,
                        help='Debug level 0-3 (default: 2)')
    parser.add_argument('--display', type=int, default=0,
                        help='X11 display number (default: 0)')
    parser.add_argument('--timeout', type=int, default=120,
                        help='Max seconds to wait for game exit (default: 120)')
    parser.add_argument('--verbose', action='store_true',
                        help='Print detailed progress')
    args = parser.parse_args()

    # Default target path if not specified
    if args.target is None:
        # Try common locations
        candidates = [
            os.path.expanduser('~/Projects/Assets-Production/'),
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

    if args.verbose:
        print("Getting screen geometry...")
    screen_width, screen_height = get_display_bounds(verbose=args.verbose)
    if args.verbose:
        print(f"Screen bounds: {screen_width, screen_height}")

    # Start the game
    if args.verbose:
        print("Starting Vega Strike...")

    env = os.environ.copy()
    # Run as native Wayland; ydotool handles input via uinput

    game_proc = subprocess.Popen(
        [args.binary, '--target', args.target, f'--debug={args.debug}'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        env=env
    )

    # Wait for the game to start and load
    if args.verbose:
        print("Waiting for game to start...")
    time.sleep(5)

    if args.verbose:
        print("Waiting for game to fully load...")
    time.sleep(15)

    # Take a screenshot and use OCR to find the Quit button
    with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
        screenshot_path = f.name

    try:
        if args.verbose:
            print("Finding game window...")
        window_id: str = find_vega_strike_window(verbose=args.verbose)
        if window_id is None or window_id == '':
            print("ERROR: Could not find Vega Strike window")
            game_proc.kill()
            game_proc.wait()
            return 1

        if args.verbose:
            print("Activating game window...")
        if not activate_window(window_id, verbose=args.verbose):
            print("ERROR: Could not activate Vega Strike window")
            game_proc.kill()
            game_proc.wait()
            return 1

        if args.verbose:
            print("Getting window geometry...")
        window_geometry: tuple[int, int, int, int] | None = get_window_geometry(window_id, verbose=args.verbose)
        if window_geometry is None:
            print("ERROR: Could not get geometry of Vega Strike window")
            game_proc.kill()
            game_proc.wait()
            return 1
        window_x, window_y, window_width, window_height = window_geometry

        if args.verbose:
            print("Taking screenshot...")
        if not take_screenshot(screenshot_path, active_window_only=True, verbose=args.verbose):
            print("ERROR: Could not take screenshot")
            game_proc.kill()
            game_proc.wait()
            return 1

        quit_pos = find_quit_button_via_ocr(screenshot_path, window_x=window_x, window_y=window_y, verbose=args.verbose)
        if quit_pos is None:
            print("ERROR: Could not find Quit button via OCR")
            game_proc.kill()
            game_proc.wait()
            return 1

        screen_x, screen_y = quit_pos
        screen_x += SCREEN_X_ADJUST
        if args.verbose:
            print(f"Quit button screen position: ({screen_x}, {screen_y})")

        # Click the Quit button
        click_at_position(screen_x, screen_y, screen_width, screen_height, args.verbose)

        # After clicking Quit, the game may show a confirmation dialog.
        # Wait a moment, then check if the game is still running.
        time.sleep(2)
        if game_proc.poll() is None:
            if args.verbose:
                print("Game still running, checking for confirmation dialog...")
            # Take another screenshot to look for confirmation dialog
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
                confirm_screenshot = f.name
            try:
                if take_screenshot(confirm_screenshot, active_window_only=True, verbose=args.verbose):
                    confirm_pos = find_confirmation_button_via_ocr(
                        confirm_screenshot, window_x=window_x, window_y=window_y, verbose=args.verbose)
                    if confirm_pos is not None:
                        if args.verbose:
                            print(f"Clicking confirmation button at {confirm_pos}")
                        click_at_position(confirm_pos[0], confirm_pos[1], screen_width, screen_height, args.verbose)
            finally:
                if os.path.exists(confirm_screenshot):
                    os.unlink(confirm_screenshot)

    finally:
        if os.path.exists(screenshot_path):
            os.unlink(screenshot_path)

    # Wait for game to exit
    if args.verbose:
        print(f"Waiting for game to exit (timeout: {args.timeout}s)...")

    try:
        return_code = game_proc.wait(timeout=args.timeout)
        print(f"SUCCESS: Game exited with code: {return_code}")
        return return_code
    except subprocess.TimeoutExpired:
        print(f"ERROR: Game did not exit within {args.timeout} seconds")
        game_proc.kill()
        game_proc.wait()
        return 1


if __name__ == '__main__':
    sys.exit(main())
