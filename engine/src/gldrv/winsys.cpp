/*
 * Tux Racer
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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

#include <SDL3/SDL_video.h>

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
static SDL_Surface *screen = nullptr;
static SDL_Renderer *renderer = nullptr;

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

SDL_Rect total_display_bounds{};
SDL_Rect usable_display_bounds{};
SDL_Rect total_window_size_in_pixels{};
SDL_Rect usable_window_size_in_pixels{};
SDL_Rect total_logical_window_size{};
SDL_Rect usable_logical_window_size{};

/*---------------------------------------------------------------------------*/
// pmx-20251026
bool get_sdl_display_name_by_number(int screen_number, std::string& screen_name, SDL_DisplayID& id) {
    std::ostringstream display_names;
    int num_displays = 0;
    bool found = false;
    SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
    if (displays) {
        for (int i = 0; i < num_displays; ++i) {
            const SDL_DisplayID instance_id = displays[i];
            const char* name = SDL_GetDisplayName(instance_id);
            if (i == screen_number) {
                screen_name = name ? name : "Unknown";
                id = instance_id;
                found = true;
            }
        }
        SDL_free(displays);
    }

    return found;
}

#define VS_LOG_SDL_ERROR(operation)                                                                                         \
    do {                                                                                                                    \
        const std::string log_msg = (boost::format("%1%:%2%: %3%: %4% encountered SDL Error %5%") % __FILE__ % __LINE__    \
                % __FUNCTION__ % (operation) % SDL_GetError()).str();                                                       \
        VegaStrikeLogging::VegaStrikeLogger::instance().LogAndFlush(VegaStrikeLogging::vega_log_level::error, log_msg);     \
        SDL_ClearError();                                                                                                   \
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", log_msg.c_str());                                                  \
    } while (false)
#define VS_LOG_SDL_SUCCESS(operation)                                                                                       \
    do {                                                                                                                    \
        const std::string log_msg = (boost::format("%1%:%2%: %3%: %4% succeeded") % __FILE__ % __LINE__ % __FUNCTION__      \
                % (operation)).str();                                                                                       \
        VegaStrikeLogging::VegaStrikeLogger::instance().LogAndFlush(VegaStrikeLogging::vega_log_level::important_info,      \
                log_msg);                                                                                                   \
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", log_msg.c_str());                                                   \
    } while (false)
#define VS_LOG_SDL_INFO(message)                                                                                            \
    do {                                                                                                                    \
        const std::string log_msg = (boost::format("%1%:%2%: %3%: %4%") % __FILE__ % __LINE__ % __FUNCTION__                \
                % (message)).str();                                                                                         \
        VegaStrikeLogging::VegaStrikeLogger::instance().LogAndFlush(VegaStrikeLogging::vega_log_level::important_info,      \
                log_msg);                                                                                                   \
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", log_msg.c_str());                                                   \
    } while (false)

static bool try_creating_window_and_renderer(const char *title, const int width, const int height,
                                             const char *render_driver_hint, const Uint32 video_flags,
                                             SDL_Window **window, SDL_Renderer **renderer) {
    *window = nullptr;
    *renderer = nullptr;

    const std::string operation_description = (boost::format("SDL_SetHint(SDL_HINT_RENDER_DRIVER, %1%)") % render_driver_hint).str();
    if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, render_driver_hint)) {
        VS_LOG_SDL_SUCCESS(operation_description);
    } else {
        VS_LOG_SDL_ERROR(operation_description);
    }

    return SDL_CreateWindowAndRenderer(title, width, height, video_flags, window, renderer);
}

static bool get_screen_measurements() {
    total_display_bounds.x = 0;
    total_display_bounds.y = 0;
    total_display_bounds.w = 0;
    total_display_bounds.h = 0;
    usable_display_bounds.x = 0;
    usable_display_bounds.y = 0;
    usable_display_bounds.w = 0;
    usable_display_bounds.h = 0;
    total_window_size_in_pixels.x = 0;
    total_window_size_in_pixels.y = 0;
    total_window_size_in_pixels.w = 0;
    total_window_size_in_pixels.h = 0;
    usable_window_size_in_pixels.x = 0;
    usable_window_size_in_pixels.y = 0;
    usable_window_size_in_pixels.w = 0;
    usable_window_size_in_pixels.h = 0;
    total_logical_window_size.x = 0;
    total_logical_window_size.y = 0;
    total_logical_window_size.w = 0;
    total_logical_window_size.h = 0;
    usable_logical_window_size.x = 0;
    usable_logical_window_size.y = 0;
    usable_logical_window_size.w = 0;
    usable_logical_window_size.h = 0;

    std:string screen_name = "";
    SDL_DisplayID instance_ID;
    SDL_DisplayMode mode_for_ID;
    int configured_width = configuration().graphics.resolution_x;
    int configured_height = configuration().graphics.resolution_y;
    int screen_number = configuration().graphics.screen;
    bool full_screen = configuration().graphics.full_screen;

    std::string operation_description = (boost::format("get_sdl_display_name_by_number(%1%, screen_name, instance_ID)") % screen_number).str();
    if (get_sdl_display_name_by_number(screen_number, screen_name, instance_ID)) {
        VS_LOG_SDL_SUCCESS(operation_description);
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    {
        VS_LOG_SDL_INFO(
            (boost::format("configured width: %1%; configured height: %2%; full screen configured: %3%") %
                configured_width % configured_height % full_screen).str());
    }

    operation_description = (boost::format("SDL_GetDisplayBounds(%1%, &total_display_bounds)") % instance_ID).str();
    if (SDL_GetDisplayBounds(instance_ID, &total_display_bounds)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("total display bounds: x=%1%, y=%2%, w=%3%, h=%4%") % total_display_bounds.x % total_display_bounds.y % total_display_bounds.w % total_display_bounds.h).str());
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    operation_description = (boost::format("SDL_GetDisplayUsableBounds(%1%, &usable_display_bounds)") % instance_ID).str();
    if (SDL_GetDisplayUsableBounds(instance_ID, &usable_display_bounds)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("usable display bounds: x=%1%, y=%2%, w=%3%, h=%4%") % usable_display_bounds.x % usable_display_bounds.y % usable_display_bounds.w % usable_display_bounds.h).str());
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    operation_description = (boost::format("SDL_GetWindowSizeInPixels(%1%, &total_window_size_in_pixels.w, &total_window_size_in_pixels.h)") % window).str();
    if (SDL_GetWindowSizeInPixels(window, &total_window_size_in_pixels.w, &total_window_size_in_pixels.h)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("window size in pixels: w=%1%, h=%2%") % total_window_size_in_pixels.w % total_window_size_in_pixels.h).str());
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    operation_description = (boost::format("SDL_GetWindowSize(%1%, &total_logical_window_size.w, &total_logical_window_size.h)") % window).str();
    if (SDL_GetWindowSize(window, &total_logical_window_size.w, &total_logical_window_size.h)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("total logical window size: w=%1%, h=%2%") % total_logical_window_size.w % total_logical_window_size.h).str());
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    operation_description = (boost::format("SDL_GetRenderOutputSize(%1%, &usable_window_size_in_pixels.w, &usable_window_size_in_pixels.h)") % renderer).str();
    if (SDL_GetRenderOutputSize(renderer, &usable_window_size_in_pixels.w, &usable_window_size_in_pixels.h)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("usable window size in pixels: w=%1%, h=%2%") % usable_window_size_in_pixels.w % usable_window_size_in_pixels.h).str());
        native_resolution_x = usable_window_size_in_pixels.w;
        native_resolution_y = usable_window_size_in_pixels.h;
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    operation_description = (boost::format("SDL_GetCurrentRenderOutputSize(%1%, &usable_logical_window_size.w, &usable_logical_window_size.h)") % renderer).str();
    if (SDL_GetCurrentRenderOutputSize(renderer, &usable_logical_window_size.w, &usable_logical_window_size.h)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("usable logical window size: w=%1%, h=%2%") % usable_logical_window_size.w % usable_logical_window_size.h).str());
        (const_cast<vega_config::Configuration &>(configuration())).graphics.resolution_x = usable_logical_window_size.w;
        (const_cast<vega_config::Configuration &>(configuration())).graphics.resolution_y = usable_logical_window_size.h;
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        return false;
    }

    return true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets up the SDL OpenGL rendering context
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 */
static bool setup_sdl_video_mode() {
    int screen_number = 0;
    Uint32 video_flags = 0;
    int bpp = 0;
    SDL_Rect display_bounds;
    std:string screen_name = "";
    SDL_DisplayID instance_ID;
    SDL_DisplayMode* mode_for_ID = static_cast<SDL_DisplayMode*>(std::calloc(sizeof(SDL_DisplayMode), 1));
    if (mode_for_ID == nullptr) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("%1%:%2%: Memory allocation error") % __FILE__ % __LINE__), 1);
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
    int width = configuration().graphics.resolution_x;
    int height = configuration().graphics.resolution_y;
    screen_number = configuration().graphics.screen;
    bool full_screen = configuration().graphics.full_screen;

    bool result = get_sdl_display_name_by_number(screen_number, screen_name, instance_ID);

    // width and height are from the config file. We check if this resolution
    // is supported in full screen mode.
    if (full_screen) {
        video_flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_HIDDEN;
        int num_modes = 0;
        bool found = false;
        SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(instance_ID, &num_modes);
        if (modes) {
            SDL_DisplayMode* mode = nullptr;
            for (int i = 0; i < num_modes; ++i) {
                mode = modes[i];
                SDL_GetDisplayBounds(instance_ID, &display_bounds);
                if ((mode->w == width) && (mode->h == height)) {
                    found = true;
                    std::memcpy(mode_for_ID, mode, sizeof(SDL_DisplayMode)); // pmx-20251026 I'm not sure of the life length of the data pointed
                    // by 'mode', for the time being, I prefer to copy. May recheck later.
                    break;
                }
            }
        }
        if (found == false) {
            VS_LOG_FLUSH_EXIT(fatal, (boost::format("Could not find desktop display mode with specified width %1% and specified height %2%") % width % height), -42);
        }
        SDL_free(modes);
    } else { // Not full screen
        video_flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
        const SDL_DisplayMode* desktop_mode = SDL_GetDesktopDisplayMode(instance_ID);
        if (desktop_mode != nullptr) {
            std::memcpy(mode_for_ID, desktop_mode, sizeof(SDL_DisplayMode));
            SDL_GetDisplayBounds(instance_ID, &display_bounds);
        } else {
            VS_LOG_FLUSH_EXIT(fatal, "Could not get desktop display mode", 1);
        }
    }


    window = nullptr;
    renderer = nullptr;

    if (configuration().graphics.gl_accelerated_visual) {
        result = try_creating_window_and_renderer("Vega Strike", width, height, "opengl", video_flags, &window, &renderer);
        if (result == false) {
            VS_LOG_AND_FLUSH(serious_warning, (boost::format("Creating window and renderer failed with error: %1%") % SDL_GetError()));
            VS_LOG_AND_FLUSH(serious_warning, "Please make sure a graphics card driver is installed and functioning properly.");
            SDL_ClearError();
            (const_cast<vega_config::Configuration &>(configuration())).graphics.gl_accelerated_visual = false;

            result = try_creating_window_and_renderer("Vega Strike", width, height, "software", video_flags, &window, &renderer);
            if (result == false) {
                VS_LOG_FLUSH_EXIT(fatal, (boost::format("Creating window and renderer failed even with software rendering! Error message: %1%") % SDL_GetError()), 1);
            }
        }
    } else {
        result = try_creating_window_and_renderer("Vega Strike", width, height, "software", video_flags, &window, &renderer);
        if (result == false) {
            VS_LOG_FLUSH_EXIT(fatal, (boost::format("Creating window and renderer failed even with software rendering! Error message: %1%") % SDL_GetError()), 1);
        }
    }
    std::string operation_description = (boost::format("SDL_SetWindowSize(%1%, %2%, %3%)") % window % width % height).str();
    if (SDL_SetWindowSize(window, width, height)) {
        VS_LOG_SDL_SUCCESS(operation_description);
    } else {
        VS_LOG_SDL_ERROR(operation_description);
    }

    operation_description = (boost::format("SDL_GetCurrentRenderOutputSize(%1%, &native_resolution_x, &native_resolution_y)") % renderer).str();
    if (SDL_GetCurrentRenderOutputSize(renderer, &native_resolution_x, &native_resolution_y)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("native resolution: width=%1%, height=%2%") % native_resolution_x % native_resolution_y).str());
    } else {
        VS_LOG_SDL_ERROR(operation_description);
    }
    operation_description = (boost::format("SDL_GetWindowSize(%1%, &width, &height)") % window).str();
    if (SDL_GetWindowSize(window, &width, &height)) {
        VS_LOG_SDL_SUCCESS(operation_description);
        VS_LOG_SDL_INFO((boost::format("window size: width=%1%, height=%2%") % width % height).str());
    } else {
        VS_LOG_SDL_ERROR(operation_description);
    }
    const int refx = display_bounds.x + display_bounds.w / 2 - width / 2;
    const int refy = display_bounds.y + display_bounds.h / 2 - height / 2;

    if (full_screen) {
        SDL_SetWindowPosition(window, display_bounds.x, display_bounds.y);
        SDL_SetWindowFullscreenMode(window, mode_for_ID);
    } else {
        SDL_SetWindowPosition(window, refx, refy);
    }

    operation_description = (boost::format("SDL_GL_CreateContext(%1%)") % window).str();
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context) {
        VS_LOG_SDL_SUCCESS(operation_description);
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        VS_LOG_FLUSH_EXIT(fatal, "No GL context", 1);
    }

    VS_LOG_SDL_INFO((boost::format("GL Vendor: %1%") % glGetString(GL_VENDOR)).str());
    VS_LOG_SDL_INFO((boost::format("GL Renderer: %1%") % glGetString(GL_RENDERER)).str());
    VS_LOG_SDL_INFO((boost::format("GL Version: %1%") % glGetString(GL_VERSION)).str());

    operation_description = (boost::format("SDL_GL_MakeCurrent(%1%, %2%)") % window % context).str();
    if (SDL_GL_MakeCurrent(window, context)) {
        VS_LOG_SDL_SUCCESS(operation_description);
    } else {
        VS_LOG_SDL_ERROR(operation_description);
        VS_LOG_FLUSH_EXIT(fatal, "Failed to make window context current", 1);
    }

    SDL_ShowWindow(window);
    SDL_SyncWindow(window);

    get_screen_measurements();

    // Initialize imgui
    InitGui();

    return true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Initializes the OpenGL rendering context, and creates a window (or
 *  sets up fullscreen mode if selected)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 */

