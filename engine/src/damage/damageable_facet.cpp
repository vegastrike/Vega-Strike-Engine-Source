#include "damageable_facet.h"
#include "core_vector.h"



void DamageableFacet::DealDamage( Damage &damage ) {
    health.DealDamage(damage);
}


