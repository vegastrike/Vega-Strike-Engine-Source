
#include "computer.h"
#include "game_config.h"

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

Computer::RADARLIM::RADARLIM() {
    static float default_max_range = GameConfig::GetVariable( "graphics", "hud", "radarRange", 20000 );
    static float default_tracking_cone = GameConfig::GetVariable( "physics", "autotracking", 0.93 );     //DO NOT CHANGE see unit_customize.cpp

    // Note: below is probably a stale comment
    // DO NOT CHANGE see unit_customize.cpp
    static float default_lock_cone = GameConfig::GetVariable( "physics", "lock_cone", 0.8 );

    maxrange      = default_max_range;
    trackingcone  = default_tracking_cone;
    lockcone      = default_lock_cone;
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
