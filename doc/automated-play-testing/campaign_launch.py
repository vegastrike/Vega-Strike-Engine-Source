#!/usr/bin/env python3
"""
Start Vega Strike, launch a new campaign, launch into space, then exit.

Steps:
1. Start the game process
2. Wait for the "Vega Strike" window to appear
3. Click the "Campaign" button on the Main Menu
4. Wait for the Intro Monologue screen, click "Continue"
5. Wait for the Hangar view, click the "Launch" hotspot
6. Wait for launch animation, press Esc then Q to quit
7. Confirm the game exits with return code 0

Usage:
    python3 doc/campaign_launch.py [options]

Options:
    --target DIR        Path to Assets-Production directory (default: auto-detect)
    --binary PATH       Path to vegastrike-engine binary (default: ./bin/vegastrike-engine)
    --debug LEVEL       Debug level 0-3 (default: 2)
    --display NUM       X11 display number (default: 0)
    --timeout SECS      Max seconds to wait for game exit (default: 120)
    --verbose           Print detailed progress
    --kdotool-path      Path to folder containing kdotool, if not installed at the system level
    --x-adjust          Number of pixels by which to adjust button X coordinates (default: 0)
    --y-adjust          Number of pixels by which to adjust button Y coordinates (default: 0)

Dependencies:
    - dotool (install: sudo dnf install dotool) — for mouse movement and clicking
    - kdotool (install: sudo dnf install kdotool) — for window management on Wayland
    - spectacle (KDE screenshot tool) — for screenshots on Wayland
    - Tesseract OCR (install: sudo dnf install tesseract) — for finding button text
    - pytesseract (install: pip install pytesseract) — Python Tesseract wrapper
    - Pillow (install: pip install Pillow) — image processing
    - vegastrike-engine built binary
    - Assets-Production data directory

See doc/campaign-launch-test.md for details on the coordinate systems used.
"""

import subprocess
import time
import sys
import os
import argparse
import tempfile

from vega_automation import (
    take_screenshot,
    find_vega_strike_window,
    activate_window,
    get_window_geometry,
    refresh_window_geometry,
    get_display_bounds,
    click_at_position,
    press_key,
    verify_screen,
    find_text_in_region_via_ocr,
    find_hotspot_via_pixel_search,
    calculate_main_menu_button_position,
    calculate_intro_button_position,
)


# === Campaign button definition (from bases/main_menu.py) ===
# Design resolution: 1280x1024
# GUI.GUIRect(50, 220, 177, 43, "pixel", (1280, 1024))
CAMPAIGN_X = 50
CAMPAIGN_Y = 220
CAMPAIGN_W = 177
CAMPAIGN_H = 43

# === Intro Monologue "Continue" button (from bases/main_menu.py) ===
# Design resolution: 1024x768
# GUI.GUIRect(8, 697, 420, 47, "pixel", (1024, 768))
CONTINUE_X = 8
CONTINUE_Y = 697
CONTINUE_W = 420
CONTINUE_H = 47


