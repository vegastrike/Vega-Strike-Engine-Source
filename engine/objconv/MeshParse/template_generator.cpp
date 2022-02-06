/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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


#include <string>
#include <cstring>
#include <vector>
///Stores all the load-time vertex info in the XML struct FIXME light calculations
#include <expat.h>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
using std::vector;
using std::string;

#include "xml_support.h"
#include "hashtable.h"
using namespace std;
using namespace XMLSupport;
struct GFXVertex {
    float x, y, z;
    float i, j, k;
    float s, t;

    GFXVertex operator*(float s)
    {
        GFXVertex ret(*this);
        ret.x *= s;
        ret.y *= s;
        ret.z *= s;
        return ret;
    }
};
struct XML {
    int num_shield_facings;
    float total_shield_value;
    float energy_recharge;
    float energy_limit;
    float maxhull;
    FILE *tfp;
    FILE *bfp;
    enum Names {
        //elements
        UNKNOWN,
        MATERIAL,
        AMBIENT,
        DIFFUSE,
        SPECULAR,
        EMISSIVE,
        MESH,
        POINTS,
        POINT,
        LOCATION,
        NORMAL,
        POLYGONS,
        LINE,
        LOD,
        TRI,
        QUAD,
        LODFILE,
        LINESTRIP,
        TRISTRIP,
        TRIFAN,
        QUADSTRIP,
        VERTEX,
        LOGO,
        REF,
        //attributes
        POWER,
        REFLECT,
        LIGHTINGON,
        FLATSHADE,
        TEXTURE,
        FORCETEXTURE,
        ALPHAMAP,
        SHAREVERT,
        ALPHA,
        RED,
        GREEN,
        BLUE,
        X,
        Y,
        Z,
        I,
        J,
        K,
        S,
        T,
        SCALE,
        BLENDMODE,
        TYPE,
        ROTATE,
        WEIGHT,
        SIZE,
        OFFSET,
        ANIMATEDTEXTURE,
        REVERSE
    };
    ///Saves which attributes of vertex have been set in XML file
    enum PointState {
        P_X = 0x1,
        P_Y = 0x2,
        P_Z = 0x4,
        P_I = 0x8,
        P_J = 0x10,
        P_K = 0x20
    };
    ///Saves which attributes of vertex have been set in Polygon for XML file
    enum VertexState {
        V_POINT = 0x1,
        V_S = 0x2,
        V_T = 0x4
    };
    ///Save if various logo values have been set
    enum LogoState {
        V_TYPE = 0x1,
        V_ROTATE = 0x2,
        V_SIZE = 0x4,
        V_OFFSET = 0x8,
        V_REF = 0x10
    };
    ///To save the constructing of a logo
    struct ZeLogo {
        ///Which type the logo is (0 = faction 1 = squad >2 = internal use
        unsigned int type;
        ///how many degrees logo is rotated
        float rotate;
        ///Size of the logo
        float size;
        ///offset of polygon of logo
        float offset;
        ///the reference points that the logo is weighted against
        vector<int> refpnt;
        ///the weight of the points in weighted average of refpnts
        vector<float> refweight;
    };
    static const EnumMap::Pair element_names[];
    static const EnumMap::Pair attribute_names[];
    static const EnumMap element_map;
    static const EnumMap attribute_map;
    vector<std::string> meshnames;
    float unitscale;
    vector<Names> state_stack;
    vector<GFXVertex> vertices;
    GFXVertex vertex;
    float scale;
};

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;
using XMLSupport::parse_float;
using XMLSupport::parse_bool;
using XMLSupport::parse_int;

const EnumMap::Pair XML::element_names[] = {
        EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
        EnumMap::Pair("Material", XML::MATERIAL),
        EnumMap::Pair("LOD", XML::LOD),
        EnumMap::Pair("Ambient", XML::AMBIENT),
        EnumMap::Pair("Diffuse", XML::DIFFUSE),
        EnumMap::Pair("Specular", XML::SPECULAR),
        EnumMap::Pair("Emissive", XML::EMISSIVE),
        EnumMap::Pair("Mesh", XML::MESH),
        EnumMap::Pair("Points", XML::POINTS),
        EnumMap::Pair("Point", XML::POINT),
        EnumMap::Pair("Location", XML::LOCATION),
        EnumMap::Pair("Normal", XML::NORMAL),
        EnumMap::Pair("Polygons", XML::POLYGONS),
        EnumMap::Pair("Line", XML::LINE),
        EnumMap::Pair("Tri", XML::TRI),
        EnumMap::Pair("Quad", XML::QUAD),
        EnumMap::Pair("Linestrip", XML::LINESTRIP),
        EnumMap::Pair("Tristrip", XML::TRISTRIP),
        EnumMap::Pair("Trifan", XML::TRIFAN),
        EnumMap::Pair("Quadstrip", XML::QUADSTRIP),
        EnumMap::Pair("Vertex", XML::VERTEX),
        EnumMap::Pair("Logo", XML::LOGO),
        EnumMap::Pair("Ref", XML::REF)
};

