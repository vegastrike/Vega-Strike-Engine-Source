/*
 * in_joystick.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alexander Rawass
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 *  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#include <list>
#include "root_generic/lin_time.h"
#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"
#include "src/vs_logging.h"

#include "src/in_handler.h"
#include "src/in_joystick.h"
#include "src/config_xml.h"
#include "src/in_mouse.h"
#ifndef HAVE_SDL
#include "gldrv/gl_include.h"
#if (GLUT_API_VERSION >= 4 || GLUT_XLIB_IMPLEMENTATION >= 13)
#else
#define NO_SDL_JOYSTICK
#endif
#endif

#include "root_generic/options.h"
#ifdef HAVE_SDL
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_error.h>
#endif
#include "configuration/configuration.h"
#include "gldrv/mouse_cursor.h"


//Used for storing the max and min values of the tree Joystick Axes - Okona
static int maxx = 1;
static int minx = -1;
static int maxy = 1;
static int miny = -1;
static int maxz = 1;
static int minz = -1;

JoyStick *joystick[MAX_JOYSTICKS]{}; //until I know where I place it
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
    if ((configuration().joystick.joystick_exponent_flt != 1.0F) && (configuration().joystick.joystick_exponent_flt > 0.0F)) {
        for (int a = 0; a < j->nr_of_axes; a++) {
            j->joy_axis[a] =
                    ((j->joy_axis[a] < 0) ? -std::pow(-j->joy_axis[a], configuration().joystick.joystick_exponent_flt) : std::pow(j->joy_axis[a],
                            configuration().joystick.joystick_exponent_flt));
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
    instanceID  = 0; // SDL3
}

int JoystickPollingRate() {
    return configuration().joystick.polling_rate;
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
    num_joysticks = 0;

#ifndef NO_SDL_JOYSTICK
    SDL_JoystickID *joysticks = SDL_GetJoysticks(&num_joysticks);
    if (num_joysticks > MAX_JOYSTICKS) {
        num_joysticks = MAX_JOYSTICKS;
    }
    VS_LOG(important_info, (boost::format("%1% joysticks were found.\n\n") % num_joysticks));
    VS_LOG(important_info, "The names of the joysticks are:\n");
    if (joysticks) {
        for (int i1 = 0; i1 < MAX_JOYSTICKS; ++i1) {
            if (i1 < num_joysticks) {
                const SDL_JoystickID instance_id = joysticks[i1];
                VS_LOG(important_info, (boost::format("    %1%\n") % SDL_GetJoystickNameForID(instance_id)));
                joystick[i1] = new JoyStick(i1, instance_id);
            } else {
                joystick[i1] = new JoyStick(i1, 0);
            }
        }
    }
    SDL_free(joysticks);

#endif
}

void DeInitJoystick() {
    for (auto & i : joystick) {
        delete i;
    }
    num_joysticks = 0;
}

JoyStick::JoyStick(const int which, const SDL_JoystickID instance_id) : mouse(which == MOUSE_JOYSTICK) {
    for (int j = 0; j < MAX_AXES; ++j) {
        axis_axis[j] = -1;
        axis_inverse[j] = false;
        joy_axis[j] = 0;
    }
    instanceID = instance_id;

    joy_buttons = 0;
    player = 0; //which;     //by default bind players to whichever joystick it is
    debug_digital_hatswitch = configuration().joystick.debug_digital_hatswitch;
    if (which != MOUSE_JOYSTICK) {
        deadzone = configuration().joystick.deadband_flt;
    } else {
        deadzone = configuration().joystick.mouse_deadband_flt;
    };
    joy_available = false;
    joy_x = joy_y = joy_z = 0;
    if (which == MOUSE_JOYSTICK) {
        InitMouse(which);
    }
#if defined (NO_SDL_JOYSTICK)
    return;

#else
#ifdef HAVE_SDL
    if (which >= num_joysticks) {
        if (which != MOUSE_JOYSTICK) {
            joy_available = false;
        }
        return;
    }
    joy = SDL_OpenJoystick(instance_id);     //joystick nr should be configurable
    if (joy == nullptr) {
        VS_LOG(warning, (boost::format("warning: no joystick nr %1%\n") % which));
        joy_available = false;
        return;
    }
    joy_available = true;
    nr_of_axes = SDL_GetNumJoystickAxes(joy);
    nr_of_buttons = SDL_GetNumJoystickButtons(joy);
    nr_of_hats = SDL_GetNumJoystickHats(joy);

    nr_of_axes = std::min(nr_of_axes, MAX_AXES);
    nr_of_buttons = std::min(nr_of_buttons, MAX_BUTTONS);
    nr_of_hats = std::min(nr_of_hats, MAX_DIGITAL_HATSWITCHES);
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
    std::pair<double, double> pair = GetJoystickFromMouse();
    x = pair.first;
    y = pair.second;
    z = 0;
    joy_axis[0] = x;
    joy_axis[1] = y;
    joy_axis[2] = z = 0;
    buttons = getMouseButtonStatus();
}

void JoyStick::GetJoyStick(float &x, float &y, float &z, long long& buttons) {
    //int status;
    if (!joy_available) {
        for (int a = 0; a < MAX_AXES; a++) {
            joy_axis[a] = 0;
        }
        x = y = z = 0;
        joy_buttons = 0;
        buttons = 0L;
        return;
    } else if (mouse) {
        int btn;
        GetMouse(x, y, z, btn);
        buttons = btn;
        return;
    }
    int a;
#ifndef NO_SDL_JOYSTICK
#if defined (HAVE_SDL)
    std::vector<Sint16> axi(nr_of_axes);
    for (a = 0; a < nr_of_axes; a++) {
        axi[a] = SDL_GetJoystickAxis(joy, a);
    }
    joy_buttons = 0;
    // nr_of_buttons = SDL_JoystickNumButtons(joy);
    for (int i = 0; i < nr_of_buttons; i++) {
        int butt = SDL_GetJoystickButton(joy, i);
        if (butt == 1) {
            joy_buttons |= (1LL << i);
        }
    }
    for (int h = 0; h < nr_of_hats; h++) {
        digital_hat[h] = SDL_GetJoystickHat(joy, h);
    }
    for (a = 0; a < nr_of_axes; a++) {
        joy_axis[a] = (static_cast<float>(axi[a]) / 32768.0F);
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

