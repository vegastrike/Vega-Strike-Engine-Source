#ifndef _UNITCONTAINER_H_
#define _UNITCONTAINER_H_

#include "debug_vs.h"

class Unit;

class UnitContainer
{
protected:
    Unit *unit = nullptr;
public: UnitContainer();
    UnitContainer( Unit* );
    UnitContainer( const UnitContainer &un )
    {
        VSCONSTRUCT1( 'U' )
        unit = 0;
        SetUnit( un.unit );
    }
    const UnitContainer& operator=( const UnitContainer &a )
    {
        SetUnit( a.unit );
        return a;
    }
    bool operator==( const Unit *oth ) const
    {
        return unit == oth;
    }
    bool operator!=( const Unit *oth ) const
    {
        return unit != oth;
    }
    bool operator==( const UnitContainer &oth ) const
    {
        return unit == oth.unit;
    }
    bool operator!=( const UnitContainer &oth ) const
    {
        return unit != oth.unit;
    }
    ~UnitContainer();
    void SetUnit( Unit* );
    Unit * GetUnit();
    
    const Unit * GetConstUnit() const
    {
        return unit;
    }
};

#endif