const EnumMap::Pair XML::attribute_names[] = {
        EnumMap::Pair("UNKNOWN", XML::UNKNOWN),
        EnumMap::Pair("Scale", XML::SCALE),
        EnumMap::Pair("Blend", XML::BLENDMODE),
        EnumMap::Pair("texture", XML::TEXTURE),
        EnumMap::Pair("alphamap", XML::ALPHAMAP),
        EnumMap::Pair("sharevertex", XML::SHAREVERT),
        EnumMap::Pair("red", XML::RED),
        EnumMap::Pair("green", XML::GREEN),
        EnumMap::Pair("blue", XML::BLUE),
        EnumMap::Pair("alpha", XML::ALPHA),
        EnumMap::Pair("power", XML::POWER),
        EnumMap::Pair("reflect", XML::REFLECT),
        EnumMap::Pair("x", XML::X),
        EnumMap::Pair("y", XML::Y),
        EnumMap::Pair("z", XML::Z),
        EnumMap::Pair("i", XML::I),
        EnumMap::Pair("j", XML::J),
        EnumMap::Pair("k", XML::K),
        EnumMap::Pair("Shade", XML::FLATSHADE),
        EnumMap::Pair("point", XML::POINT),
        EnumMap::Pair("s", XML::S),
        EnumMap::Pair("t", XML::T),
        //Logo stuffs
        EnumMap::Pair("Type", XML::TYPE),
        EnumMap::Pair("Rotate", XML::ROTATE),
        EnumMap::Pair("Weight", XML::WEIGHT),
        EnumMap::Pair("Size", XML::SIZE),
        EnumMap::Pair("Offset", XML::OFFSET),
        EnumMap::Pair("meshfile", XML::LODFILE),
        EnumMap::Pair("Animation", XML::ANIMATEDTEXTURE),
        EnumMap::Pair("Reverse", XML::REVERSE),
        EnumMap::Pair("LightingOn", XML::LIGHTINGON),
        EnumMap::Pair("ForceTexture", XML::FORCETEXTURE)
};

const EnumMap XML::element_map(XML::element_names, 23);
const EnumMap XML::attribute_map(XML::attribute_names, 32);

