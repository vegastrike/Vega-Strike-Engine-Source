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
  if (un==NULL||un->Killed()) {
    if (unit)
      unit->UnRef();
    unit = NULL;
    return;
  }
  unit = un;
  unit->Ref();
}
Unit * UnitContainer::GetUnit() {
  if (unit==NULL)
    return NULL;
  if (unit->Killed()) {
    unit->UnRef();
    unit = NULL;
    return NULL;
  }
  return unit;
}
