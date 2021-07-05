#ifndef DAMAGEABLEOBJECT_H
#define DAMAGEABLEOBJECT_H

#include "damage.h"
#include "damageable_layer.h"
#include "gfx/vec.h"

/**
 * @brief The DamageableObject class repesents an overall game object - ship, space station, missile, etc.
 */
class DamageableObject
{
    std::vector<DamageableLayer> layers;        // Typically shield/armor/hull
    std::vector<DamageableObject> components;   // Propoulsion, life support,

public:
    DamageableObject();

    void DealDamage( const Vector &attack_vector, Damage &damage );
};

#endif // DAMAGEABLEOBJECT_H
