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
  currentcamera = 0;	

  Iterator *iter = primaries->createIterator();
  units->prepend(iter);

  delete iter;
  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  
}

StarSystem::~StarSystem() {
  delete primaries;
}

ClickList *StarSystem::getClickList() {
  return new ClickList (this, units);

}

void StarSystem::modelGravity() {
  primaries->gravitate(units);
}

void StarSystem::AddUnit(Unit *unit) {
  units->prepend(unit);
}

void StarSystem::RemoveUnit(Unit *unit) {
  assert(0);
}

void StarSystem::Draw() {
  //primaries->Draw();

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
    ///    unit->ResolveForces();
    // Do something with AI state here eventually
    unit->ExecuteAI();
    iter->advance();
  }
  delete iter;
}


void StarSystem::SelectCamera(int cam){
    if(cam<NUM_CAM&&cam>=0)
      currentcamera = cam;
}
Camera* StarSystem::AccessCamera(int num){
  if(num<NUM_CAM&&num>=0)
    return &cam[num];
  else
    return NULL;
}		
