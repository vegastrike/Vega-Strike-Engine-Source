#include "cmd/images.h"

template < typename BOGUS >
UnitImages<BOGUS>::UnitImages()
{
    VSCONSTRUCT1( 'i' )
    pHudImage = NULL;
    pExplosion = NULL;
}
template < typename BOGUS >
UnitImages<BOGUS>::~UnitImages()
{
    delete pExplosion;
//    delete pHudImage;
    VSDESTRUCT1
}

//explicit instantiation needed
template struct UnitImages<void>;

