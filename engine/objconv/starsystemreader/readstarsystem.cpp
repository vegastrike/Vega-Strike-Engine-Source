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

#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <set>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <algorithm>
#ifndef _WIN32
#include <sys/stat.h>
#endif
#include <sys/stat.h>

unsigned long genrand_int32(void);
void init_genrand(unsigned long s);

const unsigned int GENRAND_MAX = (unsigned int) (0 - 1);

using std::vector;
using std::map;
using std::string;
using std::pair;
using std::set;
map<string, int> numfactions;
map<string, map<string, int> > numsecfactions;

set<string> allnames;

std::string readfiledata(const char *name) {
    FILE *fp = fopen(name, "r");
    if (!fp) {
        printf("unable to open %s\n", name);
        return "";
    }
    int len;
    struct stat st;
    if (fstat(fileno(fp), &st) == 0) {
        len = st.st_size;
    } else {
        //fstat B0rken.
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }
    char *line = (char *) malloc(len + 1);
    line[len] = 0;
    fread(line, sizeof(char), len, fp);
    return std::string(line, line + len);
}

double sqr(double x) {
    return x * x;
}

std::string itostr(int i) {
    char test[256];
    sprintf(test, "%d", i);
    return test;
}

std::string ftostr(double i) {
    char test[256];
    sprintf(test, "%lf", i);
    return test;
}

char *milky_way = "../../../data/universe/milky_way.xml";
char *path_to_universe = "../../../data4.x/universe";
class vec3 {
public:
    double x, y, z;

    vec3() {
    }

    vec3(double x, double y, double z) : x(x), y(y), z(z) {
    }

    vec3 max(const vec3 &in) const {
        return vec3(in.x > x ? in.x : x,
                in.y > y ? in.y : y,
                in.z > z ? in.z : z);
    }

    vec3 min(const vec3 &in) const {
        return vec3(in.x < x ? in.x : x,
                in.y < y ? in.y : y,
                in.z < z ? in.z : z);
    }

    vec3 operator+(const vec3 &oth) const {
        return vec3(x + oth.x, y + oth.y, z + oth.z);
    }

    vec3 operator-(const vec3 &oth) const {
        return vec3(x - oth.x, y - oth.y, z - oth.z);
    }

    vec3 &operator+=(const vec3 &oth) {
        x += oth.x;
        y += oth.y;
        z += oth.z;
        return *this;
    }

    vec3 &operator-=(const vec3 &oth) {
        x -= oth.x;
        y -= oth.y;
        z -= oth.z;
        return *this;
    }

    bool operator<(const vec3 &oth) const {
        return x < oth.x && y < oth.y && z < oth.z;
    }

    bool operator>(const vec3 &oth) const {
        return x > oth.x && y > oth.y && z > oth.z;
    }

    bool operator<=(const vec3 &oth) const {
        return x <= oth.x && y <= oth.y && z <= oth.z;
    }

    bool operator>=(const vec3 &oth) const {
        return x >= oth.x && y >= oth.y && z >= oth.z;
    }

    bool operator==(const vec3 &oth) const {
        return x == oth.x && y == oth.y && z == oth.z;
    }

    bool operator!=(const vec3 &oth) const {
        return x != oth.x || y != oth.y || z != oth.z;
    }
};

class System : public map<string, string> {
public:
    static System *findSystem(vector<System> &s, string outgoing) {
        int slash = outgoing.find("/");
        if (slash == std::string::npos) {
            std::vector<System *> habitableSystems;
            for (unsigned int j = 0; j < s.size(); ++j) {
                if (s[j].sector == outgoing) {
                    if (s[j].habitable) {
                        habitableSystems.push_back(&s[j]);
                    }
                }
            }
            if (habitableSystems.empty()) {
                fprintf(stderr, "Fatal Error: No habitable systems in the %s sector!!!\n", outgoing.c_str());
                return NULL;
            } else {
                return habitableSystems[(int) ((((float) genrand_int32()) / GENRAND_MAX) * habitableSystems.size())];
            }
        } else {
            string sys = outgoing;
            sys = outgoing.substr(slash);
            System *bestChoice = NULL;
            if (sys.length()) {
                if (sys[0] == '/') {
                    sys = sys.substr(1);
                }
            }
            outgoing = outgoing.substr(0, slash);
            for (unsigned int j = 0; j < s.size(); ++j) {
                if (s[j].name == sys) {
                    if (s[j].sector != outgoing) {
                        fprintf(stderr, "Error: System %s not in %s but in %s\n", sys.c_str(),
                                outgoing.c_str(), s[j].sector.c_str());
                        if (bestChoice == NULL) {
                            bestChoice = &s[j];
                        } else if (bestChoice != &s[j]) {
                            fprintf(stderr,
                                    " -- Duplicate ambiguous system name... Some systems may have invalid jump points!!!!\n");
                        }
                    } else {
                        return &s[j];
                    }
                }
            }
            return bestChoice;
        }
    }

    System() {
    }

