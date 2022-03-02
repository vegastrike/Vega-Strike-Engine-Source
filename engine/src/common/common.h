/***************************************************************************
*                          common.h  -  description
*                             -------------------
*    begin                : Wed Jun 26 2002
*    copyright            : (C) 2002 by jhunt
*    email                : jhunt@jaja
*    copyright            : (C) 2022 by Stephen G. Tuggy
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


/* This file is for common (as in shared between vegastrike, vegasettings as vsconfig) stuff
 * that is not important enough to warrant its own file.
 */

#ifndef _WIN32
#include <string>

//Returns where the data directory is. Returns the cwd if it can't find the data dir.
//Note: When it returns it has already changed dir to where the data directory is
std::string getdatadir();

#endif

