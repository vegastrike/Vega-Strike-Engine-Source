#include <assert.h>
#include "star_system.h"
#include "planet.h"
#include "cmd_unit.h"
#include "UnitCollection.h"
#include "gfx_click_list.h"
#include "gfx_hud.h"
#include "cmd_input_dfa.h"
StarSystem::StarSystem(Planet *primaries) : 
  primaries(primaries), 
  units(new UnitCollection()), 
  drawList(new UnitCollection()),
  missiles(new UnitCollection()), tp(new TextPlane("9x12.fon")) {
  currentcamera = 0;	
  systemInputDFA = new InputDFA (this);
  primaries->SetPosition(0,0,5);
  Iterator *iter = primaries->createIterator();
  drawList->prepend(iter);

  delete iter;

  tp->SetPosition(0.5,0.5,1);

  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  
}

StarSystem::~StarSystem() {
  delete systemInputDFA;
  delete primaries;
}

ClickList *StarSystem::getClickList() {
  return new ClickList (this, drawList);

}

void StarSystem::modelGravity() {
  primaries->gravitate(units);
}

void StarSystem::AddUnit(Unit *unit) {
  units->prepend(unit);
  drawList->prepend(unit);
}

void StarSystem::RemoveUnit(Unit *unit) {
  assert(0);
}

void StarSystem::Draw() {
  //primaries->Draw();
  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->TDraw();
    iter->advance();
  }
  delete iter;
  // slightly inefficient and stupid, but it's a quick hack to test caching effects

  Mesh::ProcessUndrawnMeshes();
  systemInputDFA->Draw();

  /*
  UnitCollection *col = systemInputDFA->getCollection();
  string selected_units("");
  if(0 != col) {
    iter = col->createIterator();
    Unit *u;
    while(0!=(u = iter->current())) {
      selected_units += string(", ") + u->get_name();
      iter->advance();
    }
    tp->Draw();
    delete iter;
  }
  tp->SetText(selected_units);
  */
}

void StarSystem::Update() {
  modelGravity();

  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
        unit->ResolveForces();
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
