/*
 * ship_commands.cpp
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

#include "cmd/unit_generic.h"
#include "src/config_xml.h"
#include "root_generic/xml_support.h"
#include "root_generic/vs_globals.h"
#include "src/universe_util.h"
#include "gldrv/winsys.h"
#include "root_generic/options.h"

#include <math.h>

class ShipCommands {
    Functor<ShipCommands> *csetkps;
    Functor<ShipCommands> *cleft;
    Functor<ShipCommands> *cright;
    Functor<ShipCommands> *cup;
    Functor<ShipCommands> *cdown;
    Functor<ShipCommands> *cslide;
    Functor<ShipCommands> *cfire;
    Functor<ShipCommands> *croll;
    Functor<ShipCommands> *cpymenu;
    bool broll;
    bool bleft;
    bool bright;
    bool bup;
    bool bdown;
public:
    virtual ~ShipCommands() {
        CommandInterpretor->remCommand(cpymenu);
        CommandInterpretor->remCommand(csetkps);
    }

    ShipCommands() {
        //create some functors, register them with the command interp {{{
        cpymenu = new Functor<ShipCommands>(this, &ShipCommands::pymenu);
        CommandInterpretor->addCommand(cpymenu, "pymenu");
        csetkps = new Functor<ShipCommands>(this, &ShipCommands::setkps);
        CommandInterpretor->addCommand(csetkps, "setspeed");
        //}}}
        //set some local bools false {{{
        broll = false;
        bleft = false;
        bright = false;
        bup = false;
        bdown = false;
        //}}}
        //a test menu {{{
        {
            static char const python_test[20] = "python test";
            static char const test_string[40] = "This is a test of the menusystem";
            menu *m = new menu(python_test, test_string, "\r\n");
            CommandInterpretor->addMenu(m);
            {
                mItem *mi = new mItem;
                mi->autoreprint = true;
                mi->Name.append("1");                       //argument to access menu
                mi->display.append("Python One Line input");                       //menu's display name
                mi->func2call.append("python");
                mi->inputbit = true;                         //set single-line input mode
                mi->selectstring
                        .append("Type a single line of Python");                       //call function "Display" with this string
                CommandInterpretor->addMenuItem(mi);
            }
            {
                mItem *mi = new mItem;
                mi->autoreprint = true;                         //auto-re-print the
                //menu after this menuitem is finished
                mi->Name.append("2");                       //argument to access menu
                mi->display.append("(Python Multi-Line input)");                       //menu's display name
                mi->func2call
                        .append("python");                       //call this function when this menuitem is called and input is all recieved, user input is appened with a space, along with the action string if there is one. (string generated: "func2call action userinput")
                mi->inputbit2 = true;                         //set single-line input mode
                mi->selectstring
                        .append("Type multiple lines of python input. Use <ENTER> on a line ALONE to finish");                       //Call function "Display" with this string
                CommandInterpretor->addMenuItem(mi);
            }
        }
    }

    void pymenu();
    void left(bool *isKeyDown);
    void right(bool *isKeyDown);
    void up(bool *isKeyDown);
    void down(bool *isKeyDown);
    void roll(bool *isKeyDown);
    void setkps(const char *in);
};

//these _would_ work if the physics routines polled the ship_commands object
//for these bools..
void ShipCommands::pymenu() {
    std::string response(CommandInterpretor->setMenu("python test"));
    CommandInterpretor->conoutf(response);
}

void ShipCommands::left(bool *isKeyDown) {
    bleft = isKeyDown;
}

void ShipCommands::right(bool *isKeyDown) {
    bright = isKeyDown;
}

void ShipCommands::up(bool *isKeyDown) {
    bup = isKeyDown;
}

void ShipCommands::down(bool *isKeyDown) {
    bdown = isKeyDown;
}

void ShipCommands::roll(bool *isKeyDown) {
    broll = isKeyDown;
}

static ShipCommands *ship_commands = NULL;

void ShipCommands::setkps(const char *in) {
    if (in == NULL) {
        throw "What speed?";
    }
    float kps = XMLSupport::parse_float(std::string(in));
    Unit *player = UniverseUtil::getPlayer();
    if (player) {
        if (configuration().physics.game_speed_lying) {
            kps *= configuration().physics.game_speed;
        } else {
            kps /= configuration().physics.display_in_meters ? 1.0f : 3.6f;
        }
        player->computer.set_speed = fmin(player->MaxSpeed(), kps);
    }
}

void InitShipCommands() {
    if (ship_commands != nullptr) {
        delete ship_commands;
        ship_commands = nullptr;
    }
    ship_commands = new ShipCommands;
}

void UninitShipCommands() {
    if (ship_commands != nullptr) {
        delete ship_commands;
        ship_commands = nullptr;
    }
}
