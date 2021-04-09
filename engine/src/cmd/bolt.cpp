#include <vector>

#include <string>
#include <algorithm>

#include "bolt.h"
#include "gfxlib.h"
#include "gfx/mesh.h"
#include "gfxlib_struct.h"
#include "gfx/aux_texture.h"
#include "gfx/animation.h"
#include "gfx/decalqueue.h"
#include "unit.h"
#include "audiolib.h"
#include "config_xml.h"
#include "gfx/camera.h"
#include "options.h"
#include "universe.h"

using std::vector;
using std::string;
GFXVertexList*bolt_draw::boltmesh = NULL;
bolt_draw::~bolt_draw()
{
    unsigned int i;
    for (i = 0; i < cachedecals.size(); i++)
        boltdecals->DelTexture( cachedecals[i] );
    cachedecals.clear();
    for (i = 0; i < animations.size(); i++)
        delete animations[i];
    for (i = 0; i < balls.size(); i++)
        for (int j = balls[i].size()-1; j >= 0; j--)
            balls[i][j].Destroy( j );
    for (i = 0; i < bolts.size(); i++)
        for (int j = bolts[i].size()-1; j >= 0; j--)
            bolts[i][j].Destroy( j );
    delete boltdecals;
}
bolt_draw::bolt_draw()
{
    boltdecals = new DecalQueue;
    if (!boltmesh) {
        GFXVertex    vtx[12];
#define V( ii, xx, yy, zz, ss,                                                                                                \
           tt ) vtx[ii].x = xx; vtx[ii].y = yy; vtx[ii].z = zz+game_options.bolt_offset+.875; vtx[ii].i = 0; vtx[ii].j = 0; vtx[ii].k = 1; \
    vtx[ii].s = ss; vtx[ii].t = tt;
        V( 0, 0, 0, -.875, 0, .5 );
        V( 1, 0, -1, 0, .875, 1 );
        V( 2, 0, 0, .125, 1, .5 );
        V( 3, 0, 1, 0, .875, 0 );
        V( 4, 0, 0, -.875, 0, .5 );
        V( 5, -1, 0, 0, .875, 1 );
        V( 6, 0, 0, .125, 1, .5 );
        V( 7, 1, 0, 0, .875, 0 );
        V( 8, 1, 0, 0, .1875, 0 );
        V( 9, 0, 1, 0, .375, .1875 );
        V( 10, -1, 0, 0, .1875, .375 );
        V( 11, 0, -1, 0, 0, .1875 );
        boltmesh = new GFXVertexList( GFXQUAD, 12, vtx, 12, false );         //not mutable;
    }
}

extern double interpolation_blend_factor;

inline void BlendTrans( Matrix &drawmat, const QVector &cur_position, const QVector &prev_position )
{
    drawmat.p = prev_position.Scale( 1-interpolation_blend_factor )+cur_position.Scale( interpolation_blend_factor );
}
int Bolt::AddTexture( bolt_draw *q, std::string file )
{
    int decal = q->boltdecals->AddTexture( file.c_str(), MIPMAP );
    if ( decal >= (int) q->bolts.size() ) {
        q->bolts.push_back( vector< Bolt > () );
        int blargh = q->boltdecals->AddTexture( file.c_str(), MIPMAP );
        if ( blargh >= (int) q->bolts.size() )
            q->bolts.push_back( vector< Bolt > () );
        q->cachedecals.push_back( blargh );
    }
    return decal;
}
int Bolt::AddAnimation( bolt_draw *q, std::string file, QVector cur_position )
{
    int decal = -1;
    for (unsigned int i = 0; i < q->animationname.size(); i++)
        if (file == q->animationname[i])
            decal = i;
    if (decal == -1) {
        decal = q->animations.size();
        q->animationname.push_back( file );
        q->animations.push_back( new Animation( file.c_str(), true, .1, MIPMAP, false ) );         //balls have their own orientation
        q->animations.back()->SetPosition( cur_position );
        q->balls.push_back( vector< Bolt > () );
    }
    return decal;
}

