#include "faction_generic.h"
#include "cmd/unit_generic.h"

using namespace FactionUtil;

vector< boost::shared_ptr<Faction> >factions;  //the factions

void Faction::ParseAllAllies()
{
    //MSVC has a bug where it won't let you reuse variables intitliezed in the paramater list of the for loop
    //the work around is A.) make the scope of the variable in the function level or not to re-define it in subsequent loops
    unsigned int i = 0;
    for (i = 0; i < factions.size(); i++)
        factions[i]->ParseAllies( i );
    for (i = 0; i < factions.size(); i++)
        factions[i]->faction[i].relationship = 1;
}
void Faction::ParseAllies( unsigned int thisfaction )
{
    unsigned int i, j;
    vector< faction_stuff >tempvec;
    for (i = 0; i < faction.size(); i++)
        for (j = 0; j < factions.size(); j++)
            if (strcmp( faction[i].stats.name, factions[j]->factionname ) == 0) {
                delete[] faction[i].stats.name;
                faction[i].stats.index = j;
                break;
            }
    for (i = 0; i < factions.size(); i++) {
        tempvec.push_back( faction_stuff() );
        tempvec[i].stats.index  = i;
        tempvec[i].relationship = ( (i == thisfaction) ? 1 : 0 );
    }
    for (i = 0; i < faction.size(); i++) {
        faction_stuff::faction_name tmp = tempvec[faction[i].stats.index].stats;
        tempvec[faction[i].stats.index] = faction[i];
        tempvec[faction[i].stats.index].stats = tmp;
    }
    faction.swap( tempvec );

}

