#include "cmd/images.h"

//Refactors code to otherwise break the dependency to VSSprite for the VegaServer.
template < typename BOGUS >
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
