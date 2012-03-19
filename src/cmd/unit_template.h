#ifndef UNIT_TEMPLATE_H_
#define UNIT_TEMPLATE_H_
#error
//currnetly causes multiple definitions for a STUPID reason... die, gcc die
#include "unit_generic.h"
#include "unit.h"
#include "asteroid_generic.h"
#include "enhancement_generic.h"
#include "planet_generic.h"
#include "building_generic.h"
#include "missile_generic.h"
#include "nebula_generic.h"

template class GameUnit< Enhancement >;
template class GameUnit< Nebula >;
template class GameUnit< Asteroid >;
template class GameUnit< Unit >;
template class GameUnit< Building >;
template class GameUnit< Missile >;
template class GameUnit< Planet >;
#endif

