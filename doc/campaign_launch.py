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
    --display NUM       X11 display number (default: 1)
    --timeout SECS      Max seconds to wait for game exit (default: 120)
    --verbose           Print detailed progress

Dependencies:
    - pyautogui (install: pip install pyautogui)
    - xdotool (install: sudo apt install xdotool)
    - vegastrike-engine built binary
    - Assets-Production data directory

See doc/campaign-launch-test.md for details on the coordinate systems used.
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


# === Main Menu button definitions (from bases/main_menu.py) ===
# Design resolution: 1280x1024
# GUI.GUIRect(x, y, width, height, "pixel", (1280, 1024))
# Button text is at ~90% into the button width, not centered.

MAIN_MENU_DESIGN_W = 1280
MAIN_MENU_DESIGN_H = 1024
MAIN_MENU_TEXT_OFFSET_X = 0.9  # 90% into button width

# Campaign (New Game) button: GUI.GUIRect(50, 220, 177, 43, "pixel", (1280, 1024))
CAMPAIGN_X = 50
CAMPAIGN_Y = 220
CAMPAIGN_W = 177
CAMPAIGN_H = 43

# === Intro Monologue button definitions (from bases/main_menu.py) ===
# Design resolution: 1024x768
# GUI.GUIRect(8, 697, 420, 47, "pixel", (1024, 768))
# The "Continue" text on this button IS centered (50% offset).

INTRO_DESIGN_W = 1024
INTRO_DESIGN_H = 768
INTRO_TEXT_OFFSET_X = 0.5  # Centered

CONTINUE_X = 8
CONTINUE_Y = 697
CONTINUE_W = 420
CONTINUE_H = 47

# === Launch hotspot (from bases/ocean_lib.py, Atlantis/ocean base) ===
# Base.LaunchPython(room0, 'my_launch_id', 'bases/launch_music.py',
#                   -0.609375, -0.502604, 0.712891, 0.65625, 'Launch')
# Coordinates are normalized (-1 to +1), where:
#   X: -1 (left) to +1 (right)
#   Y: -1 (bottom) to +1 (top)
# Engine conversion (base_interface.cpp):
#   nx = 2.0 * (mx - vx) / vw - 1.0
#   ny = 1.0 - 2.0 * (my - vy) / vh
# Reversed:
#   mx = vx + (nx + 1) / 2 * vw
#   my = vy + (1 - ny) / 2 * vh

LAUNCH_MINX = -0.609375
LAUNCH_MINY = -0.502604
LAUNCH_WID = 0.712891
LAUNCH_HEI = 0.65625


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

    for attempt in range(120):
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
            first_wid = out.split('\n')[0].strip()
            if first_wid:
                if verbose:
                    print(f"Found window (fallback): {first_wid}")
                return first_wid

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


def calculate_main_menu_button_position(wx, wy, ww, wh, btn_x, btn_y, btn_w, btn_h,
                                         text_offset_x=MAIN_MENU_TEXT_OFFSET_X,
                                         design_w=MAIN_MENU_DESIGN_W,
                                         design_h=MAIN_MENU_DESIGN_H,
                                         verbose=False):
    """
    Calculate the screen position of a main menu button's text.

    Main menu buttons use design resolution 1280x1024.
    Button text is typically at 90% into the button width.
    """
    sx = ww / design_w
    sy = wh / design_h

    text_x = btn_x + btn_w * text_offset_x
    text_y = btn_y + btn_h / 2

    screen_x = int(wx + text_x * sx)
    screen_y = int(wy + text_y * sy)

    if verbose:
        print(f"Scale: sx={sx:.4f}, sy={sy:.4f}")
        print(f"Design text pos: ({text_x:.1f}, {text_y:.1f})")
        print(f"Screen click pos: ({screen_x}, {screen_y})")

    return screen_x, screen_y


def calculate_intro_button_position(wx, wy, ww, wh, btn_x, btn_y, btn_w, btn_h,
                                     text_offset_x=INTRO_TEXT_OFFSET_X,
                                     design_w=INTRO_DESIGN_W,
                                     design_h=INTRO_DESIGN_H,
                                     verbose=False):
    """
    Calculate the screen position of an intro monologue button's text.

    Intro monologue uses design resolution 1024x768.
    Button text is centered (50% offset).
    """
    return calculate_main_menu_button_position(
        wx, wy, ww, wh, btn_x, btn_y, btn_w, btn_h,
        text_offset_x=text_offset_x, design_w=design_w, design_h=design_h,
        verbose=verbose
    )


def calculate_launch_hotspot_position(wx, wy, ww, wh, verbose=False):
    """
    Calculate the screen position of the Launch hotspot in the hangar view.

    Uses normalized coordinates (-1 to +1) converted to screen pixels.
    Assumes the base fills the entire viewport (no letterbox).
    """
    center_x = LAUNCH_MINX + LAUNCH_WID / 2
    center_y = LAUNCH_MINY + LAUNCH_HEI / 2

    screen_x = int(wx + (center_x + 1) / 2 * ww)
    screen_y = int(wy + (1 - center_y) / 2 * wh)

    if verbose:
        print(f"Normalized center: ({center_x:.4f}, {center_y:.4f})")
        print(f"Screen click pos: ({screen_x}, {screen_y})")

    return screen_x, screen_y