    bool habitable;
    bool interesting;
    string sector;
    string name;
    string alphaonlyname;
    float distance;
    float ascension;
    float declination;
    map<string, float> takenover;
    vector<string> jumps;
    vec3 xyz;
    float luminosity;   //in sun
    int
            type;  //0 = 1O blue (-2000)  B = 20 blue(2000-4000) // A = 30 Bluish-white (4000-8000) F = 40 White G = 50 (13500-15000) yellow  (15000-43000) K = 60 Orange (36500-80000) M = Red 70 (giant 80,000 dwarf 8,000-13500 )
    int size;  //0 dwwarf 1 normal 2 giant
    bool operator<(const System &a) const {
        if (sector == a.sector) {
            return name < a.name;
        }
        return sector < a.sector;
    }

    string fullName() {
        return sector + "/" + name;
    }

    void computeProperties(bool interestingname, const vector<string> homeworlds) {
        //make sure to propogate changes to wcSystemReader.py
        this->interesting = interestingname;
        double rad = 16000;
        double lifeprob = .25;
        if (type < 30) {
            rad = type * 4000. / 30;
            lifeprob = .01;
        } else if (type < 40) {
            rad = 6000;
            lifeprob = .02;
            if (size == 0) {
                rad = 4200;
            } else if (size == 2) {
                rad = 7500;
            }
        } else if (type < 50) {
            lifeprob = .05;
            rad = 14250;
            if (size == 0) {
                rad = 13600;
                lifeprob = .08;
            } else if (size == 2) {
                rad = 14750;
            }
        } else if (type < 60) {
            lifeprob = .125;
            rad = 25000;
            if (size == 0) {
                rad = 16600;
                lifeprob = .25;
                if (xyz.x * xyz.x + xyz.y * xyz.y + xyz.z * xyz.z > 500 * 500) {
                    lifeprob = 1;
                }
            } else if (size == 2) {
                rad = 36500;
                lifeprob = .0625;
            }
        } else if (type < 70) {
            rad = 50000;
            lifeprob = .02;
            if (size == 0) {
                lifeprob = .125;
                rad = 37000;
            } else if (size == 2) {
                rad = 75000;
            }
        } else if (type < 80) {
            rad = 85000;
            lifeprob = .005;
            if (size == 0) {
                rad = 10000;
                lifeprob = .125;
            } else if (size == 2) {
                rad = 150000;
            }
        }
        if (interesting) {
            lifeprob *= 1;           //the answer to the question...
        } else {
            lifeprob /= 1;
        }
        (*this)["sun_radius"] = ftostr(rad);
        (*this)["data"] = itostr(genrand_int32());
        (*this)["faction"] = "unknown";
        lifeprob *= 1;
        habitable = false;
        bool force = false;
        for (int i = 0; i < homeworlds.size(); i++) {
            if (homeworlds[i] == name) {
                //All homeworlds have life!
                //(Or else I would not exist or be able to write this program)
                lifeprob = 1;
                force = true;
                break;
            }
        }
        if (!force) {
            if (sector.find("RBL-") != string::npos) {
                lifeprob = .01;
            }
        }
        if (genrand_int32() < GENRAND_MAX * lifeprob) {
            habitable = true;
            //living
            if (genrand_int32() < GENRAND_MAX * .995) {
                (*this)["num_gas_giants"] = itostr(genrand_int32() % 3);
            } else {
                (*this)["num_gas_giants"] = itostr(genrand_int32() % 6);
            }
            if (genrand_int32() < GENRAND_MAX * .995) {
                (*this)["num_planets"] = itostr(1 + genrand_int32() % 3);
            } else {
                (*this)["num_planets"] = itostr(1 + genrand_int32() % 9);
            }
            if (genrand_int32() < GENRAND_MAX * .995) {
                (*this)["num_moons"] = itostr(genrand_int32() % 3);
            } else {
                (*this)["num_moons"] = itostr(genrand_int32() % 18);
            }
            if (genrand_int32() < GENRAND_MAX * .025) {
                (*this)["num_natural_phenomena"] = "2";
            } else if (genrand_int32() < GENRAND_MAX * .1) {
                (*this)["num_natural_phenomena"] = "1";
            } else {
                (*this)["num_natural_phenomena"] = "0";
            }
        } else {
//dead(
            if (genrand_int32() < GENRAND_MAX * .1) {
                (*this)["num_natural_phenomena"] = "1";
            } else {
                (*this)["num_natural_phenomena"] = "0";
            }
            (*this)["num_moons"] = "0";
            if (genrand_int32() < GENRAND_MAX * .85) {
                (*this)["planetlist"] = "planets.desolate.txt";
                (*this)["num_planets"] = itostr(genrand_int32() % 2 + 1);
                (*this)["num_gas_giants"] = "0";
            } else {
                (*this)["num_planets"] = itostr(0);
                (*this)["num_gas_giants"] = itostr(genrand_int32() % 2 + 1);
                if (genrand_int32() > GENRAND_MAX * .99) {
                    (*this)["num_moons"] = itostr(genrand_int32() % 5);
                }
            }
        }
    }

    void computeXYZ() {
        xyz.z = distance * sin(declination);
        float xy = distance * cos(declination);
        xyz.y = xy * cos(ascension);
        xyz.x = xy * sin(ascension);
        char str[16384];
        sprintf(str, "%lf %lf %lf", xyz.x, xyz.y, xyz.z);
        (*this)["xyz"] = str;
    }
};

