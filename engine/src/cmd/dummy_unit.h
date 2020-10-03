#ifndef DUMMY_UNIT_H
#define DUMMY_UNIT_H

#include "unit_generic.h"

class DummyUnit : public Unit
{
public:
    DummyUnit();
    DummyUnit( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    // This is a hack. It won't work for more than one class
    virtual enum clsptr isUnit() const { return(MISSILEPTR);}
};

#endif // DUMMY_UNIT_H