void Bolt::Draw()
{
    bolt_draw *qq = _Universe->activeStarSystem()->bolts;
    GFXDisable( LIGHTING );
    GFXDisable( CULLFACE );


    GFXBlendMode( ONE, game_options.BlendGuns ? ONE : ZERO );
    GFXTextureCoordGenMode( 0, NO_GEN, NULL, NULL );

    GFXAlphaTest( GREATER, .1 );
    vector< vector< Bolt > >::iterator i;
    vector< Bolt >::iterator j;
    vector< Animation* >::iterator     k = qq->animations.begin();
    float etime = GetElapsedTime();
    float pixel_angle = 2
                        *sin( g_game.fov*M_PI/180.0
                             /(g_game.y_resolution
                               > g_game.x_resolution ? g_game.y_resolution : g_game.x_resolution) )*game_options.bolt_pixel_size;
    pixel_angle *= pixel_angle;
    Vector  p, q, r;
    _Universe->AccessCamera()->GetOrientation( p, q, r );
    QVector campos = _Universe->AccessCamera()->GetPosition();
    for (i = qq->balls.begin(); i != qq->balls.end(); i++, k++) {
        Animation *cur = *k;
        if ( i->begin() != i->end() ) {
            float bolt_size = 2*i->begin()->type->radius*2;
            bolt_size *= bolt_size;
            //Matrix result;
            //FIXME::MuST USE DRAWNO	TRANSFORMNOW cur->CalculateOrientation (result);
            for (j = i->begin(); j != i->end(); j++) {
                //don't update time more than once
                Bolt *bolt     = &*j;
                float distance = (bolt->cur_position-campos).MagnitudeSquared();
                if (distance*pixel_angle < bolt_size) {
                    const weapon_info *type = bolt->type;
                    BlendTrans( bolt->drawmat, bolt->cur_position, bolt->prev_position );
                    Matrix tmp;
                    VectorAndPositionToMatrix( tmp, p, q, r, bolt->drawmat.p );
                    cur->SetDimensions( bolt->type->radius, bolt->type->radius );
                    GFXLoadMatrixModel( tmp );
                    GFXColor4f( type->r, type->g, type->b, type->a );
                    cur->DrawNoTransform( false, true );
                }
            }
        }
    }
    GFXVertexList *qmesh = qq->boltmesh;
    if (qmesh && qq->bolts.begin() != qq->bolts.end()) {
        GFXAlphaTest( ALWAYS, 0 );
        GFXDisable( DEPTHWRITE );
        GFXDisable( TEXTURE1 );
        GFXEnable( TEXTURE0 );
        GFXTextureCoordGenMode( 0, NO_GEN, NULL, NULL );
        
        BLENDFUNC bsrc, bdst;
        if (game_options.BlendGuns == true)
            GFXBlendMode( bsrc=ONE, bdst=ONE );
        else
            GFXBlendMode( bsrc=ONE, bdst=ZERO );
        
        qmesh->LoadDrawState();
        qmesh->BeginDrawState();
        int decal = 0;
        for (i = qq->bolts.begin(); i != qq->bolts.end(); decal++, i++) {
            Texture *dec = qq->boltdecals->GetTexture( decal );
            if ( dec && i->begin() != i->end() ) {
                float bolt_size = 2*i->begin()->type->radius+i->begin()->type->length;
                bolt_size *= bolt_size;
                for (size_t pass = 0, npasses = dec->numPasses(); pass < npasses; ++pass) {
                    GFXTextureEnv( 0, GFXMODULATETEXTURE );
                    if (dec->SetupPass(0, bsrc, bdst)) {
                        dec->MakeActive();
                        GFXToggleTexture( true, 0 );
                        for (j = i->begin(); j != i->end(); j++) {
                            Bolt &bolt     = *j;
                            float distance = (bolt.cur_position-campos).MagnitudeSquared();
                            if (distance*pixel_angle < bolt_size) {
                                const weapon_info *wt = bolt.type;

                                BlendTrans( bolt.drawmat, bolt.cur_position, bolt.prev_position );
                                Matrix drawmat( bolt.drawmat );
                                if (game_options.StretchBolts > 0)
                                    ScaleMatrix( drawmat, Vector( 1, 1, bolt.type->speed*etime*game_options.StretchBolts/bolt.type->length ) );
                                GFXLoadMatrixModel( drawmat );
                                GFXColor4f( wt->r, wt->g, wt->b, wt->a );
                                qmesh->Draw();
                            }
                        }
                    }
                }
            }
        }
        qmesh->EndDrawState();
    }
    GFXEnable( LIGHTING );
    GFXEnable( CULLFACE );
    GFXBlendMode( ONE, ZERO );
    GFXEnable( DEPTHTEST );
    GFXEnable( DEPTHWRITE );
    GFXEnable( TEXTURE0 );
    GFXColor4f( 1, 1, 1, 1 );
}

extern void BoltDestroyGeneric( Bolt *whichbolt, unsigned int index, int decal, bool isBall );
void Bolt::Destroy( unsigned int index )
{
    VSDESTRUCT2
    bolt_draw *q = _Universe->activeStarSystem()->bolts;
    bool isBall  = true;
    if (type->type == WEAPON_TYPE::BOLT) {
        q->boltdecals->DelTexture( decal );
        isBall = false;
    } else {}
    BoltDestroyGeneric( this, index, decal, isBall );
}

