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

#include "vec.h"

#include <stack>

#include "quaternion.h"

#include "ani_texture.h"

namespace VSFileSystem
{
	class VSFile;
};

class Animation: public AnimatedTexture {

  GFXColor mycolor;

  Matrix local_transformation;

  float height; //half the height so you can do fancy vector translatons to campspace

  float width;

  unsigned char options;

  void InitAnimation();

public:

  Animation();

  Animation(VSFileSystem::VSFile * f, bool Rep=0, float priority=.1,enum FILTER ismipmapped=MIPMAP,bool camorient=false, bool appear_near_by_radius=false, const GFXColor &col=GFXColor(1,1,1,1));
  Animation(const char *, bool Rep=0, float priority=.1,enum FILTER ismipmapped=MIPMAP,bool camorient=false, bool appear_near_by_radius=false, const GFXColor &col=GFXColor(1,1,1,1));

  ~Animation();

  void Draw();
  void SetFaceCam(bool face);
  void CalculateOrientation (Matrix &result);

  void DrawNow(const Matrix & final_orientation);

  void DrawNoTransform ();

  void DrawAsSprite (class Sprite *spr);

  static void ProcessDrawQueue(std::vector <Animation *> &,float);

  static void ProcessDrawQueue();

  static void ProcessFarDrawQueue(float);

  void SetDimensions(float wid, float hei);

  void GetDimensions (float &wid, float & hei);

  QVector Position();

  void SetPosition (const QVector &);

  void SetOrientation(const Vector &p, const Vector &q, const Vector &r);

};



#endif