namespace UnitNS {
enum Names {
    UNKNOWN,
    UNIT,
    SUBUNIT,
    MESHFILE,
    SHIELDMESH,
    BSPMESH,
    RAPIDMESH,
    MOUNT,
    MESHLIGHT,
    DOCK,
    XFILE,
    X,
    Y,
    Z,
    RI,
    RJ,
    RK,
    QI,
    QJ,
    QK,
    RED,
    GREEN,
    BLUE,
    ALPHA,
    MOUNTSIZE,
    WEAPON,
    DEFENSE,
    ARMOR,
    FORWARD,
    RETRO,
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    SHIELDS,
    RECHARGE,
    LEAK,
    HULL,
    STRENGTH,
    STATS,
    MASS,
    MOMENTOFINERTIA,
    FUEL,
    THRUST,
    MANEUVER,
    YAW,
    ROLL,
    PITCH,
    ENGINE,
    COMPUTER,
    AACCEL,
    ENERGY,
    REACTOR,
    LIMIT,
    RESTRICTED,
    MAX,
    MIN,
    MAXSPEED,
    AFTERBURNER,
    SHIELDTIGHT,
    ITTS,
    AMMO,
    HUDIMAGE,
    SOUND,
    MINTARGETSIZE,
    MAXCONE,
    LOCKCONE,
    RANGE,
    ISCOLOR,
    RADAR,
    CLOAK,
    CLOAKRATE,
    CLOAKMIN,
    CLOAKENERGY,
    CLOAKGLASS,
    CLOAKWAV,
    CLOAKMP3,
    ENGINEWAV,
    ENGINEMP3,
    HULLWAV,
    HULLMP3,
    ARMORWAV,
    ARMORMP3,
    SHIELDWAV,
    SHIELDMP3,
    EXPLODEWAV,
    EXPLODEMP3,
    EXPLOSIONANI,
    COCKPIT,
    JUMP,
    DELAY,
    JUMPENERGY,
    JUMPWAV,
    DOCKINTERNAL,
    WORMHOLE,
    RAPID,
    USEBSP,
    AFTERBURNENERGY,
    MISSING,
    UNITSCALE,
    PRICE,
    VOLUME,
    QUANTITY,
    CARGO,
    HOLD,
    CATEGORY,
    IMPORT,
    PRICESTDDEV,
    QUANTITYSTDDEV,
    DAMAGE,
    COCKPITDAMAGE,
    REPAIRDROID,
    ECM,
    DESCRIPTION,
    UPGRADE,
    MOUNTOFFSET,
    SUBUNITOFFSET,
    SLIDE_START,
    SLIDE_END,
    TRACKINGCONE,
    MISSIONCARGO,
    MAXIMUM,
    LIGHTTYPE
};

const EnumMap::Pair element_names[36] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("Unit", UNIT),
        EnumMap::Pair("SubUnit", SUBUNIT),
        EnumMap::Pair("Sound", SOUND),
        EnumMap::Pair("MeshFile", MESHFILE),
        EnumMap::Pair("ShieldMesh", SHIELDMESH),
        EnumMap::Pair("RapidMesh", RAPIDMESH),
        EnumMap::Pair("BSPMesh", BSPMESH),
        EnumMap::Pair("Light", MESHLIGHT),
        EnumMap::Pair("Defense", DEFENSE),
        EnumMap::Pair("Armor", ARMOR),
        EnumMap::Pair("Shields", SHIELDS),
        EnumMap::Pair("Hull", HULL),
        EnumMap::Pair("Stats", STATS),
        EnumMap::Pair("Thrust", THRUST),
        EnumMap::Pair("Maneuver", MANEUVER),
        EnumMap::Pair("Engine", ENGINE),
        EnumMap::Pair("Computer", COMPUTER),
        EnumMap::Pair("Cloak", CLOAK),
        EnumMap::Pair("Energy", ENERGY),
        EnumMap::Pair("Reactor", REACTOR),
        EnumMap::Pair("Restricted", RESTRICTED),
        EnumMap::Pair("Yaw", YAW),
        EnumMap::Pair("Pitch", PITCH),
        EnumMap::Pair("Roll", ROLL),
        EnumMap::Pair("Mount", MOUNT),
        EnumMap::Pair("Radar", RADAR),
        EnumMap::Pair("Cockpit", COCKPIT),
        EnumMap::Pair("Jump", JUMP),
        EnumMap::Pair("Dock", DOCK),
        EnumMap::Pair("Hold", HOLD),
        EnumMap::Pair("Cargo", CARGO),
        EnumMap::Pair("Category", CATEGORY),
        EnumMap::Pair("Import", IMPORT),
        EnumMap::Pair("CockpitDamage", COCKPITDAMAGE),
        EnumMap::Pair("Upgrade", UPGRADE)
};
const EnumMap::Pair attribute_names[94] = {
        EnumMap::Pair("UNKNOWN", UNKNOWN),
        EnumMap::Pair("missing", MISSING),
        EnumMap::Pair("file", XFILE),
        EnumMap::Pair("x", X),
        EnumMap::Pair("y", Y),
        EnumMap::Pair("z", Z),
        EnumMap::Pair("ri", RI),
        EnumMap::Pair("rj", RJ),
        EnumMap::Pair("rk", RK),
        EnumMap::Pair("qi", QI),
        EnumMap::Pair("qj", QJ),
        EnumMap::Pair("qk", QK),
        EnumMap::Pair("red", RED),
        EnumMap::Pair("green", GREEN),
        EnumMap::Pair("blue", BLUE),
        EnumMap::Pair("alpha", ALPHA),
        EnumMap::Pair("size", MOUNTSIZE),
        EnumMap::Pair("forward", FORWARD),
        EnumMap::Pair("retro", RETRO),
        EnumMap::Pair("front", FRONT),
        EnumMap::Pair("back", BACK),
        EnumMap::Pair("left", LEFT),
        EnumMap::Pair("right", RIGHT),
        EnumMap::Pair("top", TOP),
        EnumMap::Pair("bottom", BOTTOM),
        EnumMap::Pair("recharge", RECHARGE),
        EnumMap::Pair("leak", LEAK),
        EnumMap::Pair("strength", STRENGTH),
        EnumMap::Pair("mass", MASS),
        EnumMap::Pair("momentofinertia", MOMENTOFINERTIA),
        EnumMap::Pair("fuel", FUEL),
        EnumMap::Pair("yaw", YAW),
        EnumMap::Pair("pitch", PITCH),
        EnumMap::Pair("roll", ROLL),
        EnumMap::Pair("accel", AACCEL),
        EnumMap::Pair("limit", LIMIT),
        EnumMap::Pair("max", MAX),
        EnumMap::Pair("min", MIN),
        EnumMap::Pair("weapon", WEAPON),
        EnumMap::Pair("maxspeed", MAXSPEED),
        EnumMap::Pair("afterburner", AFTERBURNER),
        EnumMap::Pair("tightness", SHIELDTIGHT),
        EnumMap::Pair("itts", ITTS),
        EnumMap::Pair("ammo", AMMO),
        EnumMap::Pair("HudImage", HUDIMAGE),
        EnumMap::Pair("ExplosionAni", EXPLOSIONANI),
        EnumMap::Pair("MaxCone", MAXCONE),
        EnumMap::Pair("TrackingCone", TRACKINGCONE),
        EnumMap::Pair("LockCone", LOCKCONE),
        EnumMap::Pair("MinTargetSize", MINTARGETSIZE),
        EnumMap::Pair("Range", RANGE),
        EnumMap::Pair("EngineMp3", ENGINEMP3),
        EnumMap::Pair("EngineWav", ENGINEWAV),
        EnumMap::Pair("HullMp3", HULLMP3),
        EnumMap::Pair("HullWav", HULLWAV),
        EnumMap::Pair("ArmorMp3", ARMORMP3),
        EnumMap::Pair("ArmorWav", ARMORWAV),
        EnumMap::Pair("ShieldMp3", SHIELDMP3),
        EnumMap::Pair("ShieldWav", SHIELDWAV),
        EnumMap::Pair("ExplodeMp3", EXPLODEMP3),
        EnumMap::Pair("ExplodeWav", EXPLODEWAV),
        EnumMap::Pair("CloakRate", CLOAKRATE),
        EnumMap::Pair("CloakGlass", CLOAKGLASS),
        EnumMap::Pair("CloakEnergy", CLOAKENERGY),
        EnumMap::Pair("CloakMin", CLOAKMIN),
        EnumMap::Pair("CloakMp3", CLOAKMP3),
        EnumMap::Pair("CloakWav", CLOAKWAV),
        EnumMap::Pair("Color", ISCOLOR),
        EnumMap::Pair("Restricted", RESTRICTED),
        EnumMap::Pair("Delay", DELAY),
        EnumMap::Pair("AfterburnEnergy", AFTERBURNENERGY),
        EnumMap::Pair("JumpEnergy", JUMPENERGY),
        EnumMap::Pair("JumpWav", JUMPWAV),
        EnumMap::Pair("DockInternal", DOCKINTERNAL),
        EnumMap::Pair("RAPID", RAPID),
        EnumMap::Pair("BSP", USEBSP),
        EnumMap::Pair("Wormhole", WORMHOLE),
        EnumMap::Pair("Scale", UNITSCALE),
        EnumMap::Pair("Price", PRICE),
        EnumMap::Pair("Volume", VOLUME),
        EnumMap::Pair("Quantity", QUANTITY),
        EnumMap::Pair("PriceStdDev", PRICESTDDEV),
        EnumMap::Pair("QuantityStdDev", QUANTITYSTDDEV),
        EnumMap::Pair("Damage", DAMAGE),
        EnumMap::Pair("RepairDroid", REPAIRDROID),
        EnumMap::Pair("ECM", ECM),
        EnumMap::Pair("Description", DESCRIPTION),
        EnumMap::Pair("MountOffset", MOUNTOFFSET),
        EnumMap::Pair("SubunitOffset", SUBUNITOFFSET),
        EnumMap::Pair("SlideEnd", SLIDE_START),
        EnumMap::Pair("SlideStart", SLIDE_END),
        EnumMap::Pair("MissionCargo", MISSIONCARGO),
        EnumMap::Pair("Maximum", MAXIMUM),
        EnumMap::Pair("LightType", LIGHTTYPE)
};

