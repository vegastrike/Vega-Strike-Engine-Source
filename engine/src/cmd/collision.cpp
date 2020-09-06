#include "collision.h"

#include "unit_generic.h"
#include "universe.h"
#include "universe_util.h"
#include "game_config.h"
#include "missile.h"
#include "enhancement.h"
#include <typeinfo>

static const float kilojoules_per_damage = GameConfig::GetVariable( "physics", "kilojoules_per_unit_damage", 5400 );
static const float collision_scale_factor =
        GameConfig::GetVariable( "physics", "collision_damage_scale", 1.0f );
static const float inelastic_scale = GameConfig::GetVariable( "physics", "inelastic_scale", 0.8f);
static const float min_time =
        GameConfig::GetVariable( "physics", "minimum_time_between_recorded_player_collisions", 0.1f);
static const float minimum_mass = 1e-6f;

//Collision force caps primarily for AI-AI collisions. Once the AIs get a real collision avoidance system, we can
// turn damage for AI-AI collisions back on, and then we can remove these caps.
//value, in seconds of desired maximum recovery time
static const float max_torque_multiplier =
        GameConfig::GetVariable( "physics", "maxCollisionTorqueMultiplier", 0.67f);
//value, in seconds of desired maximum recovery time
static const float max_force_multiplier  =
        GameConfig::GetVariable( "physics", "maxCollisionForceMultiplier", 5);

static const int upgrade_faction =
        GameConfig::GetVariable( "physics", "cargo_deals_collide_damage",
                                                        false) ? -1 : FactionUtil::GetUpgradeFaction();

static const float collision_hack_distance =
        GameConfig::GetVariable( "physics", "collision_avoidance_hack_distance", 10000);
static const float front_collision_hack_distance =
        GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_distance", 200000);

static float front_collision_hack_angle = cos( 3.1415926536f * GameConfig::GetVariable( "physics", "front_collision_avoidance_hack_angle", 40)/180.0f );

static const bool collision_damage_to_ai = GameConfig::GetVariable( "physics", "collisionDamageToAI", false);

static const bool crash_dock_unit = GameConfig::GetVariable( "physics", "unit_collision_docks", false);

static const bool crash_dock_hangar = GameConfig::GetVariable( "physics", "only_hangar_collision_docks", false);

// Disabled bouncing missile option. Missiles always explode when colliding with something.
//static const bool does_missile_bounce = GameConfig::GetVariable( "physics", "missile_bounce", false);



Collision::Collision(Unit* unit, const QVector& location, const Vector& normal):
    unit(unit), location(location), normal(normal)
{
    cockpit = _Universe->isPlayerStarship( unit ); // smcp/thcp
    unit_type = unit->isUnit();
    is_player_ship = _Universe->isPlayerStarship(unit);
    mass = std::max(unit->GetMass(), minimum_mass);
    position = unit->Position();
    velocity = unit->GetVelocity();
    angular_velocity = unit->GetAngularVelocity();
    linear_velocity = velocity - angular_velocity.Cross( location - position );
}


void Collision::Collision2(Matrix& from_new_reference, Matrix& to_new_reference,
                           Vector inelastic_vf, Collision& other_data)
{
    aligned_velocity = Transform( to_new_reference, linear_velocity );

    //compute along aligned dimension, then return to previous reference frame
    aligned_velocity.k = (aligned_velocity.k*(mass-other_data.mass)/(mass+other_data.mass)+( 2.0f*other_data.mass/(mass+other_data.mass) )*other_data.aligned_velocity.k);

    elastic_vf = Transform( from_new_reference, aligned_velocity );
    final_velocity = inelastic_scale * inelastic_vf + (1.0f - inelastic_scale) * elastic_vf;
    delta_e = (0.5f)*mass*(final_velocity-elastic_vf).MagnitudeSquared();
}


