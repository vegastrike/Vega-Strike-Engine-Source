#ifndef DAMAGEABLEOBJECT_H
#define DAMAGEABLEOBJECT_H

#include "damage.h"
#include "damageable_layer.h"
#include "core_vector.h"

/**
 * @brief The DamageableObject class repesents an overall game object - ship, space station, missile, etc.
 */
class DamageableObject
{
    std::vector<DamageableLayer> layers;        // Typically shield/armor/hull
    std::vector<DamageableObject> components;   // Propoulsion, life support,

public:
    DamageableObject(std::vector<DamageableLayer> layers,
                     std::vector<DamageableObject> components);
    DamageableObject();

    void DealDamage( const CoreVector &attack_vector, Damage &damage );
};

#endif // DAMAGEABLEOBJECT_H