const EnumMap element_map(element_names, 36);
const EnumMap attribute_map(attribute_names, 94);
}
typedef pair<float, std::string> fs;
typedef vector<fs> lookuptable;

lookuptable shieldLookup()
{
    lookuptable r;
    r.push_back(fs(5, ""));
    r.push_back(fs(100, "_Level1"));
    r.push_back(fs(200, "_Level2"));
    r.push_back(fs(300, "_Level3"));
    r.push_back(fs(400, "_Level4"));
    r.push_back(fs(550, "_Level5"));
    r.push_back(fs(650, "_Level6"));
    r.push_back(fs(800, "_Level7"));
    r.push_back(fs(900, "_Level8"));
    r.push_back(fs(1000, "_Level9"));
    r.push_back(fs(FLT_MAX, "_Level10"));
    return r;
}

lookuptable hullLookup()
{
    lookuptable r;
    r.push_back(fs(120, "hull"));
    r.push_back(fs(200, "polymer_hull"));
    r.push_back(fs(360, "reinforced_hull"));
    return r;
}

lookuptable engineLookup()
{
    lookuptable r;
    r.push_back(fs(41, "_level_1"));
    r.push_back(fs(71, "_level_2"));
    r.push_back(fs(111, "_level_3"));
    r.push_back(fs(161, "_level_4"));
    r.push_back(fs(221, "_level_5"));
    r.push_back(fs(291, "_level_6"));
    r.push_back(fs(371, "_level_7"));
    r.push_back(fs(461, "_level_8"));
    r.push_back(fs(561, "_level_9"));
    r.push_back(fs(FLT_MAX, "_level_10"));
    return r;
}

