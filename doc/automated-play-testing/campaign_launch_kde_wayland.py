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
    python3 doc/campaign_launch_kde_wayland.py [options]

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
    get_display_bounds_logical,
    click_at_position,
    press_key,
    verify_screen,
    verify_screen_single,
    find_text_in_region_via_ocr,
    find_text_in_region_via_ocr_multi,
    find_text_in_letterbox_region_via_ocr,
    find_hotspot_via_pixel_search,
    get_hotspot_hint_position,
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

    # Step 0: Screen geometry will be computed after the first screenshot is taken,
    # so that the HiDPI scale factor can be derived from the screenshot vs window size.
    screen_width: int = 0
    screen_height: int = 0

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

        # Compute the logical screen geometry now that we have a screenshot and window size.
        # The scale factor is derived from the ratio of screenshot pixels to logical window pixels.
        from PIL import Image
        img = Image.open(screenshot_path)
        img_w, img_h = img.size
        screen_width, screen_height = get_display_bounds_logical(
            img_w, img_h, ww, wh, verbose=args.verbose
        )
        if args.verbose:
            print(f"Screen bounds (logical): {screen_width}, {screen_height}")

        # Verify we're on the Main Menu screen by looking for "Campaign" text
        verify_screen_single(screenshot_path, "Campaign", "Main Menu", verbose=args.verbose)

        # Find the Campaign button via OCR in the upper portion of the letterboxed area.
        # The Main Menu is letterboxed to 1.33:1. The Campaign button is in the upper
        # third of the letterboxed area, in the left half.
        campaign_pos = find_text_in_letterbox_region_via_ocr(
            screenshot_path, "Campaign", wx, wy, ww, wh,
            region_x_percent=0, region_y_percent=0,
            region_w_percent=50, region_h_percent=33,
            main_menu=True,
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

        # Verify we're on the Intro Monologue screen by checking for the title text
        # near the top of the letterboxed area. Tesseract reads "---Intro Monologue---"
        # as two separate words: "---Intro" and "Monolaque---", so we only search for
        # the first part. The title may appear as "---Intro", "--Intro", or "~-Intro"
        # depending on how Tesseract parses the dashes.
        intro_verified = verify_screen(
            screenshot_path,
            {"---Intro", "--Intro", "~-Intro"},
            "Intro Monologue (title)", verbose=args.verbose
        )
        if not intro_verified:
            print("ERROR: Could not verify we are on the Intro Monologue screen")
            game_proc.kill()
            game_proc.wait()
            return 1

        # Also check for the Continue button text as a secondary confirmation.
        continue_verified = verify_screen_single(screenshot_path, "Continue", "Intro Monologue (button)", verbose=args.verbose)
        if not continue_verified:
            print("ERROR: Could not verify 'Continue' button is visible on Intro Monologue screen")
            game_proc.kill()
            game_proc.wait()
            return 1

        # Find the Continue button via OCR in the bottom 10% of the letterboxed area.
        # The Intro Monologue screen is letterboxed to 1.33:1. The Continue button
        # is in the bottom 10% of the letterboxed area, in the left half.
        continue_pos = find_text_in_letterbox_region_via_ocr(
            screenshot_path, "Continue", wx, wy, ww, wh,
            region_x_percent=0, region_y_percent=90,
            region_w_percent=50, region_h_percent=10,
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

    # Wait and verify we've moved past the Intro Monologue screen
    if args.verbose:
        print("Waiting for Intro Monologue to disappear...")
    max_wait = 30  # seconds
    waited = 0
    intro_gone = False
    while waited < max_wait:
        time.sleep(2)
        waited += 2

        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            check_screenshot_path = f.name

        try:
            if take_screenshot(check_screenshot_path, active_window_only=True, verbose=args.verbose):
                # Check if we are still on the Intro Monologue screen
                still_intro = verify_screen(
                    screenshot_path,
                    {"---Intro", "--Intro", "~-Intro"},
                    "Intro Monologue (title)", verbose=args.verbose
                )
                if not still_intro:
                    if args.verbose:
                        print(f"Intro Monologue screen gone after {waited}s")
                    intro_gone = True
                    break
                else:
                    if args.verbose:
                        print(f"Still on Intro Monologue after {waited}s, waiting...")
        finally:
            if os.path.exists(check_screenshot_path):
                os.unlink(check_screenshot_path)

    if not intro_gone:
        print(f"ERROR: Intro Monologue screen still visible after {max_wait}s")
        game_proc.kill()
        game_proc.wait()
        return 1

    # Step 5: Wait for Hangar view, click Launch
    # The Launch hotspot is on the Atlantis ocean base.
    if args.verbose:
        print("\n=== Step 5: Waiting for Hangar view, clicking Launch ===")

    # Wait for the Hangar view to load by checking for "Landing Center" text.
    # The loading splash screen has many blue pixels that confuse the hotspot
    # detector, so we must wait until the splash screen is gone.
    max_hangar_wait = 60
    hangar_loaded = False
    for waited in range(0, max_hangar_wait + 5, 5):
        time.sleep(5 if waited == 0 else 5)
        geom = refresh_window_geometry(window_id, args.verbose)
        if not geom:
            print("ERROR: Could not refresh window geometry")
            game_proc.kill()
            game_proc.wait()
            return 1
        wx, wy, ww, wh = geom

        with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
            hangar_check_path = f.name
        try:
            if not take_screenshot(hangar_check_path, active_window_only=True, verbose=args.verbose):
                if args.verbose:
                    print(f"Failed to take screenshot after {waited}s")
                continue
            # The bottom-left caption may say "Landing Center" (green, no hotspot
            # hovered), "Launch" (orange, mouse over the Launch hotspot), or
            # "Main Concourse" (orange, mouse over the Main Concourse hotspot).
            # Check for any of these texts to confirm the Hangar view is loaded.
            hangar_loaded = verify_screen(
                hangar_check_path,
                {"Landing Center", "Launch", "Main Concourse"},
                "Hangar", verbose=args.verbose
            )
            if hangar_loaded:
                if args.verbose:
                    print(f"Hangar view loaded after {waited}s")
                break
            else:
                if args.verbose:
                    print(f"Hangar not ready after {waited}s, waiting...")
        finally:
            if os.path.exists(hangar_check_path):
                os.unlink(hangar_check_path)

    if not hangar_loaded:
        print(f"ERROR: Hangar view not loaded after {max_hangar_wait}s")
        game_proc.kill()
        game_proc.wait()
        return 1

    with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as f:
        screenshot_path = f.name

    try:
        if not take_screenshot(screenshot_path, active_window_only=True, verbose=args.verbose):
            print("ERROR: Could not take screenshot")
            game_proc.kill()
            game_proc.wait()
            return 1

        # Find the Launch hotspot using pixel-based search (more reliable than OCR).
        # Hotspots are marked by bright blue dots on the screen. The pixel search
        # finds the blue indicator cluster closest to the expected design position,
        # then verifies the caption text via OCR in the bottom caption area.
        #
        # Look up the expected window position from the per-base-type geometry table
        # so the pixel search knows which blue cluster to pick.
        # The starting base, Atlantis, is an "ocean" type base.
        base_type = "ocean"
        launch_hint = get_hotspot_hint_position(base_type, "Launch", ww, wh)
        if launch_hint is not None:
            launch_hint_x, launch_hint_y = launch_hint
            if args.verbose:
                print(f"Launch hotspot hint position (window coords): ({launch_hint_x:.1f}, {launch_hint_y:.1f})")
        else:
            launch_hint_x = launch_hint_y = None
            if args.verbose:
                print(f"Unknown hotspot geometry for base type '{base_type}', 'Launch' — no hint")

        # Narrow the fallback search region to the letterboxed area around the hint
        # position, so the splash-screen blue pixels are not scanned.
        if launch_hint_x is not None and launch_hint_y is not None:
            search_radius_pct = 25.0  # Search +/-25% around the hint in the letterbox
            search_x_pct = max(0, (launch_hint_x / ww * 100) - search_radius_pct)
            search_y_pct = max(0, (launch_hint_y / wh * 100) - search_radius_pct)
            search_w_pct = min(100 - search_x_pct, search_radius_pct * 2)
            search_h_pct = min(100 - search_y_pct, search_radius_pct * 2)
        else:
            search_x_pct, search_y_pct = 0.0, 0.0
            search_w_pct, search_h_pct = 100.0, 100.0

        launch_pos = find_hotspot_via_pixel_search(
            screenshot_path, wx, wy, ww, wh,
            caption_text="Launch",
            hint_window_x=launch_hint_x,
            hint_window_y=launch_hint_y,
            search_x_percent=search_x_pct,
            search_y_percent=search_y_pct,
            search_w_percent=search_w_pct,
            search_h_percent=search_h_pct,
            screen_width=screen_width,
            screen_height=screen_height,
            verbose=args.verbose
        )
        if launch_pos is None:
            launch_pos = find_hotspot_via_pixel_search(
                screenshot_path, wx, wy, ww, wh,
                caption_text="Paunch",
                hint_window_x=launch_hint_x,
                hint_window_y=launch_hint_y,
                search_x_percent=search_x_pct,
                search_y_percent=search_y_pct,
                search_w_percent=search_w_pct,
                search_h_percent=search_h_pct,
                screen_width=screen_width,
                screen_height=screen_height,
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
            launch_pos = find_text_in_region_via_ocr_multi(
                screenshot_path, {"Launch", "Paunch", "sunch"}, wx, wy, ww, wh,
                region_x_percent=0, region_y_percent=0,
                region_w_percent=100, region_h_percent=100,
                verbose=args.verbose
            )
            if launch_pos is not None:
                launch_x, launch_y = launch_pos
                if args.verbose:
                    print(f"Found Launch hotspot via OCR at ({launch_x}, {launch_y})")
            else:
                print("ERROR: Could not find Launch hotspot — giving up")
                game_proc.kill()
                game_proc.wait()
                return 1

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

    press_key('Esc', args.verbose)
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
                    confirm_screenshot, "Yes", wx, wy, ww, wh,
                    region_x_percent=20, region_y_percent=20,
                    region_w_percent=60, region_h_percent=60,
                    verbose=args.verbose
                )
                if confirm_pos is None:
                    confirm_pos = find_text_in_region_via_ocr(
                        confirm_screenshot, "Ok", wx, wy, ww, wh,
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
