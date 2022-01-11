/**
* bolt.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _CMD_BOLT_H_
#define _CMD_BOLT_H_
#include "gfxlib.h"
#include "gfxlib_struct.h"
#include "weapon_xml.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include "collide_map.h"

class Unit;
class StarSystem;
class BoltDrawManager;
class Animation;

class Bolt {
private:
  const weapon_info* type;//beam or bolt;
  Matrix drawmat;
  QVector cur_position;
  Vector ShipSpeed;
  QVector prev_position;//beams don't change heading.
  void *owner;
  float curdist;
  int decal;//which image it uses
 public:
  CollideMap::iterator location;
  static int AddTexture(BoltDrawManager *q, std::string filename);
  static int AddAnimation(BoltDrawManager *q, std::string filename, QVector cur_position);
  bool Collide (Unit * target);
  static bool CollideAnon (Collidable::CollideRef bolt_name, Unit* target);
  static Bolt * BoltFromIndex(Collidable::CollideRef bolt_name);
  static Collidable::CollideRef BoltIndex(int index, int decal, bool isBall);
  bool operator == (const Bolt & b) const{
    
    return owner==b.owner
      &&curdist==b.curdist
      &&cur_position==b.cur_position
      &&prev_position==b.prev_position;
  }
  Bolt(const weapon_info *type, const Matrix &orientationpos, const Vector & ShipSpeed, void *owner, CollideMap::iterator hint);//makes a bolt
  void Destroy(unsigned int index);
  //static void Draw();
  static void DrawAllBolts();
  static void DrawAllBalls();
  void DrawBolt(float& bolt_size, GFXVertexList *qmesh);
  void DrawBall(float& bolt_size, Animation *cur);
  bool Update(Collidable::CollideRef index);
  bool Collide(Collidable::CollideRef index);
  static void UpdatePhysics(StarSystem *ss);//updates all physics in the starsystem
  void noop()const{}
};


#endif