lookuptable engineLimitLookup()
{
    lookuptable r;
    r.push_back(fs(41, "100"));
    r.push_back(fs(71, "300"));
    r.push_back(fs(111, "475"));
    r.push_back(fs(161, "675"));
    r.push_back(fs(221, "900"));
    r.push_back(fs(291, "1150"));
    r.push_back(fs(371, "1425"));
    r.push_back(fs(461, "1725"));
    r.push_back(fs(561, "2050"));
    r.push_back(fs(FLT_MAX, "2400"));
    return r;
}

std::string LookUp(const lookuptable &a, float x)
{
    for (unsigned int i = 0; i < a.size(); i++) {
        if (x < (a[i].first) || i == a.size() - 1) {
            return a[i].second;
        }
    }
    return "";
}

int globaltab = 0; //go go kludge master!
int globalfileout = 1; //go go gadget kludge!
int globallasttab = 0; //go go uber kludge!
float globalmassrescale = 1; //go go... ok, at this point, it's obvious I'm not trying
#ifdef _WIN32
#define strcasecmp stricmp
#endif

bool xeq(std::string s, std::string t)
{
    return strtoupper(s) == strtoupper(t);
}

float xpf(std::string s)
{
    return XMLSupport::parse_float(s);
}

int xpi(std::string s)
{
    return XMLSupport::parse_int(s);
}

#define xts(s) ( XMLSupport::tostring( s ) )

std::string RemoveAutotracking(std::string s)
{
    s = strtoupper(s);
    int pos;
    int len = strlen("AUTOTRACKING");
    while ((pos = s.find("AUTOTRACKING")) != string::npos) {
        s = s.substr(0, pos) + s.substr(pos + len);
    }
    return s;
}

