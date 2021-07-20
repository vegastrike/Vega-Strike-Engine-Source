#include "damageable_layer.h"

#include "iostream"

void DamageableLayer::DealDamage( const CoreVector &attack_vector, Damage &damage ) {
    int impacted_facet_index = GetFacetIndex(attack_vector);
    facets[impacted_facet_index].DealDamage(damage);
}


void DamageableLayer::Destroy() {
    for(DamageableFacet& facet : facets) {
        facet.health.health = 0;
        facet.health.destroyed = true;
    }
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


int DamageableLayer::GetFacetIndex(const CoreVector& attack_vector) {
    for(int i=0;i<facets.size();i++) {
        if(facets[i].InFacet(attack_vector)) {
            return i;
        }
    }

    return 0;
}
