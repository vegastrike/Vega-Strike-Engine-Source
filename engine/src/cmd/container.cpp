#include <stdlib.h>
#include "container.h"
#include "unit_generic.h"
UnitContainer::UnitContainer() : unit( nullptr )
{
    VSCONSTRUCT1( 'U' )
}
UnitContainer::UnitContainer( Unit *un ) : unit( nullptr )
{
    SetUnit( un );
    VSCONSTRUCT1( 'U' );
}
UnitContainer::~UnitContainer()
{
    VSDESTRUCT1
    if (unit)
        unit->UnRef();
    //bad idea...arrgh!
}
void UnitContainer::SetUnit( Unit *un )
{
    //if the unit is null then go here otherwise if the unit is killed then go here
    if (un != NULL ? un->Killed() == true : true) {
        if (unit)
            unit->UnRef();
        unit = NULL;
        return;
    } else {
        if (unit)
            unit->UnRef();
        unit = un;
        unit->Ref();
    }
}

