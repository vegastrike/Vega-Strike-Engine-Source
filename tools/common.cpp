/*
 * Vega Strike
 * This file is Copyright (C) 2003 Konstantinos Arvanitis
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

/** @file common.cpp Common code for the VSR package file tools.

  @todo Proper documentation.
 */

#include "common.h"
#include <errno.h>
#include <vector>

FILEHandle::FILEHandle(const std::string &fname, const char perms[])
{
	file = fopen(fname.c_str(), perms);
	if (!file)
		throw errno;
}