def main():
    parser = argparse.ArgumentParser(
        description='Start Vega Strike, launch campaign, launch into space, then quit.'
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
    parser.add_argument('--kdotool-path', default=None,
                        help='Path to folder containing kdotool, if not installed at the system level')
    parser.add_argument('--x-adjust', type=int, default=0,
                        help='Number of pixels by which to adjust button X coordinates (default: 0)')
    parser.add_argument('--y-adjust', type=int, default=0,
                        help='Number of pixels by which to adjust button Y coordinates (default: 0)')
    args = parser.parse_args()

    # Default target path if not specified
    if args.target is None:
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

    if args.kdotool_path is not None:
        if not os.path.isdir(args.kdotool_path):
            print(f"ERROR: kdotool path not valid: {args.kdotool_path}")
            return 1
        os.environ['PATH'] = f"{args.kdotool_path}:{os.environ['PATH']}"
        if args.verbose:
            print(f"Modified PATH: {os.environ['PATH']}")

    if args.verbose:
        print(f"Binary: {args.binary}")
        print(f"Target: {args.target}")

    # Step 0: Get the screen geometry
    if args.verbose:
        print("Getting screen geometry...")
    screen_width, screen_height = get_display_bounds(verbose=args.verbose)
    if args.verbose:
        print(f"Screen bounds: {screen_width}, {screen_height}")

    # Step 1: Start the game
    if args.verbose:
        print("\n=== Step 1: Starting Vega Strike ===")

    env = os.environ.copy()
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

    # Step 2: Find and activate the game window
    if args.verbose:
        print("Finding game window...")
    window_id = find_vega_strike_window(verbose=args.verbose)
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
    window_geometry = get_window_geometry(window_id, verbose=args.verbose)
    if window_geometry is None:
        print("ERROR: Could not get geometry of Vega Strike window")
        game_proc.kill()
        game_proc.wait()
        return 1
    wx, wy, ww, wh = window_geometry

    # Step 3: Click Campaign button on the Main Menu
    if args.verbose:
        print("\n=== Step 3: Clicking Campaign button ===")

    with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
        screenshot_path = f.name

    try:
        if not take_screenshot(screenshot_path, active_window_only=True, verbose=args.verbose):
            print("ERROR: Could not take screenshot")
            game_proc.kill()
            game_proc.wait()
            return 1

        # Verify we're on the Main Menu screen by looking for "Campaign" text
        verify_screen(screenshot_path, "Campaign", "Main Menu", verbose=args.verbose)

        # Find the Campaign button via OCR in the lower portion of the screen
        # (main menu buttons are in the bottom area)
        campaign_pos = find_text_in_region_via_ocr(
            screenshot_path, "Campaign", wx, wy,
            region_x_percent=0, region_y_percent=45,
            region_w_percent=100, region_h_percent=55,
            verbose=args.verbose
        )
        if campaign_pos is not None:
            campaign_x, campaign_y = campaign_pos
            if args.verbose:
                print(f"Found Campaign button via OCR at ({campaign_x}, {campaign_y})")
        else:
            # Fallback: calculate position from design coordinates
            if args.verbose:
                print("OCR did not find Campaign button; using calculated position")
            campaign_x, campaign_y = calculate_main_menu_button_position(
                wx, wy, ww, wh, CAMPAIGN_X, CAMPAIGN_Y, CAMPAIGN_W, CAMPAIGN_H,
                verbose=args.verbose
            )

        campaign_x += args.x_adjust
        campaign_y += args.y_adjust
        click_at_position(campaign_x, campaign_y, screen_width, screen_height, args.verbose)

    finally:
        if os.path.exists(screenshot_path):
            os.unlink(screenshot_path)

    # Step 4: Wait for Intro Monologue, click Continue
    if args.verbose:
        print("\n=== Step 4: Waiting for Intro Monologue, clicking Continue ===")
    time.sleep(5)  # Wait for intro screen to appear

    geom = refresh_window_geometry(window_id, args.verbose)
    if not geom:
        print("ERROR: Could not refresh window geometry")
        game_proc.kill()
        game_proc.wait()
        return 1
    wx, wy, ww, wh = geom

    with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
        screenshot_path = f.name

    try:
        if not take_screenshot(screenshot_path, active_window_only=True, verbose=args.verbose):
            print("ERROR: Could not take screenshot")
            game_proc.kill()
            game_proc.wait()
            return 1

        # Verify we're on the Intro Monologue screen
        verify_screen(screenshot_path, "Continue", "Intro Monologue", verbose=args.verbose)

        # Find the Continue button via OCR in the lower portion of the screen
        continue_pos = find_text_in_region_via_ocr(
            screenshot_path, "Continue", wx, wy,
            region_x_percent=0, region_y_percent=50,
            region_w_percent=100, region_h_percent=50,
            verbose=args.verbose
        )
        if continue_pos is not None:
            continue_x, continue_y = continue_pos
            if args.verbose:
                print(f"Found Continue button via OCR at ({continue_x}, {continue_y})")
        else:
            # Fallback: calculate position from design coordinates
            if args.verbose:
                print("OCR did not find Continue button; using calculated position")
            continue_x, continue_y = calculate_intro_button_position(
                wx, wy, ww, wh, CONTINUE_X, CONTINUE_Y, CONTINUE_W, CONTINUE_H,
                verbose=args.verbose
            )

        continue_x += args.x_adjust
        continue_y += args.y_adjust
        click_at_position(continue_x, continue_y, screen_width, screen_height, args.verbose)

    finally:
        if os.path.exists(screenshot_path):
            os.unlink(screenshot_path)

    # Step 5: Wait for Hangar view, click Launch
    # The Launch hotspot is on the Atlantis ocean base.
    if args.verbose:
        print("\n=== Step 5: Waiting for Hangar view, clicking Launch ===")
    time.sleep(10)  # Wait for hangar to load

    geom = refresh_window_geometry(window_id, args.verbose)
    if not geom:
        print("ERROR: Could not refresh window geometry")
        game_proc.kill()
        game_proc.wait()
        return 1
    wx, wy, ww, wh = geom

    with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
        screenshot_path = f.name

    try:
        if not take_screenshot(screenshot_path, active_window_only=True, verbose=args.verbose):
            print("ERROR: Could not take screenshot")
            game_proc.kill()
            game_proc.wait()
            return 1

        # Verify we're on the Hangar screen - look for "Launch" text
        hangar_verified = verify_screen(screenshot_path, "Launch", "Hangar", verbose=args.verbose)
        if not hangar_verified:
            if args.verbose:
                print("Trying alternative verification for Hangar screen...")
            verify_screen(screenshot_path, "Landing Center", "Hangar (alt)", verbose=args.verbose)

        # Find the Launch hotspot using pixel-based search first (more reliable than OCR).
        # Hotspots are marked by bright blue dots on the screen. The pixel search
        # finds the blue indicator, then verifies orange caption text near the bottom.
        launch_pos = find_hotspot_via_pixel_search(
            screenshot_path, wx, wy,
            caption_text="Launch",
            search_x_percent=0, search_y_percent=0,
            search_w_percent=100, search_h_percent=100,
            verbose=args.verbose
        )
        if launch_pos is not None:
            launch_x, launch_y = launch_pos
            if args.verbose:
                print(f"Found Launch hotspot via pixel search at ({launch_x}, {launch_y})")
        else:
            # Fallback: try OCR to find the "Launch" text
            if args.verbose:
                print("Pixel search did not find Launch hotspot; trying OCR...")
            launch_pos = find_text_in_region_via_ocr(
                screenshot_path, "Launch", wx, wy,
                region_x_percent=0, region_y_percent=0,
                region_w_percent=100, region_h_percent=100,
                verbose=args.verbose
            )
            if launch_pos is not None:
                launch_x, launch_y = launch_pos
                if args.verbose:
                    print(f"Found Launch hotspot via OCR at ({launch_x}, {launch_y})")
            else:
                if args.verbose:
                    print("Could not find Launch hotspot; skipping click")
                launch_x = launch_y = None

        if launch_x is not None:
            launch_x += args.x_adjust
            launch_y += args.y_adjust
            click_at_position(launch_x, launch_y, screen_width, screen_height, args.verbose)

    finally:
        if os.path.exists(screenshot_path):
            os.unlink(screenshot_path)

    # Step 6: Wait for launch animation, press Esc then Q
    if args.verbose:
        print("\n=== Step 6: Waiting for launch, pressing Esc then Q ===")
    time.sleep(5)  # Wait for launch animation

    # Take a screenshot to confirm the ship has launched into space
    with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
        space_screenshot = f.name
    try:
        if args.verbose:
            print("Taking screenshot after launch...")
        take_screenshot(space_screenshot, active_window_only=True, verbose=args.verbose)
    finally:
        if os.path.exists(space_screenshot):
            os.unlink(space_screenshot)

    press_key('escape', args.verbose)
    time.sleep(1)
    press_key('q', args.verbose)

    # After pressing Q, the game may show a confirmation dialog.
    time.sleep(2)
    if game_proc.poll() is None:
        if args.verbose:
            print("Game still running, checking for confirmation dialog...")
        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            confirm_screenshot = f.name
        try:
            if take_screenshot(confirm_screenshot, active_window_only=True, verbose=args.verbose):
                confirm_pos = find_text_in_region_via_ocr(
                    confirm_screenshot, "Yes", wx, wy,
                    region_x_percent=20, region_y_percent=20,
                    region_w_percent=60, region_h_percent=60,
                    verbose=args.verbose
                )
                if confirm_pos is None:
                    confirm_pos = find_text_in_region_via_ocr(
                        confirm_screenshot, "Ok", wx, wy,
                        region_x_percent=20, region_y_percent=20,
                        region_w_percent=60, region_h_percent=60,
                        verbose=args.verbose
                    )
                if confirm_pos is not None:
                    if args.verbose:
                        print(f"Clicking confirmation button at {confirm_pos}")
                    click_at_position(confirm_pos[0], confirm_pos[1], screen_width, screen_height, args.verbose)
        finally:
            if os.path.exists(confirm_screenshot):
                os.unlink(confirm_screenshot)

    # Step 7: Wait for game to exit
    if args.verbose:
        print(f"\n=== Step 7: Waiting for game to exit (timeout: {args.timeout}s) ===")

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
