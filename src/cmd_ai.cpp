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
#include "cmd_ai.h"
#include "UnitCollection.h"
AI * Order::Execute () {
  for (unsigned int i=0;i<suborders.size();i++) {
    suborders[i]->Execute();
  }
  return this;
}

bool Order::AppendOrder (Order *ord) {
  if (ord->getType()&getType()) {
    return false;
  }
  suborders.push_back (ord);

  return true;
}
bool Order::AttachOrder (UnitCollection *targets1) {
  if (!(type&TARGET))
    return false;
  if (targets)
    delete targets;
  
  targets = new UnitCollection();
  UnitCollection::UnitIterator *iter = targets1->createIterator();
  Unit *u;
  while(0!=(u = iter->current())) {
    targets->prepend (u);
    iter->advance();
  }
  delete iter;
  return true;
}
bool Order::AttachSelfOrder (UnitCollection *targets1) {
  if (!(type&SELF))
    return false;
  if (group)
    delete group;
  group = new UnitCollection();
  UnitCollection::UnitIterator *iter = targets1->createIterator();
  Unit *u;
  while(0!=(u = iter->current())) {
    group->prepend (u);
    iter->advance();
  }
  delete iter;
  return true;
}
bool Order::AttachOrder (Vector targetv) {
  if (type!=LOCATION) {
    return false;
  }
  targetlocation = targetv;
  return true;
}

/*
AI::AI(Unit *parent1)
{
	parent = parent1;
}
*/

AI *AI::Execute()
{
	return this;
}
