/*
 * xml_serializer.h
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

#ifndef VEGA_STRIKE_ENGINE_XML_SERIALIZER_H
#define VEGA_STRIKE_ENGINE_XML_SERIALIZER_H

#include "xml_support.h"
#include <vector>
#include <string>
#include "vsfilesystem.h"

using std::string;
using std::vector;

struct XMLType {
    union wordlength {
        int *i;
        unsigned int *ui;
        float *f;
        void *p;
        char *c;
        //short * s;
        bool *b;
        double *d;
        //unsigned short * us;
        unsigned char *uc;
        int hardint;
        float hardfloat;
    }
            w{};
    std::string str;

    explicit XMLType(bool *mybool) {
        w.b = mybool;
    }

    explicit XMLType(double *mydouble) {
        w.d = mydouble;
    }

    explicit XMLType(int *myint) {
        w.i = myint;
    }

    explicit XMLType(unsigned int *myuint) {
        w.ui = myuint;
    }

    explicit XMLType(int myhardint) {
        w.hardint = myhardint;
    }

    explicit XMLType(float myhardfloat) {
        w.hardfloat = myhardfloat;
    }

    explicit XMLType(float *myfloat) {
        w.f = myfloat;
    }

    explicit XMLType(void *myvoid) {
        w.p = myvoid;
    }

    explicit XMLType(char *mychar) {
        w.c = mychar;
    }

    //XMLType (short * mychar) {w.s=mychar;} // removing useless shorts - use integers instead
    //XMLType (unsigned short * mychar) {w.us=mychar;} // removing useless shorts - use integers instead
    explicit XMLType(unsigned char *mychar) {
        w.uc = mychar;
    }

    explicit XMLType(const std::string &s) : str(s) {
        w.p = &this->str;
    }

    XMLType(const std::string &s, void *v) : str(s) {
        w.p = v;
    }

    XMLType(const std::string &s, int myhardint) : str(s) {
        w.hardint = myhardint;
    }

    XMLType(const std::string &s, float *f) : str(s) {
        w.f = f;
    }
};
typedef std::string (XMLHandler)(const XMLType &input, void *mythis);
XMLHandler intHandler;
XMLHandler floatHandler;
XMLHandler intStarHandler;
//XMLHandler shortStarHandler;
//XMLHandler shortToFloatHandler;
//XMLHandler ushortStarHandler;
XMLHandler intToFloatHandler;
XMLHandler uintStarHandler;
XMLHandler charStarHandler;
XMLHandler boolStarHandler;
XMLHandler doubleStarHandler;
XMLHandler ucharStarHandler;
XMLHandler negationCharStarHandler;
XMLHandler floatStarHandler;
XMLHandler fabsFloatStarHandler;
//XMLHandler absShortStarHandler;
XMLHandler absIntStarHandler;

XMLHandler speedStarHandler;
XMLHandler accelStarHandler;

XMLHandler scaledFloatStarHandler;
XMLHandler angleStarHandler;
XMLHandler negationFloatStarHandler;
XMLHandler negationIntStarHandler;
XMLHandler stringStarHandler;
XMLHandler stringHandler;
XMLHandler lessNeg1Handler;
XMLHandler cloakHandler;

struct XMLElement {
    std::string elem;
    XMLType value;
    XMLHandler *handler;

    XMLElement(const std::string &ele, const XMLType &val, XMLHandler *hand) : value(val) {
        elem = ele;
        handler = hand;
    }

    void Write(VSFileSystem::VSFile &f, void *mythis);
    string WriteString(void *mythis);
};
struct XMLnode {
    XMLnode *up;
    std::string val;
    vector<XMLElement> elements;
    vector<XMLnode> subnodes;

    XMLnode() {
        up = nullptr;
    }

    XMLnode(const std::string &val, XMLnode *newup) {
        this->val = val;
        up = newup;
    }

    void Write(VSFileSystem::VSFile &f, void *mythis, int tablevel);
    string WriteString(void *mythis, int tablevel);
};
class XMLSerializer {
    std::string filename;
    std::string savedir;
    void *mythis;
    XMLnode topnode;
    XMLnode *curnode;
public:
    std::string randomdata[1];
    XMLSerializer(const char *filename, const char *modificationname, void *mythis);
    void AddTag(const std::string &tag);
    void AddElement(const std::string &element, XMLHandler *handler, const XMLType &input);
    void Write(const char *modificationname = "");
    std::string WriteString();
    void EndTag(const std::string endname = string(""));

    const std::string &getName() {
        return filename;
    }

    void setName(const std::string &fil) {
        this->filename = fil;
    }
};

#endif //VEGA_STRIKE_ENGINE_XML_SERIALIZER_H

