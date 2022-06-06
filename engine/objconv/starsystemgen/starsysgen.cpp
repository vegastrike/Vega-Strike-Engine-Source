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


#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <math.h>
#include <time.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.1415926536
#endif

static const size_t BUFFER_SIZE = 16000;
static const char SCANF_FORMAT_STRING[] = "%15999s";

using namespace std;
static unsigned int starsysrandom = time(NULL);

static void seedrand(int seed) {
    starsysrandom = seed;
}

static unsigned int ssrand() {
    starsysrandom = (starsysrandom * 1103515245 + 12345) % ((unsigned long) RAND_MAX + 1);
    return starsysrandom;
}

namespace StarSystemGent {
float mmax(float a, float b) {
    return (a > b) ? a : b;
}

int rnd(int lower, int upper) {
    return (int) (lower + (((float(upper - lower)) * ssrand()) / (float(RAND_MAX) + 1.)));
}

const char nada[1] = "";

string getRandName(vector<string> &s) {
    if (s.empty()) {
        return string(nada);
    }
    unsigned int i = rnd(0, s.size());
    string k = s[i];
    s.erase(s.begin() + i);
    return k;
}

string getGenericName(vector<string> &s) {
    if (s.empty()) {
        return string(nada);
    }
    return s[rnd(0, s.size())];
}

struct Color {
    float r, g, b, a;
    float nr, ng, nb, na;

    Color(float rr, float gg, float bb) {
        r = rr;
        b = bb;
        g = gg;
    }
};
class Vector {
public:
    float i;
    float j;
    float k;
    float s;
    float t;

    Vector() {
        i = j = k = 0;
    }

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

    float Mag() {
        return sqrtf(i * i + j * j + k * k);
    }

    Vector Cross(const Vector &v) const {
        return Vector(this->j * v.k - this->k * v.j,
                this->k * v.i - this->i * v.k,
                this->i * v.j - this->j * v.i);
    }

