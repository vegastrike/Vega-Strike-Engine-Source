#include "vegastrike.h"
#include <vector>
#include "beam.h"
#include "unit_generic.h"
#include "images.h"

Beam::Beam( const Transformation &trans, const weapon_info &clne, void *own, Unit *self, int sound ) : vlist( NULL )
    , Col( clne.r, clne.g, clne.b, clne.a )
{
    VSCONSTRUCT2( 'B' )
    listen_to_owner = false;     //warning this line of code is also present in beam.cpp change one, change ALL

    Init( trans, clne, own, self );
    impact = UNSTABLE;
}

Beam::~Beam()
{
    VSDESTRUCT2
#ifdef BEAMCOLQ
    RemoveFromSystem( true );
#endif
}

void Beam::Draw( const Transformation &trans, const Matrix &m, Unit *targ, float tracking_cone )
{
}

void Beam::ProcessDrawQueue() {}