vector<std::string> readCSV(std::string s) {
    vector<std::string> v;
    int loc;
    int sub1 = s.find("\r");
    s = s.substr(0, sub1);
    int sub2 = s.find("\n");
    s = s.substr(0, sub2);
    do {
        loc = s.find(",");
        string t = s.substr(0, loc);
        v.push_back(t);
        if (loc != string::npos) {
            s = s.substr(loc + 1);
        }
    } while (loc != string::npos);
    return v;
}

void computeminmax(vector<System> sys, vec3 &min, vec3 &max) {
    min = vec3(DBL_MAX, DBL_MAX, DBL_MAX);
    max = vec3(-DBL_MAX, -DBL_MAX, -DBL_MAX);
    const vec3 fudgeFactor(0.001, 0.001, 0.001);
    for (unsigned int i = 0; i < sys.size(); ++i) {
        min = min.min(sys[i].xyz - fudgeFactor);
        max = max.max(sys[i].xyz + fudgeFactor);
    }
}

std::string strtoupper(std::string s) {
    for (string::iterator i = s.begin(); i != s.end(); ++i) {
        *i = toupper(*i);
    }
    return s;
}

std::string unpretty(std::string s) {
    for (string::iterator i = s.begin(); i != s.end(); ++i) {
        if (isspace(*i)) {
            *i = '_';
        }
        if (*i == '\"' || *i == '\'') {
            *i = '_';
        }
    }
    return s;
}

std::string alphaOnly(std::string s) {
    std::string out = s;
    int count = 0;
    for (string::iterator i = s.begin(); i != s.end(); ++i) {
        if ((*i > 'a' && *i < 'z') || (*i > 'A' && *i < 'Z')) {
            out[count] = toupper(*i);
            ++count;
        }
    }
    return out.substr(0, count);
}

class AlphaOnlySort {
public:
    bool operator()(const System &a, const System &b) {
        return a.alphaonlyname < b.alphaonlyname;
    }
};

std::vector<string> readMilkyWayNames() {
    string s = readfiledata(milky_way);
    vector<string> retval;
    unsigned int where = string::npos;
    do {
        where = s.find("system name=\"");
        if (where != string::npos) {
            s = s.substr(where);
            where = s.find("\"");
            s = s.substr(where + 1);             //gotta be there cus we found it earlier
            unsigned int quote = s.find("\"");
            if (quote != string::npos) {
                string newname = s.substr(0, quote);
                if (newname.length() > 0 && newname != "max" && newname != "min" && newname != "maxlimit" && newname
                        != "minlimit" && newname != "hardwicke" && newname != "reid" && newname != "lesnick" && newname
                        != "midgard"
                        && newname.find("blockade") == string::npos && newname != "wolf359" && newname.find("wolf")
                        == string::npos && newname != "sol" && newname.find("polaris") == string::npos) {
                    newname[0] = toupper(newname[0]);
                }
                retval.push_back(newname);
            }
        }
    } while (where != string::npos);
    return retval;
}

class FactionInfo {
    unsigned maxsystems;
    float takeoverprob;  //The chace that an enemy system will be taken over.
    float takeneutralprob;  //The chance that a neutral system will be taken over.
    string name;
    System *homeworld;

    unsigned turn;
    unsigned numsystems;
    unsigned startingyear;
    std::vector<System *> borderSystems;
    std::set<System *> developingSystems;
    std::set<System *> systems; //for quick access.

public:
    System *getHomeworld() {
        return homeworld;
    }

    const System *getHomeworld() const {
        return homeworld;
    }

    void developBorderSystems() {
        //reserve memory to increse speed.
        for (int i = borderSystems.size() - 1; i >= 0; i--) {
            developingSystems.insert(borderSystems[i]);
        }
//borderSystems.erase(borderSystems.begin()+i);
        //DANNY RUINEDborderSystems.clear();
    }

    void addNewSystems(const vector<System *> &newSystems) {
        developBorderSystems();
//DANNY RUINED		borderSystems=newSystems;
        for (int i = 0; i < newSystems.size(); i++) {
            borderSystems.push_back(newSystems[i]);
            systems.insert(newSystems[i]);
        }
        numsystems += newSystems.size();
    }

/*
 *  FactionInfo(const FactionInfo &other)
 *               : turn(other.turn), numsystems(other.numsystems), name(other.name), takeoverprob(other.takeoverprob),
 *                 takeneutralprob(other.takeneutralprob), maxsystems(other.maxsystems),
 *                 homeworld(other.homeworld), systems(other.systems), developingSystems(other.developingSystems),
 *                 borderSystems(other.borderSystems) {
 *  }
 */
    FactionInfo(vector<string> stuff, vector<System> &s) :
            turn(0),
            numsystems(1),
            name(stuff[0]),
            takeoverprob(atof(stuff[1].c_str())),
            takeneutralprob(1 - takeoverprob),
            maxsystems(atoi(stuff[2].c_str())),
            startingyear(atoi(stuff[3].c_str())),
            homeworld(System::findSystem(s, stuff[4])) {
        if (!homeworld) {
            fprintf(stderr, "Fatal error: homeworld \"%s\" not found!!!\n", stuff[4].c_str());
        } else if (homeworld->jumps.empty()) {
            fprintf(
                    stderr,
                    "Fatal error: homeworld \"%s\" has no jump points!!!\nThis means that the %s faction will wait forever for a jump point\nto come into existance.  The application will probably get stuck in an endless loop somewhere!",
                    stuff[4].c_str(),
                    name.c_str());
        } else {
            (*homeworld)["faction"] = stuff[0];
        }
        std::vector<System *> newsys;
        newsys.push_back(homeworld);
        addNewSystems(newsys);
    }

