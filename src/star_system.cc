#include "star_system.h"
#include "cmd_unit.h"

StarSystem::StarSystem(UnitCollection *primaries) : 
  primaries(primaries), 
  units(new UnitCollection()), 
  missiles(new UnitCollection()) {
  // Calculate movement arcs; set behavior of primaries
}

ClickList *StarSystem::getClickList() {
}

void StarSystem::modelPhysics() {
}

void StarSystem::Draw() {
}
