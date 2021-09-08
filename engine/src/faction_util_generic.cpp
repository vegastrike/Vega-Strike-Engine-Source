/*
 * faction_util_generic.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>
#include "faction_generic.h"
// #include "vsfilesystem.h"
#include "vs_logging.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "gfx/cockpit_generic.h"
#include "cmd/unit_generic.h"
#include "universe.h"

#include "options.h"



using namespace FactionUtil;
int FactionUtil:: upgradefac = 0;
int FactionUtil:: planetfac  = 0;
int FactionUtil:: neutralfac = 0;
FSM* FactionUtil::GetConversation( int Myfaction, int TheirFaction )
{
    assert( factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction );
    return factions[Myfaction]->faction[TheirFaction].conversation.get();
}

const char* FactionUtil::GetFaction( int i )
{
    if ( i >= 0 && i < (int) factions.size() )
        return factions[i]->factionname;
    return "";
}

static int GetFactionLookup( const char *factionname )
{
#ifdef _MSC_VER
  #define strcasecmp stricmp
#endif
    for (unsigned int i = 0; i < factions.size(); i++)
        if (strcasecmp( factionname, factions[i]->factionname ) == 0)
            return i;
    return 0;
}

Unit* FactionUtil::GetContraband( int faction )
{
    return factions[faction]->contraband.get();
}
/**
 * Returns the relationship between myfaction and theirfaction
 * 1 is happy. 0 is neutral (btw 1 and 0 will not attack)
 * -1 is mad. <0 will attack
 */
int FactionUtil::GetFactionIndex( const string& name )
{
    static Hashtable< string, int, 47 >factioncache;
    int *tmp = factioncache.Get( name );
    if (tmp)
        return *tmp;
    int  i   = GetFactionLookup( name.c_str() );
    tmp  = new int;
    *tmp = i;
    factioncache.Put( name, tmp );
    return i;
}
bool FactionUtil::isCitizenInt( int faction )
{
    return factions[faction]->citizen;
}
bool FactionUtil::isCitizen( const std::string& name )
{
    return isCitizenInt( GetFactionIndex( name ) );
}
string FactionUtil::GetFactionName( int index )
{
    const char *tmp = GetFaction( index );
    if (tmp) return tmp;
    static std::string nullstr;
    return nullstr;
}

static bool isPlayerFaction( const int MyFaction )
{
    unsigned int numplayers = _Universe->numPlayers();
    for (unsigned int i = 0; i < numplayers; ++i) {
        Unit *un = _Universe->AccessCockpit( i )->GetParent();
        if (un)
            if (un->faction == MyFaction)
                return true;
    }
    return false;
}
void FactionUtil::AdjustIntRelation( const int Myfaction, const int TheirFaction, float factor, float rank )
{
    assert( factions[Myfaction]->faction[TheirFaction].stats.index == TheirFaction );
    if (strcmp( factions[Myfaction]->factionname, "neutral" ) != 0) {
        if (strcmp( factions[Myfaction]->factionname, "upgrades" ) != 0) {
            if (strcmp( factions[TheirFaction]->factionname, "neutral" ) != 0) {
                if (strcmp( factions[TheirFaction]->factionname, "upgrades" ) != 0) {
                    if (isPlayerFaction( TheirFaction ) || game_options.AllowNonplayerFactionChange) {
                        if (game_options.AllowCivilWar || Myfaction != TheirFaction) {
                            factions[Myfaction]->faction[TheirFaction].relationship += factor*rank;
                            if (factions[Myfaction]->faction[TheirFaction].relationship > 1 && game_options.CappedFactionRating)
                                factions[Myfaction]->faction[TheirFaction].relationship = 1;
                            if (factions[Myfaction]->faction[TheirFaction].relationship < game_options.min_relationship)
                                factions[Myfaction]->faction[TheirFaction].relationship = game_options.min_relationship;
                            if (!game_options.AllowNonplayerFactionChange)
                                factions[TheirFaction]->faction[Myfaction].relationship =
                                    factions[Myfaction]->faction[TheirFaction].relationship;                                                                                     //reflect if player
                        }
                    }
                }
            }
        }
    }
}
int FactionUtil::GetPlaylist( const int myfaction )
{
    return factions[myfaction]->playlist;     //can be -1
}
const float* FactionUtil::GetSparkColor( const int myfaction )
{
    return factions[myfaction]->sparkcolor;     //can be -1
}
unsigned int FactionUtil::GetNumFactions()
{
    return factions.size();
}
void FactionUtil::SerializeFaction( FILE *fp )
{
    for (unsigned int i = 0; i < factions.size(); i++) {
        for (unsigned int j = 0; j < factions[i]->faction.size(); j++)
            VSFileSystem::vs_fprintf( fp, "%g ", factions[i]->faction[j].relationship );
        VSFileSystem::vs_fprintf( fp, "\n" );
    }
}

