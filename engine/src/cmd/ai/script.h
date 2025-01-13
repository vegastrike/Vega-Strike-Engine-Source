/*
 * script.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_SCRIPT_H
#define VEGA_STRIKE_ENGINE_CMD_AI_SCRIPT_H

#include "order.h"
#include "navigation.h"
#include "xml_support.h"

/**
 * Loads a script from a given XML file
 * FIXME: This data is not cached and is streamed
 * from harddrive and expat'ed each time a script is loaded
 */
struct AIScriptXML;
class AIScript : public Order {
///File name the AI script takes, to be loaded upon first execute (needs ref to parent)
    char *filename;
///Temporary data to hold while AI script loads
    AIScriptXML *xml;
///Loads the XML file, filename when Execute() is called
    void LoadXML(); //load the xml
///Internal functions to expat
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
///internal functions for use of expat
    static void endElement(void *userData, const XML_Char *name);
///The top float on the current stack
    float &topf();
///Rid of the top float on the current stack
    void popf();
///The top vector on the current stack
    QVector &topv();
///Pop the top vector of teh current stack
    void popv();
///member function begin elements... deals with pushing vectors on stack
    void beginElement(const string &name, const XMLSupport::AttributeList &attributes);
///member function end elements...deals with calling AI scripts from the stack
    void endElement(const string &name);
public:
///saves scriptname in the filename var
    AIScript(const char *scriptname);
    ~AIScript();
///Loads the AI script from the hard drive, or executes if loaded
    void Execute();
};

#endif //VEGA_STRIKE_ENGINE_CMD_AI_SCRIPT_H
