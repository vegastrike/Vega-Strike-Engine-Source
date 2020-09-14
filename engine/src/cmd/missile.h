#ifndef MISSILE_H_
#define MISSILE_H_

#include "unit.h"
#include "cmd/unit_util.h"

class DummyUnit;

class MissileEffect {
private:
    QVector pos;
    float  damage;
    float  phasedamage;
    float  radius;
    float  radialmultiplier;
    void  *ownerDoNotDereference;

public:
    MissileEffect( const QVector &pos, float dam, float pdam, float radius, float radmult, void *owner );
    void ApplyDamage( Unit* );
    const QVector& GetCenter() const;
    float GetRadius() const;

private:
    void DoApplyDamage( Unit *parent, Unit *un, float distance, float damage_fraction );
};

class Missile : public GameUnit< DummyUnit >
{
protected:
    float time;
    float damage;
    float phasedamage;
    float radial_effect;
    float radial_multiplier;
    float detonation_radius;
    bool  discharged;
    bool  had_target;
    signed char retarget;
public:
/// constructor only to be called by UnitFactory
    Missile( const char *filename,
                 int faction,
                 const string &modifications,
                 const float damage,
                 float phasedamage,
                 float time,
                 float radialeffect,
                 float radmult,
                 float detonation_radius );

    Missile( std::vector< Mesh* >m, bool b, int i) :
        GameUnit(m, b, i) {}

    friend class UnitFactory;

public:
    virtual void Kill( bool erase = true );
    void Discharge();
    float ExplosionRadius();

    virtual enum clsptr isUnit() const;

    virtual void UpdatePhysics2( const Transformation &trans,
                                 const Transformation &old_physical_state,
                                 const Vector &accel,
                                 float difficulty,
                                 const Matrix &transmat,
                                 const Vector &CumulativeVelocity,
                                 bool ResolveLast,
                                 UnitCollection *uc = NULL );

    Unit* breakECMLock(Unit* target);
    bool proximityFuse(Unit* target);
    bool useFuel(Unit* target, bool had_target);

private:
    // TODO: consider if this is really necessary and if so, use = delete
/// default constructor forbidden
    Missile();
/// copy constructor forbidden
    Missile( const Missile& );
/// assignment operator forbidden
    Missile& operator=( const Missile& );
};

#endif