string FactionUtil::SerializeFaction()
{
    char   temp[8192];
    string ret( "" );
    for (unsigned int i = 0; i < factions.size(); i++) {
        for (unsigned int j = 0; j < factions[i]->faction.size(); j++) {
            sprintf( temp, "%g ", factions[i]->faction[j].relationship );
            ret += string( temp );
        }
        sprintf( temp, "\n" );
        ret += string( temp );
    }
    return ret;
}

int FactionUtil::numnums( const char *str )
{
    int count = 0;
    for (int i = 0; str[i]; i++) {
        count += (str[i] >= '0' && str[i] <= '9') ? 1 : 0;
    }
    return count;
}

void FactionUtil::LoadSerializedFaction( FILE *fp )
{
    for (unsigned int i = 0; i < factions.size(); i++) {
        char *tmp  = new char[24*factions[i]->faction.size()];
        fgets( tmp, 24*factions[i]->faction.size()-1, fp );
        char *tmp2 = tmp;
        if (numnums( tmp ) == 0) {
            i--;
            continue;
        }
        for (unsigned int j = 0; j < factions[i]->faction.size(); j++) {
            if ( 1 != sscanf( tmp2, "%f ", &factions[i]->faction[j].relationship ) ) {
                VS_LOG(error, "err");
            }
            int  k = 0;
            bool founddig = false;
            while (tmp2[k]) {
                if ( isdigit( tmp2[k] ) ) {
                    founddig = true;
                }
                if ( founddig && (!isdigit( tmp2[k] ) && tmp2[k] != '.') ) {
                    break;
                }
                k++;
            }
            tmp2 += k;
            if (*tmp2 == '\r' || *tmp2 == '\n') {
                break;
            }
        }
        delete[] tmp;
    }
}

bool whitespaceNewline( char *inp )
{
    for (; *inp; inp++) {
        if (inp[0] == '\n' || inp[0] == '\r') {
            return true;
        }
        if (inp[0] != ' ' && inp[0] != '\t') {
            break;
        }
    }
    return false;
}

string savedFactions;

void FactionUtil::LoadSerializedFaction( char* &buf )
{
    if (buf == NULL) {
        char *bleh = strdup( savedFactions.c_str() );
        char *blah = bleh;
        LoadSerializedFaction( blah );
        free( bleh );
        return;
    }
    if (factions.size() == 0) {
        savedFactions = buf;
        return;
    }
    for (unsigned int i = 0; i < factions.size(); i++) {
        if (numnums( buf ) == 0)
            return;
        for (unsigned int j = 0; j < factions[i]->faction.size(); j++) {
            sscanf( buf, "%f ", &factions[i]->faction[j].relationship );
            int  k = 0;
            bool founddig = false;
            while (buf[k]) {
                if ( isdigit( buf[k] ) )
                    founddig = true;
                if ( founddig && (!isdigit( buf[k] ) && buf[k] != '.') )
                    break;
                k++;
            }
            buf += k;
            if ( whitespaceNewline( buf ) )
                break;
        }
    }
}