void Collision::Collision3(Collision& other_data)
{
    //Damage distribution (NOTE: currently arbitrary - no known good model for calculating how much energy object endures as a result of the collision)
    damage = (0.25f*delta_e+0.75f*other_data.delta_e)/kilojoules_per_damage*collision_scale_factor;

    //Vector ThisDesiredVelocity = ThisElastic_vf*(1-inelastic_scale/2)+Inelastic_vf*inelastic_scale/2;
    //Vector SmallerDesiredVelocity = SmallerElastic_vf*(1-inelastic_scale)+Inelastic_vf*inelastic_scale;
    //FIXME need to resolve 2 problems -
    //1) simulation_atom_var for small != simulation_atom_var for large (below smforce line should mostly address this)
    //2) Double counting due to collision occurring for each object in a different physics frame.
    force = (final_velocity - velocity) * mass
            / ( simulation_atom_var * ((float)unit->sim_atom_multiplier)
                / ( (float) other_data.unit->sim_atom_multiplier ));

    if (cockpit)
    {
        if ( (getNewTime()-cockpit->TimeOfLastCollision) > min_time )
            cockpit->TimeOfLastCollision = getNewTime();
        else
            not_player_or_min_time_passed = false;
    }

    delta = ( _Universe->AccessCamera()->GetPosition()-position ).Cast();
    magnitude = delta.Magnitude();
}


// This function handles the initial reaction of the unit to hitting the other unit
// Note: I'm changing the expected behavior here
// Return value indicates whether to continue processing
/*UNITPTR,
PLANETPTR,
BUILDINGPTR,
NEBULAPTR,
ASTEROIDPTR,
ENHANCEMENTPTR,
MISSILEPTR*/
Collision::ReactionResult Collision::reactionMatrix(Unit* other_unit)
{
    ReactionResult result = ReactionResult(false, false);
    // Collision with a nebula does nothing
    if(other_unit->isUnit() == NEBULAPTR)
    {
        return ReactionResult(false, false);
    }

    // Collision with a enhancement improves your shield apparently
    if(other_unit->isUnit() == ENHANCEMENTPTR)
    {
        return ReactionResult(true, false);
    }


    switch(unit_type)
    {
    // Missiles and asteroids always explode on impact with anything except Nebula and Enhancement.
    case MISSILEPTR:
        // Missile should explode when killed
        // If not, uncomment this
        //((Missile*)unit)->Discharge();
        unit->Kill();
        return ReactionResult(false, false);

    case ASTEROIDPTR:
        unit->Kill();
        return ReactionResult(false, false);

    // Planets and Nebulas can't be killed right now
    case PLANETPTR:
    case NEBULAPTR:
        return ReactionResult(false, false);

    // Buildings should not calculate actual damage
    case BUILDINGPTR:
        return ReactionResult(false, false);

    // Units (ships) should calculate actual damage
    case UNITPTR:
        return ReactionResult(true, true);

    // Not sure what an enhancement is, but it looks like it's something that can increase the shields of the unit it collided with.
    case ENHANCEMENTPTR:
        if (other_unit->isUnit() == ASTEROIDPTR)
        {
            return ReactionResult(true, false);
        }

        Enhancement* enhancement = dynamic_cast<Enhancement*>(unit);

        double percent;
        char tempdata[sizeof (unit->shield)];
        memcpy( tempdata, &unit->shield, sizeof (unit->shield) );
        unit->shield.number = 0;     //don't want them getting our boosted shields!
        unit->shield.shield2fb.front = unit->shield.shield2fb.back =
                unit->shield.shield2fb.frontmax = unit->shield.shield2fb.backmax = 0;
        other_unit->Upgrade( unit, 0, 0, true, true, percent );
        memcpy( &unit->shield, tempdata, sizeof (unit->shield) );
        string fn( unit->filename );
        string fac( FactionUtil::GetFaction( unit->faction ) );
        unit->Kill();
        _Universe->AccessCockpit()->savegame->AddUnitToSave( fn.c_str(), ENHANCEMENTPTR, fac.c_str(), (long) unit );
        return ReactionResult(true, false);
    }
}

