#include "star_system.h"
#include "planet.h"
#include "cmd_unit.h"
#include "UnitCollection.h"

StarSystem::StarSystem(Planet *primaries) : 
  primaries(primaries), 
  units(new UnitCollection()), 
  missiles(new UnitCollection()) {
  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  primaries->SetPosition(0,0,5);
  
}

ClickList *StarSystem::getClickList() {
}

void StarSystem::modelPhysics() {
}

void StarSystem::Draw() {
  primaries->Draw();
}
