#ifndef _ENHANCERGENERIC_H_
#define _ENHANCERGENERIC_H_
#include "cmd/unit_generic.h"
#include "savegame.h"
#include "universe.h"

class Enhancement : public Unit
{
    friend class UnitFactory;
protected:
    std::string filename;
    virtual enum clsptr isUnit() const
    {
        return ENHANCEMENTPTR;
    }
/// constructor only to be called by UnitFactory
    Enhancement( const char *filename,
                 int faction,
                 const string &modifications,
                 Flightgroup *flightgrp = NULL,
                 int fg_subnumber = 0 ) :
        Unit( filename, false, faction, modifications, flightgrp, fg_subnumber )
        , filename( filename ) {}
protected:
/// default constructor forbidden
    Enhancement() {}
    Enhancement( std::vector< Mesh* >m, bool b, int i ) : Unit( m, b, i ) {}
/// copy constructor forbidden
//Enhancement( const Enhancement& );
/// assignment operator forbidden
//Enhancement& operator=( const Enhancement& );
};

#endif