// This stops the unit from going through the other unit
void Collision::applyForce()
{
    //for torque... location -- approximation hack of MR^2 for rotational inertia (moment of inertia currently just M)
    Vector torque = force/(unit->radial_size*unit->radial_size);
    Vector force  = this->force-torque; // Note the variable shadowing

    float  max_force  = max_force_multiplier*(unit->limits.forward + unit->limits.retro + unit->limits.lateral + unit->limits.vertical);
    float  max_torque = max_torque_multiplier*(unit->limits.yaw + unit->limits.pitch+unit->limits.roll);

    //Convert from frames to seconds, so that the specified value is meaningful
    max_force  = max_force / (unit->sim_atom_multiplier * simulation_atom_var);
    max_torque = max_torque / (unit->sim_atom_multiplier * simulation_atom_var);
    float torque_magnitude = torque.Magnitude();
    float force_magnitude = force.Magnitude();
    if (torque_magnitude > max_torque) {
        torque *= (max_torque/torque_magnitude);
    }
    if (force_magnitude > max_force) {
        force *= (max_force/force_magnitude);
    }
    unit->ApplyTorque( torque, location );
    unit->ApplyForce( force-torque );
}


// This needs careful review
// I have no idea what this does or why
// TODO: review if doesn't work and review anyway at some point
bool Collision::shouldDealDamage(float radial_size)
{
    // I think this is another way of checking if the ship is a player ship
    if(cockpit != nullptr)
    {
        return true;
    }

    if (magnitude <= collision_hack_distance + radial_size)
    {
        return true;
    }

    if (magnitude <= front_collision_hack_distance + radial_size)
    {
        return true;
    }

    return false;
}

void Collision::applyDamage(Collision other_collision)
{
    if(unit->faction == upgrade_faction) {
        return;
    }

    unit->ApplyDamage(other_collision.location.Cast(),
                      other_collision.normal,
                      damage, unit, GFXColor( 1,1,1,2 ), other_collision.unit->owner
                      != nullptr ? other_collision.unit->owner : this );
}


// This plays a docking sound
extern void abletodock( int dock );

// Some variants (e.g. privateer) automatically land when colliding with a planet/station
// Return true if successfully docked
// this->unit is the one attempting to land
bool Collision::crashLand(Unit *base)
{
    if (!crash_dock_unit) {
        return false;
    }

    if (UnitUtil::getFlightgroupName( base ) != "Base") {
        return false;
    }

    // Set docking port
    int docking_port = base->CanDockWithMe( unit, !crash_dock_hangar );
    if(docking_port == -1)
    {
        return false;
    }

    if (unit->ForceDock( base, static_cast<unsigned int>(docking_port)) <= 0)
    {
        return false;
    }

    // Set docking position
    QVector docking_position = UniverseUtil::SafeEntrancePoint( unit->Position(), unit->rSize()*1.5 );
    unit->SetPosAndCumPos( docking_position );

    // This plays a docking sound
    // TODO: really doesn't belong here. Refactor somehow.
    abletodock( 3 );

    // Probably does not belong here
    // Set the upgrade interface of a planet or base
    unit->UpgradeInterface( base );
}

