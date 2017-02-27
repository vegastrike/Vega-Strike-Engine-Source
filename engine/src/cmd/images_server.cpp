#include "cmd/images.h"

template < typename BOGUS >
UnitImages<BOGUS>::~UnitImages()
{
    if (pExplosion)
        delete pExplosion;
    VSDESTRUCT1
}

//explicit instantiation needed
template struct UnitImages<void>;
static UnitImages<void> bleh;