/*! Container
 * Sets up an object that will serve as the master container for all 
 * within.
 */

#include <stdlib.h>
#include "container.h"
#include "unit_generic.h"
UnitContainer::UnitContainer()
{
    unit = NULL;
    VSCONSTRUCT1( 'U' )
}
UnitContainer::UnitContainer( Unit *un ) : unit( NULL )
{
    SetUnit( un );
    VSCONSTRUCT1( 'U' );
}

//Delete the unit container.
UnitContainer::~UnitContainer()
{
    VSDESTRUCT1
    if (unit)
        unit->UnRef();
}
void UnitContainer::SetUnit( Unit *un )
{
	//If the unit is NULL (or) it is killed ->
    if (un != NULL ? un->Killed() == true : true) {
		//Then...Get rid of the unit.
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