    FactionInfo(string nam, float prob, int max, System *homeworld) :
            turn(0),
            numsystems(1),
            name(nam),
            takeoverprob(prob),
            takeneutralprob(1 - takeoverprob),
            maxsystems(max),
            homeworld(homeworld) {
    }

/*
 *  FactionInfo &operator= (const FactionInfo &other) {
 *       turn=(other.turn);
 *       numsystems=(other.numsystems);
 *       name=(other.name);
 *       takeoverprob=(other.takeoverprob);
 *       takeneutralprob=(other.takeneutralprob);
 *       maxsystems=(other.maxsystems);
 *       homeworld=(other.homeworld);
 *       systems=(other.systems);
 *       developingSystems=(other.developingSystems);
 *       borderSystems=(other.borderSystems);
 *       return *this;
 *  }
 */
    void simulateTurn(unsigned int totalturn, bool allowTakeover, vector<System> &s) {
        allowTakeover = false;
        ++turn;
        if (turn < startingyear) {
            return;
        }
        vector<System *> systemsToAdd;
        if (borderSystems.empty()) {
            numsystems = maxsystems;
        }
        for (int i = 0; i < borderSystems.size(); i++) {
            bool foundvalidplacetogo = false;
            std::vector<std::string>::const_iterator end = borderSystems[i]->jumps.end();
            string faction = (*borderSystems[i])["faction"];
            for (std::vector<std::string>::const_iterator iter = borderSystems[i]->jumps.begin(); iter != end; ++iter) {
                System *jump = System::findSystem(s, *iter);
                if (jump != NULL && systems.find(jump) == systems.end()) {
                    //not in our territory! and it is valid.
                    if (((*jump)["faction"] == "unknown"
                            && (jump->takenover[faction] += takeneutralprob) > 1)
                            || ((((float) genrand_int32()) / GENRAND_MAX) < takeoverprob && allowTakeover)) {
                        (*jump)["faction"] = name;
                        systemsToAdd.push_back(jump);
//jump->takenover=0;
                    } else if ((*jump)["faction"] == "unknown") {
                        foundvalidplacetogo = true;
                        //fprintf(stderr,"jump probability %f",jump->takenover);
                    }
                }
            }
            if (!foundvalidplacetogo) {
                borderSystems.erase(borderSystems.begin() + i);
                i--;
            }
        }
        addNewSystems(systemsToAdd);
    }

    bool active() {
        return numsystems < maxsystems;
    }
};

std::vector<FactionInfo> readFactions(vector<System> &s) {
    std::vector<FactionInfo> ret;
    std::string file = readfiledata("factions.csv");
    while (true) {
        vector<string> temp = readCSV(file);
        int r = file.find("\r");
        int n = file.find("\n");
        if (r == std::string::npos && n == std::string::npos) {
            break;
        }
        file = file.substr(r > n ? r + 1 : n + 1);
        if (temp.size()) {
            if (temp.size() < 4) {
                if (temp.size() < 3) {
                    if (temp.size() < 2) {
                        temp.push_back(".1");
                    }
                    temp.push_back("10");
                }
                temp.push_back("You are getting this error due to lack of required columns.");
            }
            ret.push_back(FactionInfo(temp, s));
        }
    }
    return ret;
}

void simulateFactionTurns(vector<System> &s) {
    std::vector<FactionInfo> factions = readFactions(s);
    unsigned turn;
    for (turn = 0;; turn++) {
        int num_inactive = 0;
        for (unsigned i = 0; i < factions.size(); i++) {
            if (factions[i].active()) {
                factions[i].simulateTurn(turn, false, s);
            } else {
                num_inactive++;
            }
        }
        if (num_inactive >= factions.size()) {
            break;
        }
    }
    for (unsigned i = 0; i < factions.size(); i++) {
        factions[i].simulateTurn(turn, true, s);
    }
}

class SectorInfo {
private:
    std::string nam;
    vec3 min;
    vec3 max;
public:
    SectorInfo(std::string nam, vec3 min, vec3 max) :
            nam(nam), min(min), max(max) {
    }

    vec3 &minimum() {
        return min;
    }

    vec3 &maximum() {
        return max;
    }

    std::string &name() {
        return nam;
    }
};

void stripwhitespace(std::string &str) {
    unsigned int i;
    //Strip whitespace from a string.
    for (i = str.size() - 1; i >= 0; i--) {
        if (isspace(str[i])) {
            str.erase(i);
        } else {
            break;
        }
    }
    for (i = 0; i < str.size(); i++) {
        if (isspace(str[i])) {
            str.erase(i);
        } else {
            break;
        }
    }
}

void computeCoord(vec3 &lo, vec3 &hi, int wid, int hei, int x, int y) {
    vec3 min(lo);
    vec3 max(hi);
//lo.z=min.z; // Already equal!
//hi.z=max.z;
    lo.x = (((max.x - min.x) / wid) * x) + min.x;
    lo.y = (((max.y - min.y) / hei) * y) + min.y;
    hi.x = (((max.x - min.x) / wid) * (x + 1)) + min.x;
    hi.y = (((max.y - min.y) / hei) * (y + 1)) + min.y;
}

