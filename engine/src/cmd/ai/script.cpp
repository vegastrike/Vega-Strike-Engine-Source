/**
* script.cpp
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#include "script.h"
#include "navigation.h"
#include "xml_support.h"
#include "flybywire.h"
#include <stdio.h>
#include <vector>
#include <stack>
#include "vsfilesystem.h"
#include "tactics.h"
#include "cmd/unit_generic.h"
#include "hard_coded_scripts.h"
#include "universe_util.h"
#include "configxml.h"
#include "universe.h"

#include <assert.h>

using namespace XMLSupport;

typedef vsUMap< string, CCScript* >HardCodedMap;
static HardCodedMap MakeHardCodedScripts()
{
    HardCodedMap tmp;
    typedef std::pair< std::string, CCScript* >MyPair;
    tmp.insert( MyPair( "loop around fast", &LoopAroundFast ) );
    tmp.insert( MyPair( "aggressive loop around fast", &AggressiveLoopAroundFast ) );
    tmp.insert( MyPair( "loop around slow", &LoopAroundSlow ) );
    tmp.insert( MyPair( "aggressive loop around slow", &AggressiveLoopAroundSlow ) );
    tmp.insert( MyPair( "loop around", &LoopAround ) );
    tmp.insert( MyPair( "aggressive loop around", &AggressiveLoopAround ) );
    tmp.insert( MyPair( "barrel roll", &BarrelRoll ) );
    tmp.insert( MyPair( "veer away", &VeerAway ) );
    tmp.insert( MyPair( "veer away itts", &VeerAwayITTS ) );
    tmp.insert( MyPair( "veer and turn away", &VeerAndVectorAway ) );
    tmp.insert( MyPair( "veer and vector away", &VeerAndVectorAway ) );
    tmp.insert( MyPair( "afterburn veer away", &AfterburnVeerAndTurnAway ) );
    tmp.insert( MyPair( "afterburn vector away", &AfterburnVeerAndVectorAway ) );
    tmp.insert( MyPair( "match velocity", &MatchVelocity ) );
    tmp.insert( MyPair( "fly straight", &FlyStraight ) );
    tmp.insert( MyPair( "fly straight afterburner", &FlyStraightAfterburner ) );
    tmp.insert( MyPair( "afterburn fly straight", &FlyStraightAfterburner ) );
    tmp.insert( MyPair( "do nothing", &DoNothing ) );
    tmp.insert( MyPair( "take off", &Takeoff ) );
    tmp.insert( MyPair( "coast to stop", &CoastToStop ) );
    tmp.insert( MyPair( "self destruct", &SelfDestruct ) );
    tmp.insert( MyPair( "take off every zig", &TakeoffEveryZig ) );
    tmp.insert( MyPair( "afterburn turn towards", &AfterburnTurnTowards ) );
    tmp.insert( MyPair( "afterburn turn towards itts", &AfterburnTurnTowardsITTS ) );
    tmp.insert( MyPair( "cloak", &CloakForScript ) );
    tmp.insert( MyPair( "evade", &AfterburnEvadeLeftRight ) );
    tmp.insert( MyPair( "kick stop", &Kickstop ) );
    tmp.insert( MyPair( "move to", &MoveTo ) );
    tmp.insert( MyPair( "shelton slide", &SheltonSlide ) );
    tmp.insert( MyPair( "skilled afterburner slide", &SkilledABSlide ) );
    tmp.insert( MyPair( "afterburner slide", &AfterburnerSlide ) );
    tmp.insert( MyPair( "stop", &Stop ) );
    tmp.insert( MyPair( "turn away", &TurnAway ) );
    tmp.insert( MyPair( "afterburn turn away", &AfterburnTurnAway ) );
    tmp.insert( MyPair( "turn towards", &TurnTowards ) );
    tmp.insert( MyPair( "turn towards itts", &TurnTowardsITTS ) );
    tmp.insert( MyPair( "drop cargo", &DropCargo ) );
    tmp.insert( MyPair( "drop half cargo", &DropHalfCargo ) );
    tmp.insert( MyPair( "drop one cargo", &DropOneCargo ) );
    tmp.insert( MyPair( "roll right", &RollRight ) );
    tmp.insert( MyPair( "roll right hard", &RollRightHard ) );
    tmp.insert( MyPair( "roll left", &RollLeft ) );
    tmp.insert( MyPair( "roll left hard", &RollLeftHard ) );
    tmp.insert( MyPair( "evade left right", &EvadeLeftRight ) );
    tmp.insert( MyPair( "evade up down", &EvadeUpDown ) );
    tmp.insert( MyPair( "afterburn evade left right", &AfterburnEvadeLeftRight ) );
    tmp.insert( MyPair( "afterburn evade up down", &AfterburnEvadeUpDown ) );
    tmp.insert( MyPair( "face perpendicular", &FacePerpendicular ) );
    tmp.insert( MyPair( "face perpendicular slow", &FacePerpendicularSlow ) );
    tmp.insert( MyPair( "face perpendicular fast", &FacePerpendicularFast ) );
    tmp.insert( MyPair( "roll perpendicular", &RollFacePerpendicular ) );
    tmp.insert( MyPair( "roll perpendicular slow", &RollFacePerpendicularSlow ) );
    tmp.insert( MyPair( "roll perpendicular fast", &RollFacePerpendicularFast ) );
    if (tmp.find("roll perpendicular fast")==tmp.end()){
        VSExit(1);
    }
    return tmp;
}

static HardCodedMap hard_coded_scripts = MakeHardCodedScripts();
bool validateHardCodedScript( std::string s )
{
    if (s.length() == 0) return true;
    return hard_coded_scripts.find( s ) != hard_coded_scripts.end();
}
struct AIScriptXML
{
    int     unitlevel;
    int     acc;
    vector< float >executefor;
    bool    itts;
    bool    afterburn;
    bool    terminate;
    char    lin;
    QVector defaultvec;
    float   defaultf;
    std::stack< QVector >vectors;
    std::stack< float >  floats;
    std::vector< Order* >orders;
};
float& AIScript::topf()
{
    if ( !xml->floats.size() ) {
        xml->floats.push( xml->defaultf );
        BOOST_LOG_TRIVIAL(error) << boost::format("ERROR: Float stack is empty... Will return %1%") % xml->defaultf;
    }
    return xml->floats.top();
}
void AIScript::popf()
{
    if (xml->floats.size() <= 0) {
        BOOST_LOG_TRIVIAL(error) << "ERROR: Float stack is empty... Will not delete";
        return;
    }
    xml->floats.pop();
}
QVector& AIScript::topv()
{
    if ( !xml->vectors.size() ) {
        xml->vectors.push( xml->defaultvec );
        BOOST_LOG_TRIVIAL(error) << boost::format(
                                  "ERROR: Vector stack is empty... Will return <%1%, %2%, %3%>")
                                  % xml->defaultvec.i
                                  % xml->defaultvec.j
                                  % xml->defaultvec.k;
    }
    return xml->vectors.top();
}
void AIScript::popv()
{
    if (xml->vectors.size() <= 0) {
        BOOST_LOG_TRIVIAL(error) << "ERROR: Vector stack is empty... Will not delete";
        return;
    }
    xml->vectors.pop();
}

void AIScript::beginElement( void *userData, const XML_Char *name, const XML_Char **atts )
{
    ( (AIScript*) userData )->beginElement( name, AttributeList( atts ) );
}

void AIScript::endElement( void *userData, const XML_Char *name )
{
    ( (AIScript*) userData )->endElement( name );
}

namespace AiXml
{
enum Names
{
    SCRIPT,
    MOVETO,
    VECTOR,
    FFLOAT,
    X,
    Y,
    Z,
    ACCURACY,
    UNKNOWN,
    EXECUTEFOR,
    TIME,
    AFTERBURN,
    CHANGEHEAD,
    MATCHLIN,
    MATCHANG,
    MATCHVEL,
    ANGULAR,
    LINEAR,
    LOCAL,
    TERMINATE,
    VALUE,
    ADD,
    SUB,
    NEG,
    NORMALIZE,
    SCALE,
    CROSS,
    DOT,
    MULTF,
    ADDF,
    FROMF,
    TOF,
    FACETARGET,
    ITTTS,
    TARGETPOS,
    THREATPOS,
    YOURPOS,
    TARGETV,
    THREATV,
    YOURV,
    TARGETWORLD,
    THREATWORLD,
    TARGETLOCAL,
    THREATLOCAL,
    YOURLOCAL,
    YOURWORLD,
    SIMATOM,
    DUPLIC,
    CLOAKFOR,
    DEFAULT
};

const EnumMap::Pair element_names[] = {
    EnumMap::Pair( "UNKNOWN",     UNKNOWN ),
    EnumMap::Pair( "Float",       FFLOAT ),
    EnumMap::Pair( "Script",      SCRIPT ),
    EnumMap::Pair( "Vector",      VECTOR ),
    EnumMap::Pair( "Moveto",      MOVETO ),
    EnumMap::Pair( "Default",     DEFAULT ),
    EnumMap::Pair( "Targetworld", TARGETWORLD ),
    EnumMap::Pair( "Yourworld",   YOURWORLD ),
    EnumMap::Pair( "Targetlocal", TARGETLOCAL ),
    EnumMap::Pair( "Yourlocal",   YOURLOCAL ),
    EnumMap::Pair( "FaceTarget",  FACETARGET ),
    EnumMap::Pair( "CloakFor",    CLOAKFOR ),
    EnumMap::Pair( "ExecuteFor",  EXECUTEFOR ),
    EnumMap::Pair( "ChangeHead",  CHANGEHEAD ),
    EnumMap::Pair( "MatchLin",    MATCHLIN ),
    EnumMap::Pair( "MatchAng",    MATCHANG ),
    EnumMap::Pair( "MatchVel",    MATCHVEL ),
    EnumMap::Pair( "Angular",     ANGULAR ),
    EnumMap::Pair( "Add",         ADD ),
    EnumMap::Pair( "Neg",         NEG ),
    EnumMap::Pair( "Sub",         SUB ),
    EnumMap::Pair( "Normalize",   NORMALIZE ),
    EnumMap::Pair( "Scale",       SCALE ),
    EnumMap::Pair( "Cross",       CROSS ),
    EnumMap::Pair( "Dot",         DOT ),
    EnumMap::Pair( "Multf",       MULTF ),
    EnumMap::Pair( "Addf",        ADDF ),
    EnumMap::Pair( "Fromf",       FROMF ),
    EnumMap::Pair( "Tof",         TOF ),
    EnumMap::Pair( "Linear",      LINEAR ),
    EnumMap::Pair( "Threatworld", THREATWORLD ),
    EnumMap::Pair( "Threatlocal", THREATLOCAL )
};
const EnumMap::Pair attribute_names[] = {
    EnumMap::Pair( "UNKNOWN",       UNKNOWN ),
    EnumMap::Pair( "accuracy",      ACCURACY ),
    EnumMap::Pair( "x",             X ),
    EnumMap::Pair( "y",             Y ),
    EnumMap::Pair( "z",             Z ),
    EnumMap::Pair( "Time",          TIME ),
    EnumMap::Pair( "Terminate",     TERMINATE ),
    EnumMap::Pair( "Local",         LOCAL ),
    EnumMap::Pair( "Value",         VALUE ),
    EnumMap::Pair( "ITTS",          ITTTS ),
    EnumMap::Pair( "Afterburn",     AFTERBURN ),
    EnumMap::Pair( "Position",      YOURPOS ),
    EnumMap::Pair( "TargetPos",     TARGETPOS ),
    EnumMap::Pair( "ThreatPos",     THREATPOS ),
    EnumMap::Pair( "Velocity",      YOURV ),
    EnumMap::Pair( "TargetV",       TARGETV ),
    EnumMap::Pair( "ThreatV",       THREATV ),
    EnumMap::Pair( "SimlationAtom", SIMATOM ),
    EnumMap::Pair( "Dup",           DUPLIC )
};

const EnumMap element_map( element_names, 32 );
const EnumMap attribute_map( attribute_names, 19 );
}


void AIScript::beginElement( const string &name, const AttributeList &attributes )
{
    using namespace AiXml;
    xml->itts = false;
    Unit *tmp;
#ifdef AIDBG
    BOOST_LOG_TRIVIAL(debug) << "0";
#endif
    Names elem = (Names) element_map.lookup( name );
#ifdef AIDBG
    BOOST_LOG_TRIVIAL(debug) << boost::format("1%1$x ") % &elem;
#endif
    AttributeList::const_iterator iter;
    switch (elem)
    {
    case DEFAULT:
        xml->unitlevel += 2;         //pretend it's at a reasonable level
        break;
    case UNKNOWN:
        xml->unitlevel++;
        return;
    case SCRIPT:
        xml->unitlevel++;
        break;
    case LINEAR:
        xml->lin = 1;
    case ANGULAR:
    case VECTOR:
        xml->unitlevel++;
        xml->vectors.push( QVector( 0, 0, 0 ) );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case X:
                topv().i = parse_float( (*iter).value );
                break;
            case Y:
                topv().j = parse_float( (*iter).value );
                break;
            case Z:
                topv().k = parse_float( (*iter).value );
                break;
            case DUPLIC:
#ifdef AIDBG
                BOOST_LOG_TRIVIAL(debug) << boost::format("1%1$x ") % &elem;
#endif
                xml->vectors.pop();                 //get rid of dummy vector pushed on above
                xml->vectors.push( xml->vectors.top() );
                break;
            case THREATPOS:
                if ( ( tmp = this->parent->Threat() ) ) {
                    topv() = ( tmp->Position() );
                } else {
                    if ( ( tmp = this->parent->Target() ) )
                        topv() = ( tmp->Position() );
                    else
                        topv() = (xml->defaultvec);
                }
                break;
            case TARGETPOS:
                if ( ( tmp = this->parent->Target() ) )
                    topv() = ( tmp->Position() );
                else
                    topv() = (xml->defaultvec);
                break;

            case YOURPOS:
                topv() = ( this->parent->Position() );
                break;

            case THREATV:
                if ( ( tmp = this->parent->Threat() ) ) {
                    topv() = ( tmp->GetVelocity() );
                } else {
                    if ( ( tmp = this->parent->Target() ) )
                        topv() = ( tmp->GetVelocity() );
                    else
                        topv() = (xml->defaultvec);
                }
                break;
            case TARGETV:
                if ( ( tmp = this->parent->Target() ) )
                    topv() = ( tmp->GetVelocity() );
                else
                    topv() = (xml->defaultvec);
                break;

            case YOURV:
                topv() = ( this->parent->GetVelocity() );
                break;
            }
        }
        break;
    case MOVETO:
        xml->unitlevel++;
        xml->acc = 2;
        xml->afterburn = true;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case AFTERBURN:
                xml->afterburn = parse_bool( (*iter).value );
            case ACCURACY:
                xml->acc = parse_int( (*iter).value );
                break;
            }
        }
        break;
    case FACETARGET:
#ifdef AIDBG
        BOOST_LOG_TRIVIAL(debug) << "ft";
#endif
        xml->unitlevel++;
        xml->acc  = 3;
        xml->itts = false;
        xml->afterburn = true;
        xml->terminate = true;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case TERMINATE:
                xml->terminate = parse_bool( (*iter).value );
                break;
            case ACCURACY:
                xml->acc  = parse_int( (*iter).value );
                break;
            case ITTTS:
                xml->itts = parse_bool( (*iter).value );
                break;
            }
        }
#ifdef AIDBG
        BOOST_LOG_TRIVIAL(debug) << "eft";
#endif

        break;

    case CHANGEHEAD:
        xml->unitlevel++;
        xml->acc = 2;
        xml->afterburn = true;
        xml->terminate = true;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case TERMINATE:
                xml->terminate = parse_bool( (*iter).value );
                break;
            case ACCURACY:
                xml->acc = parse_int( (*iter).value );
                break;
            }
        }
        break;
    case YOURPOS:
        xml->unitlevel++;
        xml->vectors.push( this->parent->Position() );

        break;
    case THREATWORLD:
        xml->unitlevel++;
        break;
    case TARGETWORLD:
        xml->unitlevel++;
        break;
    case THREATLOCAL:
        xml->unitlevel++;
        break;
    case TARGETLOCAL:
        xml->unitlevel++;
        break;
    case YOURLOCAL:
        xml->unitlevel++;
        break;
    case YOURWORLD:
        xml->unitlevel++;
        break;

    case NORMALIZE:
    case SCALE:
    case CROSS:
    case DOT:
    case MULTF:
    case ADDF:
    case FROMF:
    case TOF:
    case ADD:
    case SUB:
    case NEG:
        xml->unitlevel++;
        break;

    case FFLOAT:
        xml->unitlevel++;
        xml->floats.push( 0 );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case VALUE:
                topf() = parse_float( (*iter).value );
                break;
            case SIMATOM:
                topf() = SIMULATION_ATOM;
            case DUPLIC:
                xml->floats.pop();                 //get rid of dummy vector pushed on above
                xml->floats.push( xml->floats.top() );
                break;
            }
        }
        break;
    case MATCHLIN:
    case MATCHANG:
    case MATCHVEL:
#ifdef AIDBG
        BOOST_LOG_TRIVIAL(debug) << "mlv";
#endif

        xml->unitlevel++;
        xml->acc = 0;
        xml->afterburn = false;
        xml->terminate = true;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case AFTERBURN:
                xml->afterburn = parse_bool( (*iter).value );
                break;
            case TERMINATE:
                xml->terminate = parse_bool( (*iter).value );
                break;
            case LOCAL:
                xml->acc = parse_bool( (*iter).value );
                break;
            }
        }
#ifdef AIDBG
        BOOST_LOG_TRIVIAL(debug) << "emlv ";
#endif

        break;
    case CLOAKFOR:
        xml->unitlevel++;
        xml->executefor.push_back( 0 );
        xml->terminate = true;
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case TERMINATE:
                xml->terminate = parse_bool( (*iter).value );
                break;
            case TIME:
                xml->executefor.back() = parse_float( (*iter).value );
                break;
            }
        }
        break;
    case EXECUTEFOR:
        xml->unitlevel++;
        xml->executefor.push_back( 0 );
        for (iter = attributes.begin(); iter != attributes.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case TIME:
                xml->executefor.back() = parse_float( (*iter).value );
                break;
            }
        }
        break;

    default:

        break;
    }
}

void AIScript::endElement( const string &name )
{
    using namespace AiXml;
    QVector temp( 0, 0, 0 );
    Names   elem = (Names) element_map.lookup( name );
    Unit   *tmp;
    switch (elem)
    {
    case UNKNOWN:
        xml->unitlevel--;
        break;
    //Vector
    case THREATWORLD:
        xml->unitlevel++;
        if ( ( tmp = parent->Threat() ) ) {
            xml->vectors.push( tmp->ToWorldCoordinates( topv() ) );
        } else {
            if ( ( tmp = parent->Target() ) )
                xml->vectors.push( tmp->ToWorldCoordinates( topv() ) );
            else
                xml->vectors.push( xml->defaultvec );
        }
        break;
    case THREATLOCAL:
        xml->unitlevel++;
        if ( ( tmp = parent->Threat() ) ) {
            xml->vectors.push( tmp->ToLocalCoordinates( topv() ) );
        } else {
            if ( ( tmp = parent->Target() ) )
                xml->vectors.push( tmp->ToLocalCoordinates( topv() ) );
            else
                xml->vectors.push( xml->defaultvec );
        }
        break;

    case TARGETWORLD:
        xml->unitlevel++;
        if ( ( tmp = parent->Target() ) )
            xml->vectors.push( tmp->ToWorldCoordinates( topv() ) );
        else
            xml->vectors.push( xml->defaultvec );
        break;
    case TARGETLOCAL:
        xml->unitlevel++;
        if ( ( tmp = parent->Target() ) )
            xml->vectors.push( tmp->ToLocalCoordinates( topv() ) );
        else
            xml->vectors.push( xml->defaultvec );
        break;
    case YOURLOCAL:
        xml->unitlevel++;
        xml->vectors.push( this->parent->ToLocalCoordinates( topv() ) );
        break;
    case YOURWORLD:
        xml->unitlevel++;
        xml->vectors.push( this->parent->ToWorldCoordinates( topv() ) );
        break;

    case NORMALIZE:
        xml->unitlevel--;
        if (topv().i || topv().j || topv().k)
            topv().Normalize();
        break;
    case SCALE:
        xml->unitlevel--;
        topv() *= topf();
        popf();
        break;
    case CROSS:
        xml->unitlevel--;
        temp   = topv();
        popv();
        topv() = CrossProduct( topv(), temp );
        break;
    case DOT:
        xml->unitlevel--;
        xml->floats.push( 0 );
        temp   = topv();
        popv();
        topf() = DotProduct( topv(), temp );
        popv();
        break;
    case MULTF:
        xml->unitlevel--;
        temp.i  = topf();
        popf();
        topf() *= temp.i;
        break;
    case ADDF:
        xml->unitlevel--;
        temp.i  = topf();
        popf();
        topf() += temp.i;
        break;
    case FROMF:
        xml->unitlevel--;
        temp.i = topf();
        popf();
        temp.j = topf();
        popf();
        xml->vectors.push( QVector( temp.i, temp.j, topf() ) );
        popf();
        break;
    case TOF:
        xml->unitlevel--;
        xml->floats.push( topv().i );
        xml->floats.push( topv().j );
        xml->floats.push( topv().k );
        popv();
        break;
    case ADD:
        xml->unitlevel--;
        temp    = topv();
        popv();
        topv() += temp;
        break;
    case SUB:
        xml->unitlevel--;
        temp   = topv();
        popv();
        topv() = topv()-temp;
        break;
    case NEG:
        xml->unitlevel--;
        topv() = -topv();
        break;
    case MOVETO:
        BOOST_LOG_TRIVIAL(debug) << boost::format("Moveto <%1%,%2%,%3%>") % topv().i % topv().j % topv().k;
        xml->unitlevel--;
        xml->orders.push_back( new Orders::MoveTo( topv(), xml->afterburn, xml->acc ) );
        popv();
        break;
    case CHANGEHEAD:
        xml->unitlevel--;
        xml->orders.push_back( new Orders::ChangeHeading( topv(), xml->acc ) );
        popv();
        break;
    case MATCHANG:
        xml->unitlevel--;
        xml->orders.push_back( new Orders::MatchAngularVelocity( parent->ClampAngVel( topv() ), ( (bool) xml->acc ),
                                                                 xml->terminate ) );
        popv();
        break;
    case FACETARGET:
        xml->unitlevel--;
        if (xml->itts || parent->GetComputerData().itts)
            xml->orders.push_back( new Orders::FaceTargetITTS( xml->terminate,
                                                               (bool) xml->acc ) );
        else
            xml->orders.push_back( new Orders::FaceTarget( xml->terminate,
                                                           (bool) xml->acc ) );
        break;
    case MATCHLIN:
        xml->unitlevel--;
        xml->orders.push_back( new Orders::MatchLinearVelocity( parent->ClampVelocity( topv(),
                                                                                       xml->afterburn ), ( (bool) xml->acc ),
                                                                xml->afterburn, xml->terminate ) );
        popv();
        break;
    case MATCHVEL:
        xml->unitlevel--;
        temp = topv();
        popv();
        if (xml->lin == 1) {
            xml->orders.push_back( new Orders::MatchVelocity( parent->ClampVelocity( topv(),
                                                                                     xml->afterburn ),
                                                              parent->ClampAngVel( temp ), ( (bool) xml->acc ), xml->afterburn,
                                                              xml->terminate ) );
        } else {
            xml->orders.push_back( new Orders::MatchVelocity( parent->ClampVelocity( temp,
                                                                                     xml->afterburn ),
                                                              parent->ClampAngVel( topv() ), ( (bool) xml->acc ),
                                                              xml->afterburn, xml->terminate ) );
        }
        xml->lin = 0;
        popv();
        break;
    case EXECUTEFOR:
        xml->unitlevel--;
        if ( !xml->executefor.empty() ) {
            if (xml->executefor.back() > 0) {
                xml->orders[xml->orders.size()-1] = new Orders::ExecuteFor( xml->orders[xml->orders.size()-1], xml->executefor.back() );
                xml->executefor.pop_back();
            }
        }
        break;
    case CLOAKFOR:
        xml->unitlevel--;
        if ( !xml->executefor.empty() ) {
            xml->orders.push_back( new CloakFor( xml->terminate, xml->executefor.back() ) );
            xml->executefor.pop_back();
        }
        break;
    case DEFAULT:
        xml->unitlevel -= 2;
        xml->defaultvec = topv();
        popv();
        xml->defaultf   = topf();
        popf();
        break;
    default:
        xml->unitlevel--;
        break;
    }
}

void AIScript::LoadXML()
{
    static int aidebug = XMLSupport::parse_int( vs_config->getVariable( "AI", "debug_level", "0" ) );
    using namespace AiXml;
    using namespace VSFileSystem;
    string     full_filename = filename;
    bool doroll = false;
    HardCodedMap::const_iterator iter = hard_coded_scripts.find( full_filename );
    if (iter==hard_coded_scripts.end() && full_filename.length() > 5 && full_filename[0] == 'r' && full_filename[1] == 'o' && full_filename[2] == 'l'
        && full_filename[3] == 'l' && full_filename[4] == ' ') {
        doroll = true;
        full_filename = full_filename.substr( 5 );
        iter = hard_coded_scripts.find( full_filename );
    }
    if ( iter != hard_coded_scripts.end() ) {
        CCScript *myscript = (*iter).second;
        (*myscript)(this, parent);
        if (doroll) {
            unsigned int val = rand();
            if (val < RAND_MAX/4)
                RollRightHard( this, parent );
            else if (val < RAND_MAX/2)
                RollLeftHard( this, parent );
            else
                RollLeft( this, parent );
        }
        if (aidebug > 1) {
            BOOST_LOG_TRIVIAL(debug) << boost::format("%1% using hcs %2% for %3% threat %4%")
                                      % mission->getGametime() % filename % parent->name
                                      % parent->GetComputerData().threatlevel;
        }
        if ( _Universe->isPlayerStarship( parent->Target() ) ) {
            float value;
            static float game_speed = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
            static float game_accel = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_accel", "1" ) );
            {
                Unit *targ = parent->Target();
                if (targ) {
                    Vector PosDifference = ( targ->Position()-parent->Position() ).Cast();
                    float  pdmag = PosDifference.Magnitude();
                    value = ( pdmag-parent->rSize()-targ->rSize() );
                    float  myvel = pdmag > 0 ? PosDifference.Dot( parent->GetVelocity()-targ->GetVelocity() )/pdmag : 0;
                    if (myvel > 0)
                        value -= myvel*myvel/( 2*( parent->limits.retro/parent->getMass() ) );
                } else {
                    value = 10000;
                }
                value /= game_speed*game_accel;
            }
            if (aidebug > 0) {
                UniverseUtil::IOmessage( 0, parent->name, "all", string( "using script " )+string(
                                            filename )+" threat "+XMLSupport::tostring(
                                            parent->GetComputerData().threatlevel )+" dis "
                                        +XMLSupport::tostring( value ) );
            }
        }
        return;
    } else {
        if (aidebug > 1)
            BOOST_LOG_TRIVIAL(debug) << boost::format("using soft coded script %1%") % filename;
        if (aidebug > 0)
            UniverseUtil::IOmessage( 0, parent->name, "all", string( "FAILED(or missile) script " )+string(
                                        filename )+" threat "+XMLSupport::tostring( parent->GetComputerData().threatlevel ) );
    }
#ifdef AIDBG
    BOOST_LOG_TRIVIAL(debug) << "chd";
#endif

#ifdef AIDBG
    BOOST_LOG_TRIVIAL(debug) << "echd";
#endif
    VSFile    f;
    VSError   err = f.OpenReadOnly( filename, AiFile );
#ifdef AIDBG
    BOOST_LOG_TRIVIAL(debug) << "backup ";
#endif
    if (err > Ok) {
        BOOST_LOG_TRIVIAL(error) << boost::format("cannot find AI script %1%") % filename;
        if (hard_coded_scripts.find(filename)!=hard_coded_scripts.end()) {
            assert(0);
        }
        return;
    }
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << "nxml";
#endif
    xml = new AIScriptXML;
    xml->unitlevel  = 0;
    xml->terminate  = true;
    xml->afterburn  = true;
    xml->acc = 2;
    xml->defaultvec = QVector( 0, 0, 0 );
    xml->defaultf   = 0;
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << "parscrea";
#endif
    XML_Parser parser = XML_ParserCreate( NULL );
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << boost::format("usdat %1$x") % parser;
#endif
    XML_SetUserData( parser, this );
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << "elha";
#endif
    XML_SetElementHandler( parser, &AIScript::beginElement, &AIScript::endElement );
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << "do";
#endif
    XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << boost::format("%1$xxml_free") % parser;
    VSFileSystem::flushLogs();
#endif
    XML_ParserFree( parser );
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << "xml_freed";
#endif
    f.Close();
    for (unsigned int i = 0; i < xml->orders.size(); i++) {
#ifdef BIDBG
        BOOST_LOG_TRIVIAL(debug) << "parset";
#endif
        xml->orders[i]->SetParent( parent );
        EnqueueOrder( xml->orders[i] );
#ifdef BIDBG
        BOOST_LOG_TRIVIAL(debug) << "cachunkx";
#endif
    }
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << boost::format("xml%1$x") % xml;
    VSFileSystem::flushLogs();
#endif
    delete xml;
#ifdef BIDBG
    BOOST_LOG_TRIVIAL(debug) << "\\xml\n";
    VSFileSystem::flushLogs();
#endif
}

AIScript::AIScript( const char *scriptname ) : Order( Order::MOVEMENT|Order::FACING, STARGET )
{
    filename = new char[strlen( scriptname )+1];
    strcpy( filename, scriptname );
}

AIScript::~AIScript()
{
#ifdef ORDERDEBUG
    BOOST_LOG_TRIVIAL(debug) << boost::format("sc%1$x") % this;
    VSFileSystem::flushLogs();
#endif
    if (filename)
        delete[] filename;
#ifdef ORDERDEBUG
    BOOST_LOG_TRIVIAL(debug) << "sc";
    VSFileSystem::flushLogs();
#endif
}

void AIScript::Execute()
{
    if (filename) {
        LoadXML();
#ifdef ORDERDEBUG
        BOOST_LOG_TRIVIAL(debug) << boost::format("fn%1$x") % this;
        VSFileSystem::flushLogs();
#endif
        delete[] filename;
        filename = NULL;
#ifdef ORDERDEBUG
        BOOST_LOG_TRIVIAL(debug) << "fn";
        VSFileSystem::flushLogs();
#endif
    }
    Order::Execute();
}

