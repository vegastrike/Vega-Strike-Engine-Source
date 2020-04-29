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
#ifndef VSR_TOOLS_H_
#define VSR_TOOLS_H_
#include <stdint.h>
/** The standard header found in all Vega Strike resource package files. */
struct VSRHeader {
	char magic[4];  ///Must always be 'V' 'S' 'R' '\0'
	uint32_t entries; ///The number of files in this package.
	uint32_t entryTableOffset;  ///The offset of the entry table in this file.
};

/// A single entry in the Vega Strike resource package file table of entries.
struct VSRPEntry {
	uint32_t fileLength; ///The total length of this file.
	uint32_t offset;	///The offset this file starts in the package.
	char filename[256];	///The complete path to the file, terminating with '\0'.
};
#endif //VSR_TOOLS_H_
