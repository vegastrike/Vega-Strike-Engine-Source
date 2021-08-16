#include "damageable_facet.h"
#include "core_vector.h"



void DamageableFacet::DealDamage( Damage &damage, InflictedDamage& inflicted_damage ) {
    health.DealDamage(damage, inflicted_damage);
}


