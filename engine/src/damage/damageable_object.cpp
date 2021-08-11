#include "damageable_object.h"
#include "damageable_factory.h"
#include "core_vector.h"

DamageableObject::DamageableObject() {
    Health hull_health = Health(1, 1, 0);
    Health armor_health = Health(0, 0, 0);
    Health shield_health = Health(0, 0, 0);

    DamageableLayer hull_layer = DamageableFactory::CreateLayer(FacetConfiguration::one, hull_health, true);
    DamageableLayer armor_layer = DamageableFactory::CreateLayer(FacetConfiguration::one, armor_health, false);
    DamageableLayer shield_layer = DamageableFactory::CreateLayer(FacetConfiguration::one, shield_health, false);

    layers = { shield_layer, armor_layer, hull_layer };
}


DamageableObject::DamageableObject(std::vector<DamageableLayer> layers,
                                   std::vector<DamageableObject> components) {
    number_of_layers = layers.size();
    this->layers = layers;
    this->components = components;
}

/*DamageableObject::DamageableObject()
{
    layers.push_back(DamageableLayer());
}*/


void DamageableObject::DealDamage( const CoreVector &attack_vector, Damage &damage ) {
    for(DamageableLayer layer : layers) {
        layer.DealDamage(attack_vector, damage);

        if(layer.core_layer) {

        }

        if(damage.Spent()) {
            break;
        }
    }
}


void DamageableObject::Destroy() {
    for(DamageableLayer layer : layers) {
        layer.Destroy();
    }
}


// We make a lot of assumptions here:
// 1. The last layer is THE layer
// 2. There's only one facet in the last layer
// 3. Destroying it destroys the unit
bool DamageableObject::Destroyed() {
    if(layers.empty()) {
        return true;
    }

    return layers[0].facets[0].health.destroyed;
}