    void Yaw(float rad) //only works with unit vector
    {
        float theta = 0.0f;
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
        float theta = 0.0f;
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
        float theta = 0.0f;
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
    return float(ssrand()) / RAND_MAX;
}

vector<Color> lights;
FILE *fp = NULL;

float difffunc(float inputdiffuse) {
    return sqrt(((inputdiffuse)));
}

void WriteLight(unsigned int i) {
    float ambient = (lights[i].r + lights[i].g + lights[i].b);
    ambient /= 12;
    fprintf(
            fp,
            "<Light>\n\t<ambient red=\"%f\" green=\"%f\" blue=\"%f\"/>\n\t<diffuse red=\"%f\" green=\"%f\" blue=\"%f\"/>\n\t<specular red=\"%f\" green=\"%f\" blue=\"%f\"/>\n</Light>\n",
            ambient,
            ambient,
            ambient,
            difffunc(lights[i].r),
            difffunc(lights[i].g),
            difffunc(lights[i].b),
            lights[i].nr,
            lights[i].ng,
            lights[i].nb);
}

const int PLANET = 2;
const int GAS = 1;
const int STAR = 0;
const int MOON = 3;
const int JUMP = 4;
int nument[5];
vector<string> entities[5];
vector<string> gradtex;
int numun[2];
vector<string> units[2];
vector<string> background;
vector<string> names;
const float moonofmoonprob = .01;
string systemname;
vector<float> radii;
const float minspeed = .001;
const float maxspeed = 8;

void Tab() {
    for (unsigned int i = 0; i < radii.size(); i++) {
        fprintf(fp, "\t");
    }
}

vector<float> starradius;
string faction;

struct GradColor {
    float minrad;
    float r;
    float g;
    float b;
    float variance;
};

vector<GradColor> colorGradiant;

void readColorGrads(vector<string> &entity, const char *file) {
    FILE *fp = fopen(file, "r");
    if (!fp) {
        return;
    }
    char input_buffer[1000];
    char output_buffer[1000];
    GradColor g;
    while (!feof(fp)) {
        fgets(input_buffer, 999, fp);
        if (sscanf(input_buffer, "%f %f %f %f %f %s", &g.minrad, &g.r, &g.g, &g.b, &g.variance, output_buffer) == 6) {
            colorGradiant.push_back(g);
            entity.push_back(output_buffer);
        }
    }
    fclose(fp);
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
    entities[STAR].push_back(gradtex[gradindex]);
    float h = lights.back().r;
    if (h < lights.back().g) {
        h = lights.back().g;
    }
    if (h < lights.back().b) {
        h = lights.back().b;
    }
    float norm = (.5 + .5 / nument[0]);
    lights.back().nr = lights.back().r / h;
    lights.back().ng = lights.back().g / h;
    lights.back().nb = lights.back().b / h;

    lights.back().r *= norm / h;
    lights.back().g *= norm / h;
    lights.back().b *= norm / h;
    WriteLight(i);
}

Vector generateCenter(float minradii) {
    Vector r;
    r = Vector(3 * grand() + 1, 3 * grand() + 1, 3 * grand() + 1);
    r.i *= minradii;
    r.j *= minradii;
    r.k *= minradii;
    int i = (rnd(0, 8));
    r.i = (i & 1) ? -r.i : r.i;
    r.j = (i & 2) ? -r.j : r.j;
    r.k = (i & 4) ? -r.k : r.k;
    return r;
}

float makeRS(Vector &r, Vector &s, float minradii) {
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
        return makeRS(r, s, minradii);
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
    rm = (2 * grand() + 1);
    if (rm < 1) {
        rm = (1 + grand() * .5);
        tmp = true;
    }
    rm *= minradii;
    r.i *= rm;
    r.j *= rm;
    r.k *= rm;
    sm = (2 * grand() + 1);
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

Vector generateAndUpdateRS(Vector &r, Vector &s, float thisplanetradius) {
    if (radii.empty()) {
        r = Vector(0, 0, 0);
        s = Vector(0, 0, 0);
        return generateCenter(starradius[0]);
    }
    float tmp = radii.back() + thisplanetradius;
    Updateradii(makeRS(r, s, tmp), thisplanetradius);
    return generateCenter(tmp);
}

vector<string> parseBigUnit(string input) {
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
            *ptr++;
        }
        ans.push_back(string(oldptr));
        oldptr = ptr;
    }
    free(mystr);
    return ans;
}

void WriteUnit(string tag,
        string name,
        string filename,
        Vector r,
        Vector s,
        Vector center,
        string nebfile,
        string destination,
        bool faction) {
    Tab();
    fprintf(fp, "<%s name=\"%s\" file=\"%s\" ", tag.c_str(), name.c_str(), filename.c_str());
    if (nebfile.length() > 0) {
        fprintf(fp, "nebfile=\"%s\" ", nebfile.c_str());
    }
    fprintf(fp, "ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", r.i, r.j, r.k, s.i, s.j, s.k);
    fprintf(fp, "x=\"%f\" y=\"%f\" z=\"%f\" ", center.i, center.j, center.k);
    float loy = LengthOfYear(r, s);
    if (loy) {
        fprintf(fp, "year= \"%f\" ", loy);
    }
    if (destination.length()) {
        fprintf(fp, "destination=\"%s\" ", destination.c_str());
    } else if (faction) {
        fprintf(fp, "faction=\"%s\" ", StarSystemGent::faction.c_str());
    }
    fprintf(fp, " />\n");
}

string getJumpTo(string s) {
    char tmp[1000] = "";
    if (1 == sscanf(s.c_str(), "JumpTo%s", tmp)) {
        tmp[0] = tolower(tmp[0]);
    } else {
        return s;
    }
    return string(tmp);
}

string starin(string input) {
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
    return string("");
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
    return string("");
}

string AnalyzeType(string &input, string &nebfile, float &radius) {
    if (input.empty()) {
        return "";
    }
    char ptr = *input.begin();
    string ip;
    if (0 == sscanf(GetNebFile(input).c_str(), "%f", &radius)) {
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
        case 'U':
        default:
            retval = "Unit";
    }
    return retval;
}

void MakeSmallUnit() {
    Vector r, S;

    string nam;
    string s = string("");
    while (s.length() == 0) {
        nam = getRandName(units[1]);
        if (nam.length() == 0) {
            return;
        }
        string tmp;
        if ((tmp = starin(nam)).length() > 0) {
            nam = (tmp);
            s = getRandName(entities[JUMP]);
        } else {
            break;
        }
    }
    string nebfile("");
    float radius;
    string type = AnalyzeType(nam, nebfile, radius);
    Vector center = generateAndUpdateRS(r, S, radius);
    WriteUnit(type, nam, nam, r, S, center, nebfile, s, true);
}

void MakePlanet(float radius, int entitytype, bool forceRS = false, Vector R = Vector(0, 0, 0), Vector S = Vector(0,
        0,
        0),
        Vector center = Vector(0, 0, 0));

void MakeBigUnit(string name = string(""), float orbitalradius = 0) {
    vector<string> fullname;
    if (name.length() == 0) {
        string s = getRandName(units[0]);
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
    Vector r, s;

    float size;
    string tmp;
    string nebfile("");
    bool first = true;
    Vector center;
    for (unsigned int i = 0; i < fullname.size(); i++) {
        if (1 == sscanf(fullname[i].c_str(), "jump%f", &size)) {
            if (first) {
                center = generateAndUpdateRS(r, s, size);
            }
            MakePlanet(size, JUMP, true, r, s, center);
        } else if (1 == sscanf(fullname[i].c_str(), "planet%f", &size)) {
            if (first) {
                center = generateAndUpdateRS(r, s, size);
            }
            MakePlanet(size, PLANET, true, r, s, center);
        } else if (1 == sscanf(fullname[i].c_str(), "moon%f", &size)) {
            if (first) {
                center = generateAndUpdateRS(r, s, size);
            }
            MakePlanet(size, MOON, true, r, s, center);
        } else if (1 == sscanf(fullname[i].c_str(), "gas%f", &size)) {
            if (first) {
                center = generateAndUpdateRS(r, s, size);
            }
            MakePlanet(size, GAS, true, r, s, center);
        } else if ((tmp = starin(fullname[i])).length() > 0) {
            string S = getRandName(entities[JUMP]);
            if (S.length() > 0) {
                string type = AnalyzeType(tmp, nebfile, size);
                if (first) {
                    center = generateAndUpdateRS(r, s, size);
                }
                WriteUnit(type, S, tmp, r, s, center, nebfile, getJumpTo(S), false);
            }
        } else {
            string type = AnalyzeType(fullname[i], nebfile, size);
            if (first) {
                center = generateAndUpdateRS(r, s, size);
            }
            WriteUnit(type, fullname[i], fullname[i], r, s, center, nebfile, string(""), i != 0);
        }
        first = false;
    }
}

void MakeMoons(float radius, int entitytype, int callingentitytype, bool forceone = false);

void MakePlanet(float radius, int entitytype, bool forceRS, Vector R, Vector S, Vector center) {
    string s = getRandName(entities[entitytype]);
    if (s.length() == 0) {
        return;
    }
    Vector r, SS;
    if (forceRS) {
        r = R;
        SS = S;
        Updateradii(mmax(r.Mag(), SS.Mag()), radius);
    } else {
        center = generateAndUpdateRS(r, SS, radius);
    }
    Tab();
    fprintf(fp, "<Planet name=\"%s\" file=\"%s\" ", entitytype == JUMP ? s.c_str() : getRandName(
            names).c_str(), entitytype == JUMP ? "jump.png" : s.c_str());
    fprintf(fp, "ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", r.i, r.j, r.k, SS.i, SS.j, SS.k);
    fprintf(fp, "radius=\"%f\" ", radius);
    fprintf(fp, "x=\"%f\" y=\"%f\" z=\"%f\" ", center.i, center.j, center.k);
    float loy = LengthOfYear(r, SS);
    if (loy) {
        fprintf(fp, "year= \"%f\" ", loy);
        loy *= grand();
        if (loy) {
            fprintf(fp, "day=\"%f\" ", loy);
        }
    }
    if (entitytype == JUMP) {
        fprintf(fp, "alpha=\"ONE ONE\" destination=\"%s\" ", getJumpTo(s).c_str());
    }
    fprintf(fp, " >\n");

    radii.push_back(entitytype != GAS ? radius : 1.4 * radius);
    if (entitytype == GAS && grand() < .3 && radius > 8000 && radius < 10500) {
        MakeBigUnit("planetary-ring", 0);
    }
    if ((entitytype != JUMP && entitytype != MOON) || grand() < moonofmoonprob) {
        int numu = numun[1] / (nument[PLANET] + nument[GAS]) + grand() * (nument[PLANET] + nument[GAS])
                > (numun[1] % (nument[PLANET] + nument[GAS])) ? 1 : 0;
        if (entitytype == MOON) {
            if (numu > 1) {
                numu = 1;
            }
        }
        for (unsigned int i = 0; i < numu; i++) {
            MakeSmallUnit();
        }
        MakeMoons((entitytype != JUMP && entitytype != MOON) ? .4 * radius : .8 * radius, MOON, entitytype,
                entitytype == JUMP || entitytype == MOON);
    }
    radii.pop_back();
    Tab();
    fprintf(fp, "</Planet>\n");
    ///writes out some pretty planet tags
}

void MakeMoons(float radius, int entitytype, int callingentitytype, bool forceone) {
    unsigned int nummoon = nument[entitytype] / nument[callingentitytype]
            + (grand() * nument[callingentitytype]
                    < (nument[entitytype] % nument[callingentitytype])) ? 1 : 0;
    if (forceone) {
        nummoon = 1;
    }
    for (unsigned int i = 0; i < nummoon; i++) {
        MakePlanet((.5 + .5 * grand()) * radius, entitytype);
    }
}

void beginStar(float radius, unsigned int which) {
    Vector r, s;
    Vector center = generateAndUpdateRS(r, s, radius);

    char b[2] = "A";
    b[0] += which;
    Tab();
    fprintf(fp, "<Planet name=\"%s\" file=\"%s\" ", (systemname + b).c_str(), entities[0][which].c_str());
    fprintf(fp, "ri=\"%f\" rj=\"%f\" rk=\"%f\" si=\"%f\" sj=\"%f\" sk=\"%f\" ", r.i, r.j, r.k, s.i, s.j, s.k);
    fprintf(fp, "radius=\"%f\" x=\"0\" y=\"0\" z=\"0\" ", radius);
    float loy = LengthOfYear(r, s);
    if (loy) {
        fprintf(fp, "year= \"%f\" ", loy);
        loy *= grand();
        if (loy) {
            fprintf(fp, "day=\"%f\" ", loy);
        }
    }
    fprintf(fp,
            " Red=\"%f\" Green=\"%f\" Blue=\"%f\" ReflectNoLight=\"true\" light=\"%d\">\n",
            lights[which].r,
            lights[which].g,
            lights[which].b,
            which);

    radii.push_back(1.5 * radius);
    MakeMoons(.3 * radius, PLANET, STAR);
    MakeMoons(.6 * radius, GAS, STAR);

    int numu = numun[0] / nument[0] + (grand() < float(numun[0] % nument[0]) / nument[0]);
    if (which == nument[0] - 1) {
        numu = numun[0];
    }
    numun[0] -= numu;
    for (unsigned int i = 0; i < numu; i++) {
        MakeBigUnit();
    }
    MakeMoons(100 + grand() * 300, JUMP, STAR);
    if (which == nument[STAR] - 1) {
        while (!entities[JUMP].empty()) {
            MakeMoons(100 + grand() * 300, JUMP, STAR);
        }
    }
}

void endStar() {
    radii.pop_back();
    Tab();
    fprintf(fp, "</Planet>\n");
}

void CreateStar(float radius, unsigned int which) {
    beginStar(radius, which);
    endStar();
}

void CreateFirstStar(float radius, unsigned int which) {
    beginStar(radius, which);
    for (unsigned int i = which + 1; i < nument[0]; i++) {
        if (grand() > .5) {
            CreateFirstStar(starradius[i], i);
            break;
        } else {
            CreateStar(starradius[i], i);
        }
    }
    endStar();
}

void CreatePrimaries(float starradius) {
    unsigned int numprimaryunits = rnd(0, 1 + numun[0]);
    numun[0] -= numprimaryunits;
    for (unsigned int i = 0; i < nument[0]; i++) {
        CreateLight(i);
    }
    for (unsigned int i = 0; i < numprimaryunits; i++) {
        MakeBigUnit();
    }
    CreateFirstStar(starradius, 0);
}

void CreateStarSystem() {
    assert(!starradius.empty());
    assert(starradius[0]);
    fprintf(fp, "<system name=\"%s\" background=\"%s\" nearstars=\"%d\" stars=\"%d\" starspread=\"150\">\n",
            systemname.c_str(), getRandName(background).c_str(), 500, 1000, 150);
    CreatePrimaries(starradius[0]);
    fprintf(fp, "</system>\n");
}

void readentity(vector<string> &entity, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return;
    }
    char input_buffer[BUFFER_SIZE];
    while (1 == fscanf(fp, SCANF_FORMAT_STRING, input_buffer)) {
        entity.push_back(input_buffer);
    }
    fclose(fp);
}

void readnames(vector<string> &entity, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return;
    }
    char input_buffer[BUFFER_SIZE];
    while (!feof(fp)) {
        fgets(input_buffer, BUFFER_SIZE - 1, fp);
        if (input_buffer[0] == '\0' || input_buffer[0] == '\n' || input_buffer[0] == '\r') {
            continue;
        }
        for (unsigned int i = 0; input_buffer[i] != '\0'; i++) {
            if (input_buffer[i] == '\r') {
                input_buffer[i] = '\0';
            }
            if (input_buffer[i] == '\n') {
                input_buffer[i] = '\0';
                break;
            }
        }
        entity.push_back(input_buffer);
    }
    fclose(fp);
}

