/*
 * galaxy_gen.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Evert Vorster
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


#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include "root_generic/macosx_math.h"
#include <math.h>
#include <time.h>
#include <assert.h>

#include "root_generic/vs_globals.h"
#include "root_generic/xml_support.h"
#include "src/gfxlib.h"
#include "root_generic/galaxy_xml.h"
#include "root_generic/galaxy_gen.h"
#include "src/vs_random.h"
#include "root_generic/options.h"
#include "src/universe.h"
#include "src/vs_logging.h"

#ifndef _WIN32
#include <ctype.h>
#endif

#if _MSC_VER >= 1300
#define snprintf _snprintf
#endif

#ifndef M_PI
#define M_PI 3.1415926536
#endif
#include "vegadisk/vsfilesystem.h"

using namespace VSFileSystem;
using std::string;
using std::vector;

static VSRandom starsysrandom(time(nullptr));

static void seedrand(unsigned long seed) {
    starsysrandom = VSRandom(seed);
}

static int stringhash(const string &key) {
    unsigned int k = 0;
    string::const_iterator start = key.begin();
    for (; start != key.end(); start++) {
        k += (k * 128) + *start;
    }
    return k;
}

static unsigned int ssrand() {
    return starsysrandom.rand();
}

static string GetWrapXY(string cname, int &wrapx, int &wrapy) {
    string wrap = cname;
    wrapx = wrapy = 1;
    string::size_type pos = wrap.find("wrapx");
    if (pos != string::npos) {
        string Wrapx = wrap.substr(pos + 5, wrap.length());
        cname = cname.substr(0, pos);
        sscanf(Wrapx.c_str(), "%d", &wrapx);
        pos = Wrapx.find("wrapy");
        if (pos != string::npos) {
            string Wrapy = Wrapx.substr(pos + 5, Wrapx.length());
            sscanf(Wrapy.c_str(), "%d", &wrapy);
        }
    }
    return cname;
}

string getStarSystemName(const string &in);

namespace StarSystemGent {
float mmax(float a, float b) {
    return (a > b) ? a : b;
}

int rnd(int lower, int upper) {
    if (upper > lower) {
        return lower + ssrand() % (upper - lower);
    } else {
        return lower;
    }
}

const char nada[1] = "";

string getGenericName(vector<string> &s) {
    if (s.empty()) {
        return string(nada);
    }
    return s[rnd(0, s.size())];
}

string getRandName(vector<string> &s) {
    if (s.empty()) {
        return string(nada);
    }
    unsigned int i = rnd(0, s.size());
    string k = s[i];
    s.erase(s.begin() + i);
    return k;
}

struct Color {
    float r, g, b, a{};
    float nr{}, ng{}, nb{}, na{};

    Color(float rr, float gg, float bb) {
        r = rr;
        b = bb;
        g = gg;
    }
};
class Vector {
public:
    float i{};
    float j{};
    float k{};
    float s{};
    float t{};

    Vector() {
        i = j = k = 0;
    }

    template<class vec>
    Vector(const vec &in) {
        memcpy(this, &in, sizeof(*this));
    }

    Vector(const Vector &in) : i(in.i), j(in.j), k(in.k), s(in.s), t(in.t) {}

    Vector(float x, float y, float z) {
        i = x;
        j = y;
        k = z;
    }

    Vector(float x, float y, float z, float s, float t) {
        i = x;
        j = y;
        k = z;
        this->s = s;
        this->t = t;
    }

    float Mag() const {
        return sqrtf(i * i + j * j + k * k);
    }

    Vector Cross(const Vector &v) const {
        return Vector(this->j * v.k - this->k * v.j,
                this->k * v.i - this->i * v.k,
                this->i * v.j - this->j * v.i);
    }

    void Yaw(float rad) //only works with unit vector
    {
        float theta = 0;
        float m = Mag();
        if (i > 0) {
            theta = (float) atan(k / i);
        } else if (i < 0) {
            theta = M_PI + (float) atan(k / i);
        } else if (k <= 0 && i == 0) {
            theta = -M_PI / 2;
        } else if (k > 0 && i == 0) {
            theta = M_PI / 2;
        }
        theta += rad;
        i = m * cosf(theta);
        k = m * sinf(theta);
    }

    void Roll(float rad) {
        float theta = 0;
        float m = Mag();
        if (i > 0) {
            theta = (float) atan(j / i);
        } else if (i < 0) {
            theta = M_PI + (float) atan(j / i);
        } else if (j <= 0 && i == 0) {
            theta = -M_PI / 2;
        } else if (j > 0 && i == 0) {
            theta = M_PI / 2;
        }
        theta += rad;
        i = m * cosf(theta);
        j = m * sinf(theta);
    }

    void Pitch(float rad) {
        float theta = 0;
        float m = Mag();
        if (k > 0) {
            theta = (float) atan(j / k);
        } else if (k < 0) {
            theta = M_PI + (float) atan(j / k);
        } else if (j <= 0 && k == 0) {
            theta = -M_PI / 2;
        } else if (j > 0 && k == 0) {
            theta = M_PI / 2;
        }
        theta += rad;
        k = m * cosf(theta);
        j = m * sinf(theta);
    }
};

float grand() {
    return float(ssrand()) / VS_RAND_MAX;
}

vector<Color> lights;
VSFile f;
int xmllevel;

static void Tab() {
    for (int i = 0; i < xmllevel; i++) {
        f.Fprintf("\t");
    }
}

float difffunc(float inputdiffuse) {
    return sqrt(((inputdiffuse)));
}

void WriteLight(unsigned int i) {
    float ambient = (lights[i].r + lights[i].g + lights[i].b);

    ambient *= configuration().galaxy.ambient_light_factor_flt;
    Tab();
    f.Fprintf("<Light>\n");
    xmllevel++;
    Tab();
    f.Fprintf("<ambient red=\"%f\" green=\"%f\" blue=\"%f\"/>\n", ambient, ambient, ambient);
    Tab();
    f.Fprintf("<diffuse red=\"%f\" green=\"%f\" blue=\"%f\"/>\n", difffunc(lights[i].r), difffunc(lights[i].g),
            difffunc(lights[i].b));
    Tab();
    f.Fprintf("<specular red=\"%f\" green=\"%f\" blue=\"%f\"/>\n", lights[i].nr, lights[i].ng, lights[i].nb);
    xmllevel--;
    Tab();
    f.Fprintf("</Light>\n");
}

struct GradColor {
    float minrad;
    float r;
    float g;
    float b;
    float variance;
};

const int STAR = 0;
const int PLANET = 1;
const int MOON = 2;
const int JUMP = 3;
//begin global variables

vector<string> starentities;
vector<string> jumps;
vector<string> gradtex;
vector<string> naturalphenomena;
vector<string> starbases;
unsigned int numstarbases;
unsigned int numnaturalphenomena;
unsigned int numstarentities;
vector<string> background;
vector<string> names;
vector<string> rings;
const float moonofmoonprob = .01;
string systemname;
vector<float> radii;
const float minspeed = .001;
const float maxspeed = 8;
vector<float> starradius;
string faction;
vector<GradColor> colorGradiant;
float compactness = 2;
float jumpcompactness = 2;

struct PlanetInfo {
    string name;
    string unitname;
    string technique;
    unsigned int num;     //The texture number for the city lights
    unsigned int
            moonlevel;     //0==top-level planet, 1==first-level moon, 2 is second-level moon... probably won't be used.
    unsigned int
            numstarbases;     //Number of starbases allocated to orbit around this planet. Usually 1 or 0 but quite possibly more.
    unsigned int numjumps;     //Number of jump points.
    PlanetInfo() :
            num(0), moonlevel(0), numstarbases(0), numjumps(0) {
    }
};

struct StarInfo {
    vector<PlanetInfo> planets;
    unsigned int numjumps;
    unsigned int numstarbases;

    StarInfo() :
            numjumps(0), numstarbases(0) {
    }
};
vector<StarInfo> stars;
unsigned int planetoffset, staroffset, moonlevel;

void ResetGlobalVariables() {
    xmllevel = 0;
    lights.clear();
    gradtex.clear();
    naturalphenomena.clear();
    starbases.clear();
    starentities.clear();
    numstarentities = 0;
    numstarbases = 0;
    numnaturalphenomena = 0;
    background.clear();
    stars.clear();
    planetoffset = 0;
    staroffset = 0;
    moonlevel = 0;
    names.clear();
    jumps.clear();
    rings.clear();
    systemname.erase();
    starradius.clear();
    faction.erase();
    colorGradiant.clear();
    compactness = 2;
    jumpcompactness = 2;
    radii.clear();
}

void readColorGrads(vector<string> &entity, const char *file) {
    VSFile f;
    VSError err = f.OpenReadOnly(file, UniverseFile);
    if (err > Ok) {
        VS_LOG(error, (boost::format("Failed to load %1%") % file));
        GradColor g{};
        g.minrad = 0;
        g.r = g.g = g.b = .9;
        g.variance = .1;
        entity.emplace_back("white_star.png");
        colorGradiant.push_back(g);
        return;
    }
    char input_buffer[BUFFER_SIZE];
    char output_buffer[BUFFER_SIZE];
    GradColor g{};
    while (!f.Eof()) {
        f.ReadLine(input_buffer, BUFFER_SIZE - 1);
        if (sscanf(input_buffer, "%f %f %f %f %f %s", &g.minrad, &g.r, &g.g, &g.b, &g.variance, output_buffer) == 6) {
            g.minrad *= configuration().galaxy.star_radius_scale_flt;
            colorGradiant.push_back(g);
            entity.emplace_back(output_buffer);
        }
    }
    f.Close();
}

float clamp01(float a) {
    if (a > 1) {
        a = 1;
    }
    if (a < 0) {
        a = 0;
    }
    return a;
}

float getcolor(float c, float var) {
    return clamp01(c - var + 2 * var * grand());
}

GradColor whichGradColor(float r, unsigned int &j) {
    unsigned int i;
    if (colorGradiant.empty()) {
        vector<string> entity;
        string fullpath = "stars.txt";
        readColorGrads(entity, fullpath.c_str());
    }
    for (i = 1; i < colorGradiant.size(); i++) {
        if (colorGradiant[i].minrad > r) {
            break;
        }
    }
    j = i - 1;
    return colorGradiant[i - 1];
}

Color StarColor(float radius, unsigned int &entityindex) {
    GradColor gc = whichGradColor(radius, entityindex);
    float r = getcolor(gc.r, gc.variance);
    float g = getcolor(gc.g, gc.variance);
    float b = getcolor(gc.b, gc.variance);
    return Color(r, g, b);
}

GFXColor getStarColorFromRadius(float radius) {
    unsigned int myint = 0;
    Color tmp = StarColor(radius * configuration().galaxy.star_radius_scale_flt, myint);
    return GFXColor(tmp.r, tmp.g, tmp.b, 1);
}

float LengthOfYear(Vector r, Vector s) {
    float a = 2 * M_PI * mmax(r.Mag(), s.Mag());
    float speed = minspeed + (maxspeed - minspeed) * grand();
    return a / speed;
}

void CreateLight(unsigned int i) {
    if (i == 0) {
        assert(!starradius.empty());
        assert(starradius[0]);
    } else {
        assert(starradius.size() == i);
        starradius.push_back(starradius[0] * (.5 + grand() * .5));
    }
    unsigned int gradindex;
    lights.push_back(StarColor(starradius[i], gradindex));
    starentities.push_back(gradtex[gradindex]);
    float h = lights.back().r;
    if (h < lights.back().g) {
        h = lights.back().g;
    }
    if (h < lights.back().b) {
        h = lights.back().b;
    }
    float norm = (.5 + .5 / numstarentities);
    if (h > .001) {
        lights.back().nr = lights.back().r / h;
        lights.back().ng = lights.back().g / h;
        lights.back().nb = lights.back().b / h;

        lights.back().r *= norm / h;
        lights.back().g *= norm / h;
        lights.back().b *= norm / h;
    } else {
        lights.back().nr = 0;
        lights.back().ng = 0;
        lights.back().nb = 0;
    }
    WriteLight(i);
}

Vector generateCenter(float minradii, bool jumppoint) {
    Vector r;
    float tmpcompactness = compactness;
    if (jumppoint) {
        tmpcompactness = jumpcompactness;
    }
    r = Vector(tmpcompactness * grand() + 1, tmpcompactness * grand() + 1, tmpcompactness * grand() + 1);
    r.i *= minradii;
    r.j *= minradii;
    r.k *= minradii;
    int i = (rnd(0, 8));
    r.i = (i & 1) ? -r.i : r.i;
    r.j = (i & 2) ? -r.j : r.j;
    r.k = (i & 4) ? -r.k : r.k;
    return r;
}

float makeRS(Vector &r, Vector &s, float minradii, bool jumppoint) {
    r = Vector(grand(), grand(), grand());
    int i = (rnd(0, 8));
    r.i = (i & 1) ? -r.i : r.i;
    r.j = (i & 2) ? -r.j : r.j;
    r.k = (i & 4) ? -r.k : r.k;

    Vector k(grand(), grand(), grand());
    i = (rnd(0, 8));
    k.i = (i & 1) ? -k.i : k.i;
    k.j = (i & 2) ? -k.j : k.j;
    k.k = (i & 4) ? -k.k : k.k;

    s = r.Cross(k);
    float sm = s.Mag();
    if (sm < .01) {
        return makeRS(r, s, minradii, jumppoint);
    }
    s.i /= sm;
    s.j /= sm;
    s.k /= sm;
    sm = r.Mag();
    r.i /= sm;
    r.j /= sm;
    r.k /= sm;
    bool tmp = false;
    float rm;
    float tmpcompactness = compactness;
    if (jumppoint) {
        tmpcompactness = jumpcompactness;
    }
    rm = (tmpcompactness * grand() + 1);
    if (rm < 1) {
        rm = (1 + grand() * .5);
        tmp = true;
    }
    rm *= minradii;
    r.i *= rm;
    r.j *= rm;
    r.k *= rm;
    sm = (tmpcompactness * grand() + 1);
    if (tmp) {
        sm = (1 + grand() * .5);
    }
    sm *= minradii;
    s.i *= sm;
    s.j *= sm;
    s.k *= sm;
    return mmax(rm, sm);
}

void Updateradii(float orbitsize, float thisplanetradius) {
#ifdef HUGE_SYSTEMS
    orbitsize   += thisplanetradius;
    radii.back() = orbitsize;
#endif
}

Vector generateAndUpdateRS(Vector &r, Vector &s, float thisplanetradius, bool jumppoint) {
    if (radii.empty()) {
        r = Vector(0, 0, 0);
        s = Vector(0, 0, 0);
        return generateCenter(starradius[0], jumppoint);
    }
    float tmp = radii.back() + thisplanetradius;
    Updateradii(makeRS(r, s, tmp, jumppoint), thisplanetradius);
    return generateCenter(tmp, jumppoint);
}

vector<string> parseBigUnit(const string &input) {
    char *mystr = strdup(input.c_str());
    char *ptr = mystr;
    char *oldptr = mystr;
    vector<string> ans;
    while (*ptr != '\0') {
        while (*ptr != '&' && *ptr != '\0') {
            ptr++;
        }
        if (*ptr == '&') {
            *ptr = '\0';
            ptr++;
        }
        ans.emplace_back(oldptr);
        oldptr = ptr;
    }
    free(mystr);
    return ans;
}

void WriteUnit(const string &tag,
        const string &name,
        const string &filename,
        const Vector &r,
        const Vector &s,
        const Vector &center,
        const string &nebfile,
        const string &destination,
        bool faction,
        float thisloy = 0) {
    Tab();
    f.Fprintf("<%s name=\"%s\" file=\"%s\" ", tag.c_str(), name.c_str(), filename.c_str());
    if (nebfile.length() > 0) {
        f.Fprintf("nebfile=\"%s\" ", nebfile.c_str());
    }
    f.Fprintf("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", r.i, r.j, r.k, s.i, s.j, s.k);
    f.Fprintf("x=\"%f\" y=\"%f\" z=\"%f\" ", center.i, center.j, center.k);
    float loy = LengthOfYear(r, s);
    if (loy || thisloy) {
        f.Fprintf("year= \"%f\" ", thisloy ? thisloy : loy);
    }
    if (destination.length()) {
        f.Fprintf("destination=\"%s\" ", destination.c_str());
    } else if (faction) {
        f.Fprintf("faction=\"%s\" ", StarSystemGent::faction.c_str());
    }
    f.Fprintf("/>\n");
}

string getJumpTo(const string &s) {
    char tmp[BUFFER_SIZE] = "";
    if (1 == sscanf(s.c_str(), "Jump_To_%s", tmp)) {
        tmp[0] = tolower(tmp[0]);
    } else {
        return s;
    }
    return string(tmp);
}

string starin(const string &input) {
    char *tmp = strdup(input.c_str());
    for (unsigned int i = 0; tmp[i] != '\0'; i++) {
        if (tmp[i] == '*') {
            tmp[i] = '\0';
            string ans(tmp);
            free(tmp);
            return ans;
        }
    }
    free(tmp);
    return string();
}

string GetNebFile(string &input) {
    string ip = input.c_str();
    char *ptr = strdup(ip.c_str());
    for (unsigned int i = 0; ptr[i] != '\0'; i++) {
        if (ptr[i] == '^') {
            ptr[i] = '\0';
            string ans(ptr);
            input = ptr + i + 1;
            free(ptr);
            return ans;
        }
    }
    free(ptr);
    return string();
}

string AnalyzeType(string &input, string &nebfile, float &radius) {
    if (input.empty()) {
        return "";
    }
    char ptr = *input.begin();
    string ip;
    int sscanf_result = sscanf(GetNebFile(input).c_str(), "%f", &radius);
    if (0 == sscanf_result || EOF == sscanf_result) {
        radius = 100;
        ip = (input.c_str() + 1);
    } else {
        ip = (input);
    }
    string retval;
    switch (ptr) {
        case 'N':
            nebfile = GetNebFile(input);
            retval = "Nebula";
            break;
        case 'A':
            retval = "Asteroid";
            break;
        case 'B':
            retval = "Building";
            break;
        case 'E':
            retval = "Enhancement";
            break;
        case 'U':
        default:
            retval = "Unit";
    }
    return retval;
}

void MakeSmallUnit() {
    Vector R, S;

    string nam;
    string base_type;
    string s = string("");
    nam = getRandName(names);
    while (s.length() == 0) {
        base_type = getRandName(starbases);
        if (base_type.length() == 0) {
            return;
        }
        string tmp;
        if ((tmp = starin(base_type)).length() > 0) {
            base_type = (tmp);
            s = getRandName(jumps);
        } else {
            break;
        }
    }
    numstarbases--;
    string nebfile("");
    float radius;
    string type = AnalyzeType(base_type, nebfile, radius);
    Vector center = generateAndUpdateRS(R, S, radius, true);
    WriteUnit(type, nam, base_type, R, S, center, nebfile, s, true);
}

void MakeJump(float radius,
        bool forceRS = false,
        Vector R = Vector(0, 0, 0),
        Vector S = Vector(0,
                0,
                0),
        Vector center = Vector(
                0,
                0,
                0),
        float thisloy = 0) {
    string s = getRandName(jumps);
    if (s.length() == 0) {
        return;
    }
    Vector RR, SS;
    if (forceRS) {
        RR = R;
        SS = S;
        Updateradii(mmax(RR.Mag(), SS.Mag()), radius);
    } else {
        center = generateAndUpdateRS(RR, SS, radius, true);
    }
    string thisname;
    thisname = string("Jump_To_") + getStarSystemName(s);
    if (thisname.length() > 8) {
        *(thisname.begin() + 8) = toupper(*(thisname.begin() + 8));
    }
    Tab();
    //backwards compatibility
    static bool usePNGFilename = (VSFileSystem::LookForFile("jump.png", VSFileSystem::TextureFile) <= VSFileSystem::Ok);
    static const char *jumpFilename = usePNGFilename ? "jump.png" : "jump.texture";
    f.Fprintf("<Jump name=\"%s\" file=\"%s\" ", thisname.c_str(), jumpFilename);
    f.Fprintf("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", RR.i, RR.j, RR.k, SS.i, SS.j, SS.k);
    f.Fprintf("radius=\"%f\" ", radius);
    f.Fprintf("x=\"%f\" y=\"%f\" z=\"%f\" ", center.i, center.j, center.k);
    float loy = LengthOfYear(RR, SS);
    float temprandom = .1 * fmod(loy, 10);     //use this so as not to alter state here
    if (loy || thisloy) {
        f.Fprintf("year= \"%f\" ", thisloy ? thisloy : loy);
        temprandom = grand();
        loy = 864 * temprandom;
        if (loy) {
            f.Fprintf("day=\"%f\" ", loy);
        }
    }
    f.Fprintf("alpha=\"ONE ONE\" destination=\"%s\" faction=\"%s\" />\n", getJumpTo(
            s).c_str(), StarSystemGent::faction.c_str());

    ///writes out some pretty planet tags
}

void MakeBigUnit(int callingentitytype, string name = string(), float orbitalradius = 0) {
    vector<string> fullname;
    if (name.length() == 0) {
        string s = getRandName(naturalphenomena);
        if (s.length() == 0) {
            return;
        }
        fullname = parseBigUnit(s);
    } else {
        fullname.push_back(name);
    }
    if (fullname.empty()) {
        return;
    }
    numnaturalphenomena--;
    Vector r, s;

    float stdloy = 0;
    bool first = false;
    float size = 0;
    string tmp;
    Vector center(0, 0, 0);
    string nebfile("");
    for (unsigned int i = 0; i < fullname.size(); i++) {
        if (1 == sscanf(fullname[i].c_str(), "jump%f", &size)) {
            if (!first) {
                first = true;
                center = generateAndUpdateRS(r, s, size, callingentitytype != STAR);
                stdloy = LengthOfYear(r, s);
            }
            MakeJump(size, true, r, s, center, stdloy);
            //We still want jumps inside asteroid fields, etcVvv.
        } else if (1
                == sscanf(fullname[i].c_str(), "planet%f",
                        &size) || 1
                == sscanf(fullname[i].c_str(), "moon%f", &size) || 1 == sscanf(fullname[i].c_str(), "gas%f", &size)) {
            //FIXME: Obsolete/not supported/too lazy to implement.
        } else if ((tmp = starin(fullname[i])).length() > 0) {
            if (!first) {
                first = true;
                center = generateAndUpdateRS(r, s, size, callingentitytype != STAR);
                stdloy = LengthOfYear(r, s);
            }
            string S = getRandName(jumps);
            if (S.length() > 0) {
                string type = AnalyzeType(tmp, nebfile, size);
                WriteUnit(type, S, tmp, r, s, center, nebfile, getJumpTo(S), false, stdloy);
            }
        } else {
            string type = AnalyzeType(fullname[i], nebfile, size);
            if (!first) {
                first = true;
                center = generateAndUpdateRS(r, s, size, callingentitytype != STAR && type == "Unit");
                stdloy = LengthOfYear(r, s);
            }
            WriteUnit(type, "", fullname[i], r, s, center, nebfile, string(""), i != 0, stdloy);
        }
    }
}

void MakeMoons(float callingradius, int callingentitytype);
void MakeJumps(float callingradius, int callingentitytype, int numberofjumps);

void MakePlanet(float radius,
        int entitytype,
        string texturename,
        string unitname,
        string technique,
        int texturenum,
        int numberofjumps,
        int numberofstarbases) {
    if (entitytype == JUMP) {
        MakeJump(radius);
        return;
    }
    if (texturename.length() == 0) {      //FIXME?
        return;
    }
    Vector RR, SS;
    Vector center = generateAndUpdateRS(RR, SS, radius, false);
    string thisname;
    thisname = getRandName(names);
    Tab();
    string atmosphere = _Universe->getGalaxy()->getPlanetVariable(texturename, "atmosphere", "false");
    if (atmosphere == "false") {
        atmosphere = "";
    } else if (atmosphere == "true") {
        atmosphere = configuration().galaxy.default_atmosphere_texture;
    }
    string cname;
    string planetlites = _Universe->getGalaxy()->getPlanetVariable(texturename, "lights", "");
    if (!planetlites.empty()) {
        planetlites = ' ' + planetlites;
        vector<string::size_type> lites;
        lites.push_back(0);
        while (lites.back() != string::npos) {
            lites.push_back(planetlites.find(lites.back() + 1, ' '));
        }
        unsigned randomnum = rnd(0, lites.size() - 1);
        cname = planetlites.substr(lites[randomnum] + 1, lites[randomnum + 1]);
    }
    f.Fprintf("<Planet name=\"%s\" file=\"%s\" unit=\"%s\" ", thisname.c_str(), texturename.c_str(), unitname.c_str());
    if (!technique.empty()) {
        f.Fprintf("technique=\"%s\" ", technique.c_str());
    }
    if (texturename.find_first_of('|') != string::npos) {
        f.Fprintf(
                "Red=\"0\" Green=\"0\" Blue=\"0\" DRed=\"0.87\" DGreen=\"0.87\" DBlue=\"0.87\" SRed=\"0.85\" SGreen=\"0.85\" SBlue=\"0.85\" ");
    }
    f.Fprintf("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", RR.i, RR.j, RR.k, SS.i, SS.j, SS.k);
    f.Fprintf("radius=\"%f\" ", radius);
    f.Fprintf("x=\"%f\" y=\"%f\" z=\"%f\" ", center.i, center.j, center.k);
    float loy = LengthOfYear(RR, SS);
    float temprandom = .1 * fmod(loy, 10);     //use this so as not to alter state here
    if (loy) {
        f.Fprintf("year= \"%f\" ", loy);
        temprandom = grand();
        loy = 864 * temprandom;
        if (loy) {
            f.Fprintf("day=\"%f\" ", loy);
        }
    }
    f.Fprintf(">\n");
    xmllevel++;
    if (!cname.empty()) {
        int wrapx = 1;
        int wrapy = 1;
        cname = GetWrapXY(cname, wrapx, wrapy);
        string::size_type t;
        while ((t = cname.find('*')) != string::npos) {
            cname.replace(t, 1, texturenum == 0 ? "" : XMLSupport::tostring(texturenum));
        }
        Tab();
        f.Fprintf("<CityLights file=\"%s\" wrapx=\"%d\" wrapy=\"%d\"/>\n", cname.c_str(), wrapx, wrapy);
    }
    if ((entitytype == PLANET && temprandom < configuration().galaxy.atmosphere_probability_flt) && (!atmosphere.empty())) {
        string NAME = thisname + " Atmosphere";
        {
            bool doalphaatmosphere = (temprandom < .08 || temprandom > .3);
            if (doalphaatmosphere) {
                float fograd = radius * 1.007;
                if (.007 * radius > 2500.0) {
                    fograd = radius + 2500.0;
                }
                Tab();
                f.Fprintf("<Atmosphere file=\"%s\" alpha=\"SRCALPHA INVSRCALPHA\" radius=\"%f\"/>\n",
                        atmosphere.c_str(), fograd);
            }
            float r = .9, g = .9, b = 1, a = 1;
            float dr = .9, dg = .9, db = 1, da = 1;
            if (!doalphaatmosphere) {
                if (temprandom > .26 || temprandom < .09) {
                    r = .5;
                    g = 1;
                    b = .5;
                } else if (temprandom > .24 || temprandom < .092) {
                    r = 1;
                    g = .6;
                    b = .5;
                }
            }

/*----------------------------------------------------------------------------------------*\
| **************************************************************************************** |
| ********************************* FIXME: USE BFXM  ************************************* |
| **************************************************************************************** |
\*----------------------------------------------------------------------------------------*/

            Tab();
            f.Fprintf("<Fog>\n");
            xmllevel++;
            Tab();
            f.Fprintf(
                    "<FogElement file=\"atmXatm.bfxm\" ScaleAtmosphereHeight=\"1.0\"  red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"%f\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"%f\" concavity=\".3\" focus=\".6\" minalpha=\"0\" maxalpha=\"0.7\"/>\n",
                    r,
                    g,
                    b,
                    a,
                    dr,
                    dg,
                    db,
                    da);
            Tab();
            f.Fprintf(
                    "<FogElement file=\"atmXhalo.bfxm\" ScaleAtmosphereHeight=\"1.0\"  red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"%f\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"%f\" concavity=\"1\" focus=\".6\" minalpha=\"0\" maxalpha=\"0.7\"/>\n",
                    r,
                    g,
                    b,
                    a,
                    dr,
                    dg,
                    db,
                    da);
            xmllevel--;
            Tab();
            f.Fprintf("</Fog>\n");
        }
    }