void winsys_init(int *argc, char **argv, char const *window_title, char const *icon_title) {
    keepRunning = true;

#if defined(NO_SDL_JOYSTICK)
    constexpr Uint32 sdl_flags = SDL_INIT_VIDEO;
#else
    constexpr Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK;
#endif

    gl_options.color_depth = configuration().graphics.color_depth;
    /*
     * Initialize SDL
     */
    if (!SDL_Init(sdl_flags)) {
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
        SDL_SetSurfaceColorKey(icon, true, static_cast<Uint32*>(icon->pixels)[0]);
    }
    /*
     * Init video
     */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if defined (USE_STENCIL_BUFFER)
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    if (!setup_sdl_video_mode()) {
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
    winsys_cleanup();
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
        if (visible) {
            SDL_ShowCursor();
        } else {
            SDL_HideCursor();
        }
        vis = visible;
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
 */
extern int shiftdown(int);
extern int shiftup(int);

void winsys_process_events() {
    SDL_Event event;
    float     x;
    float     y;
    bool      state;

    static unsigned int keysym_to_unicode[256];
    static bool keysym_to_unicode_init = false;
    if (!keysym_to_unicode_init) {
        keysym_to_unicode_init = true;
        memset(keysym_to_unicode, 0, sizeof(keysym_to_unicode));
    }
    while (keepRunning) {
        while (SDL_PollEvent(&event)) {

            state = false;
            switch (event.type) {
                case SDL_EVENT_KEY_UP:
                    state = true;
                    //does same thing as KEYDOWN, but with different state.
                case SDL_EVENT_KEY_DOWN:
                    if (keyboard_func) {
                        SDL_GetMouseState(&x, &y);
                        (*keyboard_func)(event.key.key, event.key.mod, event.key.down, x, y);
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (mouse_func) {
                        (*mouse_func)(event.button.button,
                            event.button.down,
                            event.button.x,
                            event.button.y);
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    if (event.motion.state) {
                        /* buttons are down */
                        if (motion_func) {
                            (*motion_func)(event.motion.x,
                                event.motion.y);
                        }
                    } else {
                        /* no buttons are down */
                        if (passive_motion_func) {
                            (*passive_motion_func)(event.motion.x,
                                    event.motion.y);
                        }
                    }
                    break;

                case SDL_EVENT_WINDOW_RESIZED:
                    get_screen_measurements();
                    //setup_sdl_video_mode(argc, argv);
                    if (reshape_func) {
                        (*reshape_func)(native_resolution_x,
                                native_resolution_y);
                    }
                    break;

                case SDL_EVENT_QUIT:
                    cleanexit = true;
                    keepRunning = false;
                    break;

                default:
                    break;
            }
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
    SDL_SetWindowFullscreen(window, false);

    winsys_shutdown();
    if (atexit_func) {
        (*atexit_func)();
    }
    exit(code);
}

/* EOF */
