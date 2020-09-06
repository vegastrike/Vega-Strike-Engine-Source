#ifndef COLLISION_H
#define COLLISION_H

#include "gfx/vec.h"
#include "unit_generic.h"

class Unit;
class Cockpit;

/* Rather than implement collision as part of unit as Collidable,
 * I chose to implement a class for the collision itself.
 * The reactToCollision method creates two collision object instances,
 * one for each unit.
 * It is used to reduce duplication of code, as much of the code is simple duplication.
 * Because some variables rely on previous calculations using both units,
 * we do initialization in stages.
*/

class Collision
{
    struct ReactionResult
    {
        bool apply_force = false;
        bool deal_damage = false;

        ReactionResult(bool apply_force, bool deal_damage):
            apply_force(apply_force), deal_damage(deal_damage) {}
    };

    // Stage 1
    Cockpit* cockpit;
    Unit* unit;
    clsptr unit_type;
    bool is_player_ship;
    const QVector& location;
    const Vector& normal;
    float mass;
    Vector position;
    Vector velocity;
    Vector angular_velocity;
    Vector linear_velocity; //Compute linear velocity of points of impact by taking into account angular velocities

    // Stage 2
    Vector aligned_velocity;
    Vector elastic_vf;
    Vector final_velocity;
    float delta_e;

    // Stage 3
    float damage;
    Vector force;
    bool not_player_or_min_time_passed = true;
    Vector delta;
    float  magnitude;

    Collision(Unit* unit, const QVector &location, const Vector& normal);
    void Collision2(Matrix& from_new_reference, Matrix& to_new_reference,
                      Vector inelastic_vf, Collision& other_collision);
    void Collision3(Collision& other_collision);
    ReactionResult reactionMatrix(Unit* other_unit);
    void applyForce();
    bool shouldDealDamage(float radial_size = 0.0);
    void applyDamage(Collision other_collision);
    bool crashLand(Unit *base);
public:
    static void collide( Unit* unit1,
                           const QVector &location1,
                           const Vector &normal1,
                           Unit* unit2,
                           const QVector &location2,
                           const Vector &normal2,
                           float distance );


    void dealDamageFromCollision(Unit* unit, Unit* other_unit,
                                 const QVector &other_unit_location, // biglocation
                                 const Vector &other_unit_normal, // bignormal
                                 const QVector &unit_location, // smalllocation
                                 const Vector &unit_normal, // smallernormal
                                 float distance);
};

#endif // COLLISION_H
