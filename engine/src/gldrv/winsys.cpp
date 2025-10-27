/*
 * winsys.cpp
 *
 * Copyright (C) 1999-2025 Jasmin F. Patry, Daniel Horn, pyramid3d, Roy Falk,
 * Benjamen R. Meyer, Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * This file is part of Tux Racer and has been incorporated into Vega Strike
 * (https://github.com/vegastrike/Vega-Strike-Engine-Source).
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if defined(__APPLE__) && defined(__MACH__)
#   include <gl.h>
#elif defined (_WIN32) || defined (__CYGWIN__)
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif //tells VCC not to generate min/max macros
#   include <windows.h>
#   include <gl.h>
#else
#   include <gl.h>
#endif

#include <assert.h>
#include <sstream>
#include <lin_time.h>

#include "gl_globals.h"
#include "winsys.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vs_logging.h"
#include "options.h"
#include "vs_exit.h"

#include "SDL3/SDL_video.h"

/*
 * Windowing System Abstraction Layer
 * Abstracts creation of windows, handling of events, etc.
 */

#if defined (SDL_WINDOWING) && defined (HAVE_SDL)

/*
 * *---------------------------------------------------------------------------
 * *---------------------------------------------------------------------------
 * SDL version
 *******************************---------------------------------------------------------------------------
 *******************************---------------------------------------------------------------------------
 */

static SDL_Window *window = nullptr;
//static SDL_Surface *screen = nullptr;

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

const double REFRESH_RATE = 1.0 / 120.0;

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

/*---------------------------------------------------------------------------*/
/*!
 *  Sets up the SDL OpenGL rendering context
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */

