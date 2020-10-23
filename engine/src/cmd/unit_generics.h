#ifndef UNIT_GENERICS_H
#define UNIT_GENERICS_H

#include "unit_generic.h"

class AsteroidGeneric : public Unit
{
public:
    AsteroidGeneric() {};
    AsteroidGeneric( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    virtual enum clsptr isUnit() const { return ASTEROIDPTR; }
    friend class UnitFactory;
};

class BuildingGeneric : public Unit
{
public:
    BuildingGeneric() {};
    BuildingGeneric( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    virtual enum clsptr isUnit() const { return BUILDINGPTR; }
    friend class UnitFactory;
};

class EnhancementGeneric : public Unit
{
public:
    EnhancementGeneric() {};
    EnhancementGeneric( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    virtual enum clsptr isUnit() const { return ENHANCEMENTPTR; }
    friend class UnitFactory;
};

class MissileGeneric : public Unit
{
public:
    MissileGeneric() {};
    MissileGeneric( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    virtual enum clsptr isUnit() const { return MISSILEPTR; }
    friend class UnitFactory;
};

class NebulaGeneric : public Unit
{
public:
    NebulaGeneric() {};
    NebulaGeneric( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    virtual enum clsptr isUnit() const { return NEBULAPTR; }
    friend class UnitFactory;
};

class PlanetGeneric : public Unit
{
public:
    PlanetGeneric() {};
    PlanetGeneric( std::vector< Mesh* >m, bool b, int i) :
        Unit(m, b, i) {}

    virtual enum clsptr isUnit() const { return PLANETPTR; }
    friend class UnitFactory;
};

#endif // UNIT_GENERICS_H
