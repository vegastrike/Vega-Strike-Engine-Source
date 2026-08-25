/*
 * Tux Racer
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * Copyright (C) 2001-2026 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors as part of Vega Strike (see below)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Incorporated into Vega Strike
 *
 * Copyright (C) 2001-2026 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>
#include <sstream>

#include "gl_globals.h"
#include "gl_init.h"
#include "winsys.h"
#include "root_generic/vs_globals.h"
#include "root_generic/xml_support.h"
#include "src/config_xml.h"
#include "root_generic/vs_globals.h"
#include "src/vs_logging.h"
#include "root_generic/options.h"
#include "src/vs_exit.h"
#include "configuration/configuration.h"
#include "libraries/gui/gui.h"
#include "backends/imgui_impl_sdl2.h"
#include "gui/config_screen.h"
#include "in_kb.h"
#include "in_joystick.h"

#include "SDL2/SDL_video.h"

#include "gldrv/mouse_cursor.h"

/*
 * Windowing System Abstraction Layer
 * Abstracts creation of windows, handling of events, etc.
 */

/*
 * *---------------------------------------------------------------------------
 * *---------------------------------------------------------------------------
 * SDL version
 *******************************---------------------------------------------------------------------------
 *******************************---------------------------------------------------------------------------
 */

static SDL_Window *window = nullptr;
static SDL_Renderer *renderer = nullptr;   // created at bootstrap; reused to re-issue logical size on resize
static SDL_Surface *screen = nullptr;

static winsys_display_func_t display_func = nullptr;
static winsys_idle_func_t idle_func = nullptr;
static winsys_reshape_func_t reshape_func = nullptr;
static winsys_keyboard_func_t keyboard_func = nullptr;
static winsys_mouse_func_t mouse_func = nullptr;
static winsys_motion_func_t motion_func = nullptr;
static winsys_motion_func_t passive_motion_func = nullptr;
static winsys_atexit_func_t atexit_func = nullptr;

static bool redisplay = false;
static bool keepRunning = true;

/*---------------------------------------------------------------------------*/
/*!
 *  Requests that the screen be redrawn
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_post_redisplay() {
    redisplay = true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the display callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_display_func(winsys_display_func_t func) {
    display_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the idle callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_idle_func(winsys_idle_func_t func) {
    idle_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the reshape callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_reshape_func(winsys_reshape_func_t func) {
    reshape_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the keyboard callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_keyboard_func(winsys_keyboard_func_t func) {
    keyboard_func = func;
}

// Whether the in-game config overlay is open. While active, winsys suppresses
// forwarding mouse/keyboard to the game handlers (the overlay consumes input) so
// clicks don't pass through to the game behind.
static bool config_overlay_active = false;

void winsys_set_config_overlay_active(bool active) {
    config_overlay_active = active;
}

bool winsys_config_overlay_active() {
    return config_overlay_active;
}

// Re-attach joysticks when a device is hotplugged (SDL_JOYDEVICEADDED). We use
// fake-present slots set up at startup (bindings are bound there), so a new
// device is Attach()ed to the first free physical slot without re-binding
// (which would cause input inconsistency). On removal the slot just stays; the
// device detaches. bindings stay valid because they are keyed by slot index.
static void winsys_refresh_joysticks() {
    int n = SDL_NumJoysticks();
    if (n > MAX_JOYSTICKS) {
        n = MAX_JOYSTICKS;
    }
    // Attach each present device index to the matching free physical slot.
    for (int dev = 0; dev < n; ++dev) {
        for (int i = 0; i < MAX_JOYSTICKS; ++i) {
            if (i == MOUSE_JOYSTICK) {
                continue;
            }
            if (joystick[i] != nullptr && joystick[i]->joy == nullptr) {
                joystick[i]->Attach(dev);
                VS_LOG(important_info, (boost::format("[hotplug] attached device %1% to slot %2%") % dev % i));
                break;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse button-press callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_mouse_func(winsys_mouse_func_t func) {
    mouse_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when a mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_motion_func(winsys_motion_func_t func) {
    motion_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when no mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_passive_motion_func(winsys_motion_func_t func) {
    passive_motion_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Copies the OpenGL back buffer to the front buffer
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_swap_buffers() {
    SDL_Window* current_window = SDL_GL_GetCurrentWindow();
    SDL_GL_SwapWindow(current_window);
}

/*---------------------------------------------------------------------------*/
/*!
 *  Moves the mouse pointer to (x,y)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_warp_pointer(int x, int y) {
    SDL_Window* current_window = SDL_GL_GetCurrentWindow();
    SDL_WarpMouseInWindow(current_window, x, y);
}

// Store real resolution
int native_resolution_x;
int native_resolution_y;


/*---------------------------------------------------------------------------*/
/*!
 *  Find the SDL display mode matching the requested resolution on the given
 *  display, returning true if matched. Falls back to the desktop (native) mode
 *  when the requested resolution isn't an available fullscreen mode, so we never
 *  fail to enter fullscreen (per Evert: detect available resolutions and only
 *  offer those; if not gotten, fall back to native).
 */
