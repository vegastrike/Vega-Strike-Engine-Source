#include "building.h"
#include "cont_terrain.h"

Building::Building( ContinuousTerrain *parent,
                            bool vehicle,
                            const char *filename,
                            bool SubUnit,
                            int faction,
                            const string &modifications,
                            Flightgroup *fg ) : GameUnit( filename, SubUnit, faction, modifications, fg )
{
    this->vehicle = vehicle;
    continuous    = true;
    this->parent.plane = parent;
}

Building::Building( Terrain *parent,
                            bool vehicle,
                            const char *filename,
                            bool SubUnit,
                            int faction,
                            const string &modifications,
                            Flightgroup *fg ) : GameUnit( filename, SubUnit, faction, modifications, fg )
{
    this->vehicle = vehicle;
    continuous    = false;
    this->parent.terrain = parent;
}

void Building::UpdatePhysics2( const Transformation &trans,
                                   const Transformation &old_physical_state,
                                   const Vector &accel,
                                   float difficulty,
                                   const Matrix &transmat,
                                   const Vector &CumulativeVelocity,
                                   bool ResolveLast,
                                   UnitCollection *uc )
{
    GameUnit::UpdatePhysics2( trans,
                                          old_physical_state,
                                          accel,
                                          difficulty,
                                          transmat,
                                          CumulativeVelocity,
                                          ResolveLast,
                                          uc );
    QVector tmp( LocalPosition() );
    Vector  p, q, r;
    GetOrientation( p, q, r );
    if (continuous) {
            tmp = parent.plane->GetGroundPos( tmp, p );
    } else {
        parent.terrain->GetGroundPos( tmp, p, (float) 0, (float) 0 );
    }
    if (vehicle) {
        Normalize( p );
        Vector tmp1;
#if 0
        if (k <= 0) {
            tmp1 = Vector( 0, 0, 1 );
            if ( k = tmp1.Magnitude() )
                tmp1 *= 800./k;
        } else
#endif
        {
            tmp1 = 200*q.Cross( p );
        }
        NetLocalTorque += ( ( tmp1-tmp1*( tmp1.Dot( GetAngularVelocity() )/tmp1.Dot( tmp1 ) ) ) )*1./Mass;
    }
    SetCurPosition( tmp );
}