#ifndef _WIN32
static bool setup_sdl_video_mode() {
    const int screen_number = 0;
    Uint32 video_flags = SDL_WINDOW_OPENGL;
    int bpp = 0; // Bits per pixel?
    int width, height;
    // if (configuration().graphics.full_screen) {
    //     video_flags |= SDL_WINDOW_FULLSCREEN;

    //     const SDL_DisplayMode * currentDisplayMode =  SDL_GetCurrentDisplayMode(screen_number);
    //     if (currentDisplayMode == NULL) {
    //         VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_GetCurrentDisplayMode failed: %1%") % SDL_GetError()), -1);
    //     } else {
    //         native_resolution_x = currentDisplayMode->w;
    //         native_resolution_y = currentDisplayMode->h;
    //     }
    // } else {
        video_flags |= SDL_WINDOW_RESIZABLE;

        // native_resolution_x = configuration().graphics.resolution_x;
        // native_resolution_y = configuration().graphics.resolution_y;
    // }
#endif    

    bpp = gl_options.color_depth;

    int rs, gs, bs;
    rs = gs = bs = (bpp == 16) ? 5 : 8;
    if (vs_options::instance().rgb_pixel_format == "undefined") {
        vs_options::instance().rgb_pixel_format = ((bpp == 16) ? "555" : "888");
    }
    if ((vs_options::instance().rgb_pixel_format.length() == 3) && isdigit(vs_options::instance().rgb_pixel_format[0])
        && isdigit(vs_options::instance().rgb_pixel_format[1]) && isdigit(vs_options::instance().rgb_pixel_format[2])) {
        rs = vs_options::instance().rgb_pixel_format[0] - '0';
        gs = vs_options::instance().rgb_pixel_format[1] - '0';
        bs = vs_options::instance().rgb_pixel_format[2] - '0';
    }
    int otherbpp;
    int otherattributes;
    if (bpp == 16) {
        otherattributes = 8;
        otherbpp = 32;
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rs);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gs);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bs);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, vs_options::instance().z_pixel_format);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }
    else
    {
        otherattributes = 5;
        otherbpp = 16;
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rs);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gs);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bs);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, vs_options::instance().z_pixel_format);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    }

    if (vs_options::instance().gl_accelerated_visual) {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    }
    width = g_game.x_resolution;
    height = g_game.y_resolution;

    // // Fix display in fullscreen
    // if(configuration().graphics.full_screen) {
    //     // Change base resolution to match screen resolution
    //     width = configuration().graphics.resolution_x;//currentDisplayMode.w;
    //     height = configuration().graphics.resolution_y;//currentDisplayMode.h;
    //     int* ptr_x = const_cast<int*>(&configuration().graphics.bases.max_width);
    //     int* ptr_y = const_cast<int*>(&configuration().graphics.bases.max_height);
    //     *ptr_x = width;
    //     *ptr_y = height;
    // }


    window = nullptr;
    // if(screen_number == 0) {
        window = SDL_CreateWindow("Vega Strike",
                // SDL_WINDOWPOS_UNDEFINED,
                // SDL_WINDOWPOS_UNDEFINED,
                width, height, video_flags);
    // } else {
    //     // pmx-20251021  // for the time being, only dc=create on display .
    //     // Screen numbers are replaced by displayID in SDL3, a bit moe complicated (but not that much)
    //     window = SDL_CreateWindow("Vega Strike",
    //                             // SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen_number),
    //                             // SDL_WINDOWPOS_UNDEFINED_DISPLAY(screen_number),
    //                             width, height, video_flags);
    // }

    if(!window) {
        VS_LOG_FLUSH_EXIT(fatal, "No window", 1);
    }

    // if(screen_number > 0) {
    //     // Get bounds of the secondary monitor
    //     SDL_Rect displayBounds;
    //     if (SDL_GetDisplayBounds(screen_number, &displayBounds) != 0) {
    //         const std::string error_message = (boost::format("Failed to get display bounds: %1%") % SDL_GetError()).str();
    //         VS_LOG_AND_FLUSH(error, error_message);

    //         // Fallback to primary monitor
    //         SDL_GetDisplayBounds(0, &displayBounds);
    //     }

    //     // Move to secondary monitor
    //     SDL_SetWindowPosition(window, displayBounds.x, displayBounds.y);
    // }

    // if (configuration().graphics.full_screen) {
    //     SDL_SetWindowFullscreenMode(window, NULL);
    // }

    if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl")) {
        VS_LOG_AND_FLUSH(important_info, "SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...) succeeded");
    }
    else
    {
        VS_LOG_AND_FLUSH(error, (boost::format("SDL_SetHint(SDL_HINT_RENDER_DRIVER, ...) failed. Error: %1%") % SDL_GetError()));
        SDL_ClearError();
    }

    SDL_GetWindowSizeInPixels(window, &width, &height);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        VS_LOG_FLUSH_EXIT(fatal, "No GL context", 1);
    }

    VS_LOG_AND_FLUSH(important_info, (boost::format("GL Vendor: %1%") % glGetString(GL_VENDOR)));
    VS_LOG_AND_FLUSH(important_info, (boost::format("GL Renderer: %1%") % glGetString(GL_RENDERER)));
    VS_LOG_AND_FLUSH(important_info, (boost::format("GL Version: %1%") % glGetString(GL_VERSION)));

    if (SDL_GL_MakeCurrent(window, context) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, "Failed to make window context current", 1);
    }

    // TODO : Vsync...  pmx-20251021
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr) {
        VS_LOG_AND_FLUSH(error, (boost::format(
            "SDL_CreateRenderer(...) with VSync option failed; trying again without VSync option. Error was: %1%") %
            SDL_GetError()));
        SDL_ClearError();

        renderer = SDL_CreateRenderer(window, NULL);
        if (renderer == nullptr) {
            VS_LOG_AND_FLUSH(error, (boost::format(
                "SDL_CreateRenderer(...) with SDL_RENDERER_ACCELERATED failed; trying again with software rendering option. Error was: %1%") %
                SDL_GetError()));
            SDL_ClearError();

            renderer = SDL_CreateRenderer(window, NULL);  // -1, SDL_RENDERER_SOFTWARE);
            if (renderer == nullptr) {
                VS_LOG_FLUSH_EXIT(fatal, (boost::format(
                    "SDL_CreateRenderer(...) failed on the third try, with software rendering! Error: %1%") %
                    SDL_GetError()),
                    1);
            }
        }
    }

    if (SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_DISABLED) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_RenderSetLogicalSize(...) failed! Error: %1%") % SDL_GetError()),
            8);
    }

