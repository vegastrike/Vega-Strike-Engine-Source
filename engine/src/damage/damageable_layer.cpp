#include "damageable_layer.h"



void DamageableLayer::DealDamage( const CoreVector &attack_vector, Damage &damage ) {
    DamageableFacet impacted_facet = GetFacet(attack_vector);
    impacted_facet.DealDamage(damage);
}


void DamageableLayer::Disable() {
    for(DamageableFacet& facet : facets) {
        facet.health.health = 0;
        facet.health.enabled = false;
    }
}


void DamageableLayer::Enable() {
    for(DamageableFacet& facet : facets) {
        facet.health.enabled = true;
    }
}


const DamageableFacet& DamageableLayer::GetFacet(const CoreVector& attack_vector) {
    for(const DamageableFacet& facet : facets) {
        if(facet.InFacet(attack_vector)) {
            return facet;
        }
    }

    return facets[0];
}
