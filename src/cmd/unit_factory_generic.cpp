#include "unit_factory.h"
#include "unit_generic.h"

Unit* UnitFactory::getMasterPartList( )
{
    if( _masterPartList == NULL )
    {
        _masterPartList = new GameUnit( "master_part_list",
	                            true,
				    FactionUtil::GetFaction("upgrades") );
    }
    return _masterPartList;
}

Unit* UnitFactory::_masterPartList = NULL;