#if defined (GL_RENDERER)
    std::string version{};
    const GLubyte * renderer_string = glGetString(GL_RENDERER);
    if (renderer_string) {
        version = (const char*)renderer_string;
    }
    if (version == "GDI Generic" || version == "software") {
        if (vs_options::instance().gl_accelerated_visual) {
            VS_LOG_AND_FLUSH(error, "GDI Generic software driver reported, trying to reset.");
            SDL_ClearError();
            SDL_Quit();
            vs_options::instance().gl_accelerated_visual = false;
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

    // Uint32 sdl_flags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    g_game.x_resolution = vs_options::instance().x_resolution;
    g_game.y_resolution = vs_options::instance().y_resolution;
//    gl_options.fullscreen = vs_options::instance().fullscreen;
//    gl_options.color_depth = vs_options::instance().colordepth;
    /*
     * Initialize SDL
     */
    if (SDL_Init(sdl_flags) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("Couldn't initialize SDL: %1%") % SDL_GetError()), 1);
    }

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, ...) failed! Error: %1%") % SDL_GetError()), 1);
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, ...) failed! Error: %1%") % SDL_GetError()), 1);
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY) < 0) {
        VS_LOG_FLUSH_EXIT(fatal, (boost::format("SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, ...) failed! Error: %1%") % SDL_GetError()), 1);
    }

    if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0) {
        std::cerr << "SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, ...) failed! Error: " << SDL_GetError() << std::endl;
        VSExit(-4);
    }

    //signal( SIGSEGV, SIG_DFL );
    SDL_Surface *icon = nullptr;
    if (icon_title) {
        icon = SDL_LoadBMP(icon_title);
    }
    if (icon) {
        SDL_SetSurfaceColorKey(icon, true, ((Uint32 *) (icon->pixels))[0]);
    }

#if defined (USE_STENCIL_BUFFER)
    /* Not sure if this is sufficient to activate stencil buffer  */
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
#endif

    if (!setup_sdl_video_mode()) {
        VS_LOG_FLUSH_EXIT(fatal, "setup_sdl_video_mode() failed!", 1);
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

                        //Send the event
                        (*keyboard_func)(event.key.key, event.key.mod,
                                         state,
                                         x, y);
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
                    std::cerr << "Btn "<< std::to_string(event.button.button)
                    << "; Down ? "<< std::to_string(event.button.down)
                    << "; x "<< std::to_string(event.button.x)
                    << "; y "<< std::to_string(event.button.y)
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
                    int width, height;
                    SDL_GetWindowSizeInPixels(window, &width, &height);
                    g_game.x_resolution = width;
                    g_game.y_resolution = height;
                    g_game.aspect = static_cast<float>(width) / static_cast<float>(height);
                    if (reshape_func) {
                        (*reshape_func)(width, height);
                    }
#endif
                    break;

                case SDL_EVENT_QUIT:
                    cleanexit = true;
                    keepRunning = false;
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
        }

        /* Delay for a bit.  This allows the other threads to do some
         *  work (otherwise the audio thread gets starved). */
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
 *  \date    Modified: 2024-04-25
 */
void winsys_exit(int code) {
    winsys_shutdown();
    if (atexit_func) {
        (*atexit_func)();
    }
    exit( code );
}

#else

/*
 * *---------------------------------------------------------------------------
 * *---------------------------------------------------------------------------
 * GLUT version
 *******************************---------------------------------------------------------------------------
 *******************************---------------------------------------------------------------------------
 */

