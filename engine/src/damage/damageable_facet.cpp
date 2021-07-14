#include "damageable_facet.h"
#include "core_vector.h"

void DamageableFacet::DealDamage( Damage &damage ) {
    health.DealDamage(damage);
}


/// Discussion - what happens when something comes exactly at the edge.
/// If we return false, it will miss all facets.
/// If we return true, it will hit multiple facets.
/// We return false because this is called by layer.
/// If no facet is identified a default one is returned.
bool DamageableFacet::InFacet(const CoreVector& vector) const {
    if(vector.i <= minimum_vector.i || vector.j <= minimum_vector.j || vector.k <= minimum_vector.k) {
        return false;
    }

    if(vector.i >= maximum_vector.i || vector.j >= maximum_vector.j || vector.k >= maximum_vector.k) {
        return false;
    }

    return true;
}
