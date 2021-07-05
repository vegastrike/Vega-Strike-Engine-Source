#include "damageable_object.h"

DamageableObject::DamageableObject()
{

}


void DamageableObject::DealDamage( const Vector &attack_vector, Damage &damage ) {
    for(DamageableLayer layer : layers) {
        layer.DealDamage(attack_vector, damage);

        if(layer.core_layer) {

        }

        if(damage.Spent()) {
            break;
        }
    }
}