//FIRME: need scaling of radius based on planet type.
    radii.push_back(radius);

    if (entitytype == PLANET) {
        float ringrand = grand();
        if (ringrand < configuration().galaxy.ring_probability_flt) {
            string ringname = getRandName(rings);
            double inner_rad = configuration().galaxy.inner_ring_radius_dbl * (1.0 + grand() * 0.5) * radius;
            double outer_rad = inner_rad + configuration().galaxy.outer_ring_radius_dbl * grand() * radius;
            int wrapx = 1;
            int wrapy = 1;
            if (ringname.empty()) {
                ringname = configuration().galaxy.default_ring_texture;
            }
            ringname = GetWrapXY(ringname, wrapx, wrapy);
            Vector r, s;
            makeRS(r, s, 1, false);
            float rmag = r.Mag();
            if (rmag > .001) {
                r.i /= rmag;
                r.j /= rmag;
                r.k /= rmag;
            }
            float smag = s.Mag();
            if (smag > .001) {
                s.i /= smag;
                s.j /= smag;
                s.k /= smag;
            }
            if (ringrand < (1 - configuration().galaxy.double_ring_probability_flt)) {
                Tab();
                f.Fprintf(
                        "<Ring file=\"%s\" ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" innerradius=\"%f\" outerradius=\"%f\" wrapx=\"%d\" wrapy=\"%d\" />\n",
                        ringname.c_str(),
                        r.i,
                        r.j,
                        r.k,
                        s.i,
                        s.j,
                        s.k,
                        inner_rad,
                        outer_rad,
                        wrapx,
                        wrapy);
            }
            if (ringrand < configuration().galaxy.double_ring_probability_flt
                    || ringrand >= (configuration().galaxy.ring_probability_flt - configuration().galaxy.double_ring_probability_flt)) {
                double movable = grand();
                inner_rad = outer_rad
                        * (1 + .1 * (configuration().galaxy.second_ring_difference_flt + configuration().galaxy.second_ring_difference_flt * movable));
                outer_rad = inner_rad * (configuration().galaxy.outer_ring_radius_flt * movable);
                Tab();
                f.Fprintf(
                        "<Ring file=\"%s\" ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" innerradius=\"%f\" outerradius=\"%f\" wrapx=\"%d\" wrapy=\"%d\" />\n",
                        ringname.c_str(),
                        r.i,
                        r.j,
                        r.k,
                        s.i,
                        s.j,
                        s.k,
                        inner_rad,
                        outer_rad,
                        wrapx,
                        wrapy);
            }
        }
    }
    for (int i = 0; i < numberofstarbases; i++) {
        MakeSmallUnit();
    }
    moonlevel++;
    MakeMoons(
            entitytype != MOON ? configuration().galaxy.moon_relative_to_planet_flt * radius : configuration().galaxy.moon_relative_to_moon_flt * radius,
            entitytype);
    MakeJumps(100 + grand() * 300, entitytype, numberofjumps);
    moonlevel--;
    radii.pop_back();
    xmllevel--;
    Tab();
    f.Fprintf("</Planet>\n");

    //writes out some pretty planet tags
}

