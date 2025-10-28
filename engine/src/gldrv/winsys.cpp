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

#include "SDL3/SDL_video.h"

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

static SDL_Window* window = nullptr;
static SDL_Surface* screen = nullptr;

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


/*---------------------------------------------------------------------------*/
// pmx-20251026
bool get_sdl_display_name_by_nr(int screen_number, std::string& screen_name, SDL_DisplayID& id) {
    std::ostringstream display_names;
    int num_displays = 0;
    bool found = false;
    std::cerr << "===============================================================" << std::endl;
    SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
    if (displays) {
        int i;
        for (i = 0; i < num_displays; ++i) {
            SDL_DisplayID instance_id = displays[i];
            const char* name = SDL_GetDisplayName(instance_id);
            display_names << "Display " << i << ": " << (name ? name : "Unknown") << "\n";
            if (i == screen_number) {
                screen_name = name ? name : "Unknown";
                id = instance_id;
                found = true;
            }
            std::cerr << "Display " << i << ": " << name << std::endl;
        }
        SDL_free(displays);
    }
    std::cerr << "===============================================================" << std::endl;

    return found;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets up the SDL OpenGL rendering context
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2025-01-10 - stephengtuggy
 */

static bool setup_sdl_video_mode(int* argc, char** argv) {
    int screen_number = 0;
    Uint32 video_flags = 0;
    int bpp = 0;
    int width;
    int height;
    bool full_screen;
    SDL_Rect display_bounds;
    bool result;

std:string screen_name = "";
    SDL_DisplayID instance_ID;
    SDL_DisplayMode* mode_for_ID = (SDL_DisplayMode*)std::calloc(sizeof(SDL_DisplayMode), 1);
    if (mode_for_ID == nullptr) {
        VS_LOG_FLUSH_EXIT(fatal, "Memory allocation error", 1);
    }


    bpp = gl_options.color_depth;

    int rs, gs, bs;
    rs = gs = bs = (bpp == 16) ? 5 : 8;
    if (configuration().graphics.rgb_pixel_format == "undefined") {
        (const_cast<vega_config::Configuration&>(configuration()).graphics.rgb_pixel_format = ((bpp == 16) ? "555" : "888"));
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
    screen_number = configuration().graphics.screen;
    full_screen = configuration().graphics.full_screen;

    result = get_sdl_display_name_by_nr(screen_number, screen_name, instance_ID);

    // width and height are from the config file. We check if tis resolution 
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
                std::cerr << "Screen " << screen_number << " Display " << instance_ID << " Mode " << i << ": " << mode->w << "x" << mode->h << "@"
                    << mode->refresh_rate << "Hz"
                    << " Display bounds :"
                    << " x,y = " << display_bounds.x << "," << display_bounds.y
                    << " w,h = " << display_bounds.w << "," << display_bounds.h
                    << std::endl;
                if ((mode->w == width) && (mode->h == height)) {
                    found = true;
                    std::memcpy(mode_for_ID, mode, sizeof(SDL_DisplayMode)); // pmx-20251026 NI'm not sure of the life length of the data pointed
                    // by 'mode', for ther time being, I prefer to copy. May recheck later.
                    break;
                }
            }
        }
        if (found == false) {
            // pmx-20251026 Try to find the closest resolution.or no,
            // Normally, the setup app will only offer existing resolution, so we should only get
            // there when changing the hardware configuration.
            SDL_DisplayMode* mode = nullptr;
            bool result = SDL_GetClosestFullscreenDisplayMode(instance_ID, width, height, 0, true, mode);
            if (result == true) {
                std::memcpy(mode_for_ID, mode, sizeof(SDL_DisplayMode));
            } else {
                // Fallback to the desktop display mode for the display
                SDL_ClearError();
                const SDL_DisplayMode* desktop_mode = SDL_GetDesktopDisplayMode(instance_ID);
                if (desktop_mode != nullptr) {
                    std::memcpy(mode_for_ID, desktop_mode, sizeof(SDL_DisplayMode));
                    SDL_GetDisplayBounds(instance_ID, &display_bounds);
                    std::cerr << "Fallback to desktop resolution."
                        << screen_number << " Display " << instance_ID << " Mode " << instance_ID << ": " << desktop_mode->w << "x" << desktop_mode->h << "@"
                        << desktop_mode->refresh_rate << "Hz"
                        << " x,y = " << display_bounds.x << "," << display_bounds.y
                        << " w,h = " << display_bounds.w << "," << display_bounds.h
                        << std::endl;
                    width = desktop_mode->w;
                    height = desktop_mode->h;
                } else {
                    VS_LOG_FLUSH_EXIT(fatal, "Could not get desktop display mode", 1); // Sorry, we have tried everything...
                }
            }
        }
        SDL_free(modes);
    } else { // Not full screen
        video_flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
        const SDL_DisplayMode* desktop_mode = SDL_GetDesktopDisplayMode(instance_ID);
        if (desktop_mode != nullptr) {
            std::memcpy(mode_for_ID, desktop_mode, sizeof(SDL_DisplayMode));
            SDL_GetDisplayBounds(instance_ID, &display_bounds);
            std::cerr << "Screen "
                << screen_number << " Display " << instance_ID << " Mode " << instance_ID << ": " << desktop_mode->w << "x" << desktop_mode->h << "@"
                << desktop_mode->refresh_rate << "Hz"
                << " x,y = " << display_bounds.x << "," << display_bounds.y
                << " w,h = " << display_bounds.w << "," << display_bounds.h
                << std::endl;

            width = desktop_mode->w;
            height = desktop_mode->h;
        } else {
            VS_LOG_FLUSH_EXIT(fatal, "Could not get desktop display mode", 1);
        }
    }


    window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl")) {
        VS_LOG_AND_FLUSH(important_info, "SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...) succeeded");
    } else {
        VS_LOG_AND_FLUSH(error, (boost::format("SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...) failed. Error: %1%") % SDL_GetError()));
        SDL_ClearError();
    }

    result = SDL_CreateWindowAndRenderer("Vega Strike", width, height, video_flags, &window, &renderer);
    if (result == false) {
        VS_LOG_FLUSH_EXIT(fatal, "SDL_CreateWindowAndRender(...) error", 1);
    }
    SDL_SetWindowSize(window, width, height);
    int refx = display_bounds.x + display_bounds.w / 2 - width / 2;
    int refy = display_bounds.y + display_bounds.h / 2 - height / 2;
    SDL_SetWindowPosition(window, refx, refy);
    SDL_WINDOWPOS_CENTERED;


    if (full_screen) {
        SDL_SetWindowPosition(window, display_bounds.x, display_bounds.y);
        SDL_SetWindowFullscreenMode(window, mode_for_ID);
    } else {
        SDL_WINDOWPOS_CENTERED_DISPLAY(instance_ID);


    }

    // pmx-2025-1027 SHould even work with HDPi displays; I've no way to test that.
    SDL_GetWindowSizeInPixels(window, &native_resolution_x, &native_resolution_y);
    std::cerr << "SDL_GetWindowSizeInPixels() resolution. "
        << " w,h = " << native_resolution_x << "," << native_resolution_y
        << std::endl;
    // SDL_GetWindowSizeInPixels(window, &width, &height);

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

    if (SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_DISABLED) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_RenderSetLogicalSize(...) failed! Error: %1%") % SDL_GetError()),
            8);
    }