const char *noslash(const char *in) {
    const char *tmp = in;
    while (*tmp != '\0' && *tmp != '/') {
        tmp++;
    }
    if (*tmp != '\0') {
        tmp++;
    } else {
        return tmp;
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

string getStarSystemFileName(string input) {
    return input + string(".system");
}

string getStarSystemName(string in) {
    char *tmp = strdup(noslash(in.c_str()));
    tmp[0] = toupper(tmp[0]);
    string tmP(tmp);
    free(tmp);
    return tmP;
}

string getStarSystemSector(string in) {
    char *tmp = strdup(in.c_str());
    char *freer = tmp;
    while (*tmp++) {
        if (*tmp == '/') {
            break;
        }
    }
    if (*tmp == '/') {
        tmp++;
    }
    string sectorname(tmp);
    free(freer);
    return sectorname;
}

void generateStarSystem(string datapath,
        int seed,
        string sector,
        string system,
        string outputfile,
        float sunradius,
        int numstars,
        int numgasgiants,
        int numrockyplanets,
        int nummoons,
        bool nebulae,
        bool asteroids,
        int numnaturalphenomena,
        int numstarbases,
        string factions,
        string namelist,
        const vector<string> &jumplocations) {
    if (seed) {
        seedrand(seed);
    } else {
        seedrand(time(NULL));
    }
    nument[0] = numstars;
    nument[1] = numgasgiants;
    nument[2] = numrockyplanets;
    nument[3] = nummoons;
    numun[0] = numnaturalphenomena;
    numun[1] = numstarbases;
    starradius.push_back(sunradius);
    readColorGrads(gradtex, (datapath + "stars.txt").c_str());
    readentity(entities[1], (datapath + "planets.txt").c_str());
    readentity(entities[2], (datapath + "gas_giants.txt").c_str());
    readentity(entities[3], (datapath + "moons.txt").c_str());
    readentity(units[1], (datapath + "smallunits.txt").c_str());
    readentity(background, (datapath + "background.txt").c_str());
    if (nebulae) {
        readentity(units[0], (datapath + "nebulae.txt").c_str());
    }
    if (asteroids) {
        readentity(units[0], (datapath + "asteroids.txt").c_str());
    }
    readnames(names, (datapath + namelist).c_str());

    fp = fopen(outputfile.c_str(), "w");
    for (unsigned int i = 0; i < jumplocations.size(); i++) {
        entities[JUMP].push_back(jumplocations[i]);
    }
    nument[JUMP] = entities[JUMP].size();
    ::faction = factions;
    CreateStarSystem();
    fclose(fp);
}

#define CONSOLE_APP
#ifdef CONSOLE_APP

int main(int argc, char **argv) {
    if (argc < 9) {
        fprintf(
                stderr,
                "Usage: starsysgen <seed> <sector>/<system> <sunradius> <numstars> <numgasgiants> <numrockyplanets> <nummoons> [N][A]<numnaturalphenomena> <numstarbases> <faction> <namelist> [OtherSystemJumpNodes]...\n");
        return 1;
    }
    int seed;
    if (1 != sscanf(argv[1], "%d", &seed)) {
        return 1;
    }
    string sectorname(getStarSystemSector(argv[2]));
    string filen(getStarSystemFileName(argv[2]));
    string systemname = string(getStarSystemName(argv[2]));
    int numbigthings;
    bool nebula = true;
    bool asteroid = true;
    vector<string> jumps;
    for (unsigned int i = 12; i < argc; i++) {
        jumps.push_back(string(argv[i]));
    }
    if (1 == sscanf(argv[8], "AN%d", &numbigthings) || 1 == sscanf(argv[8], "NA%d", &numun[0])) {
        nebula = asteroid = true;
    } else if (1 == sscanf(argv[8], "A%d", &numbigthings)) {
        asteroid = true;
    } else if (1 == sscanf(argv[8], "N%d", &numbigthings)) {
        nebula = true;
    } else if (1 == sscanf(argv[8], "%d", &numbigthings)) {
        nebula = asteroid = true;
    } else {
        return -1;
    }
    generateStarSystem("./",
            seed,
            sectorname,
            systemname,
            filen,
            strtol(argv[3], NULL, 10),
            strtol(argv[4], NULL, 10),
            strtol(argv[5], NULL, 10),
            strtol(argv[6], NULL, 10),
            strtol(argv[7], NULL, 10),
            nebula,
            asteroid,
            numbigthings,
            strtol(argv[9], NULL, 10),
            argv[10],
            argv[11],
            jumps);

    return 0;
}

#endif

