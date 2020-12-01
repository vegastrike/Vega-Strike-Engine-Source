#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "unit.h"
#include "savegame.h"
#include "universe.h"

class Enhancement : public GameUnit
{
public:
    Enhancement( const char *filename,
                     int faction,
                     const string &modifications,
                     Flightgroup *flightgrp = NULL,
                     int fg_subnumber = 0 ) :
        GameUnit( filename, false, faction, modifications, flightgrp, fg_subnumber )
    {
        string file( filename );
        this->filename = filename;
    }

protected:
    std::string filename;
    enum _UnitType isUnit() const
    {
        return _UnitType::enhancement;
    }

protected:
/// default constructor forbidden
    Enhancement();
/// copy constructor forbidden
    Enhancement( const Enhancement& );
/// assignment operator forbidden
    Enhancement& operator=( const Enhancement& );
};


#endif