#if defined (GL_RENDERER)
    std::string version{};
    const GLubyte* renderer_string = glGetString(GL_RENDERER);
    if (renderer_string) {
        version = reinterpret_cast<const char*>(renderer_string);
    }
    if (version == "GDI Generic" || version == "software") {
        if (configuration().graphics.gl_accelerated_visual) {
            VS_LOG_AND_FLUSH(error, "GDI Generic software driver reported, trying to reset.");
            SDL_ClearError();
            freeMouseCursors();
            SDL_Quit();
            (const_cast<vega_config::Configuration&>(configuration())).graphics.gl_accelerated_visual = false;
            return false;
        } else {
            VS_LOG(error, "GDI Generic software driver reported, reset failed.");
            VS_LOG_AND_FLUSH(error, "Please make sure a graphics card driver is installed and functioning properly.");
        }
    }
#endif

    // This makes our buffer swap synchronized with the monitor's vertical refresh
    // if (SDL_GL_SetSwapInterval(1) < 0) {
    //     VS_LOG_AND_FLUSH(error, "SDL_GL_SetSwapInterval(1) failed");
    //     SDL_ClearError();
    // }


    SDL_ShowWindow(window);
    SDL_SyncWindow(window);

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

void winsys_init(int* argc, char** argv, char const* window_title, char const* icon_title) {
    keepRunning = true;

    // pmx-20251021 hDPI changed in SDL3
    // https://discourse.libsdl.org/t/question-regarding-sdl3-high-pixel-density-w-opengl/50964
    // if (SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2")) {
    //     VS_LOG_AND_FLUSH(important_info, "SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, ...) succeeded");
    // } else {
    //     VS_LOG_AND_FLUSH(warning, "SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, ...) failed");
    //     SDL_ClearError();
    // }

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
    SDL_Surface* icon = nullptr;
    if (icon_title) {
        icon = SDL_LoadBMP(icon_title);
    }
    if (icon) {
        SDL_SetSurfaceColorKey(icon, true, ((Uint32*)(icon->pixels))[0]);
    }
    /*
     * Init video
     */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

#if defined (USE_STENCIL_BUFFER)
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
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
        if (visible)
            SDL_ShowCursor();
        else
            SDL_HideCursor();
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
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */
extern int shiftdown(int);
extern int shiftup(int);

void winsys_process_events() {
    SDL_Event event;
    float x, y;
    bool state;

    static unsigned int keysym_to_unicode[256];
    static bool keysym_to_unicode_init = false;
    if (!keysym_to_unicode_init) {
        keysym_to_unicode_init = true;
        memset(keysym_to_unicode, 0, sizeof(keysym_to_unicode));
    }
    while (keepRunning) {
        // SDL_LockAudio();
        // SDL_UnlockAudio();
        while (SDL_PollEvent(&event)) {

            state = false;
            switch (event.type) {
            case SDL_EVENT_KEY_UP:
                state = true;
                //does same thing as KEYDOWN, but with different state.
            case SDL_EVENT_KEY_DOWN:
                if (keyboard_func) {
                    SDL_GetMouseState(&x, &y);
                    //                        VS_LOG(debug, (boost::format("Kbd: %1$s mod:%2$x sym:%3$x scan:%4$x")
                    //                                       % ((event.type == SDL_KEYUP) ? "KEYUP" : "KEYDOWN")
                    //                                       % event.key.keysym.mod
                    //                                       % event.key.keysym.sym
                    //                                       % event.key.keysym.scancode
                    //                                      ));

                                            //Send the event
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
                std::cerr << "Btn " << std::to_string(event.button.button)
                    << "; Down ? " << std::to_string(event.button.down)
                    << "; x " << std::to_string(event.button.x)
                    << "; y " << std::to_string(event.button.y)
                    << std::endl;
                break;

            case SDL_EVENT_MOUSE_MOTION:
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
                break;

            case SDL_EVENT_WINDOW_RESIZED:
#if !(defined (_WIN32) && defined (SDL_WINDOWING ))
                (const_cast<vega_config::Configuration&>(configuration())).graphics.resolution_x = event.window.data1;
                (const_cast<vega_config::Configuration&>(configuration())).graphics.resolution_y = event.window.data2;
                //setup_sdl_video_mode(argc, argv);
                if (reshape_func) {
                    (*reshape_func)(event.window.data1,
                        event.window.data2);
                }
#endif
                break;

            case SDL_EVENT_QUIT:
                cleanexit = true;
                keepRunning = false;
                break;
            default:
                break;
            }
            // SDL_LockAudio();
            // SDL_UnlockAudio();
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
    exit(code);
}

/* EOF */
