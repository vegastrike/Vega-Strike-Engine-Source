#include "damageable_facet.h"


void DamageableFacet::DealDamage( Damage &damage ) {
    health.DealDamage(damage);
}


bool DamageableFacet::InFacet(const Vector& vector) const {
    if(vector.i < minimum_vector.i || vector.j < minimum_vector.j || vector.k < minimum_vector.k) {
        return false;
    }

    if(vector.i > minimum_vector.i || vector.j > minimum_vector.j || vector.k > minimum_vector.k) {
        return false;
    }

    return true;
}
