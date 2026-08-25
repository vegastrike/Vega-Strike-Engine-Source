/*
 * in_joystick.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alexander Rawass
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#include "root_generic/options.h"
#include <SDL2/SDL_joystick.h>
#include "configuration/configuration.h"
#include "gldrv/mouse_cursor.h"


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
    num_joysticks = SDL_NumJoysticks();
    if (num_joysticks > MAX_JOYSTICKS) {
        num_joysticks = MAX_JOYSTICKS;
    }
    VS_LOG(info, (boost::format("%1% joysticks were found.\n\n") % num_joysticks));
    VS_LOG(info, "The names of the joysticks are:\n");
    for (i = 0; i < MAX_JOYSTICKS; i++) {
        if (i == MOUSE_JOYSTICK) {
            // The mouse-joystick slot is always created as the mouse.
            joystick[i] = new JoyStick(i);
            continue;
        }
        // Always create a present physical slot so bindKeys() binds it normally
        // (no "not available" refusals / inconsistent half-bound state). It
        // reports available but has a null SDL handle until a real device
        // attaches via JoyStick::Attach() on hotplug. GetJoyStick guards the null
        // handle and returns zeros, so reads on an unattached slot are safe.
        joystick[i] = new JoyStick(i);
        joystick[i]->joy = nullptr;
        joystick[i]->joy_available = true;
        joystick[i]->nr_of_axes = 0;
        joystick[i]->nr_of_buttons = 0;
        joystick[i]->nr_of_hats = 0;
        // A real device present at startup: attach it to this slot (device index
        // maps directly to slot index for the first N physical devices).
        if (i < num_joysticks) {
            VS_LOG(info, (boost::format("    %1%\n") % SDL_JoystickNameForIndex(i)));
            joystick[i]->Attach(i);
        }
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
    debug_digital_hatswitch = configuration().joystick.debug_digital_hatswitch;
    if (which != MOUSE_JOYSTICK) {
        deadzone = configuration().joystick.deadband_flt;
    } else {
        deadzone = configuration().joystick.mouse_deadband_flt;
    };
    joy_available = 0;
    joy_x = joy_y = joy_z = 0;
    joy = nullptr;
    if (which == MOUSE_JOYSTICK) {
        InitMouse(which);
    }
}

JoyStick::~JoyStick() {
    if (joy != nullptr) {
        SDL_JoystickClose(joy);
        joy = nullptr;
    }
}

// Re-point this slot at a real SDL joystick (hotplug attach). Closes any
// existing/fake handle, opens the new device, and refreshes the axis/button/hat
// counts. The slot's bindings (keyed by slot index) are untouched.
void JoyStick::Attach(int device_index) {
    if (joy != nullptr) {
        SDL_JoystickClose(joy);
        joy = nullptr;
    }
    joy = SDL_JoystickOpen(device_index);
    if (joy == nullptr) {
        joy_available = false;
        return;
    }
    joy_available = true;
    nr_of_axes = std::min(SDL_JoystickNumAxes(joy), MAX_AXES);
    nr_of_buttons = std::min(SDL_JoystickNumButtons(joy), MAX_BUTTONS);
    nr_of_hats = std::min(SDL_JoystickNumHats(joy), MAX_DIGITAL_HATSWITCHES);
    VS_LOG(important_info, (boost::format("[joy] attached slot: axes=%1% buttons=%2% hats=%3%\n") % nr_of_axes % nr_of_buttons % nr_of_hats));
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

void JoyStick::GetJoyStick(float &x, float &y, float &z, int &buttons) {
    //int status;
    // Unavailable, or a fake slot with no real device attached yet: return zeros
    // safely so bindings stay bound but produce no input until a real joystick
    // attaches (JoyStick::Attach). Without the null-joy guard, an unattached
    // fake slot would crash in the SDL_Joystick* calls below.
    if (!joy_available || (joy == nullptr && !mouse)) {
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
    x = joy_axis[0];
    y = joy_axis[1];
    z = joy_axis[2];
    buttons = joy_buttons;
}

int JoyStick::NumButtons() {
    return nr_of_buttons;
}

