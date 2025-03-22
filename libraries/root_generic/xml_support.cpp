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
#include <cctype>
#include <cassert>
#include <cstdarg>
#include "xml_support.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

using std::string;

string strtoupper(const string &foo) {
    string rval(foo);
    for (char & it : rval) {
        it = toupper(it);
    }
    return rval;
}

string strtolower(const string &foo) {
    string rval(foo);
    for (char & it : rval) {
        it = tolower(it);
    }
    return rval;
}

namespace XMLSupport {
typedef boost::char_separator<std::string::value_type> Separator;
typedef boost::tokenizer<Separator> Tokenizer;

static const char *SEPARATORS = " \t,";

EnumMap::EnumMap(const Pair *data, unsigned int num) {
    for (unsigned int a = 0; a < num; a++) {
        forward.Put(strtoupper(data[a].name), &data[a].val);
        reverse.Put(tostring(data[a].val), &data[a].name);
    }
}

int EnumMap::lookup(const string &str) const {
    const int *result = forward.Get(strtoupper(str));
    if (!result) {
        result = forward.Get("UNKNOWN");
    }
    return result ? *result : 0;
}

const string &EnumMap::lookup(int val) const {
    const string *result = reverse.Get(tostring(val));
    assert(0 != result);
    return *result;
}

AttributeList::AttributeList(const XML_Char **atts) {
    for (; *atts != nullptr; atts += 2) {
        push_back(Attribute(atts[0], atts[1]));
    }
}

string replace_space(const string &str) {
    string retval(str);
    for (char & i : retval) {
        if (i == ' ') {
            i = '_';
        }
    }
    return retval;
}

bool parse_bool(const string &str) {
    if ((!str.empty())
            && ((*str.begin()) == 't' || (*str.begin()) == 'T' || (*str.begin()) == 'y' || (*str.begin()) == 'Y'
                    || (*str.begin()) == '1')) {
        return true;
    } else {
        return false;
    }
}

double parse_float(const string &str) {
    double ret = 0.0f;
    std::stringstream ss(str);
    ss >> ret;
    return ret;
}

float parse_floatf(const string &str) {
    float ret = 0.0f;
    std::stringstream ss(str);
    ss >> ret;
    return ret;
}

void parse_floatv(const std::string &input, size_t maxTokens, ...) {
    va_list arguments;
    va_start(arguments, maxTokens);
    Separator separator(SEPARATORS);
    Tokenizer tokens(input, separator);

    for (Tokenizer::iterator it = tokens.begin(); maxTokens > 0 && it != tokens.end(); ++it, --maxTokens) {
        double *output = va_arg(arguments, double *);
        *output = boost::lexical_cast<double>(*it);
    }
    va_end(arguments);
}

void parse_floatfv(const string &input, int maxTokens, ...) {
    va_list arguments;
    va_start(arguments, maxTokens);
    Separator separator(SEPARATORS);
    Tokenizer tokens(input, separator);

    for (Tokenizer::iterator it = tokens.begin(); maxTokens > 0 && it != tokens.end(); ++it, --maxTokens) {
        float *output = va_arg(arguments, float *);
        *output = boost::lexical_cast<float>(*it);
    }
    va_end(arguments);
}

int parse_int(const string &str) {
    int ret = 0;
    std::stringstream ss(str);
    ss >> ret;
    return ret;
}

string::size_type parse_option_find(const string &str, const string &opt, const string &sep, const string &vsep) {
    bool ini = true;
    string::size_type pos = 0;
    string::size_type optlen = opt.length();
    string::size_type strlen = str.length();
    string allsep = sep + vsep;
    if ((optlen == 0) || (strlen == 0)) {
        return string::npos;
    }
    bool found = false;
    while (!found && (pos != string::npos) && ((pos = str.find(opt, pos + (ini ? 0 : 1))) != string::npos)) {
        ini = false;
        found = (((pos == 0) || (sep.find(str[pos - 1]) != string::npos))
                && ((pos + optlen >= strlen) || (allsep.find(str[pos + optlen]) != string::npos)));
        if (!found) {
            pos = str.find_first_of(sep, pos + optlen);
        }           //quick advancement
    }
    return found ? pos : string::npos;
}

bool parse_option_ispresent(const string &str, const string &opt, const string &sep, const string &vsep) {
    return parse_option_find(str, opt, sep, vsep) != string::npos;
}

string parse_option_value(const string &str,
        const string &opt,
        const string &defvalue,
        const string &sep,
        const string &vsep) {
    string::size_type pos = parse_option_find(str, opt, sep, vsep);
    string::size_type vpos = str.find_first_of(vsep, pos + opt.length());
    string value;
    if (pos != string::npos && vpos != string::npos) {
        string::size_type vend = str.find_first_of(sep, vpos + 1);
        value = str.substr(vpos + 1, ((vend != string::npos) ? vend - vpos - 1 : string::npos));
    } else if (pos != string::npos) {
        value = "true";
    } else {
        value = defvalue;
    }
    return value;
}

string escaped_string(const string &str) {
#define ESCAPE_CASE(e, c, skip)               \
case e:                                         \
    if (rp && str[rp-1] == '\\') rv[ip++] = c;  \
    else if (!skip)                             \
        rv[ip++] = e;                           \
    break
    string::size_type rp, ip, n = str.length();
    string rv;
    rv.resize(n);
    for (rp = ip = 0; rp < n; ++rp) {
        switch (str[rp]) {
            ESCAPE_CASE('\\', '\\', true);
            ESCAPE_CASE('n', '\n', false);
            ESCAPE_CASE('r', '\r', false);
            ESCAPE_CASE('t', '\t', false);
            default:
                rv[ip++] = str[rp];
        }
    }
#undef ESCAPE_CASE
    rv.resize(ip);
    return rv;
}
} //namespace XMLSupport

