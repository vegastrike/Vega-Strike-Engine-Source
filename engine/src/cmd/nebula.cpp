#include "nebula.h"
#include "vegastrike.h"
#include "vsfilesystem.h"
#include <assert.h>
#include "config_xml.h"
#include "vs_globals.h"
#include <sys/stat.h>
#include "xml_support.h"
#include "gfx/mesh.h"
#include "gfx/cockpit.h"
#include "universe.h"
#include "unit_generics.h"

#undef BOOST_NO_CWCHAR

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

using namespace XMLSupport;
using namespace NebulaXML;
using namespace VSFileSystem;

extern double interpolation_blend_factor;

FOGMODE NebulaXML::parse_fogmode( string val )
{
    if (val == "exp")
        return FOG_EXP;
    else if (val == "exp2")
        return FOG_EXP2;
    else if (val == "linear")
        return FOG_LINEAR;
    else
        return FOG_OFF;
}



//WARNING : USED TO CALL a GameUnit constructor but now Nebula::Nebula calls a Unit one
Nebula::Nebula( const char *unitfile, bool SubU, int faction, Flightgroup *fg, int fg_snumber ) :
    GameUnit< NebulaGeneric > ( unitfile, SubU, faction, string( "" ), fg, fg_snumber )
{
    fogme     = true;
    string fullpath( unitfile );
    fullpath += ".nebula";

    LoadXML( fullpath.c_str() );

    fadeinvalue = 0;
    lastfadein  = 0;
}



void Nebula::beginElement( void *Userdata, const XML_Char *name, const XML_Char **atts )
{
    ( (Nebula*) Userdata )->beginElem( std::string( name ), AttributeList( atts ) );
}

void Nebula::beginElem( const std::string &name, const AttributeList &atts )
{
    Names elem = (Names) element_map.lookup( name );
    AttributeList::const_iterator iter;
    switch (elem)
    {
    case UNKNOWN:
        break;
    case NEBULA:
        fogme   = true;
        explosiontime = 0;
        index   = 0;
        fogmode = FOG_LINEAR;
        for (iter = atts.begin(); iter != atts.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case DENSITY:
                Density = parse_float( (*iter).value );
                break;
            case INDEX:
                index   = parse_int( (*iter).value );
                break;
            case MODE:
                fogmode = parse_fogmode( (*iter).value );
                break;
            case EXPLOSIONTIME:
                explosiontime = parse_float( (*iter).value );
                break;
            case FOGTHIS:
                fogme = parse_bool( (*iter).value );
                break;
            }
        }
        break;
    case COLOR:
        for (iter = atts.begin(); iter != atts.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case RED:
                color.i = parse_float( (*iter).value );
                break;
            case GREEN:
                color.j = parse_float( (*iter).value );
                break;
            case BLUE:
                color.k = parse_float( (*iter).value );
                break;
            }
        }
        break;
    case LIMITS:
        for (iter = atts.begin(); iter != atts.end(); iter++) {
            switch ( attribute_map.lookup( (*iter).name ) )
            {
            case NEBNEAR:
                fognear = parse_float( (*iter).value );
                break;
            case NEBFAR:
                fogfar  = parse_float( (*iter).value );
                break;
            }
        }
        break;
    default:
        break;
    }
}

static void Nebula_endElement( void *Userdata, const XML_Char* ) {}

void Nebula::LoadXML( const char *filename )
{
    VSFile      f;
    VSError     err = f.OpenReadOnly( filename, UnitFile );
    static bool usefog     = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "fog", "true" ) );
    if (err > Ok || !usefog) {
        if (err <= Ok)
            f.Close();
        else
            VSFileSystem::vs_fprintf( stderr, "\nUnit file %s not found\n", filename );
        fogmode = FOG_OFF;
        return;
    }
    XML_Parser parser = XML_ParserCreate( NULL );
    XML_SetUserData( parser, this );
    XML_SetElementHandler( parser, &Nebula::beginElement, &Nebula_endElement );
    XML_Parse( parser, ( f.ReadFull() ).c_str(), f.Size(), 1 );
    XML_ParserFree( parser );
    f.Close();
}

void Nebula::SetFogState()
{
    float thisfadein = (lastfadein*(1-interpolation_blend_factor)+(fadeinvalue)*interpolation_blend_factor);
    GFXFogMode( fogmode );
    GFXFogDensity( Density*thisfadein );
    GFXFogLimits( fognear, fogfar*thisfadein );
    GFXFogColor( GFXColor( color.i, color.j, color.k, 1 ) );
    GFXFogIndex( index );
}


void Nebula::UpdatePhysics2( const Transformation &trans,
                                 const Transformation &old_physical_state,
                                 const Vector &accel,
                                 float difficulty,
                                 const Matrix &transmat,
                                 const Vector &CumulativeVelocity,
                                 bool ResolveLast,
                                 UnitCollection *uc )
{
    static float nebdelta = XMLSupport::parse_float( vs_config->getVariable( "graphics", "fog_time", ".01" ) );
    lastfadein   = fadeinvalue;
    fadeinvalue -= nebdelta*simulation_atom_var;
    if (fadeinvalue < 0)
        fadeinvalue = 0;
    this->GameUnit< NebulaGeneric >::UpdatePhysics2( trans,
                                              old_physical_state,
                                              accel,
                                              difficulty,
                                              transmat,
                                              CumulativeVelocity,
                                              ResolveLast,
                                              uc );
    Vector t1;
    float  dis;
    unsigned int i;
    if (_Universe->activeStarSystem() == _Universe->AccessCockpit()->activeStarSystem) {
        for (i = 0; i < NUM_CAM; i++)
            if ( Inside( _Universe->AccessCamera( i )->GetPosition(), 0, t1, dis ) )
                PutInsideCam( i );
    }
    if (nummesh() > 0) {
        i = rand()%( nummesh() );
        Vector randexpl( rand()%2*rSize()-rSize(), rand()%2*rSize()-rSize(), rand()%2*rSize()-rSize() );
        if ( ( (int) (explosiontime/simulation_atom_var) ) != 0 )
            if ( !( rand()%( (int) (explosiontime/simulation_atom_var) ) ) )
                meshdata[i]->AddDamageFX( randexpl, Vector( 0, 0, 0 ), .00001, color );
    }
}

void Nebula::PutInsideCam( int i )
{
    static float nebdelta   = XMLSupport::parse_float( vs_config->getVariable( "graphics", "fog_time", ".01" ) );
    static float fadeinrate = XMLSupport::parse_float( vs_config->getVariable( "graphics", "fog_fade_in_percent", "0.5" ) );
    if ( _Universe->AccessCamera() == _Universe->AccessCamera( i ) ) {
        fadeinvalue += (1+fadeinrate)*nebdelta*simulation_atom_var;
        if (fadeinvalue > 1)
            fadeinvalue = 1;
    }
    _Universe->AccessCamera( i )->SetNebula( this );
}