// Discussion - the original code ran this once for both units. This required a comprison of the units to find out which is smaller.
void Collision::collide( Unit* unit1,
                                   const QVector &location1,
                                   const Vector &normal1,
                                   Unit* unit2,
                                   const QVector &location2,
                                   const Vector &normal2,
                                   float distance)
{
    std::cout << "Collision between " << typeid(unit1).name() << " and " << typeid(unit2).name() << "\n";

    Collision collision1 = Collision(unit1, location1, normal1);
    Collision collision2 = Collision(unit2, location2, normal2);

    // Try crash landing both ways
    if(collision1.crashLand(unit2)) {
        return;
    } else if(collision2.crashLand(unit1)) {
        return;
    }

    // If either unit jumps as a result, we avoid a collision
    // We don't check which unit is smaller
    // But jumpReactToCollision does supposedly
    if ( unit1->jumpReactToCollision( unit2 ) ||
         unit2->jumpReactToCollision( unit1 ) )
    {
        return;
    }

    ReactionResult reaction1 = collision1.reactionMatrix(unit2);
    ReactionResult reaction2 = collision2.reactionMatrix(unit1);

    // Here we calculate a whole bunch of things but don't do anything with them yet.
    // Compute reference frame conversions to align along force normals (newZ)(currently using bignormal
    // - will experiment to see if both are needed for sufficient approximation)
    Vector orthoz = ( (collision2.mass * normal2)-(collision1.mass * normal1) ).Normalize();
    Vector orthox = MakeNonColinearVector( orthoz );
    Vector orthoy( 0, 0, 0 );

    // Need z and non-colinear x to compute new basis trio. destroys x,y, preserves z.
    Orthogonize( orthox, orthoy, orthoz );

    // Transform matrix from normal aligned space
    Matrix from_new_reference( orthox, orthoy, orthoz );
    Matrix to_new_reference = from_new_reference;

    // Transform matrix to normal aligned space
    from_new_reference.InvertRotationInto( to_new_reference );

    //Compute elastic and inelastic terminal velocities (point object approximation)
    //doesn't need aligning (I think)
    Vector inelastic_vf = ( collision1.mass/(collision1.mass+collision2.mass) ) * collision1.linear_velocity + ( collision2.mass/(collision1.mass + collision2.mass) ) * collision2.linear_velocity;

    collision1.Collision2(from_new_reference, to_new_reference, inelastic_vf, collision2);
    collision2.Collision2(from_new_reference, to_new_reference, inelastic_vf, collision1);

    collision1.Collision3(collision2);
    collision2.Collision3(collision1);

    bool is_not_player_or_has_been_min_time = collision1.not_player_or_min_time_passed & collision2.not_player_or_min_time_passed;

    // Apply force applies the motion effect of a collision
    if(reaction1.apply_force && is_not_player_or_has_been_min_time)
    {
        collision1.applyForce();
    }

    if(reaction2.apply_force && is_not_player_or_has_been_min_time)
    {
        collision2.applyForce();
    }

    // Begin damage
    if(!collision1.shouldDealDamage(collision2.unit->rSize()) &&
            !collision2.shouldDealDamage())
    {
            return;
    }

    // Disable damage to NPC ships because... stupid AI ships always crash into each other.
    if ( !collision_damage_to_ai && !collision1.is_player_ship && !collision2.is_player_ship &&
         collision1.unit_type != MISSILEPTR && collision2.unit_type != MISSILEPTR)
    {
        return;
    }

    // Now we start to act on the above calculations,
    // but only if we need to
    if(reaction1.deal_damage)
    {
        collision1.applyDamage(collision2);
    }
    if(reaction2.deal_damage)
    {
        collision2.applyDamage(collision1);
    }
}


//HACK ALERT:
//following code referencing minvel and time between collisions attempts
//to alleviate ping-pong problems due to collisions being detected
//after the player has penetrated the hull of another vessel because of discretization of time.
//this should eventually be replaced by instead figuring out where
//the point of collision should have occurred, and moving the vessels to the
//actual collision location before applying forces

//Need to incorporate normals of colliding polygons somehow, without overiding directions of travel.
//We'll use the point object approximation for the magnitude of damage, and then apply the force along the appropriate normals
//ThisElastic_vf=((ThisElastic_vf.Magnitude()>minvel||!thcp)?ThisElastic_vf.Magnitude():minvel)*smallnormal;
//SmallerElastic_vf=((SmallerElastic_vf.Magnitude()>minvel||!smcp)?SmallerElastic_vf.Magnitude():minvel)*bignormal;


//float LargeKE = (0.5)*m2*GetVelocity().MagnitudeSquared();
//float SmallKE = (0.5)*m1*smalle->GetVelocity().MagnitudeSquared();
//float FinalInelasticKE = Inelastic_vf.MagnitudeSquared()*(0.5)*(m1+m2);
//float InelasticDeltaKE = LargeKE +SmallKE - FinalInelasticKE;
//1/2Mass*deltavfromnoenergyloss^2
