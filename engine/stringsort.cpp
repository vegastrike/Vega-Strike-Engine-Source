/*
 * stringsort.cpp
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

#include <vector>
#include <string>
#include <stdio.h>
#include <algorithm>
using std::string;
using std::vector;

int nextnondigit(string a)
{
    for (unsigned int i = 0; i < a.length(); ++i) {
        if (!isdigit(a[i])) {
            return i;
        }
    }
    return a.length();
}

class LexibackwardComparator {
public:
    bool operator()(string a, string b)
    {
        if (a.empty() || b.empty()) {
            return false;
        }
        if (isdigit(a[0]) && isdigit(b[0])) {
            int aint = 0;
            int bint = 0;
            sscanf(a.c_str(), "%d", &aint);
            sscanf(b.c_str(), "%d", &bint);
            if (aint == bint) {
                return (*this)(a.substr(nextnondigit(a)), b.substr(nextnondigit(b)));
            }
            return aint < bint;
        }
        if (a[0] == b[0]) {
            return (*this)(a.substr(1), b.substr(1));
        }
        return a[0] < b[0];
    }

};

int main(int argc, char **argv)
{
    vector<string> l;
    for (int i = 1; i < argc; ++i) {
        l.push_back(argv[i]);
    }
    std::sort(l.begin(), l.end(), LexibackwardComparator());
    {
        for (unsigned int i = 0; i < l.size(); ++i) {
            printf("%s\n", l[i].c_str());
        }
    }

}
