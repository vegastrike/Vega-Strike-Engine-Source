#include "damageable_object.h"
#include "core_vector.h"

DamageableObject::DamageableObject(std::vector<DamageableLayer> layers,
                                   std::vector<DamageableObject> components) {
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
