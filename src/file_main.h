/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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
#include <stdio.h>
#include <string.h>
#include "gfxlib.h"

extern FILE *fpread;

/*File utility functions*/
inline void LoadFile(char *filename)
{
	fpread = fopen(filename, "rb");
}
inline void CloseFile()
{
	fclose(fpread);
}

/*Read simple data*/
inline void ReadInt(int &integer)
{
	fread(&integer, sizeof(int), 1, fpread);
}
inline void ReadFloat(float &num)
{
	fread(&num, sizeof(float), 1, fpread);
}

inline void ReadString(char *string)
{
	int length = strlen(string);

	ReadInt(length);
	fread(string, length, 1, fpread);
	string[length] = '\0';
}

/*Read aggregated data*/
inline void ReadVector(float &x, float &y, float &z)
{
	ReadFloat(x);
	ReadFloat(y);
	ReadFloat(z);
}

inline void ReadVector(Vector &v)
{
	ReadVector(v.i, v.j, v.k);
}

inline void ReadGeneric(char *string, float &x, float &y, float &z)
{
	ReadString(string);
	ReadVector(x,y,z);
}

/*The goods*/
inline void ReadUnit(char *filename, int &type, float &x, float &y, float &z)
{
	ReadGeneric(filename, x, y, z);
}

inline void ReadMesh(char *filename, float &x, float &y, float &z)
{
	ReadGeneric(filename, x, y, z);
}

inline void ReadWeapon(char *filename, float &x, float &y, float &z)
{
	ReadGeneric(filename, x, y, z);
}

inline void ReadRestriction(int &isrestricted, float &start, float &end)
{
	ReadInt(isrestricted);
	ReadFloat(start);
	ReadFloat(end);
}

inline long GetPosition()
{
	return ftell(fpread);
}

inline void SetPosition(long position)
{
	fseek(fpread,position,SEEK_SET);
}