static bool find_fullscreen_display_mode(int display_index, int width, int height,
        SDL_DisplayMode *out_mode) {
    int nmodes = SDL_GetNumDisplayModes(display_index);
    for (int i = 0; i < nmodes; ++i) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(display_index, i, &mode) != 0) continue;
        if (mode.w == width && mode.h == height) {
            *out_mode = mode;
            return true;
        }
    }
    // Not an available fullscreen mode -> fall back to the desktop (native) mode.
    if (SDL_GetDesktopDisplayMode(display_index, out_mode) == 0) {
        return true;
    }
    return false;
}

/*!
 *  Sets up the SDL OpenGL rendering context
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2025-01-10 - stephengtuggy
 */
static bool setup_sdl_video_mode(int *argc, char **argv) {
    const int screen_number = configuration().graphics.screen;
    Uint32 video_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    int bpp = 0; // Bits per pixel?
    int width, height;
    // Fullscreen uses a real display mode change (SDL_WINDOW_FULLSCREEN +
    // SDL_SetWindowDisplayMode), so the selected resolution is honored rather
    // than always rendering at the desktop resolution (SDL_WINDOW_FULLSCREEN_DESKTOP).
    // If the requested resolution isn't an available fullscreen mode, fall back to
    // the native/desktop mode (see find_fullscreen_display_mode).
    SDL_DisplayMode requested_fullscreen_mode;
    bool have_fullscreen_mode = false;
    if (configuration().graphics.full_screen) {
        video_flags |= SDL_WINDOW_FULLSCREEN;

        // Choose the display mode: the configured resolution if available, else native.
        have_fullscreen_mode = find_fullscreen_display_mode(screen_number,
                configuration().graphics.resolution_x, configuration().graphics.resolution_y,
                &requested_fullscreen_mode);
        if (!have_fullscreen_mode) {
            VS_LOG_FLUSH_EXIT(fatal, (boost::format("Could not find any fullscreen display mode for display %1%") % screen_number), -1);
        }
        native_resolution_x = requested_fullscreen_mode.w;
        native_resolution_y = requested_fullscreen_mode.h;
    } else {
        video_flags |= SDL_WINDOW_RESIZABLE;

        native_resolution_x = configuration().graphics.resolution_x;
        native_resolution_y = configuration().graphics.resolution_y;
    }
    bpp = gl_options.color_depth;

    int rs, gs, bs;
    rs = gs = bs = (bpp == 16) ? 5 : 8;
    if (configuration().graphics.rgb_pixel_format == "undefined") {
        (const_cast<vega_config::Configuration &>(configuration())).graphics.rgb_pixel_format = ((bpp == 16) ? "555" : "888");
    }
    if ((configuration().graphics.rgb_pixel_format.length() == 3) && isdigit(configuration().graphics.rgb_pixel_format[0])
            && isdigit(configuration().graphics.rgb_pixel_format[1]) && isdigit(configuration().graphics.rgb_pixel_format[2])) {
        rs = configuration().graphics.rgb_pixel_format[0] - '0';
        gs = configuration().graphics.rgb_pixel_format[1] - '0';
        bs = configuration().graphics.rgb_pixel_format[2] - '0';
    }
    int otherbpp;
    int otherattributes;
    if (bpp == 16) {
        otherattributes = 8;
        otherbpp = 32;
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rs);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gs);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bs);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, configuration().graphics.z_pixel_format);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    } else {
        otherattributes = 5;
        otherbpp = 16;
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rs);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gs);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bs);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, configuration().graphics.z_pixel_format);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }
    if (configuration().graphics.gl_accelerated_visual) {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    }
    width = configuration().graphics.resolution_x;
    height = configuration().graphics.resolution_y;

    // Fix display in fullscreen
    if(configuration().graphics.full_screen) {
        // Change base resolution to match screen resolution
        width = configuration().graphics.resolution_x;//currentDisplayMode.w;
        height = configuration().graphics.resolution_y;//currentDisplayMode.h;
        int* ptr_x = const_cast<int*>(&configuration().graphics.bases.max_width);
        int* ptr_y = const_cast<int*>(&configuration().graphics.bases.max_height);
        *ptr_x = width;
        *ptr_y = height;
    }


    window = nullptr;
    if(screen_number == 0) {
        window = SDL_CreateWindow("Vega Strike",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                width, height, video_flags);
    } else {
        window = SDL_CreateWindow("Vega Strike",
                                SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen_number),
                                SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen_number),
                                0, 0, video_flags);
    }

    if(!window) {
        VS_LOG_FLUSH_EXIT(fatal, "No window", 1);
    }

    // Always mark the window resizable (SDL_SetWindowResizable works dynamically,
    // independent of the creation flags). On a fullscreen launch the window was
    // created without SDL_WINDOW_RESIZABLE, so switching to windowed would lose
    // the resize handles/maximize button. Setting it explicitly here ensures a
    // windowed resize is possible whether we started fullscreen or windowed.
    SDL_SetWindowResizable(window, SDL_TRUE);

    if(screen_number > 0) {
        // Get bounds of the secondary monitor
        SDL_Rect displayBounds;
        if (SDL_GetDisplayBounds(screen_number, &displayBounds) != 0) {
            const std::string error_message = (boost::format("Failed to get display bounds: %1%") % SDL_GetError()).str();
            VS_LOG_AND_FLUSH(error, error_message);

            // Fallback to primary monitor
            SDL_GetDisplayBounds(0, &displayBounds);
        }

        // Move to secondary monitor
        SDL_SetWindowPosition(window, displayBounds.x, displayBounds.y);
    }

    if (configuration().graphics.full_screen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        if (have_fullscreen_mode) {
            SDL_SetWindowDisplayMode(window, &requested_fullscreen_mode);
        }
    }

    if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl")) {
        VS_LOG_AND_FLUSH(important_info, "SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...) succeeded");
    } else {
        VS_LOG_AND_FLUSH(error, (boost::format("SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...) failed. Error: %1%") % SDL_GetError()));
        SDL_ClearError();
    }

    // Measure the ACTUAL window size after creation. In windowed mode the WM /
    // compositor may clamp a requested size down to fit the screen, so the game
    // must adopt the window it actually got rather than assume it got the
    // requested size (otherwise the viewport/layout render at the oversized
    // request while the window is smaller -> cut off). The GL drawable size is
    // the render target (native_resolution -> glViewport); the logical window
    // size is what the layout/HUD/ImGui/config-screen read (graphics.resolution).
    // On a non-scaling display these are equal; on a scaled display they differ
    // (points vs pixels) and we keep them distinct.
    int logical_w = 0, logical_h = 0;
    SDL_GetWindowSize(window, &logical_w, &logical_h);
    int drawable_w = 0, drawable_h = 0;
    SDL_GL_GetDrawableSize(window, &drawable_w, &drawable_h);
    if (logical_w <= 0) { logical_w = width; }
    if (logical_h <= 0) { logical_h = height; }
    if (drawable_w <= 0) { drawable_w = logical_w; }
    if (drawable_h <= 0) { drawable_h = logical_h; }
    native_resolution_x = drawable_w;
    native_resolution_y = drawable_h;
    (const_cast<vega_config::Configuration &>(configuration())).graphics.resolution_x = logical_w;
    (const_cast<vega_config::Configuration &>(configuration())).graphics.resolution_y = logical_h;
    // Keep the screen aspect consistent with the actual window resolution.
    if (logical_h > 0) {
        (const_cast<vega_config::Configuration &>(configuration())).graphics.aspect_flt =
                (float)logical_w / (float)logical_h;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (!context) {
        std::cerr << "No GL context\n" << std::flush;
        VS_LOG_FLUSH_EXIT(fatal, "No GL context", 1);
    }

    VS_LOG_AND_FLUSH(important_info, (boost::format("GL Vendor: %1%") % glGetString(GL_VENDOR)));
    VS_LOG_AND_FLUSH(important_info, (boost::format("GL Renderer: %1%") % glGetString(GL_RENDERER)));
    VS_LOG_AND_FLUSH(important_info, (boost::format("GL Version: %1%") % glGetString(GL_VERSION)));

    if (SDL_GL_MakeCurrent(window, context) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, "Failed to make window context current", 1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        VS_LOG_AND_FLUSH(error, (boost::format(
            "SDL_CreateRenderer(...) with VSync option failed; trying again without VSync option. Error was: %1%") %
            SDL_GetError()));
        SDL_ClearError();

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            VS_LOG_AND_FLUSH(error, (boost::format(
                "SDL_CreateRenderer(...) with SDL_RENDERER_ACCELERATED failed; trying again with software rendering option. Error was: %1%") %
                SDL_GetError()));
            SDL_ClearError();

            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
            if (renderer == nullptr) {
                VS_LOG_FLUSH_EXIT(fatal, (boost::format(
                    "SDL_CreateRenderer(...) failed on the third try, with software rendering! Error: %1%") %
                    SDL_GetError()),
                    1);
            }
        }
    }

    if (SDL_RenderSetLogicalSize(renderer, width, height) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_RenderSetLogicalSize(...) failed! Error: %1%") % SDL_GetError()),
            8);
    }

