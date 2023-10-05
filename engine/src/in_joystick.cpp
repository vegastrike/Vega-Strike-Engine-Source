/*
 * Copyright (C) 2001-2022 Daniel Horn, Alexander Rawass, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 *  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#include <list>
#include <lin_time.h>
#include "vegastrike.h"
#include "vs_globals.h"
#include "vs_logging.h"

#include "in_handler.h"
#include "in_joystick.h"
#include "config_xml.h"
#include "in_mouse.h"
#ifndef HAVE_SDL
#include "gldrv/gl_include.h"
#if (GLUT_API_VERSION >= 4 || GLUT_XLIB_IMPLEMENTATION >= 13)
#else
#define NO_SDL_JOYSTICK
#endif
#endif

#include "options.h"
#include "SDL2/SDL_joystick.h"


//Used for storing the max and min values of the tree Joystick Axes - Okona
static int maxx = 1;
static int minx = -1;
static int maxy = 1;
static int miny = -1;
static int maxz = 1;
static int minz = -1;

JoyStick *joystick[MAX_JOYSTICKS]; //until I know where I place it
int num_joysticks = 0;

void modifyDeadZone(JoyStick *j) {
    for (int a = 0; a < j->nr_of_axes; a++) {
        if (fabs(j->joy_axis[a]) <= j->deadzone) {
            j->joy_axis[a] = 0.0;
        } else if (j->joy_axis[a] > 0) {
            j->joy_axis[a] -= j->deadzone;
        } else {
            j->joy_axis[a] += j->deadzone;
        }
        if (j->deadzone < .999) {
            j->joy_axis[a] /= (1 - j->deadzone);
        }
    }
}

void modifyExponent(JoyStick *j) {
    if ((game_options()->joystick_exponent != 1.0) && (game_options()->joystick_exponent > 0)) {
        for (int a = 0; a < j->nr_of_axes; a++) {
            j->joy_axis[a] =
                    ((j->joy_axis[a] < 0) ? -pow(-j->joy_axis[a], game_options()->joystick_exponent) : pow(j->joy_axis[a],
                            game_options()->
                                    joystick_exponent));
        }
    }
}

static bool JoyStickToggle = true;

void JoyStickToggleDisable() {
    JoyStickToggle = false;
}

void JoyStickToggleKey(const KBData &key, KBSTATE a) {
    if (a == PRESS) {
        JoyStickToggle = !JoyStickToggle;
    }
}

void myGlutJoystickCallback(unsigned int buttonmask, int x, int y, int z) {
    unsigned int i;
    for (i = 0; i < MAX_AXES; i++) {
        joystick[0]->joy_axis[i] = 0.0;
    }
    joystick[0]->joy_buttons = 0;
    if (JoyStickToggle) {
        joystick[0]->joy_buttons = buttonmask;
        if (joystick[0]->nr_of_axes > 0) {
            //Set the max and min of each axis - Okona
            if (x < minx) {
                minx = x;
            }
        }
        if (x > maxx) {
            maxx = x;
        }
        //Calculate an autocalibrated value based on the max min values - Okona
        joystick[0]->joy_axis[0] = ((float) x - (((float) (maxx + minx)) / 2.0)) / (((float) (maxx - minx)) / 2.0);
        if (joystick[0]->nr_of_axes > 1) {
            if (y < miny) {
                miny = y;
            }
        }
        if (y > maxy) {
            maxy = y;
        }
        joystick[0]->joy_axis[1] = ((float) y - (((float) (maxy + miny)) / 2.0)) / (((float) (maxy - miny)) / 2.0);
        if (joystick[0]->nr_of_axes > 2) {
            if (z < minz) {
                minz = z;
            }
        }
        if (z > maxz) {
            maxz = z;
        }
        joystick[0]->joy_axis[2] = ((float) z - (((float) (maxz + minz)) / 2.0)) / (((float) (maxz - minz)) / 2.0);
        modifyDeadZone(joystick[0]);
        modifyExponent(joystick[0]);
    }
}

JoyStick::JoyStick() {
    for (int j = 0; j < MAX_AXES; ++j) {
        axis_axis[j] = -1;
        axis_inverse[j] = false;
        joy_axis[j] = axis_axis[j] = 0;
    }
    joy_buttons = 0;
}

int JoystickPollingRate() {
    return game_options()->polling_rate;
}

void InitJoystick() {
    int i;
    for (i = 0; i < NUMJBUTTONS; i++) {
        for (int j = 0; j < MAX_JOYSTICKS; j++) {
            UnbindJoyKey(j, i);
        }
    }
    for (int h = 0; h < MAX_HATSWITCHES; h++) {
        for (int v = 0; v < MAX_VALUES; v++) {
            UnbindHatswitchKey(h, v);
        }
    }
    for (int j = 0; j < MAX_JOYSTICKS; j++) {
        for (int h = 0; h < MAX_DIGITAL_HATSWITCHES; h++) {
            for (int v = 0; v < MAX_DIGITAL_VALUES; v++) {
                UnbindDigitalHatswitchKey(j, h, v);
            }
        }
    }
#ifndef NO_SDL_JOYSTICK
#ifdef HAVE_SDL
    num_joysticks = SDL_NumJoysticks();
    VS_LOG(info, (boost::format("%1% joysticks were found.\n\n") % num_joysticks));
    VS_LOG(info, "The names of the joysticks are:\n");
#else
    //use glut
    if (glutDeviceGet( GLUT_HAS_JOYSTICK ) || game_options()->force_use_of_joystick) {
        VS_LOG(info, "setting joystick functionality:: joystick online");
        glutJoystickFunc( myGlutJoystickCallback, JoystickPollingRate() );
        num_joysticks = 1;
    }
#endif
#endif
    for (i = 0; i < MAX_JOYSTICKS; i++) {
#ifndef NO_SDL_JOYSTICK
#ifdef HAVE_SDL
        if (i < num_joysticks) {
            VS_LOG(info, (boost::format("    %1%\n") % SDL_JoystickNameForIndex(i)));
        }
#else
        if (i < num_joysticks) {
            VS_LOG(info, (boost::format("Glut detects %1% joystick") % (i+1)));
        }
#endif
#endif
        joystick[i] = new JoyStick(i);         //SDL_Init is done in main.cpp
    }
}

void DeInitJoystick() {
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        delete joystick[i];
    }
}

JoyStick::JoyStick(int which) : mouse(which == MOUSE_JOYSTICK) {
    for (int j = 0; j < MAX_AXES; ++j) {
        axis_axis[j] = -1;
        axis_inverse[j] = false;
        joy_axis[j] = 0;
    }
    joy_buttons = 0;

    player = which;     //by default bind players to whichever joystick it is
    debug_digital_hatswitch = game_options()->debug_digital_hatswitch;
    if (which != MOUSE_JOYSTICK) {
        deadzone = game_options()->deadband;
    } else {
        deadzone = game_options()->mouse_deadband;
    };
    joy_available = 0;
    joy_x = joy_y = joy_z = 0;
    if (which == MOUSE_JOYSTICK) {
        InitMouse(which);
    }
#if defined (NO_SDL_JOYSTICK)
    return;

#else
#ifdef HAVE_SDL
    num_joysticks = SDL_NumJoysticks();
    if (which >= num_joysticks) {
        if (which != MOUSE_JOYSTICK) {
            joy_available = false;
        }
        return;
    }
    joy = SDL_JoystickOpen(which);     //joystick nr should be configurable
    if (joy == nullptr) {
        VS_LOG(warning, (boost::format("warning: no joystick nr %1%\n") % which));
        joy_available = false;
        return;
    }
    joy_available = true;
    nr_of_axes = SDL_JoystickNumAxes(joy);
    nr_of_buttons = SDL_JoystickNumButtons(joy);
    nr_of_hats = SDL_JoystickNumHats(joy);
#else
    //WE HAVE GLUT
    if (which > 0 && which != MOUSE_JOYSTICK) {
        joy_available = false;
        return;
    }
    joy_available = true;
    nr_of_axes    = 3;     //glutDeviceGet(GLUT_JOYSTICK_AXES);
    nr_of_buttons = 15;     //glutDeviceGet(GLUT_JOYSTICK_BUTTONS);
    nr_of_hats    = 0;
#endif //we have GLUT
#endif
    VS_LOG(info, (boost::format("axes: %1% buttons: %2% hats: %3%\n") % nr_of_axes % nr_of_buttons % nr_of_hats));
}

void JoyStick::InitMouse(int which) {
    player = 0;     //default to first player
    joy_available = true;
    nr_of_axes = 2;     //x and y for mouse
    nr_of_buttons = 15;
    nr_of_hats = 0;
}

bool JoyStick::isAvailable() {
    return joy_available;
}

struct mouseData {
    int dx;
    int dy;
    float time;

    mouseData() {
        dx = dy = 0;
        time = 0;
    }

    mouseData(int ddx, int ddy, float ttime) {
        dx = ddx;
        dy = ddy;
        time = ttime;
    }
};
extern void GetMouseXY(int &mousex, int &mousey);

void JoyStick::GetMouse(float &x, float &y, float &z, int &buttons) {
    int def_mouse_sens = 1;
    int _dx, _dy;
    float fdx, fdy;
    int _mx, _my;
    GetMouseXY(_mx, _my);
    GetMouseDelta(_dx, _dy);
    if (!game_options()->warp_mouse) {
        fdx = (float) (_dx = _mx - g_game.x_resolution / 2.0f);
        def_mouse_sens = 25;
        fdy = (float) (_dy = _my - g_game.y_resolution / 2.0f);
    } else {
        static std::list<mouseData> md;
        std::list<mouseData>::iterator i = md.begin();
        float ttime = getNewTime();
        float lasttime = ttime - game_options()->mouse_blur;
        int avg = (_dx || _dy) ? 1 : 0;
        float valx = _dx;
        float valy = _dy;
        for (; i != md.end();) {
            if ((*i).time >= lasttime) {
                bool found = false;
                int ldx = (*i).dx;
                int ldy = (*i).dy;
                if ((ldx >= 0) * _dx * ldx == (_dx >= 0) * _dx * ldx) {
                    //make sure same sign or zero
                    valx += (*i).dx;
                    found = true;
                }
                if ((ldy >= 0) * _dy * ldy == (_dy >= 0) * _dy * ldy) {
                    //make sure same sign or zero
                    valy += (*i).dy;
                    found = true;
                }
                if (found) {
                    avg++;
                }
                ++i;
            } else if ((i = md.erase(i)) == md.end()) {
                break;
            }
        }
        if (_dx || _dy) {
            md.push_back(mouseData(_dx, _dy, ttime));
        }
        if (avg) {
            _dx = float_to_int(valx / avg);
            _dy = float_to_int(valy / avg);
        }
        fdx = float(valx) / game_options()->mouse_blur;
        fdy = float(valy) / game_options()->mouse_blur;
    }
    joy_axis[0] = fdx / (g_game.x_resolution * def_mouse_sens / game_options()->mouse_sensitivity);
    joy_axis[1] = fdy / (g_game.y_resolution * def_mouse_sens / game_options()->mouse_sensitivity);
    if (!game_options()->warp_mouse) {
        modifyDeadZone(this);
    }
    joy_axis[0] *= game_options()->mouse_exponent;
    joy_axis[1] *= game_options()->mouse_exponent;
    x = joy_axis[0];
    y = joy_axis[1];

    joy_axis[2] = z = 0;
    buttons = getMouseButtonStatus();
}

void JoyStick::GetJoyStick(float &x, float &y, float &z, int &buttons) {
    //int status;
    if (!joy_available) {
        for (int a = 0; a < MAX_AXES; a++) {
            joy_axis[a] = 0;
        }
        x = y = z = 0;
        joy_buttons = buttons = 0;
        return;
    } else if (mouse) {
        GetMouse(x, y, z, buttons);
        return;
    }
    int a;
#ifndef NO_SDL_JOYSTICK
#if defined (HAVE_SDL)
    int numaxes = SDL_JoystickNumAxes(joy) < MAX_AXES ? SDL_JoystickNumAxes(joy) : MAX_AXES;
    std::vector<Sint16> axi(numaxes);
    for (a = 0; a < numaxes; a++) {
        axi[a] = SDL_JoystickGetAxis(joy, a);
    }
    joy_buttons = 0;
    nr_of_buttons = SDL_JoystickNumButtons(joy);
    for (int i = 0; i < nr_of_buttons; i++) {
        int butt = SDL_JoystickGetButton(joy, i);
        if (butt == 1) {
            joy_buttons |= (1 << i);
        }
    }
    for (int h = 0; h < nr_of_hats; h++) {
        digital_hat[h] = SDL_JoystickGetHat(joy, h);
    }
    for (a = 0; a < numaxes; a++) {
        joy_axis[a] = ((float) axi[a] / 32768.0);
    }
    modifyDeadZone(this);
    modifyExponent(this);
#else //we have glut
    if (JoystickPollingRate() <= 0)
        glutForceJoystickFunc();
#endif
    x = joy_axis[0];
    y = joy_axis[1];
    z = joy_axis[2];
    buttons = joy_buttons;
#endif //we have no joystick
}

int JoyStick::NumButtons() {
    return nr_of_buttons;
}

