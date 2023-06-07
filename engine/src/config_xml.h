/*
 * Copyright (C) 2001-2023 Daniel Horn, Alexander Rawass, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
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
#ifndef VEGA_STRIKE_ENGINE_CONFIG_XML_H
#define VEGA_STRIKE_ENGINE_CONFIG_XML_H

/*
 *  xml Configuration written by Alexander Rawass <alexannika@users.sourceforge.net>
 */

#include <expat.h>
#include <string>
#include <gnuhash.h>
#include "configxml.h"
#include "xml_support.h"
#include "easydom.h"
#include "in_kb.h"

using std::string;

using XMLSupport::AttributeList;

typedef vsUMap<string, KBHandler> CommandMap;
typedef vsUMap<string, int> KeyMap;

class GameVegaConfig : public VegaConfig {
public:
    explicit GameVegaConfig(const char *configfile);
#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXIS_THROTTLE 3
private:
    void initCommandMap();
    void initKeyMap();
    CommandMap command_map;
    KeyMap key_map;
    int hs_value_index;
//vector<vColor *> colors;
    void bindKeys();
    void doBindings(configNode *node);
    void checkBind(configNode *node);
    void doAxis(configNode *node);
    void checkHatswitch(int nr, configNode *node);
};

#endif //VEGA_STRIKE_ENGINE_CONFIG_XML_H
