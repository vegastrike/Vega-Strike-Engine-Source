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
#ifndef BACKGROUND_H
#define BACKGROUND_H
#include "star.h"
class SphereMesh;
struct Texture;
class Background {
  bool Enabled;
  StarVlist stars;
  Texture *up;
  Texture *left;
  Texture *front;
  Texture *right;
  Texture *back;
  Texture *down;
  SphereMesh * SphereBackground;
public:
  Background(const char *file, int numstars, float spread, std::string starfilename);
  ~Background();
  void EnableBG (bool);
  void Draw();
};

#endif

