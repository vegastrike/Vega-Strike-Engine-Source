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
#include "gfx_primitive.h"
class Projectile: public Primitive {
public:
	Projectile();
};

class Gun{
  vector <Projectile *> projectiles;  //every gun keeps track of its bullets like so
  string side;
  enum MOUNT {
    LIGHT =1, 
    MEDIUM =2, 
    HEAVY =4, 
    SUPERHEAVY=8,
    CAPSHIPLIGHT=16, 
    CAPSHIPMED=32, 
    CAPSHIPHEAVY=64
  };
  int mount;
  class Stats {
    float damage;
    float refire;
    float range;
    char name[64];
    float radius;
    int tracktype;
  };
  Gun::Stats *type;
public:
  Gun(char *filename);
  ~Gun();

  void Draw();
  void Fire();
};
