/*
 * cargoconvert.cpp
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <set>
//no sane programmer has a max size... but this is late...and you can set the thing...be aware that a bad datafile could crash your computer

#define MAXSIZE 2048

using namespace std;

class Averaging {
public:
    vector<int> samples;

    Averaging() {
    }

    Averaging &operator+=(int a) {
        samples.push_back(a);
        return *this;
    }

    float spread() {
        int min = -1;
        int max = -1;
        int zerocount = 0;
        for (int i = 0; i < samples.size(); ++i) {
            if (samples[i] == 0) {
                zerocount += 1;
            } else {
                if (min == -1 || min > samples[i]) {
                    min = samples[i];
                }
                if (max == -1 || max < samples[i]) {
                    max = samples[i];
                }
            }
        }
        //printf ("maxmin %d %d\n",max,min);
        if (zerocount == 0) {
            if (max == min && samples.size() <= 1) {
                return .25 * max;
            }
            if (max == min) {
                return .1 * max;
            }
            return max - min;
        } else {
            return max * 2;
        }
    }

    float realaverage() {
        float ave = 0;
        for (int i = 0; i < samples.size(); ++i) {
            ave += samples[i] / (float) samples.size();
        }
        return ave;
    }

    float average() {
        float ave = 0;
        for (int i = 0; i < samples.size(); ++i) {
            if (samples[i] == 0) {
                return 0;
            }
        }
        for (int i = 0; i < samples.size(); ++i) {
            ave += samples[i] / (float) samples.size();
        }
        return ave;
    }
};

void merge(map<string, Averaging> &inout, map<string, Averaging> in) {
    for (map<string, Averaging>::iterator i = in.begin(); i != in.end(); ++i) {
        for (int j = 0; j < i->second.samples.size(); ++j) {
            inout[i->first] += i->second.samples[j];
        }
    }
}

map<string, string> basekey;
map<string, map<string, Averaging> > total;

int main(int argc, char **argv) {
    std::string arg;
    if (argc < 2) {
        fprintf(stderr, "Usage: ./a.out csvfile pleasricultural_name\n");
    }
    if (argc == 2) {
        arg = "agricultural_planets";
    } else {
        arg = argv[2];
    }
    basekey["Sutur"] = "agricultural_planets";
    basekey["Charon"] = "mining_base";
    basekey["Heimdel"] = "mining_base";
    basekey["Smallville"] = "mining_base__pirates";
    basekey["Achilles"] = "mining_base";
    basekey["Anapolis"] = "refinery";
    basekey["Basque"] = "mining_base";
    basekey["Basque"] = "mining_base";
    basekey["Basra"] = "refinery";
    basekey["Beaconsfield"] = "refinery";
    basekey["Bodensee"] = "agricultural_planets";
    basekey["Burton"] = "agricultural_planets";
    basekey["Drake"] = "mining_base__pirates";
    basekey["Edinburg"] = "refinery";
    basekey["Edom"] = "agricultural_planets";
    basekey["Palan"] = "agricultural_planets";
    basekey["Elysia"] = "agricultural_planets";
    basekey["Erewhon"] = "agricultural_planets";
    basekey["Glasgow"] = "refinery";
    basekey["Gracchus"] = "refinery";
    basekey["Hector"] = "mining_base";
    basekey["Heimdal"] = "agricultural_planets";
    basekey["Helen"] = "agricultural_planets";
    basekey["Jolson"] = "pleasure_planets";
    basekey["Joplin"] = "refinery";
    basekey["Kronecker"] = "mining_base";
    basekey["Lisacc"] = "mining_base";
    basekey["Liverpool"] = "refinery";
    basekey["Macabee"] = "mining_base";
    basekey["Magdaline"] = "pleasure_planets";
    basekey["Matahari"] = arg;
    basekey["Meadow"] = "refinery";
    basekey["Megiddo"] = "mining_base__pirates";
    basekey["Mjolnar"] = "agricultural_planets";
    basekey["Munchen"] = "refinery";
    basekey["N1912-1"] = arg;
    basekey["New_Constantinople"] = "new_constantinople";
    basekey["New_Detroit"] = "new_detroit_planets";
    basekey["New_Iberia"] = "agricultural_planets";
    basekey["New_Reno"] = "pleasure_planets";
    basekey["Nitir"] = "agricultural_planets";
    basekey["Oakham"] = "mining_base__pirates";
    basekey["Olympus"] = arg;
    basekey["Oresville"] = "agricultural_planets";
    basekey["Oxford"] = "university_planets";
    basekey["Perry Naval Base"] = "perry";
    basekey["Remus"] = "refinery";
    basekey["Rilke"] = "refinery";
    basekey["Rodin"] = "agricultural_planets";
    basekey["Romulus"] = "mining_base";
    basekey["Rygannon"] = "mining_base";
    basekey["Saratov"] = "mining_base";
    basekey["Siva"] = "agricultural_planets";
    basekey["Speke"] = "pleasure_planets";
    basekey["Surtur"] = "agricultural_planets";
    basekey["Thisbury"] = "refinery";
    basekey["Trinsic"] = "agricultural_planets";
    basekey["Tuck's"] = "mining_base__pirates";
    basekey["Valkyrie"] = "mining_base";
    basekey["Victoria"] = "agricultural_planets";
    basekey["Vishnu"] = "mining_base";
    basekey["Wickerton"] = "refinery";
    FILE *fp = fopen(argv[1], "r");
    char name[MAXSIZE];
    char field[MAXSIZE];
    char end[MAXSIZE];
    map<string, Averaging> base;
    set<string> cargotypes;
    string key;
    while (fgets(end, MAXSIZE, fp) != NULL) {
        if (end == string(",,\n")) {
            int maxsamples = 0;
            for (map<string, Averaging>::iterator i = base.begin();
                    i != base.end();
                    ++i) {
                int tmp = i->second.samples.size();
                if (tmp > maxsamples) {
                    maxsamples = tmp;
                }
            }
            for (map<string, Averaging>::iterator i = base.begin();
                    i != base.end();
                    ++i) {
                while (i->second.samples.size() < maxsamples) {
                    i->second += 0;
                }
            }
            merge(total[key], base);
            base = map<string, Averaging>();
            key = "";
            while (fgets(field, MAXSIZE, fp) != NULL) {
                if (field != string(",,\n")) {
                    sscanf(field, "%s", end);
                    string tmp = end;
                    key = tmp.substr(0, tmp.find(","));
                    if (basekey[key] == "") {
                        printf("Key Lookup :%s:\n", key.c_str());
                    }
                    key = (basekey[key] == "") ? key : basekey[key];
                    base = map<string, Averaging>();
                    break;
                }
            }
        } else {
            int quant, price;
            char *firstcomma = strstr(end, ",");
            if (firstcomma) {
                char *secondcomma = strstr(firstcomma + 1, ",");
                if (secondcomma) {
                    *firstcomma++ = '\0';
                    *secondcomma++ = '\0';
                    //printf ("Have %s %s\n",end,firstcomma);
                    base[end] += atoi(firstcomma);
                    cargotypes.insert(end);
                }
            }
        }
    }
    for (set<string>::iterator j = cargotypes.begin(); j != cargotypes.end(); ++j) {
        cout << ", " << *j;
        cout << ", " << *j << "_MaxMinusMinOver2";
    }
    cout << endl;
    for (map<string, map<string, Averaging> >::iterator i = total.begin(); i != total.end(); ++i) {
        cout << i->first;
        for (set<string>::iterator j = cargotypes.begin(); j != cargotypes.end(); ++j) {
            if (i->second.find(*j) != i->second.end()) {
                cout << ","
                        << i->second[*j].average()
                        << ","
                        << i->second[*j].spread() / 2;
            } else {
                //cout << *j<<",NOTHING, NOTHING, ";
                cout << ",,";
            }
        }
        cout << endl;
    }
    return 0;
}