#if defined (GL_RENDERER)
    std::string version{};
    const GLubyte * renderer_string = glGetString(GL_RENDERER);
    if (renderer_string) {
        version = reinterpret_cast<const char *>(renderer_string);
    }
    if (version == "GDI Generic" || version == "software") {
        if (configuration().graphics.gl_accelerated_visual) {
            VS_LOG_AND_FLUSH(error, "GDI Generic software driver reported, trying to reset.");
            SDL_ClearError();
            freeMouseCursors();
            SDL_Quit();
            (const_cast<vega_config::Configuration &>(configuration())).graphics.gl_accelerated_visual = false;
            return false;
        } else {
            VS_LOG(error, "GDI Generic software driver reported, reset failed.");
            VS_LOG_AND_FLUSH(error, "Please make sure a graphics card driver is installed and functioning properly.");
        }
    }
#endif

    // This makes our buffer swap synchronized with the monitor's vertical refresh
    if (SDL_GL_SetSwapInterval(1) < 0) {
        VS_LOG_AND_FLUSH(error, "SDL_GL_SetSwapInterval(1) failed");
        SDL_ClearError();
    }

    // Initialize imgui
    InitGui();
    // Apply the persisted font size (config was loaded before this). Without
    // this the ImGui UI resets to the 18.0f hardcoded default in InitGui()
    // regardless of the saved graphics.font_point; the pending request is
    // applied at the next frame start via ImGui_ApplyPendingFontSize().
    RequestImGuiFontSize(configuration().graphics.font_point_flt);

    return true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Initializes the OpenGL rendering context, and creates a window (or
 *  sets up fullscreen mode if selected)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2020-07-27 stephengtuggy
 */