std::vector<SectorInfo> readSectors(vec3 min, vec3 max) {
    //Uncomment if you have problems.
//printf("\n\n--MINIMUM-- <%f,%f,%f>\n--MAXIMUM-- <%f,%f,%f>\n\n",min.x,min.y,min.z,max.x,max.y,max.z);
    std::vector<SectorInfo> ret;
    std::string file = readfiledata("sectors.txt");
    std::vector<std::vector<std::string> > vec;
    int width = 0;
    while (true) {
        vec.push_back(readCSV(file));
        int r = file.find("\r");
        int n = file.find("\n");
        if (r == std::string::npos && n == std::string::npos) {
            break;
        }
        file = file.substr(r > n ? r + 1 : n + 1);
        width = width > vec.back().size() ? width : vec.back().size();
    }
    for (int i = 0; i < vec.size(); ++i) {
        for (int j = 0; j < vec[i].size(); ++j) {
            stripwhitespace(vec[i][j]);
            vec3 lo(min);
            vec3 hi(max);
            computeCoord(lo, hi, width, vec.size(), j, i);
            ret.push_back(SectorInfo(vec[i][j], lo, hi));
        }
    }
    return ret;
}

std::string getSector(const System &s, vec3 min, vec3 max) {
    static std::vector<SectorInfo> sectors(readSectors(min, max));
    for (int i = 0; i < sectors.size(); i++) {
        if (s.xyz >= sectors[i].minimum() && s.xyz <= sectors[i].maximum()) {
            return sectors[i].name();
        }
    }
    {
        char error[65535];
        sprintf(error, "<error:_system_<%f,%f,%f>_(%s)_not_found>", s.xyz.x, s.xyz.y, s.xyz.z, s.name.c_str());
        fputs(error, stderr);
        fputc('\n', stderr);
        return error;
    }
}

string dodad(int which) {
    if (which == 0) {
        return "";
    }
    return "_" + itostr(which);
}

vector<string> shuffle(const vector<string> &inp) {
    vector<string> retval;
    for (unsigned int i = 0; i < inp.size(); ++i) {
        int index = rand() % (retval.size() + 1);
        if (std::find(retval.begin(), retval.end(), inp[i]) == retval.end()) {
            retval.insert(retval.begin() + index, inp[i]);
        }
    }
    return retval;
}

string recomputeName() {
    static int which = -1;
    static std::vector<string> genericnames = shuffle(readMilkyWayNames());
    string newname;
    do {
        which++;
        newname = genericnames[(which) % genericnames.size()] + dodad(which / genericnames.size());
    } while (allnames.find(newname) != allnames.end());
    allnames.insert(newname);
    return newname;
}

vector<System> readfile(const char *name) {
    vector<System> systems;
    std::string line = readfiledata(name);
    if (line.empty()) {
        return systems;
    }
    int len = line.size();
    std::vector<string> keys = readCSV(line);
    for (std::vector<string>::iterator i = keys.begin(); i != keys.end(); ++i) {
        *i = strtoupper(*i);
    }
    while (true) {
        int n = line.find("\n");
        if (n == std::string::npos) {
            break;
        }
        line = line.substr(n + 1);

        System in;
        std::vector<string> content = readCSV(line);
        for (int i = 0; i < content.size(); ++i) {
            if (keys[i].find("DISTANCE") != string::npos) {
                in.distance = atof(content[i].c_str());
            } else if (keys[i].find("ASCENSION") != string::npos) {
                in.ascension = atof(content[i].c_str());
            } else if (keys[i].find("DECLINATION") != string::npos) {
                in.declination = atof(content[i].c_str());
            } else if (keys[i].find("LUMIN") != string::npos) {
                in["luminosity"] = content[i];
                in.luminosity = atof(content[i].c_str());
            } else if (keys[i].find("SPECTRUM") != string::npos || keys[i].find("TYPE") != string::npos) {
                in.type = atoi(content[i].c_str());
            } else if (keys[i].find("SIZE") != string::npos) {
                in.size = atoi(content[i].c_str());
            } else {
                if (keys[i].find("NAME") != string::npos) {
                    in.name = unpretty(content[i]);
                } else {
                    printf("error key %s not found\n", keys[i].c_str());
                    in[keys[i]] = content[i];
                }
            }
/*
 *                       if (in.name=="") {
 *                               static int num=0;
 *                               num++;
 *                               in.name=string("Daniel")+itostr(num);
 *                       }
 */
            in.alphaonlyname = alphaOnly(in.name);
            in.sector = "nowhereland";
        }
        in.computeXYZ();
        systems.push_back(in);
    }
    std::sort(systems.begin(), systems.end(), AlphaOnlySort());
    vec3 min, max;
    computeminmax(systems, min, max);
    for (unsigned index = 0; index < systems.size(); ++index) {
        systems[index].sector = getSector(systems[index], min, max);
    }
    int hc = 0, bc = 0;
    int ic = 0, uc = 0;
    int ec = 0;
    if (1) {
        std::vector<string> homeworlds;
        {
            std::string factionsdata = readfiledata("factions.csv");
            while (true) {
                std::vector<std::string> vec = (readCSV(factionsdata));
                int n = factionsdata.find("\n");
                if (n == std::string::npos) {
                    break;
                }
                factionsdata = factionsdata.substr(n + 1);
                if (vec.size() >= 5) {
                    int slash = vec[4].find("/");
                    if (slash != std::string::npos) {
                        homeworlds.push_back(vec[4].substr(slash + 1));
                    }
                }
            }
        }
        for (unsigned int i = 0; i < systems.size(); ++i) {
            if (systems[i].name.length() == 0 || (allnames.find(systems[i].name) != allnames.end())) {
                systems[i].interesting = false;
                static int num = 0;
                num++;
                systems[i].name = string("Uncultivable_") + itostr(num);
                ec++;
            } else if (i < 4) {
                systems[i].interesting = true;
            } else if (i > systems.size() - 4) {
                systems[i].interesting = true;
            } else {
                //now we have a buffer zone of 4!
                //engage!
                if (systems[i].alphaonlyname == systems[i - 1].alphaonlyname
                        && systems[i].alphaonlyname == systems[i + 1].alphaonlyname
                        && systems[i].alphaonlyname == systems[i + 2].alphaonlyname
                        && systems[i].alphaonlyname == systems[i - 2].alphaonlyname) {
                    systems[i].interesting = false;
                    uc++;
                } else {
                    systems[i].interesting = true;
                }
            }
            allnames.insert(systems[i].name);
            if (systems[i].interesting) {
                ic++;
                //printf("%s interesting\n",systems[i].name.c_str());
            } else {
                //printf("%s no\n",systems[i].name.c_str());
            }
            systems[i].computeProperties(systems[i].interesting, homeworlds);
            if (systems[i].habitable) {
                hc += 1;
                if (systems[i].interesting == false) {
                    //systems[i].name = recomputeName();
                }
            } else {
                bc += 1;
            }
        }
    }
    fprintf(
            stderr,
            "\n\nParsing has been completed.\nHabitable count: %d\nBarren count: %d\n\nInteresting count: %d\nUninteresting count: %d\nEmpty count: %d\n\nGenerating data...\n\n",
            hc,
            bc,
            ic,
            uc,
            ec);
    return systems;
}

