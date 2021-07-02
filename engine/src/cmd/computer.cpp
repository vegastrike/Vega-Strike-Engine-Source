
#include "computer.h"
#include "configuration/configuration.h"



Computer::Computer() : NavPoint( 0, 0, 0 ),
    ecmactive(true),
    velocity_ref(nullptr),
    force_velocity_ref(false),
    threatlevel( 0 ),
    set_speed( 0 ),
    max_combat_speed( 1 ),
    max_combat_ab_speed( 1 ),
    max_yaw_left( 1 ),
    max_yaw_right( 1 ),
    max_pitch_down( 1 ),
    max_pitch_up( 1 ),
    max_roll_left( 1 ),
    max_roll_right( 1 ),
    slide_start( 1 ),
    slide_end( 1 ),
    itts( false ),
    combat_mode( true ) {}

float Computer::max_speed() const
{
    static float combat_mode_mult = XMLSupport::parse_float( vs_config->getVariable( "physics", "combat_speed_boost", "100" ) );
    return (!combat_mode) ? combat_mode_mult*max_combat_speed : max_combat_speed;
}

float Computer::max_ab_speed() const
{
    static float combat_mode_mult = XMLSupport::parse_float( vs_config->getVariable( "physics", "combat_speed_boost", "100" ) );
    //same capped big speed as combat...else different
    return (!combat_mode) ? combat_mode_mult*max_combat_speed : max_combat_ab_speed;
}

Computer::RADARLIM::RADARLIM() :
    maxrange(0),
    maxcone(-1),
    lockcone(0),
    trackingcone(0),
    mintargetsize(0),
    capability(Capability::IFF_NONE),
    locked(false),
    canlock(false),
    trackingactive(true) {

    maxrange      = configuration.computer.default_max_range;
    trackingcone  = configuration.computer.default_tracking_cone;
    lockcone      = configuration.computer.default_lock_cone;
}

Computer::RADARLIM::Brand::Value Computer::RADARLIM::GetBrand() const
{
    switch (capability & Capability::IFF_UPPER_MASK)
    {
    case Capability::IFF_SPHERE:
        return Brand::SPHERE;
    case Capability::IFF_BUBBLE:
        return Brand::BUBBLE;
    case Capability::IFF_PLANE:
        return Brand::PLANE;
    default:
        assert(false);
        return Brand::SPHERE;
    }
}

bool Computer::RADARLIM::UseFriendFoe() const
{
    // Backwardscompatibility
    if (capability == 0)
        return false;
    else if ((capability == 1) || (capability == 2))
        return true;

    return (capability & Capability::IFF_FRIEND_FOE);
}

bool Computer::RADARLIM::UseObjectRecognition() const
{
    // Backwardscompatibility
    if ((capability == 0) || (capability == 1))
        return false;
    else if (capability == 2)
        return true;

    return (capability & Capability::IFF_OBJECT_RECOGNITION);
}

bool Computer::RADARLIM::UseThreatAssessment() const
{
    return (capability & Capability::IFF_THREAT_ASSESSMENT);
}