void MakeJumps(float callingradius, int callingentitytype, int numberofjumps) {
    for (int i = 0; i < numberofjumps; i++) {
        MakeJump((.5 + .5 * grand()) * callingradius);
    }
}

void MakeMoons(float callingradius, int callingentitytype) {
    while (planetoffset < stars[staroffset].planets.size()
            && stars[staroffset].planets[planetoffset].moonlevel == moonlevel) {
        PlanetInfo &infos = stars[staroffset].planets[planetoffset++];
        MakePlanet(
                (.5 + .5 * grand()) * callingradius, callingentitytype == STAR ? PLANET : MOON,
                infos.name, infos.unitname, infos.technique,
                infos.num, infos.numjumps,
                infos.numstarbases);
    }
}

void beginStar() {
    float radius = starradius[staroffset];
    Vector r, s;
    unsigned int i;
    Vector center = generateAndUpdateRS(r,
            s,
            radius,
            false);     //WTF why was this commented out--that means all stars start on top of each other
    planetoffset = 0;

    char b[3] = " A";
    b[1] += staroffset;
    Tab();
    f.Fprintf("<Planet name=\"%s%s\" file=\"%s\" ", systemname.c_str(), b, starentities[staroffset].c_str());
    f.Fprintf("ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", r.i, r.j, r.k, s.i, s.j, s.k);
    if (staroffset != 0) {
        f.Fprintf("radius=\"%f\" x=\"%f\" y=\"%f\" z=\"%f\" ", radius, center.i, center.j, center.k);
    } else {
        f.Fprintf("radius=\"%f\" x=\"0\" y=\"0\" z=\"0\" ", radius);
    }
    float loy = LengthOfYear(r, s);
    if (loy) {
        f.Fprintf("year= \"%f\" ", loy);
        loy *= grand();
        if (loy) {
            f.Fprintf("day=\"%f\" ", loy);
        }
    }
    f.Fprintf(" Red=\"%f\" Green=\"%f\" Blue=\"%f\" ReflectNoLight=\"true\" light=\"%d\">\n", lights[staroffset].r,
            lights[staroffset].g, lights[staroffset].b, staroffset);
    f.Fprintf(
            "<fog>\n\t<FogElement file=\"atmXatm.bfxm\" ScaleAtmosphereHeight=\".900\" red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"1.0\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"1\" concavity=\".3\" focus=\".6\" minalpha=\".7\" maxalpha=\"1\"/>\n\t<FogElement file=\"atmXhalo.bfxm\" ScaleAtmosphereHeight=\".9000\" red=\"%f\" blue=\"%f\" green=\"%f\" alpha=\"1.0\" dired=\"%f\" diblue=\"%f\" digreen=\"%f\" dialpha=\"1\" concavity=\".3\" focus=\".6\" minalpha=\".7\" maxalpha=\"1\"/>\n</fog>\n",
            lights[staroffset].r,
            lights[staroffset].g,
            lights[staroffset].b,
            lights[staroffset].r,
            lights[staroffset].g,
            lights[staroffset].b,
            lights[staroffset].r,
            lights[staroffset].g,
            lights[staroffset].b,
            lights[staroffset].r,
            lights[staroffset].g,
            lights[staroffset].b);
    radii.push_back(1.5 * radius);
    xmllevel++;
    unsigned int numu;
    if (numstarentities) {
        numu = numnaturalphenomena
                / (numstarentities - staroffset)
                + (grand()
                        < float(numnaturalphenomena % (numstarentities - staroffset)) / (numstarentities - staroffset));
    } else {
        numu = 1;
    }
    if ((int) staroffset == (int) (numstarentities - staroffset) - 1) {
        numu = numnaturalphenomena;
    }
    for (i = 0; i < numu; ++i) {
        MakeBigUnit(STAR);
    }
    for (i = 0; i < stars[staroffset].numstarbases; ++i) {
        MakeSmallUnit();
    }
    MakeJumps(100 + grand() * 300, STAR, stars[staroffset].numjumps);
    MakeMoons(configuration().galaxy.rocky_relative_to_primary_flt * radius, STAR);
    //Fixme: no jumps should be made around the star.
    if (!jumps.empty()) {
        VS_LOG(error, (boost::format("ERROR: jumps not empty() Size==%1$u!!!!!") % jumps.size()));
    }
    staroffset++;
}

