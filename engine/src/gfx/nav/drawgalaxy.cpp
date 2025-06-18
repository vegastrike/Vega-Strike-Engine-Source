/*
 * drawgalaxy.cpp
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


/// Nav computer functions
/// Draws in-system map, and  galaxy map of known sectors and systems

#include <algorithm>
#include <cmath>
#include "root_generic/vs_globals.h"
#include "src/vegastrike.h"
#include "gfx/gauge.h"
#include "gfx/cockpit.h"
#include "src/universe.h"
#include "src/star_system.h"
#include "cmd/unit_generic.h"
#include "cmd/collection.h"
#include "gfx/hud.h"
#include "gfx/vdu.h"
#include "root_generic/lin_time.h" //for fps
#include "src/config_xml.h"
#include "root_generic/lin_time.h"
#include "cmd/images.h"
#include "cmd/script/mission.h"
#include "cmd/script/msgcenter.h"
#include "cmd/ai/flyjoystick.h"
#include "cmd/ai/firekeyboard.h"
#include "cmd/ai/aggressive.h"
#include "src/main_loop.h"
#include <assert.h>     //needed for assert() calls
#include "root_generic/savegame.h"
#include "gfx/animation.h"
#include "gfx_generic/mesh.h"
#include "src/universe_util.h"
#include "src/in_mouse.h"
#include "gui/glut_support.h"
#include "cmd/unit_util.h"
#include "src/hashtable.h"
#include "gfx/nav/navscreen.h"
#include "gfx/masks.h"
#include "gfx/nav/navscreenoccupied.h"

using std::string;
using std::vector;

float SYSTEM_DEFAULT_SIZE = 0.02;
const int systemambiguous = 0;
static GFXColor GrayColor(.5, .5, .5, .5);
float GrayColorArray[4] = {.5, .5, .5, .5};

static void DrawNodeDescription(string text,
        float x_,
        float y_,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        const GFXColor &col,
        navscreenoccupied *screenoccupation) {
    //take the head and stick it in the back
    if (text.size() == 0) {
        return;
    }
    TextPlane displayname;   //will be used to display shits names
    displayname.col = col;
    const float background_alpha = configuration()->graphics.hud.text_background_alpha;
    int length = text.size();
    float offset = (float(length) * 0.005);
    if (ignore_occupied_areas) {
        displayname.SetPos((x_ - offset), y_);
        displayname.SetText(text);
        displayname.SetCharSize(size_x, size_y);

        GFXColor tpbg = displayname.bgcol;
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            displayname.bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        displayname.Draw(text, 0, true, false, automatte);
        displayname.bgcol = tpbg;
    } else {
        float new_y = screenoccupation->findfreesector(x_, y_);
        displayname.SetPos((x_ - offset), new_y);
        displayname.SetText(text);
        displayname.SetCharSize(size_x, size_y);
        GFXColor tpbg = displayname.bgcol;
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            displayname.bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        displayname.Draw(text, 0, true, false, automatte);
        displayname.bgcol = tpbg;
    }
}

static char GetSystemColor(string source) {
    //FIXME: update me!!!
    vector<float> *v = &_Universe->AccessCockpit()->savegame->getMissionData("visited_" + source);
    if (v->size()) {
        float k = (*v)[0];
        if (k >= 2) {
            return k == 2 ? 'm' : '?';
        }
    }
    return 'v';
}

static void DrawNode(int type,
        float size,
        float x,
        float y,
        std::string source,
        navscreenoccupied *screenoccupation,
        bool moused,
        GFXColor race,
        bool mouseover = false,
        bool willclick = false,
        string insector = "") {
    char color = GetSystemColor(source);
    if (moused) {
        return;
    }
    if (willclick == true && mouseover == false) {
        //Perhaps some key binding or mouseclick will be set in the future to do this.
        mouseover = true;
    }
    static GFXColor
            highlighted_tail_col = vs_config->getColor("nav", "highlighted_unit_on_tail", GFXColor(1, .3, .3, .8));
    static GFXColor
            highlighted_tail_text = vs_config->getColor("nav", "highlighted_text_on_tail", GFXColor(1, 1, .7, 1));
    if (color == 'm') {
        race = GrayColor;
    }
    if (mouseover) {
        if (willclick) {
            race = highlighted_tail_col;
        } else {
            //Leave just a faint resemblence of the original color,
            //but also make it look whiteish.
            race.r += .75;
            race.g += .75;
            race.b += .75;
        }
    }
    NavigationSystem::DrawCircle(x, y, size, race);
    if ((!mouseover) || (willclick)) {
        string tsector, nam;
        Beautify(source, tsector, nam);
        if (willclick) {
            race = highlighted_tail_text;
            nam = tsector + " / " + nam;
        }
        if (willclick || !(insector.compare("")) || !(insector.compare(tsector))) {
            DrawNodeDescription(nam, x, y, 1.0, 1.0, 0, race, screenoccupation);
        }
    }
}

class systemdrawnode {
    int type;
    float size;
    float x;
    float y;
    unsigned index;
    std::string source;
//Vector of indicies
//std::vector<int> *dest; //let's just hope that the iterator doesn't get killed during the frame, which shouldn't happen.
//std::vector<string> *stringdest; //let's just hope that the iterator doesn't get killed during the frame, which shouldn't happen.
    bool moused;
    char color;
    GFXColor race;
    navscreenoccupied *screenoccupation;
public:
    unsigned getIndex() const {
        return index;
    }

    friend bool operator<(const systemdrawnode &a, const systemdrawnode &b) {
        return a.source < b.source;
    }

    friend bool operator==(const systemdrawnode &a, const systemdrawnode &b) {
        return a.source == b.source;
    }

    systemdrawnode(int type,
            float size,
            float x,
            float y,
            std::string source,
            unsigned index,
            navscreenoccupied *so,
            bool moused,
            GFXColor race)
            : type(type),
            size(size),
            x(x),
            y(y),
            index(index),
            source(source),
            moused(moused),
            color(GetSystemColor(source)),
            race(race),
            screenoccupation(so) {
    }

    systemdrawnode()
            : size(SYSTEM_DEFAULT_SIZE),
            x(0),
            y(0),
            index(0),
            source(),
            moused(false),
            color('v'),
            race(GrayColor),
            screenoccupation(NULL) {
    }

    void draw(bool mouseover = false, bool willclick = false) {
        DrawNode(type, size, x, y, source, screenoccupation, moused, race, mouseover, willclick);
    }
};

typedef vector<systemdrawnode> systemdrawlist;
//typedef Hashtable <std::string, const systemdrawnode, 127> systemdrawhashtable;

bool testandset(bool &b, bool val) {
    bool tmp = b;
    b = val;
    return tmp;
}

float screensmash = 1; //arbitrary constant used in calculating position below

NavigationSystem::SystemIterator::SystemIterator(string current_system, unsigned int max) {
    count = 0;
    maxcount = max;
    vstack.push_back(current_system);
    visited[current_system] = true;
    which = 0;
}

bool NavigationSystem::SystemIterator::done() const {
    return which >= vstack.size();
}

QVector NavigationSystem::SystemIterator::Position() {
    if (done()) {
        return QVector(0, 0, 0);
    }
    string currentsystem = (**this);
    string xyz = _Universe->getGalaxyProperty(currentsystem, "xyz");
    QVector pos;
    if (xyz.size() && (sscanf(xyz.c_str(), "%lf %lf %lf", &pos.i, &pos.j, &pos.k) >= 3)) {
        pos.j = -pos.j;
        return pos;
    } else {
        float ratio = ((float) count) / maxcount;
        float locatio = ((float) which) / vstack.size();
        unsigned int k = 0;
        std::string::const_iterator start = vstack[which].begin();
        std::string::const_iterator end = vstack[which].end();
        for (; start != end; start++) {
            k += (k * 128) + *start;
        }
        k %= 200000;
//float y = (k-100000)/(200000.);
        return QVector(ratio * cos(locatio * 2 * 3.1415926536), ratio * sin(locatio * 2 * 3.1415926536), 0)
                * screensmash;
    }
}

string NavigationSystem::SystemIterator::operator*() {
    if (which < vstack.size()) {
        return vstack[which];
    }
    return "-";
}

NavigationSystem::SystemIterator &NavigationSystem::SystemIterator::next() {
    return ++(*this);
}

bool checkedVisited(const std::string &n) {
    const bool dontbothervisiting =
            !configuration()->graphics.explore_for_map;
    if (dontbothervisiting) {
        return true;
    } else {
        string key(string("visited_") + n);
        vector<float> *v = &_Universe->AccessCockpit()->savegame->getMissionData(key);
        if (v->size() > 0) {
            return true;
        }
        return false;
    }
}

NavigationSystem::SystemIterator &NavigationSystem::SystemIterator::operator++() {
    which += 1;
    if (which >= vstack.size()) {
        vector<string> newsys;
        for (unsigned int i = 0; i < vstack.size(); ++i) {
            int nas = UniverseUtil::GetNumAdjacentSystems(vstack[i]);
            for (int j = 0; j < nas; ++j) {
                string n = UniverseUtil::GetAdjacentSystem(vstack[i], j);
                if (!testandset(visited[n], true)) {
//if (checkedVisited(n)) {
                    newsys.push_back(n);
                }
//}
            }
        }
        vstack.swap(newsys);
        count += 1;
        which = 0;
        if (count > maxcount) {
            vstack.clear();
        }
    }
    return *this;
}

//************************************************
//
//SYSTEM SECTION
//
//************************************************

void NavigationSystem::CachedSystemIterator::SystemInfo::UpdateColor() {
    const float *tcol =
            ((!name.empty())
                    && (name
                            != "-"))
                    ? FactionUtil::GetSparkColor(FactionUtil::GetFactionIndex(UniverseUtil::GetGalaxyFaction(name)))
                    : &(
                    GrayColorArray[0]);
    col = GFXColor(tcol[0], tcol[1], tcol[2], tcol[3]);
}

//May generate incorrect links to destinations, if called before destinations have been added.
//Since links are bidirectional, one WILL have to be created before the other
//It is recommended that placeholders are created and links updated later
NavigationSystem::CachedSystemIterator::SystemInfo::SystemInfo(const string &name,
        const QVector &position,
        const std::vector<std::string> &destinations,
        NavigationSystem::CachedSystemIterator *csi) :
        name(name), position(position), part_of_path(false) {
    //Eww... double for loop!
    UpdateColor();
    if (csi) {
        for (size_t i = 0; i < destinations.size(); ++i) {
            for (size_t j = 0; j < csi->systems.size(); ++j) {
                if ((*csi)[j].name == destinations[i]) {
                    lowerdestinations.push_back(j);
                    if (std::find((*csi)[j].lowerdestinations.begin(), (*csi)[j].lowerdestinations.end(),
                            i) == (*csi)[j].lowerdestinations.end()) {
                        (*csi)[j].lowerdestinations
                                .push_back(i);
                    }                            //this is in case of asymmetric links
                    //Push the destination back.
                    //Tasty....
                    //Mmm.......
                    //Destination tastes like chicken
                }
            }
        }
    }
}

//Create a placeholder to avoid the problem described above
NavigationSystem::CachedSystemIterator::SystemInfo::SystemInfo(const string &name) :
        name(name), part_of_path(false) {
}

void NavigationSystem::CachedSystemIterator::SystemInfo::loadData(map<string, unsigned> *index_table) {
    string xyz = _Universe->getGalaxyProperty(name, "xyz");
    QVector pos;
    if (xyz.size() && (sscanf(xyz.c_str(), "%lf %lf %lf", &pos.i, &pos.j, &pos.k) >= 3)) {
        pos.j = -pos.j;
    } else {
        pos.i = 0;
        pos.j = 0;
        pos.k = 0;
    }
    position = pos;

    UpdateColor();
    const vector<std::string> &destinations = _Universe->getAdjacentStarSystems(name);
    for (size_t i = 0; i < destinations.size(); ++i) {
        if (index_table->count(destinations[i]) != 0) {
            lowerdestinations.push_back((*index_table)[destinations[i]]);
        }
    }
}

void NavigationSystem::CachedSystemIterator::init(string current_system, unsigned max_systems) {
    systems.clear();
    unsigned count = 0;
    string sys;

    map<string, unsigned> index_table;
    std::deque<std::string> frontier;
    frontier.push_back(current_system);
    systems.push_back(SystemInfo(current_system));
    index_table[current_system] = 0;
    while (!frontier.empty()) {
        sys = frontier.front();
        frontier.pop_front();

        int nas = UniverseUtil::GetNumAdjacentSystems(sys);
        for (int j = 0; j < nas && count < max_systems; ++j) {
            string n = UniverseUtil::GetAdjacentSystem(sys, j);
            if (index_table.count(n) == 0) {
                frontier.push_back(n);
                index_table[n] = systems.size();
                systems.push_back(SystemInfo(n));
                ++count;
            }
        }
        systems[index_table[sys]].loadData(&index_table);
    }
}

NavigationSystem::CachedSystemIterator::CachedSystemIterator() {
}

NavigationSystem::CachedSystemIterator::CachedSystemIterator(string current_system, unsigned max_systems) {
    init(current_system, max_systems);
}

NavigationSystem::CachedSystemIterator::CachedSystemIterator(const CachedSystemIterator &other) :
        systems(other.systems), currentPosition(other.currentPosition) {
}

bool NavigationSystem::CachedSystemIterator::seek(unsigned position) {
    if (position < systems.size()) {
        currentPosition = position;
        return true;
    } else {
        return false;
    }
}

unsigned NavigationSystem::CachedSystemIterator::getIndex() const {
    return currentPosition;
}

unsigned NavigationSystem::CachedSystemIterator::size() const {
    return systems.size();
}

bool NavigationSystem::CachedSystemIterator::done() const {
    return currentPosition >= systems.size();
}

static NavigationSystem::CachedSystemIterator::SystemInfo nullPair("-", QVector(0, 0, 0),
        std::vector<std::string>(), NULL);

NavigationSystem::CachedSystemIterator::SystemInfo &NavigationSystem::CachedSystemIterator::operator[](unsigned pos) {
    if (pos >= size()) {
        return nullPair;
    }
    return systems[pos];
}

const NavigationSystem::CachedSystemIterator::SystemInfo
&NavigationSystem::CachedSystemIterator::operator[](unsigned pos) const {
    if (pos >= size()) {
        return nullPair;
    }
    return systems[pos];
}

NavigationSystem::CachedSystemIterator::SystemInfo &NavigationSystem::CachedSystemIterator::operator*() {
    if (done()) {
        return nullPair;
    }
    return systems[currentPosition];
}

const NavigationSystem::CachedSystemIterator::SystemInfo &NavigationSystem::CachedSystemIterator::operator*() const {
    if (done()) {
        return nullPair;
    }
    return systems[currentPosition];
}

NavigationSystem::CachedSystemIterator::SystemInfo *NavigationSystem::CachedSystemIterator::operator->() {
    if (done()) {
        return &nullPair;
    }
    return &systems[currentPosition];
}

const NavigationSystem::CachedSystemIterator::SystemInfo *NavigationSystem::CachedSystemIterator::operator->() const {
    if (done()) {
        return &nullPair;
    }
    return &systems[currentPosition];
}

string &NavigationSystem::CachedSystemIterator::SystemInfo::GetName() {
    return name;
}

const string &NavigationSystem::CachedSystemIterator::SystemInfo::GetName() const {
    return name;
}

bool NavigationSystem::CachedSystemIterator::SystemInfo::isDrawable() const {
    return checkedVisited(GetName());
}

QVector &NavigationSystem::CachedSystemIterator::SystemInfo::Position() {
    return position;
}

const QVector &NavigationSystem::CachedSystemIterator::SystemInfo::Position() const {
    return position;
}

unsigned NavigationSystem::CachedSystemIterator::SystemInfo::GetDestinationIndex(unsigned index) const {
    return lowerdestinations[index];
}

unsigned NavigationSystem::CachedSystemIterator::SystemInfo::GetDestinationSize() const {
    return lowerdestinations.size();
}

GFXColor NavigationSystem::CachedSystemIterator::SystemInfo::GetColor() {
    static unsigned long lastupdate = 0;
    lastupdate += 1299811;
    lastupdate %= 104729;
    if (lastupdate < 32) {
        UpdateColor();
    }
    return col;
}

NavigationSystem::CachedSystemIterator &NavigationSystem::CachedSystemIterator::next() {
    return ++(*this);
}

NavigationSystem::CachedSystemIterator &NavigationSystem::CachedSystemIterator::operator++() {
    ++currentPosition;
    return *this;
}

NavigationSystem::CachedSystemIterator NavigationSystem::CachedSystemIterator::operator++(int) {
    NavigationSystem::CachedSystemIterator iter(*this);
    ++(*this);
    return iter;
}

//************************************************
//
//SECTOR SECTION
//
//************************************************

NavigationSystem::CachedSectorIterator::SectorInfo::SectorInfo(const string &name) :
        name(name) {
}

string &NavigationSystem::CachedSectorIterator::SectorInfo::GetName() {
    return name;
}

const string &NavigationSystem::CachedSectorIterator::SectorInfo::GetName() const {
    return name;
}

unsigned NavigationSystem::CachedSectorIterator::SectorInfo::GetSubsystemIndex(unsigned index) const {
    return subsystems[index];
}

unsigned NavigationSystem::CachedSectorIterator::SectorInfo::GetSubsystemSize() const {
    return subsystems.size();
}

void NavigationSystem::CachedSectorIterator::SectorInfo::AddSystem(unsigned index) {
    subsystems.push_back(index);
}

NavigationSystem::CachedSectorIterator::CachedSectorIterator() {
}

NavigationSystem::CachedSectorIterator::CachedSectorIterator(CachedSystemIterator &systemIter) {
    init(systemIter);
}

void NavigationSystem::CachedSectorIterator::init(CachedSystemIterator &systemIter) {
    map<string, unsigned> index_table;
    sectors.clear();

    string sys, csector, csystem;
    unsigned index;

    systemIter.seek();
    while (!systemIter.done()) {
        sys = systemIter->GetName();
        Beautify(sys, csector, csystem);
        if (index_table.count(csector) == 0) {
            index_table[csector] = sectors.size();
            sectors.push_back(SectorInfo(csector));
        }
        index = index_table[csector];
        sectors[index].AddSystem(systemIter.getIndex());
        ++systemIter;
    }
}

NavigationSystem::CachedSectorIterator::CachedSectorIterator(const CachedSectorIterator &other) :
        sectors(other.sectors), currentPosition(other.currentPosition) {
}

bool NavigationSystem::CachedSectorIterator::seek(unsigned position) {
    if (position < sectors.size()) {
        currentPosition = position;
        return true;
    } else {
        return false;
    }
}

unsigned NavigationSystem::CachedSectorIterator::getIndex() const {
    return currentPosition;
}

unsigned NavigationSystem::CachedSectorIterator::size() const {
    return sectors.size();
}

bool NavigationSystem::CachedSectorIterator::done() const {
    return currentPosition >= sectors.size();
}

static NavigationSystem::CachedSectorIterator::SectorInfo nullSectorPair("-");

NavigationSystem::CachedSectorIterator::SectorInfo &NavigationSystem::CachedSectorIterator::operator[](unsigned pos) {
    if (pos >= size()) {
        return nullSectorPair;
    }
    return sectors[pos];
}

const NavigationSystem::CachedSectorIterator::SectorInfo
&NavigationSystem::CachedSectorIterator::operator[](unsigned pos) const {
    if (pos >= size()) {
        return nullSectorPair;
    }
    return sectors[pos];
}

NavigationSystem::CachedSectorIterator::SectorInfo &NavigationSystem::CachedSectorIterator::operator*() {
    if (done()) {
        return nullSectorPair;
    }
    return sectors[currentPosition];
}

const NavigationSystem::CachedSectorIterator::SectorInfo &NavigationSystem::CachedSectorIterator::operator*() const {
    if (done()) {
        return nullSectorPair;
    }
    return sectors[currentPosition];
}

NavigationSystem::CachedSectorIterator::SectorInfo *NavigationSystem::CachedSectorIterator::operator->() {
    if (done()) {
        return &nullSectorPair;
    }
    return &sectors[currentPosition];
}

const NavigationSystem::CachedSectorIterator::SectorInfo *NavigationSystem::CachedSectorIterator::operator->() const {
    if (done()) {
        return &nullSectorPair;
    }
    return &sectors[currentPosition];
}

NavigationSystem::CachedSectorIterator &NavigationSystem::CachedSectorIterator::next() {
    return ++(*this);
}

NavigationSystem::CachedSectorIterator &NavigationSystem::CachedSectorIterator::operator++() {
    ++currentPosition;
    return *this;
}

NavigationSystem::CachedSectorIterator NavigationSystem::CachedSectorIterator::operator++(int) {
    NavigationSystem::CachedSectorIterator iter(*this);
    ++(*this);
    return iter;
}

//static systemdrawhashtable jumptable;
float vsmax(float x, float y) {
    return x > y ? x : y;
}

void NavigationSystem::DrawGalaxy() {
//systemdrawlist mainlist;//(0, screenoccupation, factioncolours);	//	lists of items to draw that are in mouse range

    systemdrawlist
            mouselist;     //(1, screenoccupation, factioncolours);	//	lists of items to draw that are in mouse range

    string csector, csystem;

    Beautify(getCurrentSystem(), csector, csystem);
    //what's my name
    //***************************
    TextPlane systemname;       //will be used to display shits names
    string systemnamestring = "Current System : " + csystem + " in the " + csector + " Sector.";

//int length = systemnamestring.size();
//float offset = (float(length)*0.005);
    systemname.col = GFXColor(1, 1, .7, 1);
    systemname.SetPos(screenskipby4[0], screenskipby4[3]);     //Looks ugly when name is too long and goes off the edge.
//systemname.SetPos( (((screenskipby4[0]+screenskipby4[1])/2)-offset) , screenskipby4[3]);
    systemname.SetText(systemnamestring);
//systemname.SetCharSize(1, 1);
    systemname.Draw();
    //***************************

    QVector pos;        //item position
    QVector pos_flat;           //item position flat on plane

    float zdistance = 0.0;
    float zscale = 1.0;
    int l;

    Adjust3dTransformation(galaxy_view == VIEW_3D, 0);

    float center_nav_x = ((screenskipby4[0] + screenskipby4[1]) / 2);
    float center_nav_y = ((screenskipby4[2] + screenskipby4[3]) / 2);
    //**********************************

    glEnable(GL_ALPHA);
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    if (!camera_z) {
        float max_x = 0.0;
        float min_x = 0.0;
        float max_y = 0.0;
        float min_y = 0.0;
        float max_z = 0.0;
        float min_z = 0.0;

//themaxvalue = fabs(pos.i);
        themaxvalue = 0.0;

        {
            {
                systemIter.seek(focusedsystemindex);
                pos = systemIter->Position();
                ReplaceAxes(pos);
//if(galaxy_view==VIEW_3D){pos = dxyz(pos, 0, ry, 0);pos = dxyz(pos, rx, 0, 0);}

                max_x = (float) pos.i;
                min_x = (float) pos.i;
                max_y = (float) pos.j;
                min_y = (float) pos.j;
                max_z = (float) pos.k;
                min_z = (float) pos.k;

                //Find Centers
                //**********************************
                //This will use the current system as the center
                center_x = pos.i;
                center_y = pos.j;
                center_z = pos.k;
                //**********************************

                unsigned destsize = systemIter->GetDestinationSize();
                if (destsize != 0) {
                    for (unsigned i = 0; i < destsize; ++i) {
                        QVector posoth = systemIter[systemIter->GetDestinationIndex(i)].Position();
                        ReplaceAxes(posoth);
//if(galaxy_view==VIEW_3D){posoth = dxyz(pos, 0, ry, 0);posoth = dxyz(pos, rx, 0, 0);}

                        RecordMinAndMax(posoth, min_x, max_x, min_y, max_y, min_z, max_z, themaxvalue);
                    }
                }
            }
        }

        //Find Centers
        //**********************************
        //this will make the center be the center of the displayable area.
//center_x = (min_x + max_x)/2;
//center_y = (min_y + max_y)/2;
//center_z = (min_z + max_z)/2;
        //**********************************

/*	        min_x = (min_x+center_x)/2;
 *               min_y = (min_y+center_y)/2;
 *               min_z = (min_z+center_z)/2;
 *               max_x = (max_x+center_x)/2;
 *               max_y = (max_y+center_y)/2;
 *               max_z = (max_z+center_z)/2;
 */
        //Set Camera Distance
        //**********************************