void writesystems(FILE *fp, std::vector<System> s) {
    std::sort(s.begin(), s.end());     //sort into sector categories
    string cursector;
    fprintf(fp, "<galaxy><systems>\n");
    int iter = 0;
    for (std::vector<System>::iterator i = s.begin(); i != s.end(); ++i) {
        //use FLOAT_MIN because of accuracy problems.
        if (i->luminosity <= FLT_MIN && i->luminosity >= -FLT_MIN && i->jumps.size() == 0) {
            continue;
        }
        if ((*i).sector != cursector) {
            //start sectortag;
            if (cursector != "") {
                fprintf(fp, "\t</sector>\n");
            }
            fprintf(fp, "\t<sector name=\"%s\">\n", (*i).sector.c_str());
            cursector = (*i).sector;
        }
        fprintf(fp, "\t\t<system name=\"%s\">\n", (*i).name.c_str());
        for (std::map<string, string>::iterator j = (*i).begin(); j != (*i).end(); ++j) {
            if ((*i).jumps.size() > 0 || (*j).first == "luminosity" || (*j).first == "xyz" || (*j).first
                    == "sun_radius") {
                fprintf(fp, "\t\t\t<var name=\"%s\" value=\"%s\"/>\n", (*j).first.c_str(), (*j).second.c_str());
            }
        }
        fprintf(fp, "\t\t\t<var name=\"jumps\" value=\"");
        if ((*i).jumps.size()) {
            fprintf(fp, "%s", (*i).jumps[0].c_str());
            for (unsigned int k = 1; k < (*i).jumps.size(); ++k) {
                fprintf(fp, " %s", (*i).jumps[k].c_str());
            }
        }
        fprintf(fp, "\"/>\n");
/*		if (iter>4 && iter+4<s.size()) {
 *                       fprintf (fp,"\t\t\t<var name=\"jumps\" value=\"nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s nowhereland/%s\"/>\n",s[iter-1].name.c_str(),s[iter-2].name.c_str(),s[iter-3].name.c_str(),s[iter-4].name.c_str(),s[iter+1].name.c_str(),s[iter+2].name.c_str(),s[iter+3].name.c_str());
 *                       }*/
        fprintf(fp, "\t\t</system>\n");
        iter++;
    }
    fprintf(fp, "\t</sector>\n</systems></galaxy>\n");
}

string getNameForFaction(std::string faction) {
    if (faction.empty()) {
        return "";
    }
    static map<string, vector<string> > factionnames;
    if (factionnames.find(faction) == factionnames.end()) {
        vector<string> factionnameslist;
        string filename = faction + ".txt";
        FILE *fp = fopen(filename.c_str(), "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            int whence = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *buf = (char *) malloc(whence + 1);
            buf[whence] = 0;
            while (fgets(buf, whence, fp)) {
                string ner = buf;
                ner = ner.substr(0, ner.find("\n"));
                ner = ner.substr(0, ner.find("\r"));

                factionnameslist.push_back(unpretty(ner));
            }
            fclose(fp);
        }
        factionnames.insert(pair<string, vector<string> >(faction, shuffle(factionnameslist)));
    }
    vector<string> *temp = &factionnames[faction];
    while (temp->size()) {
        string rez = temp->back();
        temp->pop_back();
        if (allnames.find(rez) == allnames.end()) {
            allnames.insert(rez);
            return rez;
        }
    }
    return "";
}

