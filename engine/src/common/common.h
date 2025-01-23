/*
 * common.h
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
/***************************************************************************
*                          common.h  -  description
*                             -------------------
*    begin                : Wed Jun 26 2002
*    copyright            : (C) 2002 by jhunt
*    email                : jhunt@jaja
*    copyright            : (C) 2023 by Stephen G. Tuggy, Benjamen R. Meyer
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

// NO HEADER GUARD


/* This file is for common (as in shared between vegastrike, vegasettings as vsconfig) stuff
 * that is not important enough to warrant its own file.
 */

#ifndef _WIN32
#include <string>

//Returns where the data directory is. Returns the cwd if it can't find the data dir.
//Note: When it returns it has already changed dir to where the data directory is
std::string getdatadir();

#endif
