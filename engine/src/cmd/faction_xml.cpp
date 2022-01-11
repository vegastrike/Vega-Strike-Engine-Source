/**
* faction_xml.cpp
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#include <vector>
#include <string>
#include <gnuhash.h>
#include <expat.h>
#include "vegastrike.h"
#include "xml_support.h"
#include <assert.h>
#include "ai/communication.h"
#include "unit.h"
#include "cmd/music.h"
#include "faction_generic.h"

using namespace XMLSupport;

static int unitlevel;

namespace FactionXML
{
//

enum Names
{
    UNKNOWN,
    FACTIONS,
    FACTION,
    NAME,
    LOGORGB,
    LOGOA,
    SECLOGORGB,
    SECLOGOA,
    RELATION,
    STATS,
    FRIEND,
    ENEMY,
    CONVERSATION,
    COMM_ANIMATION,
    MOOD_ANIMATION,
    CONTRABAND,
    EXPLOSION,
    SEX,
    BASE_ONLY,
    DOCKABLE_ONLY,
    SPARKRED,
    SPARKGREEN,
    SPARKBLUE,
    SPARKALPHA,
    SHIPMODIFIER,
    ISCITIZEN
};

const EnumMap::Pair element_names[] = {
    EnumMap::Pair( "UNKNOWN",       UNKNOWN ),
    EnumMap::Pair( "Factions",      FACTIONS ),
    EnumMap::Pair( "Faction",       FACTION ),
    EnumMap::Pair( "Friend",        FRIEND ),
    EnumMap::Pair( "Enemy",         ENEMY ),
    EnumMap::Pair( "Stats",         STATS ),
    EnumMap::Pair( "CommAnimation", COMM_ANIMATION ),
    EnumMap::Pair( "MoodAnimation", MOOD_ANIMATION ),
    EnumMap::Pair( "Explosion",     EXPLOSION ),
    EnumMap::Pair( "ShipModifier",  SHIPMODIFIER )
};

const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair( "UNKNOWN",       UNKNOWN ),
    EnumMap::Pair( "name",          NAME ),
    EnumMap::Pair( "SparkRed",      SPARKRED ),
    EnumMap::Pair( "SparkGreen",    SPARKGREEN ),
    EnumMap::Pair( "SparkBlue",     SPARKBLUE ),
    EnumMap::Pair( "SparkAlpha",    SPARKALPHA ),
    EnumMap::Pair( "logoRGB",       LOGORGB ),
    EnumMap::Pair( "logoA",         LOGOA ),
    EnumMap::Pair( "secLogoRGB",    SECLOGORGB ),
    EnumMap::Pair( "secLogoA",      SECLOGOA ),
    EnumMap::Pair( "relation",      RELATION ),
    EnumMap::Pair( "Conversation",  CONVERSATION ),
    EnumMap::Pair( "Contraband",    CONTRABAND ),
    EnumMap::Pair( "sex",           SEX ),
    EnumMap::Pair( "base_only",     BASE_ONLY ),
    EnumMap::Pair( "dockable_only", DOCKABLE_ONLY ),
    EnumMap::Pair( "citizen",       ISCITIZEN )
};

const EnumMap element_map( element_names, 10 );

const EnumMap attribute_map( attribute_names, 17 );

//
}

static vector< std::string >contrabandlists;

void Faction::beginElement( void *userData, const XML_Char *names, const XML_Char **atts )
{
    using namespace FactionXML;
    AttributeList attributes = AttributeList( atts );
    string name     = names;
    AttributeList::const_iterator iter;
    Names  elem     = (Names) element_map.lookup( name );
    std::string secString;
    std::string secStringAlph;
    switch (elem)
    {
    case UNKNOWN:
        unitlevel++;
        return;
    case FACTIONS:
        assert( unitlevel == 0 );
        unitlevel++;
        break;
    case EXPLOSION:
        assert( unitlevel == 2 );
        unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case NAME:
                factions.back()->explosion.push_back( boost::shared_ptr<Animation>(FactionUtil::createAnimation( (*iter).value.c_str() )) );
                factions.back()->explosion_name.push_back( (*iter).value );
                break;
            }
        }
        break;
    case SHIPMODIFIER:
        assert( unitlevel == 2 );
        unitlevel++;
        {
            string name;
            float  rel = 0;
            for (iter = attributes.begin(); iter != attributes.end(); iter++) {
                switch ( attribute_map.lookup( (*iter).name ) )
                {
                case NAME:
                    name = (*iter).value;
                    break;
                case RELATION:
                    rel  = XMLSupport::parse_float( (*iter).value );
                    break;
                }
            }
            if ( rel != 0 && name.length() )
                factions.back()->ship_relation_modifier[name] = rel;
        }
        break;
    case COMM_ANIMATION:
        assert( unitlevel == 2 );
        unitlevel++;
        factions.back()->comm_faces.push_back( Faction::comm_face_t() );
        factions.back()->comm_face_sex.push_back( 0 );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case SEX:
                factions.back()->comm_face_sex.back() = parse_int( (*iter).value );
                break;
            case DOCKABLE_ONLY:
                factions.back()->comm_faces.back().dockable =
                    parse_bool( (*iter).value ) ? comm_face_t::CYES : comm_face_t::CNO;

                break;
            case BASE_ONLY:
                factions.back()->comm_faces.back().base = parse_bool( (*iter).value ) ? comm_face_t::CYES : comm_face_t::CNO;
                break;
            }
        }
        break;
    case MOOD_ANIMATION:
        assert( unitlevel == 3 );
        unitlevel++;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case NAME:
                factions.back()->comm_faces.back().animations.push_back( FactionUtil::createAnimation( (*iter).value.c_str() ) );
                break;
            }
        }
        break;
    case FACTION:
        assert( unitlevel == 1 );
        unitlevel++;
        factions.push_back(boost::shared_ptr<Faction>(new Faction()));
        assert( factions.size() > 0 );
        contrabandlists.push_back( "" );
        //factions[factions.size()-1];
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case SPARKRED:
                factions.back()->sparkcolor[0] = XMLSupport::parse_float( (*iter).value );
                break;
            case SPARKGREEN:
                factions.back()->sparkcolor[1] = XMLSupport::parse_float( (*iter).value );
                break;
            case SPARKBLUE:
                factions.back()->sparkcolor[2] = XMLSupport::parse_float( (*iter).value );
                break;
            case SPARKALPHA:
                factions.back()->sparkcolor[3] = XMLSupport::parse_float( (*iter).value );
                break;
            case NAME:
                factions[factions.size()-1]->factionname = new char[strlen( (*iter).value.c_str() )+1];

                strcpy( factions[factions.size()-1]->factionname, (*iter).value.c_str() );
                if (strcmp( factions[factions.size()-1]->factionname, "neutral" ) == 0)
                    FactionUtil::neutralfac = factions.size()-1;
                if (strcmp( factions[factions.size()-1]->factionname, "planets" ) == 0)
                    FactionUtil::planetfac = factions.size()-1;
                if (strcmp( factions[factions.size()-1]->factionname, "upgrades" ) == 0)
                    FactionUtil::upgradefac = factions.size()-1;
                break;
            case CONTRABAND:
                contrabandlists.back()   = ( (*iter).value );
                break;
            case ISCITIZEN:
                factions.back()->citizen = XMLSupport::parse_bool( (*iter).value );
                break;
            case LOGORGB:
                factions[factions.size()-1]->logoName = (*iter).value;
                break;
            case LOGOA:
                factions[factions.size()-1]->logoAlphaName     = (*iter).value;
                break;
            case SECLOGORGB:
                factions[factions.size()-1]->secondaryLogoName = secString;
                break;
            case SECLOGOA:
                factions[factions.size()-1]->secondaryLogoAlphaName = secStringAlph;
                break;
            }
        }
        break;
    case STATS:
    case FRIEND:
    case ENEMY:
        assert( unitlevel == 2 );
        unitlevel++;
        factions[factions.size()-1]->faction.push_back( faction_stuff() );
        assert( factions.size() > 0 );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case NAME:

                factions[factions.size()-1]->
                faction[factions[factions.size()-1]->faction.size()-1].stats.name =
                    new char[strlen( (*iter).value.c_str() )+1];

                strcpy( factions[factions.size()-1]->faction[factions[factions.size()-1]->faction.size()-1].stats.name,
                       (*iter).value.c_str() );
                break;
            case RELATION:
                factions[factions.size()
                         -1]->faction[factions[factions.size()-1]->faction.size()-1].relationship = parse_float(
                    (*iter).value );
                break;
            case CONVERSATION:
                factions[factions.size()
                         -1]->faction[factions[factions.size()-1]->faction.size()-1].conversation.reset(new FSM( (*iter).value ));
                break;
            }
        }
        break;
    default:
        break;
    }
}

void Faction::endElement( void *userData, const XML_Char *name )
{
    using namespace FactionXML;
    Names elem = (Names) element_map.lookup( name );
    switch (elem)
    {
    case UNKNOWN:
        unitlevel--;
        break;
    default:
        unitlevel--;
        break;
    }
}

void Faction::LoadXML( const char *filename, char *xmlbuffer, int buflength )
{
    using namespace FactionXML;
    using namespace VSFileSystem;
    using std::endl;
    using std::pair;    
    unitlevel = 0;
    VSFile    f;
    VSError   err;
    if (buflength == 0 || xmlbuffer == NULL) {
        BOOST_LOG_TRIVIAL(trace) << "FactionXML:LoadXML "<<filename<<endl;
        err = f.OpenReadOnly( filename, UnknownFile );
        if (err > Ok) {
            BOOST_LOG_TRIVIAL(trace) << "Failed to open '"<<filename<<"' this probably means it can't find the data directory"<<endl;
            assert( 0 );
        }
    }
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetUserData( parser, NULL );
    XML_SetElementHandler( parser, &Faction::beginElement, &Faction::endElement );
    if (buflength != 0 && xmlbuffer != NULL)
        XML_Parse( parser, xmlbuffer, buflength, 1 );
    else
        XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
    XML_ParserFree( parser );
    ParseAllAllies();

    // Results are cached to avoid looking for too many files
    typedef vsUMap< string, boost::shared_ptr<FSM> > Cache;
    Cache cache;

    std::string fileSuffix(".xml");
    std::string neutralName("neutral");
    boost::shared_ptr<FSM> neutralComm;
    neutralComm.reset(new FSM(neutralName + fileSuffix));
    cache.insert(Cache::value_type(neutralName, neutralComm));

    for (unsigned int i = 0; i < factions.size(); i++)
    {
        boost::shared_ptr<Faction> fact = factions[i];
        std::string myCommFile = fact->factionname + fileSuffix;
        boost::shared_ptr<FSM> myComm;
        Cache::iterator it = cache.find(myCommFile);
        if ( it != cache.end() )
        {
            myComm = it->second;
        }
        else
        {
            if (VSFileSystem::LookForFile(myCommFile, CommFile) <= Ok)
            {
                myComm.reset(new FSM(myCommFile));
            }
            else
            {
                myComm.reset();
            }
            cache.insert(Cache::value_type(myCommFile, myComm));
        }

        for (unsigned int j = 0; j < factions[i]->faction.size(); j++)
        {
            std::string jointCommFile = fact->factionname
                + std::string("to")
                + factions[j]->factionname
                + fileSuffix;
            boost::shared_ptr<FSM> jointComm;
            if (!fact->faction[j].conversation)
            {
                it = cache.find(jointCommFile);
                if ( it != cache.end() )
                {
                    jointComm = it->second;
                }
                else
                {
                    if (VSFileSystem::LookForFile(jointCommFile, CommFile) <= Ok)
                    {
                        jointComm.reset(new FSM(jointCommFile));
                    }
                    else
                    {
                        jointComm.reset();
                    }
                    cache.insert(Cache::value_type(jointCommFile, jointComm));
                }
                factions[i]->faction[j].conversation = jointComm
                    ? jointComm
                    : (myComm ? myComm : neutralComm);
            }
        }
    }
    char *munull = NULL;
    cache.clear();
    FactionUtil::LoadSerializedFaction( munull );
}

void FactionUtil::LoadContrabandLists()
{
    for (unsigned int i = 0; i < factions.size() && i < contrabandlists.size(); i++)
        if (contrabandlists[i].length() > 0)
            factions[i]->contraband.reset(new GameUnit( contrabandlists[i].c_str(), true, i ));
    contrabandlists.clear();
}

