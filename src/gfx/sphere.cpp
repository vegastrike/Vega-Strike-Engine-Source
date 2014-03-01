#include "sphere.h"
#include "ani_texture.h"
#include "vegastrike.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "xml_support.h"
#ifndef M_PI
#define M_PI (3.1415926536F)
#endif
#include "gfx/camera.h"

using XMLSupport::tostring;

void SphereMesh::ProcessDrawQueue( int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr )
{
    static GFXColor spherecol = vs_config->getColor( "planet_ambient" );
    GFXColor tmpcol( 0, 0, 0, 1 );
    GFXGetLightContextAmbient( tmpcol );
    GFXLightContextAmbient( spherecol );
    Mesh::ProcessDrawQueue( whichpass, whichdrawqueue, zsort, sortctr );
    GFXLightContextAmbient( tmpcol );
    GFXPolygonOffset( 0, 0 );
}

void SphereMesh::SelectCullFace( int whichdrawqueue )
{
    GFXEnable( CULLFACE );
}

void CityLights::ProcessDrawQueue( int whichpass, int whichdrawqueue, bool zsort, const QVector &sortctr )
{
    const GFXColor citycol( 1, 1, 1, 1 );
    GFXColor tmpcol( 0, 0, 0, 1 );
    GFXGetLightContextAmbient( tmpcol );
    GFXLightContextAmbient( citycol );
    Mesh::ProcessDrawQueue( whichpass, whichdrawqueue, zsort, sortctr );
    GFXLightContextAmbient( tmpcol );
    GFXPolygonOffset( 0, 0 );
}

