#include <stdlib.h>
#include "container.h"
#include "unit.h"
UnitContainer::UnitContainer() {
  unit=NULL;
}
UnitContainer::UnitContainer(Unit * un):unit (NULL) {
  SetUnit (un);
}
UnitContainer::~UnitContainer() {
  //  if (unit)
  //    unit->UnRef();
  //bad idea...arrgh!
}
void UnitContainer::SetUnit (Unit * un) {
  if (unit)
    unit->UnRef();
  // if the unit is null then go here otherwise if the unit is killed then go here
  if (un!=NULL?un->Killed()==true:true) {
    unit = NULL;
    return;
  }
  unit = un;
  unit->Ref();
}