void winsys_init(int *argc, char **argv, char const *window_title, char const *icon_title) {
    keepRunning = true;

    if (SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2")) {
        VS_LOG_AND_FLUSH(important_info, "SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, ...) succeeded");
    } else {
        VS_LOG_AND_FLUSH(warning, "SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, ...) failed");
        SDL_ClearError();
    }

    //SDL_INIT_AUDIO|
#if defined(NO_SDL_JOYSTICK)
    constexpr Uint32 sdl_flags = SDL_INIT_VIDEO;
#else
    constexpr Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
#endif

    gl_options.color_depth = configuration().graphics.color_depth;
    /*
     * Initialize SDL
     */
    if (SDL_Init(sdl_flags) < 0) {
        VS_LOG_AND_FLUSH(fatal, (boost::format("Couldn't initialize SDL: %1%") % SDL_GetError()));
        exit(1);              // stephengtuggy 2020-07-27 - I would use VSExit here, but that calls winsys_exit, which I'm not sure will work if winsys_init hasn't finished yet.
    }

    // Init Mouse
    initMouseCursors();
    changeCursor(CursorType::arrow);

    //signal( SIGSEGV, SIG_DFL );
    SDL_Surface *icon = nullptr;
    if (icon_title) {
        icon = SDL_LoadBMP(icon_title);
    }
    if (icon) {
        SDL_SetColorKey(icon, SDL_TRUE, ((Uint32 *) (icon->pixels))[0]);
    }
    /*
     * Init video
     */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if defined (USE_STENCIL_BUFFER)
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    if (!setup_sdl_video_mode(argc, argv)) {
        winsys_init(argc, argv, window_title, icon_title);
    } else {
        glutInit(argc, argv);
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Deallocates resources in preparation for program termination
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_cleanup() {
    static bool cleanup = false;
    if (!cleanup) {
        CleanupGui();
        cleanup = true;
        freeMouseCursors();
        SDL_Quit();
    }
}

void winsys_shutdown() {
    keepRunning = false;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Shows/hides mouse cursor
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_show_cursor(bool visible) {
    static bool vis = true;
    if (visible != vis) {
        SDL_ShowCursor(visible);
        vis = visible;
    }
}

// Apply a new resolution/fullscreen to the live window (from the in-game config
// screen). Sets configuration(), then applies windowed/fullscreen + size via
// SDL2, and forces the reshape so the viewport/measurements update immediately.
void winsys_apply_resolution(int width, int height, bool fullscreen) {
    auto &g = const_cast<vega_config::Configuration &>(configuration()).graphics;
    g.full_screen = fullscreen;

    if (window == nullptr) {
        g.resolution_x = width;
        g.resolution_y = height;
        return;
    }

    if (fullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        // Request a real fullscreen display mode so the selected resolution is
        // honored (not always desktop-res). If the exact size isn't an available
        // fullscreen mode, fall back to native. Update width/height to the mode
        // actually used so the measurements below track the real render size.
        SDL_DisplayMode fs_mode;
        const int screen_number = g.screen;
        if (find_fullscreen_display_mode(screen_number, width, height, &fs_mode)) {
            SDL_SetWindowDisplayMode(window, &fs_mode);
            width = fs_mode.w;
            height = fs_mode.h;
        }
    } else {
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, width, height);
    }

    // Measure the ACTUAL window size after the resize. In windowed mode the WM /
    // compositor may clamp a requested size down to fit the screen, and on a
    // HiDPI/Wayland display the logical window size (points) differs from the GL
    // drawable size (pixels). Using the measured sizes (rather than the requested
    // width/height) keeps the scene viewport, ImGui overlay, and config write-out
    // all consistent with the real window. The GL drawable size is what the legacy
    // viewport must map to; the logical window size is what ImGui
    // (io.DisplaySize = SDL_GetWindowSize) and the config screen use.
    int logical_w = 0, logical_h = 0;
    SDL_GetWindowSize(window, &logical_w, &logical_h);
    int drawable_w = 0, drawable_h = 0;
    SDL_GL_GetDrawableSize(window, &drawable_w, &drawable_h);
    if (logical_w <= 0) { logical_w = width; }
    if (logical_h <= 0) { logical_h = height; }
    if (drawable_w <= 0) { drawable_w = logical_w; }
    if (drawable_h <= 0) { drawable_h = logical_h; }

    // In fullscreen, the legacy GL viewport should fill the whole drawable. In
    // windowed on a scaled display the drawable is the pixel size; the legacy GL
    // scene must render into it. The config/graphics resolution (used by ImGui and
    // the config screen) is the logical window size.
    native_resolution_x = drawable_w;
    native_resolution_y = drawable_h;
    g.resolution_x = logical_w;
    g.resolution_y = logical_h;

    // Re-issue the renderer logical size (bound once at bootstrap). Present is via
    // SDL_GL_SwapWindow, so this mainly keeps SDL_GetRenderOutputSize consistent
    // for anything that reads it.
    if (renderer != nullptr) {
        SDL_RenderSetLogicalSize(renderer, drawable_w, drawable_h);
    }

    // Recompute the screen aspect from the actual resolution so the in-game
    // camera/cockpit viewport isn't distorted after a live change.
    if (g.resolution_y > 0) {
        g.aspect_flt = (float)g.resolution_x / (float)g.resolution_y;
    }

    // Force the reshape so native_resolution_x/y and the GL viewport update; in
    // windowed this normally fires via SDL_WINDOWEVENT_RESIZED, but fullscreen
    // mode changes may not, so apply it directly here.
    if (reshape_func) {
        (*reshape_func)(native_resolution_x, native_resolution_y);
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Processes and dispatches events.  This function never returns.
 *  \return  No.
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 *  \date    Modified: 2005-08-16 - Rogue
 *  \date    Modified: 2005-12-24 - ace123
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */
extern int shiftdown(int);
extern int shiftup(int);

// Debounce state for window resizes. A live drag fires many SIZE_CHANGED/RESIZED
// events in quick succession; we re-init only after the resize has been quiet for
// RESIZE_DEBOUNCE_MS, so we don't thrash the re-init on every intermediate frame.
static int pending_resize_w = -1;
static int pending_resize_h = -1;
static Uint32 resize_deadline = 0;
static const Uint32 RESIZE_DEBOUNCE_MS = 100;

// Handle a window size change (resize or size-changed). Re-binds the game to the
// new actual window size and requests a redraw so the game re-renders at the new
// size. This is the actual re-init; callers debounce it via the resize_request below.
static void handle_window_resize(int new_w, int new_h) {
    // Use the ACTUAL window size (SDL_GetWindowSize = logical points) for
    // graphics.resolution_x/y, NOT the event data. On a scaled/HiDPI display the
    // resize event reports the drawable pixel size, which differs from the logical
    // window size that ImGui uses (io.DisplaySize = SDL_GetWindowSize). If we set
    // graphics.resolution from pixels, the ImGui config overlay is sized at the
    // pixel size but ImGui hit-tests in point space -> an "invisible cursor"
    // offset (mouseover fires for the wrong position). Using the logical window
    // size keeps graphics.resolution consistent with io.DisplaySize.
    if (window != nullptr) {
        int lw = 0, lh = 0;
        SDL_GetWindowSize(window, &lw, &lh);
        if (lw > 0) { new_w = lw; }
        if (lh > 0) { new_h = lh; }
    }
    (const_cast<vega_config::Configuration &>(configuration())).graphics.resolution_x = new_w;
    (const_cast<vega_config::Configuration &>(configuration())).graphics.resolution_y = new_h;
    if (new_h > 0) {
        (const_cast<vega_config::Configuration &>(configuration())).graphics.aspect_flt =
                (float)new_w / (float)new_h;
    }
    // Re-bind the GL viewport to the actual drawable size, so the scene renders at
    // the new size rather than the old.
    if (window != nullptr) {
        int d_w = 0, d_h = 0;
        SDL_GL_GetDrawableSize(window, &d_w, &d_h);
        if (d_w > 0 && d_h > 0) {
            native_resolution_x = d_w;
            native_resolution_y = d_h;
        }
    }
    GFXReinitConfig();
    if (reshape_func) {
        (*reshape_func)(new_w, new_h);
    }
    // A resize must trigger a redraw (the event loop only redraws on redisplay,
    // otherwise it relies on idle_func which can be paused, e.g. in the overlay).
    redisplay = true;
}

void winsys_process_events() {
    SDL_Event event;
    int x, y;
    bool state;

    static unsigned int keysym_to_unicode[256];
    static bool keysym_to_unicode_init = false;
    if (!keysym_to_unicode_init) {
        keysym_to_unicode_init = true;
        memset(keysym_to_unicode, 0, sizeof(keysym_to_unicode));
    }
    while (keepRunning) {
        SDL_LockAudio();
        SDL_UnlockAudio();
        while (SDL_PollEvent(&event)) {
            // forward all events to ImGui (drives the config screen, and any
            // other ImGui UI) without altering the game's own dispatch below.
            ImGui_ImplSDL2_ProcessEvent(&event);
            // Forward to the config screen (binding capture, joystick hotplug)
            // while it is open.
            vs_settings_ng::HandleConfigEvent(&event);

            state = false;
            switch (event.type) {
                case SDL_KEYUP:
                    state = true;
                    //does same thing as KEYDOWN, but with different state.
                case SDL_KEYDOWN:

                    // Global (always-active) actions fire in any context (in-flight,
                    // docked, nav, text) -- e.g. Alt+C (ConfigKey) opens the settings
                    // screen regardless of where the player is. Handled globally so it
                    // is not double-fired by the context-specific dispatch below.
                    SDL_GetMouseState(&x, &y);
                    if (HandleGlobalKey(event.key.keysym.sym, event.key.keysym.mod, !state, x, y)) {
                        break;
                    }

                    if (!config_overlay_active && keyboard_func) {
//                        VS_LOG(debug, (boost::format("Kbd: %1$s mod:%2$x sym:%3$x scan:%4$x")
//                                       % ((event.type == SDL_KEYUP) ? "KEYUP" : "KEYDOWN")
//                                       % event.key.keysym.mod
//                                       % event.key.keysym.sym
//                                       % event.key.keysym.scancode
//                                      ));

                        //Send the event
                        (*keyboard_func)(event.key.keysym.sym, event.key.keysym.mod,
                                state,
                                x, y);
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if (!config_overlay_active && mouse_func) {
                        (*mouse_func)(event.button.button,
                                event.button.state,
                                event.button.x,
                                event.button.y);
                    }
                    break;

                case SDL_MOUSEWHEEL:
                    // SDL2 reports the wheel as its own event (SDL_MOUSEWHEEL,
                    // not a button press). Synthesize wheel-up / wheel-down
                    // button presses so the existing mouse-button pipeline
                    // (WS_WHEEL_UP->3 / WS_WHEEL_DOWN->4 in lookupMouseButton)
                    // keeps working -- this drives scrolling in the base
                    // computer, navscreen, pickers, etc.
                    if (mouse_func) {
                        int mx = 0;
                        int my = 0;
                        SDL_GetMouseState(&mx, &my);
                        if (event.wheel.y > 0) {
                            (*mouse_func)(WS_WHEEL_UP, WS_MOUSE_DOWN, mx, my);
                            (*mouse_func)(WS_WHEEL_UP, WS_MOUSE_UP, mx, my);
                        } else if (event.wheel.y < 0) {
                            (*mouse_func)(WS_WHEEL_DOWN, WS_MOUSE_DOWN, mx, my);
                            (*mouse_func)(WS_WHEEL_DOWN, WS_MOUSE_UP, mx, my);
                        }
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (!config_overlay_active) {
                        if (event.motion.state) {
                            /* buttons are down */
                            if (motion_func) {
                                (*motion_func)(event.motion.x,
                                        event.motion.y);
                            }
                        } else
                            /* no buttons are down */
                        if (passive_motion_func) {
                            (*passive_motion_func)(event.motion.x,
                                    event.motion.y);
                        }
                    }
                    break;

                case SDL_WINDOWEVENT:
                    // event.type == SDL_WINDOWEVENT (0x200); the actual sub-type is
                    // event.window.event (RESIZED=5, SIZE_CHANGED=6, MOVED=3, ...).
                    // Record the latest resize; the actual re-init is performed once
                    // the resize has been quiet for RESIZE_DEBOUNCE_MS (debounced,
                    // checked after the event pump), so a live drag doesn't thrash it.
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            pending_resize_w = event.window.data1;
                            pending_resize_h = event.window.data2;
                            resize_deadline = SDL_GetTicks() + RESIZE_DEBOUNCE_MS;
                            break;
                        default:
                            // Other window events (moved, exposed, shown, ...)
                            // need no re-init; ignore.
                            break;
                    }
                    break;

                case SDL_JOYDEVICEADDED:
                    // A joystick appeared: attach the new device to a free slot
                    // (no re-bind — bindings are set at startup).
                    winsys_refresh_joysticks();
                    break;

                case SDL_JOYDEVICEREMOVED:
                    // Joystick removed: the fake slot stays; the device simply
                    // detaches. Nothing to re-init.
                    break;

                case SDL_QUIT:
                    cleanexit = true;
                    keepRunning = false;
                    break;
                default:
                    break;
            }
            SDL_LockAudio();
            SDL_UnlockAudio();
        }
        // Apply a pending debounced resize now that the resize events have quieted
        // down (no new event for RESIZE_DEBOUNCE_MS). This re-binds the viewport /
        // config to the final size and requests a redraw.
        if (pending_resize_w >= 0 && SDL_GetTicks() >= resize_deadline) {
            handle_window_resize(pending_resize_w, pending_resize_h);
            pending_resize_w = -1;
            pending_resize_h = -1;
        }
        if (redisplay && display_func) {
            redisplay = false;
            (*display_func)();
        } else if (idle_func) {
            (*idle_func)();
            /* Delay for 1 ms.  This allows the other threads to do some
             *  work (otherwise the audio thread gets starved). */
        }
        SDL_Delay(1);
    }
    winsys_cleanup();
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the function to be called when program ends.  Note that this
 *  function should only be called once.
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2021-09-06 - stephengtuggy
 */
void winsys_atexit(winsys_atexit_func_t func) {
    static bool called = false;
    if (called != false) {
        VS_LOG_AND_FLUSH(error, "winsys_atexit called twice");
    }
    called = true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Exits the program
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2000-10-20
 */
void winsys_exit(int code) {
    // Reverting resolution by exiting fullscreen
    SDL_SetWindowFullscreen(window, 0);

    winsys_shutdown();
    if (atexit_func) {
        (*atexit_func)();
    }
    exit( code );
}

/* EOF */
