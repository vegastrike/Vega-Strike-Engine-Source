/*
 * Copyright (C) 2001-2022 Daniel Horn, David Ranger, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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

#if defined(_WIN32) && _MSC_VER > 1300
#define __restrict
#endif
#include "../include/central.h"
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/dir.h>
#include <stdio.h>
#include <unistd.h>
#endif

int main(int argc, char *argv[])
{
    int i;
    for (i = strlen(argv[0]); argv[0][i] != '\\' && argv[0][i] != '/'; i--) {
    }
    argv[0][i + 1] = '\0';
    _chdir(argv[0]);
#if 0//def _WIN32
    FreeConsole();
#endif
    gtk_init(&argc, &argv);
    Start(1);
    return 0;
}
