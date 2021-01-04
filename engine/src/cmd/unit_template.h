#ifndef UNIT_TEMPLATE_H_
#define UNIT_TEMPLATE_H_
#error
//currnetly causes multiple definitions for a STUPID reason... die, gcc die
#include "unit_generic.h"
#include "unit.h"
#include "unit_generics.h"

template class GameUnit< EnhancementGeneric >;
template class GameUnit< NebulaGeneric >;
template class GameUnit< Unit >;
template class GameUnit< BuildingGeneric >;
template class GameUnit< MissileGeneric >;
template class GameUnit< AsteroidGeneric >;
template class GameUnit< PlanetGeneric >;
#endif

