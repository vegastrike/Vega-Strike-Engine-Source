#include "cmd/images.h"
#include "gfx/cockpit_generic.h"

template < typename BOGUS >
UnitImages<BOGUS>::UnitImages()
{
    VSCONSTRUCT1('i');

    //No cockpit reference here
    if (!cockpit_damage) {
        unsigned int numg = (1+MAXVDUS+UnitImages< void >::NUMGAUGES)*2;
        cockpit_damage = (float*) malloc( (numg)*sizeof (float) );
        for (unsigned int damageiterator = 0; damageiterator < numg; ++damageiterator)
            cockpit_damage[damageiterator] = 1;
    }
}


template < typename BOGUS > //added by chuck starchaser, to try to break dependency to VSSprite in vegaserver
UnitImages<BOGUS>::~UnitImages()
{
    if (pExplosion)
        delete pExplosion;
    if (pHudImage)
        delete pHudImage;
    VSDESTRUCT1
}

//explicit instantiation needed
template struct UnitImages<void>;

static UnitImages<void> bleh;

