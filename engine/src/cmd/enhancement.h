#ifndef _ENHANCER_H_
#define _ENHANCER_H_
#include "unit.h"
#include "savegame.h"
#include "unit_generics.h"
#include "universe.h"

class Enhancement : public GameUnit< EnhancementGeneric >
{
public:
    Enhancement( const char *filename,
                     int faction,
                     const string &modifications,
                     Flightgroup *flightgrp = NULL,
                     int fg_subnumber = 0 ) :
        GameUnit< EnhancementGeneric > ( filename, false, faction, modifications, flightgrp, fg_subnumber )
    {
        string file( filename );
        this->filename = filename;
    }

protected:
    std::string filename;
    virtual enum clsptr isUnit() const
    {
        return ENHANCEMENTPTR;
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