#define SQRT3 1.7320508
//themaxvalue = sqrt(themaxvalue*themaxvalue + themaxvalue*themaxvalue + themaxvalue*themaxvalue);
//themaxvalue = SQRT3*themaxvalue;

        themaxvalue *= 3;

        {
            float half_x = vsmax(max_x - center_x, center_x - min_x);
            float half_y = vsmax(max_y - center_y, center_y - min_y);
            float half_z = vsmax(max_z - center_z, center_z - min_z);

//float half_x =(0.5*(max_x - min_x));
//float half_y =(0.5*(max_y - min_y));
//float half_z =(0.5*(max_z - min_z));

            camera_z = sqrt((half_x * half_x) + (half_y * half_y) + (half_z * half_z));

//float halfmax = 0.5*themaxvalue;
//camera_z = sqrt( (halfmax*halfmax) + (halfmax*halfmax) + (halfmax*halfmax) );
//camera_z = 4.0*themaxvalue;
//camera_z = tihemaxvalue;
        }

        //**********************************
    }
    DrawOriginOrientationTri(center_nav_x, center_nav_y, 0);

    //Enlist the items and attributes
    //**********************************
    systemIter.seek();
    while (!systemIter.done()) {
        //this draws the points
        //IGNORE UNDRAWABLE SYSTEMS
        //**********************************
        if (!systemIter->isDrawable()) {
            ++systemIter;
            continue;
        }
        //**********************************

        //Retrieve unit data
        //**********************************
        unsigned temp = systemIter.getIndex();

        pos = systemIter->Position();

        ReplaceAxes(pos);             //poop

        //Modify by old rotation amount
        //*************************
//if(galaxy_view==VIEW_3D){pos = dxyz(pos, 0, ry, 0);pos = dxyz(pos, rx, 0, 0);}
        //*************************
        //*************************

        GFXColor col = systemIter->GetColor();
        float the_x, the_y, the_x_flat, the_y_flat, system_item_scale_temp;
        TranslateCoordinates(pos,
                pos_flat,
                center_nav_x,
                center_nav_y,
                themaxvalue,
                zscale,
                zdistance,
                the_x,
                the_y,
                the_x_flat,
                the_y_flat,
                system_item_scale_temp,
                0);
        float alphaadd;
        {
            float tmp = (1 - (zoom / MAXZOOM));
            alphaadd = (tmp * tmp) - .5;
//if (alphaadd<=0)
//alphaadd=0;
//else
            alphaadd *= 4;
        }
        col.a = (system_item_scale_temp - minimumitemscaledown) / (maximumitemscaleup - minimumitemscaledown)
                + alphaadd;
//col.a=GetAlpha(oldpos,center_x,center_y,center_z,zdistance);
        //IGNORE DIM AND OFF SCREEN SYETEMS
        //**********************************
        if ((col.a < .05)
                || (!TestIfInRange(screenskipby4[0],
                        screenskipby4[1],
                        screenskipby4[2],
                        screenskipby4[3],
                        the_x,
                        the_y))) {
            ++systemIter;
            continue;
        }
        //**********************************

        //FIND OUT IF SYSTEM IS PART OF A VISIBLE PATH
        //**********************************
        bool isPath = false;
        if (path_view != PATH_OFF) {
            if (systemIter->part_of_path) {
                for (std::set<NavPath *>::iterator paths = systemIter->paths.begin();
                        paths != systemIter->paths.end();
                        ++paths) {
                    if ((*paths)->getVisible()) {
                        isPath = true;
                        break;
                    }
                }
            }
        }
        //**********************************
        //IGNORE NON-PATH SYSTEMS IN PATH_ONLY MODE
        //**********************************
        if (!isPath && path_view == PATH_ONLY) {
            ++systemIter;
            continue;
        }
        //**********************************

        int insert_type = systemambiguous;
        float insert_size = SYSTEM_DEFAULT_SIZE;
        if (system_item_scale_temp > (system_item_scale * 3)) {
            system_item_scale_temp = (system_item_scale * 3);
        }
        insert_size *= system_item_scale_temp / 3;
        if (currentsystemindex == temp) {
            DrawTargetCorners(the_x, the_y, (insert_size), currentcol);
        }
        if (destinationsystemindex == temp) {
            DrawTargetCorners(the_x, the_y, (insert_size) * 1.2, destinationcol);
        }
        if (systemselectionindex == temp) {
            DrawTargetCorners(the_x, the_y, (insert_size) * 1.4, selectcol);
        }
        bool moused = false;
        DrawNode(insert_type, insert_size, the_x, the_y,
                (*systemIter).GetName(), screenoccupation, moused, isPath ? pathcol : col, false, false,
                isPath ? "" : csector);
        if (std::fabs(zdistance) < 2.0f * camera_z) {
            DisplayOrientationLines(the_x, the_y, the_x_flat, the_y_flat, 0);
        }
        if (TestIfInRangeRad(the_x, the_y, insert_size, mouse_x_current, mouse_y_current)) {
            mouselist.push_back(systemdrawnode(insert_type, insert_size, the_x, the_y, (*systemIter).GetName(),
                    systemIter.getIndex(), screenoccupation, false, isPath ? pathcol : col));
            moused = true;
        }
        unsigned destsize = systemIter->GetDestinationSize();
        if (destsize != 0) {
            GFXDisable(LIGHTING);
            GFXDisable(TEXTURE0);
            const int vsize = 3 + 4;
            std::vector<float> verts(2 * destsize * vsize);
            std::vector<float>::iterator v = verts.begin();
            for (unsigned i = 0; i < destsize; ++i) {
                CachedSystemIterator::SystemInfo &oth = systemIter[systemIter->GetDestinationIndex(i)];
                if (oth.isDrawable()) {
                    QVector posoth = oth.Position();
                    ReplaceAxes(posoth);

                    float the_new_x, the_new_y, new_system_item_scale_temp, the_new_x_flat, the_new_y_flat;
                    //WARNING: SOME VARIABLES FOR ORIGINAL SYSTEM MAY BE MODIFIED HERE!!!
                    TranslateCoordinates(posoth,
                            pos_flat,
                            center_nav_x,
                            center_nav_y,
                            themaxvalue,
                            zscale,
                            zdistance,
                            the_new_x,
                            the_new_y,
                            the_new_x_flat,
                            the_new_y_flat,
                            new_system_item_scale_temp,
                            0);
                    GFXColor othcol = oth.GetColor();
                    othcol.a =
                            (new_system_item_scale_temp
                                    - minimumitemscaledown) / (maximumitemscaleup - minimumitemscaledown) + alphaadd;
                    //GetAlpha(oldposoth,center_x,center_y,center_z,zdistance);
                    IntersectBorder(the_new_x, the_new_y, the_x, the_y);

                    bool isConnectionPath = false;
                    if (path_view != PATH_OFF && systemIter->part_of_path && oth.part_of_path) {
                        for (std::set<NavPath *>::iterator paths = systemIter->paths.begin();
                                !isConnectionPath && paths != systemIter->paths.end();
                                ++paths) {
                            isConnectionPath = (*paths)->getVisible()
                                    && (*paths)->isNeighborPath(temp, systemIter->GetDestinationIndex(i));
                        }
                    }
                    if (isConnectionPath) {
                        *v++ = the_x;
                        *v++ = the_y;
                        *v++ = 0;
                        *v++ = pathcol.r;
                        *v++ = pathcol.g;
                        *v++ = pathcol.b;
                        *v++ = pathcol.a;
                        *v++ = the_new_x;
                        *v++ = the_new_y;
                        *v++ = 0;
                        *v++ = pathcol.r;
                        *v++ = pathcol.g;
                        *v++ = pathcol.b;
                        *v++ = pathcol.a;
                    } else if (path_view != PATH_ONLY) {
                        *v++ = the_x;
                        *v++ = the_y;
                        *v++ = 0;
                        *v++ = col.r;
                        *v++ = col.g;
                        *v++ = col.b;
                        *v++ = col.a;
                        *v++ = the_new_x;
                        *v++ = the_new_y;
                        *v++ = 0;
                        *v++ = othcol.r;
                        *v++ = othcol.g;
                        *v++ = othcol.b;
                        *v++ = othcol.a;
                    }
                }
            }
            if (v != verts.end()) {
                verts.erase(v, verts.end());
            }
            GFXDraw(GFXLINE, &verts[0], verts.size() / vsize, 3, 4);
        }
        ++systemIter;
    }
    //**********************************
    //Adjust mouse list for 'n' kliks
    //**********************************
    //STANDARD	: (1 3 2) ~ [0] [2] [1]
    //VS			: (1 2 3) ~ [0] [1] [2]	<-- use this
    if (mouselist.size() > 0) {
        //mouse is over a target when this is > 0
        if (mouse_wentdown[2] == 1) {             //mouse button went down for mouse button 2(standard)
            rotations += 1;
        }
    }
    if (rotations >= static_cast<int>(mouselist.size())) {        //dont rotate more than there is
        rotations = 0;
    }
    systemdrawlist tmpv;
    int siz = mouselist.size();
    for (l = 0; l < siz; ++l) {
        tmpv.push_back(mouselist[((unsigned int) (l + rotations)) % ((unsigned int) siz)]);
    }
    mouselist.swap(tmpv);
    //**********************************
    //Give back the selected tail IF there is one
    //**********************************
    if (mouselist.size() > 0) {
        //mouse is over a target when this is > 0
        if (mouse_wentdown[0] == 1) {
            //mouse button went down for mouse button 1
            unsigned oldselection = systemselectionindex;
            systemselectionindex = mouselist.back().getIndex();
            //JUST FOR NOW, target == current selection. later it'll be used for other shit, that will then set target.
            if (systemselectionindex == oldselection) {
                setFocusedSystemIndex(systemselectionindex);
            }
        }
    }
    //**********************************

    //Clear the lists
    //**********************************
    {
        for (systemdrawlist::iterator it = mouselist.begin(); it != mouselist.end(); ++it) {
            (*it).draw(true, &(*it) == &mouselist.back());
        }
    }

    mouselist.clear();          //whipe mouse over'd list
    //**********************************
}

