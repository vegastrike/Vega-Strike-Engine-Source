/*
 * flyjoystick.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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


#include "src/in_joystick.h"

#include "flyjoystick.h"
#include "firekeyboard.h"
#include "flykeyboard.h"
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include  "in_kb_data.h"

FlyByJoystick::FlyByJoystick(unsigned int configfile) : FlyByKeyboard(configfile) {
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        if ((unsigned int) joystick[i]->player == configfile) {
            whichjoystick.push_back(i);
        }
    }
    //remember keybindings from config file?

}

void FlyByJoystick::Execute() {
    static boost::optional<bool> clamp_joystick_axes;
    if (clamp_joystick_axes == boost::none) {
        clamp_joystick_axes = configuration()->joystick.clamp_axes;
    }
    static boost::optional<bool> nonlinear_throttle_nav;
    if (nonlinear_throttle_nav == boost::none) {
        nonlinear_throttle_nav = configuration()->joystick.nonlinear_throttle_nav;
    }
    static boost::optional<bool> nonlinear_throttle_combat;
    if (nonlinear_throttle_combat == boost::none) {
        nonlinear_throttle_combat = configuration()->joystick.nonlinear_throttle_combat;
    }
    static boost::optional<float> expfactorn;
    if (expfactorn == boost::none) {
        expfactorn = configuration()->joystick.nonlinear_expfactor_nav_flt;
    }
    static boost::optional<float> pfactorn;
    if (pfactorn == boost::none) {
        pfactorn = configuration()->joystick.nonlinear_pfactor_nav_flt;
    }
    static boost::optional<float> expamountn;
    if (expamountn == boost::none) {
        expamountn = configuration()->joystick.nonlinear_expamount_nav_flt;
    }
    static boost::optional<float> pamountn;
    if (pamountn == boost::none) {
        pamountn = configuration()->joystick.nonlinear_pamount_nav_flt;
    }
    static boost::optional<float> expfactorc;
    if (expfactorc == boost::none) {
        expfactorc = configuration()->joystick.nonlinear_expfactor_combat_flt;
    }
    static boost::optional<float> pfactorc;
    if (pfactorc == boost::none) {
        pfactorc = configuration()->joystick.nonlinear_pfactor_combat_flt;
    }
    static boost::optional<float> expamountc;
    if (expamountc == boost::none) {
        expamountc = configuration()->joystick.nonlinear_expamount_combat_flt;
    }
    static boost::optional<float> pamountc;
    if (pamountc == boost::none) {
        pamountc = configuration()->joystick.nonlinear_pamount_combat_flt;
    }
    desired_ang_velocity = Vector(0, 0, 0);
    for (unsigned int i = 0; i < this->whichjoystick.size(); i++) {
        int which_joystick = this->whichjoystick[i];
        if (which_joystick < MAX_JOYSTICKS) {
            int joy_nr;

            joy_nr = vs_config->axis_joy[AXIS_Y];
            if (joy_nr != -1) {
                joy_nr = which_joystick;
            }
            if (joy_nr != -1 && joystick[joy_nr]->isAvailable()) {
                int config_axis = joystick[joy_nr]->axis_axis[AXIS_Y];
                if (config_axis != -1) {
                    bool inverse = joystick[joy_nr]->axis_inverse[AXIS_Y];
                    float axis_value = -joystick[joy_nr]->joy_axis[config_axis];
                    if (inverse) {
                        axis_value = -axis_value;
                    }
                    if (clamp_joystick_axes) {
                        if (axis_value < -1) {
                            axis_value = -1;
                        }
                        if (axis_value > 1) {
                            axis_value = 1;
                        }
                    }
                    if ((joy_mode == joyModeNormal) || (joy_mode & (joyModeRoll | joyModeBank))) {
                        Up(axis_value);
                    } else if (joy_mode & joyModeInertialXY) {
                        DirectThrustUp(axis_value);
                    } else if (joy_mode & joyModeInertialXZ) {
                        DirectThrustFront(-axis_value);
                    }
                }
            }
            joy_nr = vs_config->axis_joy[AXIS_X];
            if (joy_nr != -1) {
                joy_nr = which_joystick;
            }
            if (joy_nr != -1 && joystick[joy_nr]->isAvailable()) {
                int config_axis = joystick[joy_nr]->axis_axis[AXIS_X];
                if (config_axis != -1) {
                    bool inverse = joystick[joy_nr]->axis_inverse[AXIS_X];
                    float axis_value = -joystick[joy_nr]->joy_axis[config_axis];
                    if (inverse) {
                        axis_value = -axis_value;
                    }
                    if (clamp_joystick_axes) {
                        if (axis_value < -1) {
                            axis_value = -1;
                        }
                        if (axis_value > 1) {
                            axis_value = 1;
                        }
                    }
                    if (joy_mode == joyModeNormal) {
                        Right(axis_value);
                    } else if (joy_mode & joyModeRoll) {
                        RollRight(-axis_value);
                    } else if (joy_mode & (joyModeInertialXY | joyModeInertialXZ)) {
                        DirectThrustRight(axis_value);
                    } else if (joy_mode & joyModeBank) {
                        Right(axis_value), RollRight(-axis_value);
                    }
                }
            }
            joy_nr = vs_config->axis_joy[AXIS_Z];
            if (joy_nr != -1) {
                joy_nr = which_joystick;
            }
            if (joy_nr != -1 && joystick[joy_nr]->isAvailable()) {
                int config_axis = joystick[joy_nr]->axis_axis[AXIS_Z];
                if (config_axis != -1) {
                    bool inverse = joystick[joy_nr]->axis_inverse[AXIS_Z];
                    float axis_value = -joystick[joy_nr]->joy_axis[config_axis];
                    if (inverse) {
                        axis_value = -axis_value;
                    }
                    if (clamp_joystick_axes) {
                        if (axis_value < -1) {
                            axis_value = -1;
                        }
                        if (axis_value > 1) {
                            axis_value = 1;
                        }
                    }
                    if ((joy_mode == joyModeNormal) || (joy_mode & joyModeBank)) {
                        RollRight(axis_value);
                    } else if (joy_mode & joyModeRoll) {
                        Right(axis_value);
                    } else if (joy_mode & joyModeInertialXY) {
                        DirectThrustFront(axis_value);
                    } else if (joy_mode & joyModeInertialXZ) {
                        DirectThrustUp(axis_value);
                    }
                }
            }
            joy_nr = vs_config->axis_joy[AXIS_THROTTLE];
            if (joy_nr != -1) {
                joy_nr = which_joystick;
            }
            if (joy_nr != -1 && joystick[joy_nr]->isAvailable()) {
                int config_axis = joystick[joy_nr]->axis_axis[AXIS_THROTTLE];
                if (config_axis != -1) {
                    bool inverse = joystick[joy_nr]->axis_inverse[AXIS_THROTTLE];
                    float axis_value = -joystick[joy_nr]->joy_axis[config_axis];
                    if (inverse) {
                        axis_value = -axis_value;
                    }
                    Computer *cpu = &parent->computer;
                    if (axis_value > 1) {
                        axis_value = 1;
                    }
                    if (axis_value < -1) {
                        axis_value = -1;
                    }                          //this code due to paolodinca

                    //put axis from 0 to 2
                    axis_value++;
                    //put axis from 0 to 1
                    axis_value = axis_value / 2;                      //thanks!
                    if (nonlinear_throttle_nav && !cpu->combat_mode) {
                        static float norm = exp(expfactorn.get()) - 1;
                        axis_value =
                                expamountn.get() * (exp(expfactorn.get() * axis_value) - 1) / norm
                                    + pamountn.get() * std::pow(axis_value, pfactorn.get());
                    } else if (nonlinear_throttle_combat && cpu->combat_mode) {
                        static float norm = exp(expfactorc.get()) - 1;
                        axis_value =
                                expamountc.get() * (exp(expfactorc.get() * axis_value) - 1) / norm
                                + pamountc.get() * std::pow(axis_value, pfactorc.get());
                    }
                    cpu->set_speed = axis_value * parent->MaxSpeed();
                    desired_velocity = Vector(0, 0, cpu->set_speed);
                }
            }
        }
    }
    FlyByKeyboard::Execute(false);
}

FlyByJoystick::~FlyByJoystick() {
}