void Tokenize(const string &str, vector<string> &tokens, const string &delimiters = " ")
{
    //Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    //Find first "non-delimiter".
    string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        //Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        //Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        //Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

std::string CheckBasicSizes(const std::string tokens)
{
    if (tokens.find("small") != string::npos) {
        return "small";
    }
    if (tokens.find("medium") != string::npos) {
        return "medium";
    }
    if (tokens.find("large") != string::npos) {
        return "large";
    }
    if (tokens.find("cargo") != string::npos) {
        return "cargo";
    }
    if (tokens.find("LR") != string::npos || tokens.find("massive") != string::npos) {
        return "massive";
    }
    return "";
}

class VCString : public std::string {
public:
    VCString()
    {
    }

    VCString(const string &s) : string(s)
    {
    }
};

std::map<VCString, VCString> parseTurretSizes()
{
    std::map<VCString, VCString> t;
    FILE *fp = fopen("turretsize.txt", "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        int siz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *filedata = (char *) malloc(siz + 1);
        filedata[siz] = 0;
        while (fgets(filedata, siz, fp)) {
            std::string x(filedata);
            int len = x.find(",");
            if (len != std::string::npos) {
                std::string y = x.substr(len + 1);
                x = x.substr(0, len);
                len = y.find(",");
                y = y.substr(0, len);
                sscanf(y.c_str(), "%s", filedata);
                y = filedata;
                VCString key(x);
                VCString value(y);
                t[key] = value;
            }
        }
        free(filedata);
        fclose(fp);
    }
    return t;
}

std::string getTurretSize(const std::string &size)
{
    static std::map<VCString, VCString> turretmap = parseTurretSizes();
    std::map<VCString, VCString>::iterator h = turretmap.find(size);
    if (h != turretmap.end()) {
        return (*h).second;
    }
    vector<string> tokens;
    Tokenize(size, tokens, "_");
    for (unsigned int i = 0; i < tokens.size(); i++) {
        if (tokens[i].find("turret") != string::npos) {
            string temp = CheckBasicSizes(tokens[i]);
            if (!temp.empty()) {
                return temp;
            }
        } else {
            return tokens[i];
        }
    }
    return "capitol";
}

void UnitBeginElement(const string &name, const AttributeList &attributes, XML *xml)
{
    AttributeList::const_iterator iter;
    UnitNS::Names elem = (UnitNS::Names) UnitNS::element_map.lookup(name);
    UnitNS::Names top;
    if (globalfileout) {
        if (globaltab > globallasttab) {
            //fprintf (xml->tfp,">\n");
            //fprintf (xml->bfp,">\n");
            ++globallasttab;
        }
        for (int sc = 0; sc < globaltab; sc++) {
            fprintf(xml->tfp, "\t");
            fprintf(xml->bfp, "\t");
        }
        ++globaltab;
        fprintf(xml->tfp, "<%s", name.c_str());
        fprintf(xml->bfp, "<%s", name.c_str());
        bool shields = false;
        if (xeq(name, "shields")) {
            xml->num_shield_facings = 0;
            xml->total_shield_value = 0;
            shields = true;
        } else if (xeq(name, "jump")) {
            fprintf(xml->bfp, " missing = \"1\"");
        }
        if (xeq(name, "energy")) {
            fprintf(xml->bfp, " afterburnenergy = \"32767\"");
        } else if (xeq(name, "reactor")) {
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                if (xeq((*iter).name, "recharge")) {
                    xml->energy_recharge = xpf(iter->value);
                } else if (xeq(iter->name, "limit")) {
                    xml->energy_limit = xpf(iter->value);
                } else if (xeq(iter->name, "warpenergy")) {
                    fprintf(xml->tfp, " %s =\"%s\"", iter->name.c_str(), iter->value.c_str());
                    fprintf(xml->bfp, " %s =\"%s\"", iter->name.c_str(), iter->value.c_str());
                }
            }
        } else if (xeq(name, "radar")) {
            fprintf(xml->tfp, " itts=\"true\" error=\"0\" range=\"600000000\" maxcone=\"-1\" color=\"true\" ");
            fprintf(xml->bfp, " itts=\"false\" error=\"0\" range=\"30000000\" maxcone=\"-1\" color=\"false\"");
        } else {
            bool ecm = false;
            bool repair = false;
            float ecmval = 1;
            int repairval = 6;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                string tnam = (*iter).name;
                string bnam = (*iter).name;
                string tval = (*iter).value;
                string bval = (*iter).value;
                if (xeq(tnam, "ECM")) {
                    tval = xts(ecmval);
                    bval = "0";
                    ecm = true;
                }
                if (xeq(tnam, "RepairDroid")) {
                    repair = true;
                    tval = xts(repairval);
                    bval = "0";
                }
                if (shields) {
                    if (strcasecmp("front", tnam.c_str()) == 0
                            || strcasecmp("back", tnam.c_str()) == 0
                            || strcasecmp("left", tnam.c_str()) == 0
                            || strcasecmp("right", tnam.c_str()) == 0
                            || strcasecmp("top", tnam.c_str()) == 0
                            || strcasecmp("bottom", tnam.c_str()) == 0) {
                        xml->num_shield_facings++;
                        xml->total_shield_value += xpf(tval);
                    }
                    if (XMLSupport::parse_float(tval) != 0) {
                        bval = tval = "0";
                    }
                }
                /* (float) done to get rid of ambiguity in function selection */
                if (xeq(name, "hold")) {
                    if (xeq(iter->name, "volume")) {
                        tval = xts((float) (XMLSupport::parse_float(iter->value) * 1.2));
                    }
                }
                if (xeq(name, "subunit")) {
                    if (xeq(iter->name, "file")) {
                        bval = getTurretSize(iter->value) + "_blank";
                    }
                }
                if (xeq(name, "mount")) {
                    if (xeq(iter->name, "weapon")) {
                        bval = "";
                    }
                    if (xeq(iter->name, "size")) {
                        bval = RemoveAutotracking(bval);
                    }
                }
                fprintf(xml->tfp, " %s =\"%s\"", tnam.c_str(), tval.c_str());
                fprintf(xml->bfp, " %s =\"%s\"", bnam.c_str(), bval.c_str());
            }
            if (xeq(name, "defense")) {
                if (!ecm) {
                    fprintf(xml->tfp, " ECM =\"%f\"", ecmval);
                    fprintf(xml->bfp, " ECM =\"0\"");
                }
                if (!repair) {
                    fprintf(xml->tfp, " RepairDroid =\"%d\"", repairval);
                    fprintf(xml->bfp, " RepairDroid =\"0\"");
                }
            }
        }
    }
    fprintf(xml->tfp, ">\n");
    fprintf(xml->bfp, ">\n");
    switch (elem) {
        case UnitNS::UNIT:
            //IN FORMER TIMES fprintf (xml->tfp,"\t<Upgrade file=\"godsansshields\"/>\n");
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (UnitNS::attribute_map.lookup((*iter).name)) {
                    case UnitNS::UNITSCALE:
                        xml->unitscale = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case UnitNS::MESHFILE:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (UnitNS::attribute_map.lookup((*iter).name)) {
                    case UnitNS::XFILE:
                        xml->meshnames.push_back((*iter).value);
                }
            }
            break;
    }
}

