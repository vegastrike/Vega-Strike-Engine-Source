#include "unit_factory_generic.h"
#include "unit_generic.h"
#include "unit.h"
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

Unit* UnitFactory::createGenericUnit( const char *filename,
		               bool        SubUnit,
		               int         faction,
		               std::string customizedUnit,
		               Flightgroup *flightgroup,
		               int         fg_subnumber )
{
    return new GameUnit( filename,
                     SubUnit,
                     faction,
                     customizedUnit,
                     flightgroup,
                     fg_subnumber );
}

