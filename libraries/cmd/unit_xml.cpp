/**
 * unit_xml.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020-2025 pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors
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


#include "cmd/unit_xml.h"
#include "src/audiolib.h"
#include "root_generic/xml_support.h"
#include <float.h>
#include <limits.h>
#include "root_generic/configxml.h"
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include <assert.h>
#include "root_generic/xml_serializer.h"
#include "cmd/unit_collide.h"
#include "cmd/unit_generic.h"
#include "collide2/Opcode.h"
#include "collide2/CSopcodecollider.h"

using namespace XMLSupport;

/*ADDED FOR extensible use of unit pretty print and unit load */
UNITLOADTYPE current_unit_load_mode = DEFAULT;

string KillQuadZeros(string inp) {
    std::string::size_type text = 0;
    while ((text = inp.find(".000000", text)) != string::npos) {
        inp = inp.substr(0, text) + inp.substr(text + 7);
    }
    return inp;
}

string MakeUnitXMLPretty(string str, Unit *un) {
    string writestr;
    if (un) {
        writestr += "Name: " + un->name;
        writestr += " " + un->getFullname();
        Flightgroup *fg = un->getFlightgroup();
        if (fg) {
            writestr += " Designation " + fg->name + " " + XMLSupport::tostring(un->getFgSubnumber());
        }
        writestr += "\n";
    }
    static std::set<string> lookfor;
    if (lookfor.empty()) {
        lookfor.insert("Shie");
        lookfor.insert("Armo");
//lookfor.insert ("Hull");
        lookfor.insert("Reac");
        lookfor.insert("Moun");
        lookfor.insert("Comp");
//lookfor.insert ("Desc");
        lookfor.insert("Engi");
        lookfor.insert("Mane");
        lookfor.insert("Jump");
//lookfor.insert ("Defe");
        lookfor.insert("Stat");
        lookfor.insert("Engi");
//lookfor.insert ("Hold");
        lookfor.insert("Rada");
    }
    std::string::size_type foundpos;
    while ((foundpos = str.find("<")) != string::npos) {
        if (str.size() <= foundpos + 1) {
            break;
        }
        str = str.substr(foundpos + 1);
        if (str.size() > 3) {
            char mycomp[5] = {str[0], str[1], str[2], str[3], 0};
            if (lookfor.find(mycomp) != lookfor.end()) {
                int newline = str.find(">");
                if (newline > 0) {
                    if (str[newline - 1] == '/') {
                        newline -= 1;
                    }
                }
                writestr += KillQuadZeros(str.substr(0, newline) + "\n");
            }
        }
    }
    return writestr;
}

// TODO: delete this at some point.
// We no longer support add/mul modes of upgrades
int GetModeFromName(const char *input_buffer) {
    if (strlen(input_buffer) > 3) {
        if (input_buffer[0] == 'a'
                && input_buffer[1] == 'd'
                && input_buffer[2] == 'd') {
            return 1;
        }
        if (input_buffer[0] == 'm'
                && input_buffer[1] == 'u'
                && input_buffer[2] == 'l') {
            return 2;
        }
    }
    return 0;
}

extern bool CheckAccessory(Unit *);

void addShieldMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg) {
    static bool forceit = vega_config::config->graphics.forceOneOneShieldBlend;
    xml->shieldmesh = Mesh::LoadMesh(filename, Vector(scale, scale, scale), faction, fg);
    if (xml->shieldmesh && forceit) {
        xml->shieldmesh->SetBlendMode(ONE, ONE, true);
        xml->shieldmesh->setEnvMap(false, true);
        xml->shieldmesh->setLighting(true, true);
    }
}

void addRapidMesh(Unit::XML *xml, const char *filename, const float scale, int faction, class Flightgroup *fg) {
    xml->rapidmesh = Mesh::LoadMesh(filename, Vector(scale, scale, scale), faction, fg);
}

void pushMesh(std::vector<Mesh *> &meshes,
        float &randomstartframe,
        float &randomstartseconds,
        const char *filename,
        const float scale,
        int faction,
        class Flightgroup *fg,
        int startframe,
        double texturestarttime) {
    vector<Mesh *> m = Mesh::LoadMeshes(filename, Vector(scale, scale, scale), faction, fg);
    for (unsigned int i = 0; i < m.size(); ++i) {
        meshes.push_back(m[i]);
        if (startframe >= 0) {
            meshes.back()->setCurrentFrame(startframe);
        } else if (startframe == -2) {
            float r = ((float) rand()) / RAND_MAX;
            meshes.back()->setCurrentFrame(r * meshes.back()->getFramesPerSecond());
        } else if (startframe == -1) {
            if (randomstartseconds == 0) {
                randomstartseconds =
                        randomstartframe * meshes.back()->getNumLOD() / meshes.back()->getFramesPerSecond();
            }
            meshes.back()->setCurrentFrame(randomstartseconds * meshes.back()->getFramesPerSecond());
        }
        if (texturestarttime > 0) {
            meshes.back()->setTextureCumulativeTime(texturestarttime);
        } else {
            float fps = meshes.back()->getTextureFramesPerSecond();
            int frames = meshes.back()->getNumTextureFrames();
            double ran = randomstartframe;
            if (fps > 0 && frames > 1) {
                ran *= frames / fps;
            } else {
                ran *= 1000;
            }
            meshes.back()->setTextureCumulativeTime(ran);
        }
    }
}