void UnitEndElement(const string &name, XML *xml)
{
    if (xeq(name, "unit")) {
        fprintf(xml->bfp, "<Upgrade file=\"mult_general_downgrade\"/>\n");
        fprintf(xml->tfp, "<Upgrade file=\"mult_general_upgrade\"/>\n");
        fprintf(xml->bfp, "<Upgrade file=\"add_general_downgrade\"/>\n");
        fprintf(xml->tfp, "<Upgrade file=\"add_general_upgrade\"/>\n");
    }
    --globaltab;
    if (globalfileout) {
        if (globaltab == globallasttab) {
        } else {
            globallasttab--;
        }
        for (int sc = 0; sc < globaltab; sc++) {
            fprintf(xml->tfp, "\t");
            fprintf(xml->bfp, "\t");
        }
        fprintf(xml->bfp, "</%s>\n", name.c_str());
        fprintf(xml->tfp, "</%s>\n", name.c_str());
    }
    if (xeq(name, "Defense")) {
        //do defense related tasks;

        string s = "";
        if (xml->num_shield_facings) {
            s = LookUp(shieldLookup(), xml->total_shield_value / xml->num_shield_facings);
        }
        fprintf(xml->tfp, "<Upgrade file=\"shield_%d%s\"/>\n<Upgrade file=\"mult_shield_regenerator\"/>\n",
                (xml->num_shield_facings), s.c_str());
        const float num_shield_capacitance_additions = 5;
        for (unsigned int i = 0; i < num_shield_capacitance_additions; ++i) {
            fprintf(xml->tfp, "<Upgrade file=\"add_shield_%d_capacitance\"/>\n", xml->num_shield_facings);
        }
        //s = LookUp (hullLookup(),xml->maxhull);
        //fprintf (xml->tfp,"<Upgrade file=\"%s\"/>\n",s.c_str());
        //fprintf (xml->bfp,"<Upgrade file=\"hull\"/>\n");
        fprintf(xml->bfp, "<Upgrade file=\"shield_%d\"/>\n", xml->num_shield_facings);
    }
    if (xeq(name, "energy")) {
        fprintf(xml->tfp, "<Upgrade file=\"reactor%s\"/>\n<Upgrade file=\"mult_gun_cooler\"/>\n",
                LookUp(engineLookup(), xml->energy_recharge).c_str());
        float limit = xpf(LookUp(engineLimitLookup(), xml->energy_recharge));
        limit = xml->energy_limit - limit;
        if (limit > 0) {
            int numcap = int(limit / 100);
            for (int i = 0; i < numcap; ++i) {
                fprintf(xml->tfp, "	<upgrade file=\"add_reactor_capacitance\"/>\n");
            }
        }
        fprintf(xml->bfp, "<Upgrade file=\"reactor_level_0\"/>\n");
    }
}

void beginElement(const string &name, const AttributeList &attributes, XML *xml)
{
    AttributeList::const_iterator iter;
    XML::Names elem = (XML::Names) XML::element_map.lookup(name);
    XML::Names top;
    if (xml->state_stack.size() > 0) {
        top = *xml->state_stack.rbegin();
    }
    xml->state_stack.push_back(elem);
    switch (elem) {
        case XML::MATERIAL:
        case XML::DIFFUSE:
            break;
        case XML::EMISSIVE:
            break;
        case XML::SPECULAR:
            break;
        case XML::AMBIENT:
            break;
        case XML::UNKNOWN:
            fprintf(stderr, "Unknown element start tag '%s' detected\n", name.c_str());
            break;
        case XML::MESH:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::ANIMATEDTEXTURE:
                        break;
                    case XML::REVERSE:
                        break;
                    case XML::FORCETEXTURE:
                        break;
                    case XML::TEXTURE:
                        break;
                    case XML::ALPHAMAP:
                        break;
                    case XML::SCALE:
                        xml->scale = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::SHAREVERT:
                        break;
                    case XML::BLENDMODE:
                        break;
                }
            }
            break;
        case XML::POINTS:
            break;
        case XML::POINT:
