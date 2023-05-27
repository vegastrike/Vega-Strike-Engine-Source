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
#ifndef VEGA_STRIKE_ENGINE_MISSION_XML_SUPPORT_H
#define VEGA_STRIKE_ENGINE_MISSION_XML_SUPPORT_H

#include <stdio.h>
#include <string>
#ifndef WIN32
#include <strstream>
#endif
#include "hashtable.h"
#include <vector>
#include <expat.h>
#include <iostream>        // needed for cout calls in config_xml.cpp (and other places too i'm sure)

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

using std::string;
#ifndef WIN32
using std::ostrstream;
#endif

string strtoupper(const string &foo);

namespace XMLSupport {

struct Attribute {
    string name;
    string value;

    Attribute(string name, string value) : name(name), value(value) {
    };
};

class AttributeList : public vector<Attribute> {
public:
    AttributeList(const XML_Char **atts);
};
double parse_float(const string &str);
int parse_int(const string &str);
bool parse_bool(const string &str);

class EnumMap {

    //    static inline double parse_float (const string &str) {return ::parse_float (str)};
    Hashtable<string, const int, char[1001]> forward;
    Hashtable<string, const string, char[1001]> reverse;
public:

    struct Pair {
        string name;
        int val;

        Pair(const string c, int v) {
            name = c;
            val = v;
        }
    };

    EnumMap(const Pair *data, unsigned int num);

    int lookup(const string &str) const;
    const string &lookup(int val) const;
};

/*
  string tostring(int num);
  string tostring(float num);
*/
//#ifdef WIN32
string inline tostring(int num) {
    char tmp[256];
    sprintf(tmp, "%d", num);
    return string(tmp);
}

string inline tostring(float num) {
    char tmp[256];
    sprintf(tmp, "%f", num);
    return string(tmp);
}
/*#else
  template<class T> inline string tostring(T num) {
    return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
	
  }
#endif*/
}

#endif //VEGA_STRIKE_ENGINE_MISSION_XML_SUPPORT_H
