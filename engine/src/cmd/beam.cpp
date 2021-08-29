#include "vegastrike.h"
#include <vector>
#include "beam.h"
#include "unit_generic.h"
#include "gfx/aux_texture.h"
#include "gfx/decalqueue.h"
using std::vector;
#include "audiolib.h"
#include "configxml.h"
#include "images.h"
#include "mount_size.h"
#include "weapon_info.h"
#include "damageable.h"

struct BeamDrawContext
{
    Matrix m;
    class GFXVertexList *vlist;
    Beam  *beam;
    BeamDrawContext() {}
    BeamDrawContext( const Matrix &a, GFXVertexList *vl, Beam *b ) : m( a )
        ,  vlist( vl )
        ,  beam( b ) {}
};

static DecalQueue beamdecals;
static vector< vector< BeamDrawContext > >beamdrawqueue;

Beam::Beam( const Transformation &trans, const weapon_info &clne, void *own, Unit *firer, int sound ) : vlist( NULL )
    , Col( clne.r, clne.g, clne.b, clne.a )
{
    VSCONSTRUCT2( 'B' )
    listen_to_owner = false;     //warning this line of code is also present in beam_server.cpp change one, change ALL
#ifdef PERBOLTSOUND
    sound = AUDCreateSound( clne.sound, true );
#else
    this->sound = sound;
#endif
    decal = beamdecals.AddTexture( clne.file.c_str(), TRILINEAR );
    if ( decal >= beamdrawqueue.size() )
        beamdrawqueue.push_back( vector< BeamDrawContext > () );
    Init( trans, clne, own, firer );
    impact = UNSTABLE;
}

Beam::~Beam()
{
    VSDESTRUCT2
#ifdef PERBOLTSOUND
    AUDDeleteSound( sound );
#endif
#ifdef BEAMCOLQ
    RemoveFromSystem( true );
#endif
    //DO NOT DELETE - shared vlist
    //delete vlist;
}

extern void AdjustMatrixToTrackTarget( Matrix &mat, const Vector &vel, Unit *target, float speed, bool lead, float cone );

void Beam::Draw( const Transformation &trans, const Matrix &m, Unit *targ, float tracking_cone )
{
    //hope that the correct transformation is on teh stack
    if (curthick == 0)
        return;
    Matrix cumulative_transformation_matrix;
    local_transformation.to_matrix( cumulative_transformation_matrix );
    Transformation cumulative_transformation = local_transformation;
    cumulative_transformation.Compose( trans, m );
    cumulative_transformation.to_matrix( cumulative_transformation_matrix );
    AdjustMatrixToTrackTarget( cumulative_transformation_matrix, Vector( 0, 0, 0 ), targ, speed, false, tracking_cone );
#ifdef PERFRAMESOUND
    AUDAdjustSound( sound, cumulative_transformation.position, speed
                   *Vector( cumulative_transformation_matrix[8], cumulative_transformation_matrix[9],
                            cumulative_transformation_matrix[10] ) );
#endif
    AUDSoundGain( sound, curthick*curthick/(thickness*thickness) );

    beamdrawqueue[decal].push_back( BeamDrawContext( cumulative_transformation_matrix, vlist, this ) );
}

void Beam::ProcessDrawQueue()
{
    GFXDisable( LIGHTING );
    GFXDisable( CULLFACE );     //don't want lighting on this baby
    GFXDisable( DEPTHWRITE );
    GFXPushBlendMode();
    static bool blendbeams = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "BlendGuns", "true" ) );
    GFXBlendMode( ONE, blendbeams ? ONE : ZERO );

    GFXEnable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    BeamDrawContext c;
    for (unsigned int decal = 0; decal < beamdrawqueue.size(); decal++) {
        Texture *tex = beamdecals.GetTexture( decal );
        if (tex) {
            tex->MakeActive( 0 );
            GFXTextureEnv( 0, GFXMODULATETEXTURE );
            GFXToggleTexture( true, 0 );
            if ( beamdrawqueue[decal].size() ) {
                while ( beamdrawqueue[decal].size() ) {
                    c = beamdrawqueue[decal].back();
                    beamdrawqueue[decal].pop_back();

                    c.beam->RecalculateVertices( c.m );
                    GFXLoadMatrixModel( c.m );
                    c.vlist->DrawOnce();
                }
            }
        }
    }
    GFXEnable( DEPTHWRITE );
    GFXEnable( CULLFACE );
    GFXDisable( LIGHTING );
    GFXPopBlendMode();
}

