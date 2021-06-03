#ifndef JUMPCAPABLE_H
#define JUMPCAPABLE_H

#include "star_system.h"
#include "energetic.h"

#include <string>

// This includes both spec (in-system FTL) and jump drives support.
// TODO: consider moving actual decision making code to actual module providing capability to ship
class JumpCapable
{
public:
    StarSystem *activeStarSystem;

public:
    void ActivateJumpDrive( int destination = 0 );
    void AddDestination( const std::string& );
    bool AutoPilotTo( Unit *un, bool automaticenergyrealloc );
    bool AutoPilotToErrorMessage( const Unit *un, bool automaticenergyrealloc,
                                  std::string &failuremessage, int recursive_level = 2 );
    float CalculateNearestWarpUnit( float minmultiplier, Unit **nearest_unit,
                                    bool count_negative_warp_units ) const;
    float CourseDeviation( const Vector &OriginalCourse, const Vector &FinalCourse ) const;
    void DeactivateJumpDrive();
    const std::vector< std::string >& GetDestinations() const;
    const Energetic::UnitJump& GetJumpStatus() const;
    StarSystem * getStarSystem();
    const StarSystem * getStarSystem() const;
    Vector GetWarpRefVelocity() const;
    Vector GetWarpVelocity() const;
    bool InCorrectStarSystem( StarSystem* );
    virtual bool TransferUnitToSystem( StarSystem *NewSystem );
    virtual bool TransferUnitToSystem( unsigned int whichJumpQueue,
                                       class StarSystem*&previouslyActiveStarSystem,
                                       bool DoSightAndSound );
};

#endif // JUMPCAPABLE_H
