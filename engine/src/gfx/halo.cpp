#include "cmd/unit_generic.h"
#include "halo.h"
#include "gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "aux_texture.h"
#include "decalqueue.h"
#include "config_xml.h"
#include "xml_support.h"
#include "point_to_cam.h"
static DecalQueue halodecal;
static vector< GFXQuadList* >halodrawqueue;

Halo::Halo( const char *texture, const GFXColor &col, const QVector &pos, float sizx, float sizy )
{
    string texname( texture );
    position = pos;
    sizex    = sizx;
    sizey    = sizy;
    decal    = halodecal.AddTexture( texture, MIPMAP );
    if ( ( (unsigned int) decal ) >= halodrawqueue.size() )
        halodrawqueue.push_back( new GFXQuadList( GFXTRUE ) );
    GFXColorVertex coltmp[4];
    coltmp[0].SetColor( col );
    coltmp[1].SetColor( col );
    coltmp[2].SetColor( col );
    coltmp[3].SetColor( col );

    quadnum = halodrawqueue[decal]->AddQuad( NULL, coltmp );
}

Halo::~Halo()
{
    halodrawqueue[decal]->DelQuad( quadnum );
    delete halodrawqueue[decal];         //deletes the quad
    halodrawqueue[decal] = NULL;
}

void Halo::Draw( const Transformation &quat, const Matrix &m, float alpha )
{
    Vector       p, q, r;
    QVector      pos;
    static float HaloOffset = XMLSupport::parse_float( vs_config->getVariable( "graphics", "HaloOffset", ".1" ) );
    pos = position.Transform( m );
    float        wid = sizex;
    float        hei = sizey;
    static bool  far_shine =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_star_glow_halo",
                                                        "false" ) ) || XMLSupport::parse_bool(
            vs_config->getVariable( "graphics", "HaloFarDraw", "false" ) );
    CalculateOrientation( pos, p, q, r, wid, hei, HaloOffset, far_shine, NULL );
    p = p*wid;
    r = -r;
    q = q*hei;
    //offset = r*(sizex>sizey?sizex:sizey); //screws up cus of perspective
    GFXVertex tmp[4] = {
        GFXVertex( pos-(p+q).Cast(), r, 0, 1 ),
        GFXVertex( pos+(p-q).Cast(), r, 1, 1 ),
        GFXVertex( pos+(p+q).Cast(), r, 1, 0 ),
        GFXVertex( pos-(p-q).Cast(), r, 0, 0 )
    };
    halodrawqueue[decal]->ModQuad( quadnum, tmp, alpha );
}
void Halo::SetColor( const GFXColor &col )
{
    GFXColorVertex coltmp[4];
    coltmp[0].SetColor( col );
    coltmp[1].SetColor( col );
    coltmp[2].SetColor( col );
    coltmp[3].SetColor( col );

    halodrawqueue[decal]->ModQuad( quadnum, coltmp );
}

void Halo::ProcessDrawQueue()
{
    GFXDisable( LIGHTING );
    GFXDisable( DEPTHWRITE );
    GFXPushBlendMode();
    GFXBlendMode( ONE, ONE );
    GFXEnable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXLoadIdentity( MODEL );
    for (unsigned int decal = 0; decal < halodrawqueue.size(); decal++)
        if ( halodecal.GetTexture( decal ) ) {
            halodecal.GetTexture( decal )->MakeActive();
            halodrawqueue[decal]->Draw();
        }
    GFXEnable( DEPTHWRITE );
    GFXEnable( CULLFACE );
    GFXDisable( LIGHTING );
    GFXPopBlendMode();
}