void endStar() {
    radii.pop_back();
    xmllevel--;
    Tab();
    f.Fprintf("</Planet>\n");
}

void CreateStar() {
    beginStar();
    endStar();
}

void CreateFirstStar() {
    beginStar();
    while (staroffset < numstarentities) {
        if (grand() > .5) {
            CreateFirstStar();
            break;
        } else {
            CreateStar();
        }
    }
    endStar();
}

void CreatePrimaries() {
    unsigned int i;
    for (i = 0; i < numstarentities || i == 0; i++) {
        CreateLight(i);
    }
    CreateFirstStar();
}

void CreateStarSystem() {
    assert(!starradius.empty());
    assert(starradius[0]);
    xmllevel = 0;
    Tab();
    f.Fprintf("<?xml version=\"1.0\" ?>\n<system name=\"%s\" background=\"%s\">\n", systemname.c_str(),
            getRandName(background).c_str());
    xmllevel++;
    CreatePrimaries();
    xmllevel--;
    Tab();
    f.Fprintf("</system>\n");
}

void readentity(vector<string> &entity, const char *filename) {
    VSFile f;
    VSError err = f.OpenReadOnly(filename, UniverseFile);
    if (err > Ok) {
        return;
    }
    char input_buffer[BUFFER_SIZE];
    while (1 == f.Fscanf(SCANF_FORMAT_STRING, input_buffer)) {
        entity.emplace_back(input_buffer);
    }
    f.Close();
}

