#ifndef COMPUTER_H
#define COMPUTER_H

#include "container.h"
#include "config.h"
#include "vs_globals.h"
#include "configxml.h"

/**
 * The computer holds all data in the navigation computer of the current unit
 * It is outside modifyable with GetComputerData() and holds only volatile
 * Information inside containers so that destruction of containers will not
 * result in segfaults.
 * Maximum speeds and turning restrictions are merely facts of the computer
 * and have nothing to do with the limitations of the physical nature
 * of space combat
 */
class Computer
{
public:
    class RADARLIM
    {
    public:
        struct Brand
        {
            enum Value
            {
                SPHERE = 0,
                BUBBLE = 1,
                PLANE = 2
            };
        };
        struct Capability
        {
            enum Value
            {
                // For internal use
                IFF_UPPER_SHIFT = 16,
                IFF_LOWER_MASK = (1 << IFF_UPPER_SHIFT) - 1,
                IFF_UPPER_MASK = ~IFF_LOWER_MASK,

                // The lower 16 bits
                IFF_NONE               = 0,
                IFF_FRIEND_FOE         = 1 << 0,
                IFF_OBJECT_RECOGNITION = 1 << 1,
                IFF_THREAT_ASSESSMENT  = 1 << 2,

                // The upper 16 bits
                IFF_SPHERE = Brand::SPHERE << IFF_UPPER_SHIFT,
                IFF_BUBBLE = Brand::BUBBLE << IFF_UPPER_SHIFT,
                IFF_PLANE  = Brand::PLANE << IFF_UPPER_SHIFT
            };
        };
        //the max range the radar can handle
        float maxrange;
        //the dot with (0,0,1) indicating the farthest to the side the radar can handle.
        float maxcone = -1;
        float lockcone;
        float trackingcone;
        //The minimum radius of the target
        float mintargetsize = 0;
        // What kind of type and capability the radar supports
        int   capability = Computer::RADARLIM::Capability::IFF_NONE;
        bool  locked = false;
        bool  canlock;
        bool  trackingactive = true;
        RADARLIM() : maxrange( 0 )
          , maxcone( 0 )
          , lockcone( 0 )
          , trackingcone( 0 )
          , mintargetsize( 0 )
          , capability(Capability::IFF_NONE | Capability::IFF_SPHERE)
          , locked( false )
          , canlock( false )
        {}
        Brand::Value GetBrand() const;
        bool UseFriendFoe() const;
        bool UseObjectRecognition() const;
        bool UseThreatAssessment() const;
    }
    radar;
    bool ecmactive = true;
    //The nav point the unit may be heading for
    Vector NavPoint = Vector(0,0,0);
    //The target that the unit has in computer
    UnitContainer target;
    //Any target that may be attacking and has set this threat
    UnitContainer threat;
    //Unit that it should match velocity with (not speed) if null, matches velocity with universe frame (star)
    UnitContainer velocity_ref;
    bool  force_velocity_ref;
    //The threat level that was calculated from attacking unit's threat
    float threatlevel = 0;
    //The speed the flybywire system attempts to maintain
    float set_speed = 0;
    //Computers limitation of speed
    float max_combat_speed = 1;
    float max_combat_ab_speed = 1;
    //Computer's restrictions of YPR to limit space combat maneuvers
    float max_yaw_left = 1;
    float max_yaw_right = 1;
    float max_pitch_down = 1;
    float max_pitch_up = 1;
    float max_roll_left = 1;
    float max_roll_right = 1;
    //Whether or not an 'lead' indicator appears in front of target
    unsigned char slide_start = 0;
    unsigned char slide_end = 0;
    bool itts = false;
    //tells whether the speed is clamped draconian-like or not
    bool combat_mode = true;
    Computer() : NavPoint( 0, 0, 0 )
      , threatlevel( 0 )
      , set_speed( 0 )
      , max_combat_speed( 0 )
      , max_combat_ab_speed( 0 )
      , max_yaw_left( 0 )
      , max_yaw_right( 0 )
      , max_pitch_down( 0 )
      , max_pitch_up( 0 )
      , max_roll_left( 0 )
      , max_roll_right( 0 )
      , slide_start( 0 )
      , slide_end( 0 )
      , itts( false )
      , combat_mode( false ) {}

    float max_speed() const;
    float max_ab_speed() const;

};

#endif // COMPUTER_H
