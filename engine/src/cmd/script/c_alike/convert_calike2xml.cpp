/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2022 Stephen G. Tuggy
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  c_alike scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#include <stdio.h>

#include <string>

extern std::string module_string;

#include <iostream>
extern void yyparse();

void main(int argc, char **argv)
{
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