const char *noslash(const char *in) {
    const char *tmp = in;
    while (*tmp != '\0' && *tmp != '/') {
        tmp++;
    }
    if (*tmp != '\0') {
        tmp++;
    } else {
        return in;
    }
    const char *tmp2 = tmp;
    tmp2 = noslash(tmp2);
    if (tmp2[0] != '\0') {
        return tmp2;
    } else {
        return tmp;
    }
}
}
using namespace StarSystemGent;

string getStarSystemFileName(const string &input) {
    return input + string(".system");
}

string getStarSystemName(const string &in) {
    return string(noslash(in.c_str()));
}

string getStarSystemSector(const string &in) {
    string::size_type sep = in.find('/');
    if (sep == string::npos) {
        return string(".");
    } else {
        return in.substr(0, sep);
    }
}

void readnames(vector<string> &entity, const char *filename) {
    VSFile f;
    VSError err = f.OpenReadOnly(filename, UniverseFile);
    if (err > Ok) {
        return;
    }
    char input_buffer[BUFFER_SIZE];
    while (!f.Eof()) {
        f.ReadLine(input_buffer, BUFFER_SIZE - 1);
        if (input_buffer[0] == '\0' || input_buffer[0] == '\n' || input_buffer[0] == '\r') {
            continue;
        }
        for (unsigned int i = 0; input_buffer[i] != '\0' && i < BUFFER_SIZE - 1; i++) {
            if (input_buffer[i] == '\r') {
                input_buffer[i] = '\0';
            }
            if (input_buffer[i] == '\n') {
                input_buffer[i] = '\0';
                break;
            }
        }
        entity.emplace_back(input_buffer);
    }
    f.Close();
}

