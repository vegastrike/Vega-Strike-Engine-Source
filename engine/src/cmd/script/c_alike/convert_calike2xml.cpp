/*
 * convert_calike2xml.cpp
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

/*
  c_alike scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <stdio.h>

#include <string>

extern std::string module_string;

#include <iostream>
extern void yyparse();

void main(int argc, char **argv) {
    extern FILE *yyin;
    if (argc <= 1) {
        yyin = fopen("test.c", "r");
    } else {
        //    yyin=fopen(argv[1],"r");
        yyin = stdin;
    }

    yyparse();

    std::cout << module_string;
}