static winsys_keyboard_func_t keyboard_func = NULL;

static bool redisplay = false;

/*---------------------------------------------------------------------------*/
/*!
 *  Requests that the screen be redrawn
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_post_redisplay()
{
    redisplay = true;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the display callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_display_func( winsys_display_func_t func )
{
    glutDisplayFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the idle callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_idle_func( winsys_idle_func_t func )
{
    glutIdleFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the reshape callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_reshape_func( winsys_reshape_func_t func )
{
    glutReshapeFunc( func );
}

char AdjustKeyCtrl( char ch )
{
    if (ch == '\0') {
        ch = '2';
    } else if (ch >= '0' && ch <= '9') {} else if (ch >= 27 && ch <= 31) {
        ch = ch+'0'-24;
    } else if (ch == 127) {
        ch = '8';
    } else if (ch <= 26) {
        ch += 'a'-1;
    }
    return ch;
}

/* Keyboard callbacks */
static void glut_keyboard_cb( unsigned char ch, int x, int y )
{
    if (keyboard_func) {
        int gm = glutGetModifiers();
        if (gm) {
            VS_LOG(trace, (boost::format("Down Modifier %d for char %d %c") % gm % (int)ch % ch));
        }
        if (gm&GLUT_ACTIVE_CTRL) {
            ch = AdjustKeyCtrl( ch );
        }
        (*keyboard_func)(ch, gm, false, x, y);
    }
}

static void glut_special_cb( int key, int x, int y )
{
    if (keyboard_func) {
        (*keyboard_func)(key+128, glutGetModifiers(), false, x, y);
    }
}

static void glut_keyboard_up_cb( unsigned char ch, int x, int y )
{
    if (keyboard_func) {
        int gm = glutGetModifiers();
        if (gm) {
            VS_LOG(trace, (boost::format("Up Modifier %d for char %d %c") % gm % (int)ch % ch));
        }
        if (gm&GLUT_ACTIVE_CTRL) {
            ch = AdjustKeyCtrl( ch );
        }
        (*keyboard_func)(ch, gm, true, x, y);
    }
}