void readplanetentity(vector<StarInfo> &starinfos, string planetlist, unsigned int numstars) {
    if (numstars < 1) {
        numstars = 1;
        VS_LOG(warning, "No stars exist in this system!");
    }
    string::size_type i, j;
    unsigned int u;
    starinfos.reserve(numstars);
    for (u = 0; u < numstars; ++u) {
        starinfos.emplace_back();
    }
    u--;
    while (i = planetlist.find(' '), 1) {
        if (i == 0) {
            planetlist = planetlist.substr(1);
            continue;
        }
        int nummoon = 0;
        for (j = 0; j < i && j < planetlist.size() && planetlist[j] == '*'; ++j) {
            nummoon++;
        }
        if (nummoon == 0) {
            u++;
        }
        if (j == string::npos || j >= planetlist.size()) {
            break;
        }
        starinfos[u % numstars].planets.emplace_back();
        starinfos[u % numstars].planets.back().moonlevel = nummoon;
        {
            GalaxyXML::Galaxy *galaxy = _Universe->getGalaxy();

            static const string numtag("#num#");
            static const string empty;
            static const string::size_type numlen = numtag.length();
            string::size_type numpos;

            // Get planet name and texture
            string planetname =
                    galaxy->getPlanetNameFromInitial(planetlist.substr(j, i == string::npos ? string::npos : i - j));
            string texturename = galaxy->getPlanetVariable(planetname, "texture", "No texture supplied in <planets>!");

            // Get unit name, deriving a default name from its texture
            string defunitname = texturename.substr(0, texturename.find_first_of('|'));
            if (defunitname.find_last_of('/') != string::npos) {
                defunitname = defunitname.substr(defunitname.find_last_of('/') + 1);
            }
            defunitname = defunitname.substr(0, defunitname.find_last_of('.'));

            numpos = 0;
            while ((numpos = defunitname.find(numtag, numpos)) != string::npos) {
                defunitname.replace(numpos, numlen, empty);
            }

            string unitname = galaxy->getPlanetVariable(planetname, "unit", defunitname);

            // Get planet rendering technique
            string techniquename = galaxy->getPlanetVariable(planetname, "technique", "");

            // Replace randomized number placeholder tags
            starinfos[u % numstars].planets.back().num =
                    rnd(XMLSupport::parse_int(galaxy->getPlanetVariable(planetname, "texture_min", "0")),
                            XMLSupport::parse_int(galaxy->getPlanetVariable(planetname, "texture_max", "0")));

            char num[32];
            if (starinfos[u % numstars].planets.back().num == 0) {
                num[0] = 0;
            } else {
                snprintf(num, sizeof(num) - 1, "%d", starinfos[u % numstars].planets.back().num);
            }

            numpos = 0;
            while ((numpos = texturename.find(numtag, numpos)) != string::npos) {
                texturename.replace(numpos, numlen, num);
            }

            // Store info
            starinfos[u % numstars].planets.back().name = texturename;
            starinfos[u % numstars].planets.back().unitname = unitname;
            starinfos[u % numstars].planets.back().technique = techniquename;
        }
        starinfos[u % numstars].planets.back().numstarbases = 0;
        starinfos[u % numstars].planets.back().numjumps = 0;
        if (i == string::npos) {
            break;
        }
        planetlist = planetlist.substr(i + 1);
    }
    unsigned int k;
    if (!starinfos.empty()) {
        bool size = false;
        for (k = 0; k < starinfos.size(); ++k) {
            if (!starinfos[k].planets.empty()) {
                size = true;
                break;
            }
        }
        if (!size) {
            int oldjumps = jumps.size();
            int oldstarbases = numstarbases;
            int newstuff;
            for (k = starinfos.size(); k > 0; --k) {
                newstuff = oldjumps / k;
                starinfos[k - 1].numjumps = newstuff;
                oldjumps -= newstuff;
                newstuff = oldstarbases / k;
                starinfos[k - 1].numstarbases = newstuff;
                oldstarbases -= newstuff;
            }
        } else {
            for (k = 0; k < jumps.size(); ++k) {
                vector<PlanetInfo> *temp;                 //& doesn't like me so I use *.
                do {
                    temp = &starinfos[rnd(0, starinfos.size())].planets;
                } while (temp->empty());
                (*temp)[rnd(0, temp->size())].numjumps++;
            }
            for (k = 0; k < numstarbases; ++k) {
                vector<PlanetInfo> *temp;                 //& appears to still have dislike for me.
                do {
                    temp = &starinfos[rnd(0, starinfos.size())].planets;
                } while (temp->empty());
                (*temp)[rnd(0, temp->size())].numstarbases++;
            }
        }
    }
}