//assert(top==XML::POINTS);
            break;
        case XML::LOCATION:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::X:
                        xml->vertex.x = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::Y:
                        xml->vertex.y = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::Z:
                        xml->vertex.z = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::S:
                        xml->vertex.s = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::T:
                        xml->vertex.t = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case XML::NORMAL:
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch (XML::attribute_map.lookup((*iter).name)) {
                    case XML::UNKNOWN:
                        fprintf(stderr, "Unknown attribute '%s' encountered in Normal tag\n", (*iter).name.c_str());
                        break;
                    case XML::I:
                        xml->vertex.i = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::J:
                        xml->vertex.j = XMLSupport::parse_float((*iter).value);
                        break;
                    case XML::K:
                        xml->vertex.k = XMLSupport::parse_float((*iter).value);
                        break;
                }
            }
            break;
        case XML::POLYGONS:
            break;
        case XML::LINE:
            break;
        case XML::TRI:
            break;
        case XML::LINESTRIP:
            break;

        case XML::TRISTRIP:
            break;

        case XML::TRIFAN:
            break;

        case XML::QUADSTRIP:
            break;

        case XML::QUAD:
            break;
        case XML::LOD:
            break;
        case XML::VERTEX:
            break;
        case XML::LOGO:
            break;
        case XML::REF:
            break;
    }
}

void endElement(const string &name, XML *xml)
{
    xml->state_stack.pop_back();
    XML::Names elem = (XML::Names) XML::element_map.lookup(name);

    unsigned int i;
    switch (elem) {
        case XML::UNKNOWN:
            fprintf(stderr, "Unknown element end tag '%s' detected\n", name.c_str());
            break;
        case XML::POINT:
            xml->vertices.push_back(xml->vertex * (xml->scale * xml->unitscale));
            break;
        case XML::POINTS:
            break;
        case XML::LINE:
            break;
        case XML::TRI:
            break;
        case XML::QUAD:
            break;
        case XML::LINESTRIP:
            break;
        case XML::TRISTRIP:
            break;
        case XML::TRIFAN:
            break;
        case XML::QUADSTRIP:     //have to fix up nrmlquadstrip so that it 'looks' like a quad list for smooth shading
            break;
        case XML::POLYGONS:
            break;
        case XML::REF:
            break;
        case XML::LOGO:
            break;
        case XML::MATERIAL:
            break;
        case XML::DIFFUSE:
            break;
        case XML::EMISSIVE:
            break;
        case XML::SPECULAR:
            break;
        case XML::AMBIENT:
            break;
        case XML::MESH:
            break;
        default:;
    }
}

void beginElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    beginElement(name, AttributeList(atts), (XML *) userData);
}

void endElement(void *userData, const XML_Char *name)
{
    endElement(name, (XML *) userData);
}

void unitBeginElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
    UnitBeginElement(name, AttributeList(atts), (XML *) userData);
}

void unitEndElement(void *userData, const XML_Char *name)
{
    UnitEndElement(name, (XML *) userData);
}

void LoadXML(const char *filename, XML &xml)
{
    const int chunk_size = 16384;
    FILE *inFile = fopen(filename, "r");
    if (!inFile) {
        fprintf(stderr, "Cannot Open Mesh File %s\n", filename);
        exit(0);
    }
    xml.scale = 1;
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &xml);
    XML_SetElementHandler(parser, &beginElement, &endElement);
    do {
        char buf[chunk_size];
        int length;

        length = fread(buf, 1, chunk_size, inFile);
        XML_Parse(parser, buf, length, feof(inFile));
    } while (!feof(inFile));
    fclose(inFile);
    XML_ParserFree(parser);
    //Now, copy everything into the mesh data structures
}

void UnitLoadXML(const char *filename, XML &xml)
{
    const int chunk_size = 16384;
    FILE *inFile = fopen(filename, "r");
    if (!inFile) {
        fprintf(stderr, "Cannot Open Mesh File %s\n", filename);
        exit(0);
    }
    xml.unitscale = 1;
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &xml);
    XML_SetElementHandler(parser, &unitBeginElement, &unitEndElement);
    do {
        char buf[chunk_size];
        int length;

        length = fread(buf, 1, chunk_size, inFile);
        XML_Parse(parser, buf, length, feof(inFile));
    } while (!feof(inFile));
    fclose(inFile);
    XML_ParserFree(parser);
    //Now, copy everything into the mesh data structures
}

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        XML blah;
        blah.maxhull = 100;

        blah.unitscale = 1;
        blah.num_shield_facings = 0;
        blah.total_shield_value = 0;
        string templatefilename = argv[i] + string(".template");
        blah.tfp = fopen(templatefilename.c_str(), "w");
        string blankfilename = argv[i] + string(".blank");
        blah.bfp = fopen(blankfilename.c_str(), "w");
        if (!blah.bfp || !blah.tfp) {
            if (blah.tfp) {
                fclose(blah.tfp);
            }
            if (blah.bfp) {
                fclose(blah.bfp);
            }
            return 0;
        }
        UnitLoadXML(argv[i], blah);
    }
    return 0;
}