static void glut_special_up_cb( int key, int x, int y )
{
    if (keyboard_func) {
        (*keyboard_func)(key+128, glutGetModifiers(), true, x, y);
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the keyboard callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_keyboard_func( winsys_keyboard_func_t func )
{
    keyboard_func = func;
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse button-press callback
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_mouse_func( winsys_mouse_func_t func )
{
    glutMouseFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when a mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_motion_func( winsys_motion_func_t func )
{
    glutMotionFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the mouse motion callback (when no mouse button is pressed)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_set_passive_motion_func( winsys_motion_func_t func )
{
    glutPassiveMotionFunc( func );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Copies the OpenGL back buffer to the front buffer
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_swap_buffers()
{
    glutSwapBuffers();
}

/*---------------------------------------------------------------------------*/
/*!
 *  Moves the mouse pointer to (x,y)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_warp_pointer( int x, int y )
{
    glutWarpPointer( x, y );
}

/*---------------------------------------------------------------------------*/
/*!
 *  Initializes the OpenGL rendering context, and creates a window (or
 *  sets up fullscreen mode if selected)
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2021-09-07 - stephengtuggy
 */
void winsys_init( int *argc, char **argv, char const *window_title, char const *icon_title )
{
    int width, height;
    int glutWindow;
    g_game.x_resolution    = vs_options::instance().x_resolution;
    g_game.y_resolution    = vs_options::instance().y_resolution;
    gl_options.fullscreen  = vs_options::instance().fullscreen;
    gl_options.color_depth = vs_options::instance().colordepth;
    glutInit( argc, argv );
    if (vs_options::instance().glut_stencil) {
#if defined(__APPLE__) && defined(__MACH__)
        if ( !(glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_STENCIL ), 1) )
            glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE );
#else
        glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_STENCIL );
#endif
    } else {
        glutInitDisplayMode( GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE );
    }
    char str[1024];
    sprintf( str, "%dx%d:%d@60", g_game.x_resolution, g_game.y_resolution, gl_options.color_depth );
    glutGameModeString( str );
    VS_LOG(trace, (boost::format("Game Mode Params %1%x%2% at depth %3% @ %4% Hz")
                                % glutGameModeGet(GLUT_GAME_MODE_WIDTH)
                                % glutGameModeGet(GLUT_GAME_MODE_HEIGHT)
                                % glutGameModeGet(GLUT_GAME_MODE_PIXEL_DEPTH)
                                % glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE)));
    /* Create a window */
    if ( gl_options.fullscreen && (glutGameModeGet( GLUT_GAME_MODE_POSSIBLE ) != -1) ) {
        glutInitWindowPosition( 0, 0 );
        glutEnterGameMode();
        VS_LOG(trace, (boost::format("Game Mode Params %1%x%2% at depth %3% @ %4% Hz")
                                    % glutGameModeGet(GLUT_GAME_MODE_WIDTH)
                                    % glutGameModeGet(GLUT_GAME_MODE_HEIGHT)
                                    % glutGameModeGet(GLUT_GAME_MODE_PIXEL_DEPTH)
                                    % glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE)));
    } else {
        /* Set the initial window size */
        glutInitWindowSize( g_game.x_resolution, g_game.y_resolution );

        glutWindow = glutCreateWindow( window_title );
        if (glutWindow == 0) {
            VS_LOG(fatal, "Couldn't create a window.");
            VegaStrikeLogging::VegaStrikeLogger::instance().FlushLogsProgramExiting();
            exit( 1 );                  // stephengtuggy 2020-07-27 - I would use VSExit here, but that calls winsys_exit, which I'm not sure will work if winsys_init hasn't finished yet.
        }
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Deallocates resources in preparation for program termination
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_shutdown()
{
    static bool shutdown = false;
    if (!shutdown) {
        shutdown = true;
        if (gl_options.fullscreen) {
            glutLeaveGameMode();
        }
    }
}

/*---------------------------------------------------------------------------*/
/*!
 *  Shows/hides mouse cursor
 *  \author  jfpatry
 *  \date    Created:  2000-10-19
 *  \date    Modified: 2000-10-19
 */
void winsys_show_cursor( bool visible )
{
    static bool vis = true;
    if (visible != vis) {
        if (visible) {
            glutSetCursor( GLUT_CURSOR_LEFT_ARROW );
        } else {
            glutSetCursor( GLUT_CURSOR_NONE );
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
 */
void winsys_process_events()
{
    /* Set up keyboard callbacks */
    glutKeyboardFunc( glut_keyboard_cb );
    glutKeyboardUpFunc( glut_keyboard_up_cb );
    glutSpecialFunc( glut_special_cb );
    glutSpecialUpFunc( glut_special_up_cb );

    glutMainLoop();
}

/*---------------------------------------------------------------------------*/
/*!
 *  Sets the function to be called when program ends.  Note that this
 *  function should only be called once.
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2021-09-06 - stephengtuggy */
void winsys_atexit( winsys_atexit_func_t func )
{
    static bool called = false;
    if (called) {
        std::cerr << "winsys_atexit called twice\n";
        VS_LOG_AND_FLUSH(error, "winsys_atexit called twice\n");
    }
    called = true;

    //atexit(func);
}

/*---------------------------------------------------------------------------*/
/*!
 *  Exits the program
 *  \author  jfpatry
 *  \date    Created:  2000-10-20
 *  \date    Modified: 2000-10-20
 */
void winsys_exit( int code )
{
    winsys_shutdown();
    exit( code );
}

#endif /* defined( SDL_WINDOWING ) */

/* EOF */