static int pushDown(int val) {
    while (grand() > (1 / val)) {
        val--;
    }
    return val;
}

static int pushDownTowardsMean(int mean, int val) {
    int delta = mean - 1;
    return delta + pushDown(val - delta);
}

static int pushTowardsMean(int mean, int val) {
    if (!configuration().galaxy.push_values_to_mean) {
        return val;
    }
    if (val < mean) {
        return -pushDownTowardsMean(-mean, -val);
    }
    return pushDownTowardsMean(mean, val);
}

void generateStarSystem(SystemInfo &si) {
    ResetGlobalVariables();
    si.sunradius *= configuration().galaxy.star_radius_scale_flt;
    systemname = si.name;

    compactness = si.compactness * configuration().galaxy.compactness_scale_flt;
    jumpcompactness = si.compactness * configuration().galaxy.jump_compactness_scale_flt;
    if (si.seed) {
        seedrand(si.seed);
    } else {
        seedrand(stringhash(si.sector + '/' + si.name));
    }
    VS_LOG(info, (boost::format("star %1%, natural %2%, bases %3%") % si.numstars % si.numun1 % si.numun2));
    int nat = pushTowardsMean(configuration().galaxy.mean_natural_phenomena, si.numun1);
    numnaturalphenomena = nat > si.numun1 ? si.numun1 : nat;
    numstarbases = pushTowardsMean(configuration().galaxy.mean_star_bases, si.numun2);
    numstarentities = si.numstars;
    VS_LOG(info,
            (boost::format("star %1%, natural %2%, bases %3%") % numstarentities % numnaturalphenomena % numstarbases));
    starradius.push_back(si.sunradius);
    readColorGrads(gradtex, (si.stars).c_str());

    readentity(starbases, (si.smallun).c_str());
    readentity(background, (si.backgrounds).c_str());
    if (background.empty()) {
        background.push_back(si.backgrounds);
    }
    if (si.nebulae) {
        readentity(naturalphenomena, (si.nebulaelist).c_str());
    }
    if (si.asteroids) {
        readentity(naturalphenomena, (si.asteroidslist).c_str());
    }
    for (unsigned int i = 0; i < si.jumps.size(); i++) {
        jumps.push_back(si.jumps[i]);
    }
    faction = si.faction;

    readplanetentity(stars, si.planetlist, numstarentities);

    readentity(rings, (si.ringlist).c_str());
    readnames(names, (si.names).c_str());

    CreateDirectoryHome(VSFileSystem::sharedsectors + "/" + VSFileSystem::universe_name + "/" + si.sector);

    VSError err = f.OpenCreateWrite(si.filename, SystemFile);
    if (err <= Ok) {
        CreateStarSystem();
        f.Close();
    }
    ResetGlobalVariables();     //deallocate any unused memory in vectors.
}

