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
#ifndef ANIMATION_H
#define ANIMATION_H
#include "lin_time.h"
#include "gfx_primitive.h"
#include "gfx_transform_vector.h"

#include "quaternion.h"

class Animation:public Primitive {	
  Texture **Decal; 
  bool camup;
  int numframes;
  float timeperframe;
  scalar_t cumtime;
  float height; //half the height so you can do fancy vector translatons to campspace
  float width;
  bool repeat;
  void InitAnimation();
public:
  Animation();
  Animation(char *, bool Rep=0, float priority=.1,enum FILTER ismipmapped=MIPMAP,bool camorient=false);
  ~Animation();
  void Draw(const Transformation & t=identity_transformation, const float *m=identity_matrix );
  void DrawNow();
  static void ProcessDrawQueue();
  void SetDimensions(float wid, float hei);
  bool Done();
};

#endif
