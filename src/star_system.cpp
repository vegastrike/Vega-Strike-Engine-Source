#include <assert.h>
#include "star_system.h"
#include "planet.h"
#include "cmd_unit.h"
#include "UnitCollection.h"
#include "gfx_click_list.h"
StarSystem::StarSystem(Planet *primaries) : 
  primaries(primaries), 
  units(new UnitCollection()), 
  missiles(new UnitCollection()) {
  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  primaries->SetPosition(0,0,5);
  
}

ClickList *StarSystem::getClickList() {
  return new ClickList (units);

}

void StarSystem::modelGravity() {
  primaries->gravitate(units);
}

void StarSystem::AddUnit(Unit *unit) {
  Iterator *iter= units->createIterator();
  iter->insert(unit);
  delete iter;
}

void StarSystem::RemoveUnit(Unit *unit) {
  assert(0);
}

void StarSystem::Draw() {
  primaries->Draw();

  Iterator *iter = units->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->TDraw();
    iter->advance();
  }
  delete iter;
}

void StarSystem::Update() {
  modelGravity();

  Iterator *iter = units->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    //  unit->ResolveForces();
    // Do something with AI state here eventually
    iter->advance();
  }
  delete iter;
}
