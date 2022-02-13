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

#ifndef _XML_SUPPORT_H_
#define _XML_SUPPORT_H_

#include <stdio.h>
#include <string>
#ifndef WIN32
#include <sstream>
#endif
#include "hashtable.h"
#include <vector>
#include <expat.h>
#include <iostream>             //needed for cout calls in config_xml.cpp (and other places too i'm sure)
#include "gfx/vec.h"

#if defined (_MSC_VER) && defined (_WIN32) && (_MSC_VER >= 1400)
//Disable useless "sprintf depricated" errors in Visual Studio 2005 Express.
#pragma warning(disable : 4996)
#endif

#define ARRAY_LENGTH(a) ( sizeof (a)/sizeof (a[0]) )

std::string strtoupper(const std::string &foo);
std::string strtolower(const std::string &foo);

namespace XMLSupport {
struct Attribute {
    std::string name;
    std::string value;

    Attribute(std::string name, std::string value) : name(name), value(value) {
    }
};

class AttributeList : public std::vector<Attribute> {
public:
    AttributeList(const XML_Char **atts);
};
float parse_floatf(const std::string &str);
double parse_float(const std::string &str);
void parse_floatfv(const std::string &str, int max, ...);
void parse_floatv(const std::string &str, int max, ...);
std::string replace_space(const std::string &str);
int parse_int(const std::string &str);
bool parse_bool(const std::string &str);
bool parse_option_ispresent(const std::string &str,
        const std::string &opt,
        const std::string &sep = ",\r\n",
        const std::string &vsep = "=");
std::string parse_option_value(const std::string &str,
        const std::string &opt,
        const std::string &defvalue,
        const std::string &sep = ",\r\n",
        const std::string &vsep = "=");
std::string escaped_string(const std::string &str);

class EnumMap {
//static inline double parse_float (const string &str) {return ::parse_float (str)};
    Hashtable<std::string, const int, 1001> forward;
    Hashtable<std::string, const std::string, 1001> reverse;
public:

    struct Pair {
        std::string name;
        int val;

        Pair(const std::string &c, int v) {
            name = c;
            val = v;
        }
    };

    EnumMap(const Pair *data, unsigned int num);

    int lookup(const std::string &str) const;
    const std::string &lookup(int val) const;
};

/*
 *  string tostring(int num);
 *  string tostring(float num);
 */
//#ifdef WIN32
inline std::string tostring5(unsigned short num) {
    char tmp[256];
    sprintf(tmp, "%.5d", num);
    return std::string(tmp);
}

inline std::string tostring(int num) {
    char tmp[256];
    sprintf(tmp, "%d", num);
    return std::string(tmp);
}

inline std::string tostring(unsigned int num) {
    char tmp[256];
    sprintf(tmp, "%u", num);
    return std::string(tmp);
}

inline std::string tostring(long num) {
    char tmp[256];
    sprintf(tmp, "%ld", num);
    return std::string(tmp);
}

inline std::string tostring(float num) {
    char tmp[256];
    sprintf(tmp, "%g", num);
    return std::string(tmp);
}

inline std::string floattostringh(float f) {
    char c[128];
    sprintf(c, "%2.2f", f);
    return std::string(c);
}

inline std::string VectorToString(const Vector &v) {
    std::string ret(floattostringh(v.i));
    if (v.i != v.j || v.j != v.k) {
        ret += std::string(",") + floattostringh(v.j) + std::string(",") + floattostringh(v.k);
    }
    return ret;
}
/*#else
 *  inline template<class T> string tostring(T num) {
 *   return string(((ostrstream*)&(ostrstream() << num << '\0'))->str());
 *
 *  }
 #endif*/
}
#endif

