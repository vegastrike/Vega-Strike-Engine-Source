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
#include "cmd_gun.h"
#include "cmd_ai.h"
#include "cmd_order.h"
#include <stdio.h>
Missile::Missile(char *filename):Unit(filename)
{
	//FILE *fp = fopen(filename, "r");
	//fclose(fp);
	

	// might want to put in code to self-destruct if filename is bad

	aistate = new FlyStraight(0.1, 6);
	aistate->SetParent(this);
}

Gun::Gun(char *filename)
{
	num_proj = 0;
#ifdef WIN32
	ZeroMemory(&projectiles, sizeof(projectiles));
#else
	bzero (&projectiles,sizeof(projectiles));
#endif
	FILE *fp = fopen(filename, "r");
	fscanf(fp, "%s", missilename);
	fclose(fp);
	
	/*
	fp = fopen(missilename, "r");

	fclose(fp);
	*/
}

void Gun::Draw()
{
	Unit::Draw();
	int proj_index;
	for(proj_index = 0; proj_index < num_proj; proj_index++)
	{
		Missile* currproj = projectiles[proj_index];
		if(currproj)
		{
			currproj->Draw();
			currproj = (Missile *)currproj->Update();
		}
	}
	while(projectiles[proj_index-1] == NULL) // decrement proj_index
		proj_index--;
}

void Gun::Fire()
{
  //////??????????? FIXME VEGASTRIKE
	Vector pos = this->local_transformation.position;
	projectiles[num_proj] = new Missile(missilename);
	num_proj++;
}