def click_at(screen_x, screen_y, label="", verbose=False):
    """Move mouse to position and click using PyAutoGUI.

    PyAutoGUI is used for clicks because it works reliably at the X11 input level.
    The window should be focused before calling this function.
    """
    if verbose:
        print(f"Moving mouse to ({screen_x}, {screen_y}) for '{label}'...")

    pyautogui.moveTo(screen_x, screen_y, duration=0.3)
    time.sleep(0.5)

    if verbose:
        pos = pyautogui.position()
        print(f"Mouse at: {pos}")

    pyautogui.click()
    if verbose:
        print(f"Clicked '{label}'!")


def refresh_window_geometry(window_id, verbose=False):
    """
    Re-query the window geometry in case the user moved the window.

    Returns:
        Tuple of (x, y, width, height) or None on failure
    """
    geom = get_window_geometry(window_id, verbose)
    if verbose and geom:
        print(f"Refreshed window geometry: {geom}")
    return geom


def press_key(key, verbose=False):
    """Press a key using PyAutoGUI."""
    if verbose:
        print(f"Pressing '{key}'...")
    pyautogui.press(key)
    time.sleep(0.5)


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
    parser.add_argument('--display', type=int, default=1,
                        help='X11 display number (default: 1)')
    parser.add_argument('--timeout', type=int, default=120,
                        help='Max seconds to wait for game exit (default: 120)')
    parser.add_argument('--verbose', action='store_true',
                        help='Print detailed progress')
    args = parser.parse_args()

    os.environ['DISPLAY'] = f':{args.display}'

    # Default target path if not specified
    if args.target is None:
        candidates = [
            os.path.expanduser('../Assets-Production/'),
            os.path.expanduser('~/Projects/Assets-Production/'),
        ]
        for candidate in candidates:
            if os.path.isdir(candidate):
                args.target = candidate
                break
        else:
            print("ERROR: --target not specified and Assets-Production not found")
            return 1

    if not os.path.isfile(args.binary):
        print(f"ERROR: Binary not found: {args.binary}")
        return 1

    if args.verbose:
        print(f"Binary: {args.binary}")
        print(f"Target: {args.target}")
        print(f"Display: :{args.display}")

    # Step 1: Start the game
    if args.verbose:
        print("\n=== Step 1: Starting Vega Strike ===")

    game_proc = subprocess.Popen(
        [args.binary, '--target', args.target, f'--debug={args.debug}'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    # Step 2: Wait for window to appear
    window_id = find_vega_strike_window(args.verbose)
    if not window_id:
        print("ERROR: Could not find Vega Strike window")
        game_proc.kill()
        game_proc.wait()
        return 1

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

    # Step 3: Click Campaign button
    # Re-query window geometry in case the user moved the window during loading
    if args.verbose:
        print("\n=== Step 3: Clicking Campaign button ===")
    geom = refresh_window_geometry(window_id, args.verbose)
    if not geom:
        print("ERROR: Could not refresh window geometry")
        game_proc.kill()
        game_proc.wait()
        return 1
    wx, wy, ww, wh = geom

    campaign_x, campaign_y = calculate_main_menu_button_position(
        wx, wy, ww, wh, CAMPAIGN_X, CAMPAIGN_Y, CAMPAIGN_W, CAMPAIGN_H,
        verbose=args.verbose
    )
    click_at(campaign_x, campaign_y, "Campaign", args.verbose)

    # Step 4: Wait for Intro Monologue, click Continue
    if args.verbose:
        print("\n=== Step 4: Waiting for Intro Monologue, clicking Continue ===")
    time.sleep(5)  # Wait for intro screen to appear

    # Re-query window geometry — user may have moved the window
    geom = refresh_window_geometry(window_id, args.verbose)
    if not geom:
        print("ERROR: Could not refresh window geometry")
        game_proc.kill()
        game_proc.wait()
        return 1
    wx, wy, ww, wh = geom

    continue_x, continue_y = calculate_intro_button_position(
        wx, wy, ww, wh, CONTINUE_X, CONTINUE_Y, CONTINUE_W, CONTINUE_H,
        verbose=args.verbose
    )
    click_at(continue_x, continue_y, "Continue", args.verbose)

    # Step 5: Wait for Hangar view, click Launch
    # The Launch hotspot is on the Atlantis ocean base.
    # Verification: the blue dot brightens as the mouse approaches, the custom cursor
    # turns orange, and "Launch" appears in orange near the bottom-left of the screen.
    if args.verbose:
        print("\n=== Step 5: Waiting for Hangar view, clicking Launch ===")
    time.sleep(10)  # Wait for hangar to load

    # Re-query window geometry — user may have moved the window
    geom = refresh_window_geometry(window_id, args.verbose)
    if not geom:
        print("ERROR: Could not refresh window geometry")
        game_proc.kill()
        game_proc.wait()
        return 1
    wx, wy, ww, wh = geom

    launch_x, launch_y = calculate_launch_hotspot_position(wx, wy, ww, wh, args.verbose)
    click_at(launch_x, launch_y, "Launch", args.verbose)

    # Step 6: Wait for launch animation, press Esc then Q
    if args.verbose:
        print("\n=== Step 6: Waiting for launch, pressing Esc then Q ===")
    time.sleep(5)  # Wait for launch animation
    press_key('escape', args.verbose)
    time.sleep(1)
    press_key('q', args.verbose)

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
