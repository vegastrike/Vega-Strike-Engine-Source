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
#include "cmd_unit.h"

class Missile:public Unit{
public:
	Missile(char *filename);
};

class Gun:public Unit{
	Missile *projectiles[2048];  //every gun keeps track of its bullets like so
								//Maximum of 2048
	int num_proj;
	char missilename[64];

public:
	Gun(char *filename);
	~Gun()
	{
		for(int proj_index = 0; proj_index<2048; proj_index++)
			if(projectiles[proj_index])
				delete projectiles[proj_index];
	}

	void Draw();
	void Fire();
};