#ifdef CONSOLE_APP
int main( int argc, char **argv )
{
    if (argc < 9) {
        // stephengtuggy 2020-11-12: Leaving this, since it is for standalone CONSOLE_APP mode
        VSFileSystem::vs_fprintf(stderr,
            "Usage: starsysgen <seed> <sector>/<system> <sunradius>/<compactness> <numstars> [N][A]<numnaturalphenomena> <numstarbases> <faction> <namelist> [OtherSystemJumpNodes]...\n" );
        return 1;
    }
    int    seed;
    if ( 1 != sscanf( argv[1], "%d", &seed ) )
        return 1;
    string sectorname( getStarSystemSector( argv[2] ) );
    string filen( getStarSystemFileName( argv[2] ) );
    string systemname = string( getStarSystemName( argv[2] ) );
    int    numbigthings;
    bool   nebula     = true;
    bool   asteroid   = true;
    float  srad;
    float  comp;
    sscanf( argv[3], "%f/%f", &srad, &comp );
    vector< string >jumps;
    for (unsigned int i = 12; i < argc; i++)
        jumps.push_back( string( argv[i] ) );
    if ( 1 == sscanf( argv[8], "AN%d", &numbigthings ) || 1 == sscanf( argv[8], "NA%d", &numun[0] ) )
        nebula = asteroid = true;
    else if ( 1 == sscanf( argv[8], "A%d", &numbigthings ) )
        asteroid = true;
    else if ( 1 == sscanf( argv[8], "N%d", &numbigthings ) )
        nebula = true;
    else if ( 1 == sscanf( argv[8], "%d", &numbigthings ) )
        nebula = asteroid = true;
    else
        return -1;
    generateStarSystem( "./",
                        seed,
                        sectorname,
                        systemname,
                        filen,
                        srad, comp;
                        strtol( argv[4], NULL, 10 ),
                        strtol( argv[5], NULL, 10 ),
                        strtol( argv[6], NULL, 10 ),
                        strtol( argv[7], NULL, 10 ),
                        nebula,
                        asteroid,
                        numbigthings,
                        strtol( argv[9], NULL, 10 ),
                        argv[10],
                        argv[11],
                        jumps );

    return 0;
}
#endif