void planetsIn(System &which, std::string faction) {
    if (which["planetlist"].empty()) {
        string filename = string(path_to_universe) + "/planets." + faction + ".txt";
        FILE *fp = fopen(filename.c_str(), "r");
        if (fp) {
            which["planetlist"] = "planets." + faction + ".txt";
            fclose(fp);
        } else if (faction == "pirates" || faction == "luddites" || faction == "uln") {
            which["planetlist"] = "planets.fringe.txt";
            which["unitlist"] = "smallunits.pirates.txt";
        }
    }
}

void reName(std::vector<System> &s, System &which, std::string newname) {
    if (newname.empty()) {
        if (which.interesting == false && which.habitable && which["faction"] != "unknown") {
            string rec = recomputeName();
            if (!rec.empty()) {
                reName(s, which, rec);
            }
        }
        return;
    }
    string fac = which["faction"];
    if (which.interesting && fac != "rlaan" && fac != "ISO" && fac != "aera") {
        return;
    }
    string sector = which.sector;
    string oldname = which.name;
    string fullname = which.sector + "/" + which.name;
    string newfullname = which.sector + "/" + newname;
    which.name = newname;
    for (unsigned int i = 0; i < s.size(); ++i) {
        for (unsigned int j = 0; j < s[i].jumps.size(); ++j) {
            if (s[i].jumps[j] == fullname) {
                s[i].jumps[j] = newfullname;
            }
        }
    }
}

