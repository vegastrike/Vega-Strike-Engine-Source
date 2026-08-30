/*
 * controls_factory.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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

#ifndef VEGA_STRIKE_ENGINE_CMD_CONTROLS_FACTORY_H
#define VEGA_STRIKE_ENGINE_CMD_CONTROLS_FACTORY_H

#include <string>
#include <map>

#include "vegadisk/vsfilesystem.h"
#include "vsdraw/control.h"
#include "vsdraw/window.h"

std::map<std::string, std::map<std::string, std::string>> parseControlsJSON(VSFileSystem::VSFile &file, // parse controls from a given VSFile
    std::vector<unsigned int>* base_keyboard_queue); // the keyboard queue
Control* getControl(const std::map<std::string, std::string>& attributes); // construct a single control elenment
bool getControls( // reads a controls.json file and returns all control groups
    const std::string& filename, // the file name of the JSON data 
        Window* window, // the window the group controls will be added to
        std::vector<unsigned int>* base_keyboard_queue // the keyboard queue
);
#endif // VEGA_STRIKE_ENGINE_CMD_CONTROLS_FACTORY_H
