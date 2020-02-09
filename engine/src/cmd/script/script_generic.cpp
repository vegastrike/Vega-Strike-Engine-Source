#include "xml_support.h"
#include "cmd/unit_generic.h"
#include "mission.h"

void Mission::call_unit_toxml( missionNode *node, int mode, varInst *ovi )
{
    Unit *my_object = getUnitObject( node, mode, ovi );
    if (my_object) {
        string fgid = my_object->getFgID();
    }
}

Unit* Mission::getUnitObject( missionNode *node, int mode, varInst *ovi )
{
    Unit *my_object = NULL;
    if (mode == SCRIPT_RUN)
        my_object = (Unit*) ovi->object;
    return my_object;
}

