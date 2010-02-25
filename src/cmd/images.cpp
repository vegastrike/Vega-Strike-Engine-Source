#include "cmd/images.h"

template < typename BOGUS > //added by chuck starchaser, to try to break dependency to VSSprite in vegaserver
UnitImages<BOGUS>::UnitImages()
{
    VSCONSTRUCT1( 'i' )
    pHudImage = NULL;
    pExplosion = NULL;
}
template < typename BOGUS > //added by chuck starchaser, to try to break dependency to VSSprite in vegaserver
UnitImages<BOGUS>::~UnitImages()
{
    delete pExplosion;
    delete pHudImage;
    VSDESTRUCT1
}

//explicit instantiation needed
template struct UnitImages<void>;