void processsystems(std::vector<System> &s) {
    vec3 min, max;
    computeminmax(s, min, max);
    unsigned int i;
    for (i = 0; i < s.size(); ++i) {
        std::map<double, string> jumps;
        if (s[i].habitable) {
            for (unsigned int j = 0; j < s.size(); ++j) {
                if (j != i && (s[j].habitable || (s[j].interesting && genrand_int32() < GENRAND_MAX * .3))) {
                    float dissqr =
                            sqr(s[i].xyz.x - s[j].xyz.x) + sqr(s[i].xyz.y - s[j].xyz.y) + sqr(s[i].xyz.z - s[j].xyz.z);
                    int desired_size = genrand_int32() % 5 + 1;
                    if (jumps.size() >= desired_size) {
                        if (jumps.upper_bound(dissqr) != jumps.end() && genrand_int32() < GENRAND_MAX * .995) {
                            jumps[dissqr] = s[j].fullName();
                            std::map<double, string>::iterator k = jumps.end();
                            k--;
                            jumps.erase(k);                             //erase last one
                        }
                    } else {
                        jumps[dissqr] = s[j].fullName();
                    }
                }
            }
        }
        string j;
        if (jumps.size()) {
            std::map<double, string>::iterator k = jumps.begin();
            j = (*k).second;
            ++k;
            s[i].jumps.push_back(j);
            for (; k != jumps.end(); ++k) {
                j += string(" ") + (*k).second;
                s[i].jumps.push_back((*k).second);
            }
        }
//s[i]["jumps"]=j;
    }
    for (i = 0; i < s.size(); ++i) {
        if (s[i].habitable) {
            unsigned int jsize = s[i].jumps.size();
            for (unsigned int k = 0; k < jsize; ++k) {
                string outgoing = s[i].jumps[k];
                System *foundsys = System::findSystem(s, outgoing);
                if (foundsys != NULL) {
                    string fullname = s[i].sector + "/" + s[i].name;
                    unsigned int jjsize = foundsys->jumps.size();
                    bool found = false;
                    if (!foundsys->habitable) {
                        //fprintf (stderr,"looking for %s in %s\n",fullname.c_str(),s[j].name.c_str());
                    }
                    for (unsigned int l = 0; l < jjsize; ++l) {
                        if (fullname == foundsys->jumps[l]) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        /*
                         *  if (s[j].jumps.empty())
                         *   s[j]["jumps"]=fullname;
                         *  else
                         *   s[j]["jumps"]=s[j]["jumps"]+" "+fullname;
                         */
                        foundsys->jumps.push_back(fullname);
                    }
                }
            }
        }
    }
    simulateFactionTurns(s);     //Simulates the factions starting with one homeworld, and expands their territories.x
    if (1) {
        for (unsigned int i = 0; i < s.size(); ++i) {
            if ((s[i].sector.find("ega") != string::npos || s[i].sector.find("Sol") != string::npos || s[i].sector
                    == "Solace" || s[i].sector == "Torkelsen" || s[i].sector == "Redemption" || s[i].sector == "Rhubarb"
                    || s[i].sector == "Plymouth" || s[i].sector == "Rust" || s[i].sector == "Diaspora" || s[i].sector
                    == "Beckett"
                    || s[i].sector.find("RBL") != string::npos) && s[i]["faction"] != "aera"
                    && s[i]["faction"] != "rlaan"
                    && s[i].jumps.size()) {
                float mult = 1;
                if (!s[i].habitable) {
                    mult = 20;
                }
                if (genrand_int32() < GENRAND_MAX * mult * .007) {
                    s[i]["faction"] = "pirates";
                }
                if (genrand_int32() < GENRAND_MAX * mult * .003) {
                    s[i]["faction"] = "luddites";
                }
                if (genrand_int32() < GENRAND_MAX * mult * .005) {
                    s[i]["faction"] = "ISO";
                }                      //also represented elsewhere
            }
            /*
             *  if (s[i]["faction"]=="confed") {
             *       FILE * fp = fopen (fp);
             *       int siz=  fseek (fp,0,SEEK_END);
             *       char * buf = (char *)malloc (siz+1);
             *       buf[siz]=0;
             *       while (fgets(buf,siz,fp)){
             *               vector<string> facsec=readCSV (buf);
             *
             *
             *       }
             *
             *       //now choose which subconfed faction this is
             *
             *  }
             */
            reName(s, s[i], getNameForFaction(s[i]["faction"]));
            planetsIn(s[i], s[i]["faction"]);
            numfactions[s[i]["faction"]] += 1;
            numsecfactions[s[i].sector][s[i]["faction"]] += 1;
        }
    }
    if (1) {
        fprintf(stderr, "\nOwnership\n");
        if (1) {
            for (std::map<string, std::map<string, int> >::iterator i = numsecfactions.begin();
                    i != numsecfactions.end();
                    ++i) {
                fprintf(stdout, "Sector %s\n", (*i).first.c_str());
                for (std::map<string, int>::iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
                    fprintf(stdout, "\t%s owns %d systems\n", (*j).first.c_str(), (*j).second);
                }
            }
        }
        for (std::map<string, int>::iterator i = numfactions.begin(); i != numfactions.end(); ++i) {
            fprintf(stdout, "%s owns %d systems\n", (*i).first.c_str(), (*i).second);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("not enough args. Usage %s <input> <output>\n", argv[0]);
        return 1;
    }
    if (argc > 3) {
        milky_way = argv[3];
    }
    if (argc > 4) {
        path_to_universe = argv[4];
    }
    init_genrand(4294442);
    srand(4294442);
    std::vector<System> s = readfile(argv[1]);
    processsystems(s);
    FILE *fp = fopen(argv[2], "w");
    if (fp) {
        writesystems(fp, s);
        fclose(fp);
    } else {
        printf("could not open %s for writing\n", argv[2]);
    }
    fprintf(stderr, "Hit enter twice, f00");
    getchar();
    return 0;
}

/*
 *  A C-program for MT19937, with initialization improved 2002/1/26.
 *  Coded by Takuji Nishimura and Makoto Matsumoto.
 *  Before using, initialize the state by using init_genrand(seed)
 *  or init_by_array(init_key, key_length).
 *  Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. The names of its contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  Any feedback is very welcome.
 *  http://www.math.keio.ac.jp/matumoto/emt.html
 *  email: matumoto@math.keio.ac.jp
 */
#include <stdio.h>
/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */
static unsigned long mt[N]; /* the array for the state vector  */
static int mti = N + 1; /* mti==N+1 means mt[N] is not initialized */
/* initializes mt[N] with a seed */
void init_genrand(unsigned long s) {
    mt[0] = s & 0xffffffffUL;
    for (mti = 1; mti < N; mti++) {
        mt[mti] =
                (1812433253UL * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
        /*
         * See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
         * In the previous versions, MSBs of the seed affect
         * only MSBs of the array mt[].
         * 2002/01/09 modified by Makoto Matsumoto
         */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/*
 * initialize by an array with array-length
 * init_key is the array for initializing keys
 * key_length is its length
 */
void init_by_array(unsigned long init_key[], unsigned int key_length) {
    int i, j, k;
    init_genrand(19650218UL);
    i = 1;
    j = 0;
    k = (N > key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1664525UL))
                + init_key[j] + j;           /* non linear */
        mt[i] &= 0xffffffffUL;         /* for WORDSIZE > 32 machines */
        i++;
        j++;
        if (i >= N) {
            mt[0] = mt[N - 1];
            i = 1;
        }
        if (j >= key_length) {
            j = 0;
        }
    }
    for (k = N - 1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i - 1] ^ (mt[i - 1] >> 30)) * 1566083941UL))
                - i;         /* non linear */
        mt[i] &= 0xffffffffUL;         /* for WORDSIZE > 32 machines */
        i++;
        if (i >= N) {
            mt[0] = mt[N - 1];
            i = 1;
        }
    }
    mt[0] = 0x80000000UL;     /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void) {
    unsigned long y;
    static unsigned long mag01[2] = {0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
    if (mti >= N) {
        /* generate N words at one time */
        int kk;
        if (mti == N + 1) {          /* if init_genrand() has not been called, */
            init_genrand(5489UL);
        }              /* a default initial seed is used */
        for (kk = 0; kk < N - M; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (; kk < N - 1; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1UL];
        mti = 0;
    }
    y = mt[mti++];
    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void) {
    return (long) (genrand_int32() >> 1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void) {
    return genrand_int32() * (1.0 / 4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void) {
    return genrand_int32() * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void) {
    return (((double) genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void) {
    unsigned long a = genrand_int32() >> 5, b = genrand_int32() >> 6;
    return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */

