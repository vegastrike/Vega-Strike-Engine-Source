#include <assert.h>
#include "star_system.h"
#include "cmd/planet.h"
#include "cmd/unit.h"
#include "cmd/unit_collide.h"
#include "cmd/collection.h"
#include "cmd/click_list.h"
#include "lin_time.h"
#include "cmd/beam.h"
#include "gfx/sphere.h"
#include "cmd/unit_collide.h"
#include "gfx/halo.h"
#include "gfx/background.h"
#include "gfx/animation.h"
#include "gfx/aux_texture.h"
#include "gfx/star.h"
#include "cmd/bolt.h"
#include <expat.h>
#include "gfx/cockpit.h"
#include "audiolib.h"
#include "cmd/music.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "cmd/cont_terrain.h"
#include "vegastrike.h"
#include "universe.h"
#include "cmd/atmosphere.h"
#include "hashtable.h"
#include "cmd/nebula.h"
#include "galaxy_gen.h"
#include "cmd/script/mission.h"
#include "in_kb.h"
#include "cmd/script/flightgroup.h"
#include "load_mission.h"
#include "gfx/particle.h"
#include "gfx/lerp.h"
#include "gfx/warptrail.h"
#include "gfx/env_map_gent.h"
#include "vsfilesystem.h"
#include "cmd/unit_find.h"
#include "gfx/occlusion.h"

#include "options.h"



GameStarSystem::GameStarSystem( const char *filename, const Vector &centr, const float timeofyear )
{
    no_collision_time = 0;               //(int)(1+2.000/SIMULATION_ATOM);
    ///adds to jumping table;
    name  = NULL;
    _Universe->pushActiveStarSystem( this );
    GFXCreateLightContext( lightcontext );
    bolts = new bolt_draw;
    collidetable   = new CollideTable( this );

    current_stage  = MISSION_SIMULATION;

    LoadXML( filename, centr, timeofyear );
    if (!name)
        name = strdup( filename );
    AddStarsystemToUniverse( filename );
    UpdateTime();
    time = 0;

    Atmosphere::Parameters params;

    params.radius = 40000;

    params.low_color[0] = GFXColor( 0, 0.5, 0.0 );

    params.low_color[1] = GFXColor( 0, 1.0, 0.0 );

    params.low_ambient_color[0] = GFXColor( 0.0/255.0, 0.0/255.0, 0.0/255.0 );

    params.low_ambient_color[1] = GFXColor( 0.0/255.0, 0.0/255.0, 0.0/255.0 );

    params.high_color[0] = GFXColor( 0.5, 0.0, 0.0 );

    params.high_color[1] = GFXColor( 1.0, 0.0, 0.0 );

    params.high_ambient_color[0] = GFXColor( 0, 0, 0 );

    params.high_ambient_color[1] = GFXColor( 0, 0, 0 );

    params.scattering = 5;

    _Universe->popActiveStarSystem();
}

Texture* GameStarSystem::getLightMap()
{
    return LightMap[0];
}

void GameStarSystem::activateLightMap( int stage )
{
    GFXActiveTexture( stage );
#ifdef NV_CUBE_MAP
    LightMap[0]->MakeActive( stage );
#else
    LightMap[0]->MakeActive( stage );
#endif
    GFXTextureEnv( stage, GFXADDTEXTURE );
#ifdef NV_CUBE_MAP
    GFXToggleTexture( true, stage, CUBEMAP );
    GFXTextureCoordGenMode( stage, CUBE_MAP_GEN, NULL, NULL );
#else
    const float tempo[4] = {1, 0, 0, 0};
    GFXToggleTexture( true, stage, TEXTURE2D );
    GFXTextureCoordGenMode( stage, SPHERE_MAP_GEN, tempo, tempo );
#endif
    GFXActiveTexture( 0 );
}

GameStarSystem::~GameStarSystem()
{
    _Universe->pushActiveStarSystem( this );
#ifdef NV_CUBE_MAP
    delete LightMap[0];
    delete LightMap[1];
    delete LightMap[2];
    delete LightMap[3];
    delete LightMap[4];
    delete LightMap[5];
#else
    delete LightMap[0];
#endif
    delete bg;
    delete stars;
    //delete [] name;
    delete bolts;
    //delete collidetable;//BAD BAD BAD we need this to happen later!

    _Universe->popActiveStarSystem();
    RemoveStarsystemFromUniverse();
}

ClickList* GameStarSystem::getClickList()
{
    return new ClickList( this, &drawList );
}

void ConditionalCursorDraw( bool tf )
{
    if (game_options.hardware_cursor)
        winsys_show_cursor( tf );
}

void GameStarSystem::SwapIn()
{
    GFXSetLightContext( lightcontext );
}

void GameStarSystem::SwapOut()
{}

extern double saved_interpolation_blend_factor;
extern double interpolation_blend_factor;
extern bool   cam_setup_phase;

//Class for use of UnitWithinRangeLocator template
//Used to do distance based pre-culling for draw function based on sorted search structure
class UnitDrawer
{
    struct empty {};
    vsUMap< void*, struct empty >gravunits;
public:
    Unit *parent;
    Unit *parenttarget;
    UnitDrawer()
    {
        parent = NULL;
        parenttarget = NULL;
    }
    bool acquire( Unit *unit, float distance )
    {
        if ( gravunits.find( unit ) == gravunits.end() )
            return draw( unit );
        else
            return true;
    }
    void drawParents()
    {
        if (parent && parent->isSubUnit())
            parent = UnitUtil::owner( parent );
        if (parent)
            draw(parent);
        
        if (parenttarget && parenttarget->isSubUnit())
            parenttarget = UnitUtil::owner( parenttarget );
        if (parenttarget)
            draw(parenttarget);
    }

    bool draw( Unit *unit )
    {
        if ( parent == unit || (parent && parent->isSubUnit() && parent->owner == unit) )
            parent = NULL;
        if ( parenttarget == unit || (parenttarget && parenttarget->isSubUnit() && parenttarget->owner == unit) )
            parenttarget = NULL;
        float backup = SIMULATION_ATOM;
        unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
        interpolation_blend_factor = calc_blend_factor( saved_interpolation_blend_factor,
                                                        unit->sim_atom_multiplier,
                                                        unit->cur_sim_queue_slot,
                                                        cur_sim_frame );
        SIMULATION_ATOM = backup*unit->sim_atom_multiplier;
        (/*(GameUnit< Unit >*)*/ unit)->Draw();
        interpolation_blend_factor = saved_interpolation_blend_factor;
        SIMULATION_ATOM = backup;
        return true;
    }
    bool grav_acquire( Unit *unit )
    {
        gravunits[unit] = empty();
        return draw( unit );
    }
};

//#define UPDATEDEBUG  //for hard to track down bugs
void GameStarSystem::Draw( bool DrawCockpit )
{
    GFXEnable( DEPTHTEST );
    GFXEnable( DEPTHWRITE );
    saved_interpolation_blend_factor = interpolation_blend_factor =
                                           (1./PHY_NUM)*( (PHY_NUM*time)/SIMULATION_ATOM+current_stage );
    GFXColor4f( 1, 1, 1, 1 );
    if (DrawCockpit)
        AnimatedTexture::UpdateAllFrame();
    for (unsigned int i = 0; i < contterrains.size(); ++i)
        contterrains[i]->AdjustTerrain( this );
    Unit *par;
    if ( ( par = _Universe->AccessCockpit()->GetParent() ) == NULL ) {
        _Universe->AccessCamera()->UpdateGFX( GFXTRUE );
    } else if ( !par->isSubUnit() ) {
        //now we can assume world is topps
        par->cumulative_transformation = linear_interpolate( par->prev_physical_state,
                                                             par->curr_physical_state,
                                                             interpolation_blend_factor );
        Unit *targ = par->Target();
        if ( targ && !targ->isSubUnit() ) {
            targ->cumulative_transformation = linear_interpolate( targ->prev_physical_state,
                                                                  targ->curr_physical_state,
                                                                  interpolation_blend_factor );
        }
        _Universe->AccessCockpit()->SetupViewPort( true );
    }
    double setupdrawtime = queryTime();
    {
        cam_setup_phase = true;
        
        Unit *saveparent = _Universe->AccessCockpit()->GetSaveParent();
        Unit *targ = NULL;
        if (saveparent)
            targ = saveparent->Target();
        //Array containing the two interesting units, so as not to have to copy-paste code
        Unit *camunits[2] = {saveparent, targ};
        float backup = SIMULATION_ATOM;
        unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
        for (int i = 0; i < 2; ++i) {
            Unit *unit = camunits[i];
            //Make sure unit is not null;
            if ( unit && !unit->isSubUnit() ) {
                interpolation_blend_factor = calc_blend_factor( saved_interpolation_blend_factor,
                                                                unit->sim_atom_multiplier,
                                                                unit->cur_sim_queue_slot,
                                                                cur_sim_frame );
                SIMULATION_ATOM = backup*unit->sim_atom_multiplier;
                ( (GameUnit< Unit >*)unit )->GameUnit< Unit >::Draw();
            }
        }
        interpolation_blend_factor = saved_interpolation_blend_factor;
        SIMULATION_ATOM = backup;


        ///this is the final, smoothly calculated cam
        _Universe->AccessCockpit()->SetupViewPort( true );

        cam_setup_phase = false;
    }
    setupdrawtime = queryTime()-setupdrawtime;
    GFXDisable( LIGHTING );
    bg->Draw();
    double drawtime    = queryTime();

    // Initialize occluder system (we'll populate it during unit render)
    Occlusion::start();

    //Ballpark estimate of when an object of configurable size first becomes one pixel

    QVector    drawstartpos = _Universe->AccessCamera()->GetPosition();

    Collidable key_iterator( 0, 1, drawstartpos );
    UnitWithinRangeOfPosition< UnitDrawer >drawer( game_options.precull_dist, 0, key_iterator );
    //Need to draw really big stuff (i.e. planets, deathstars, and other mind-bogglingly big things that shouldn't be culled despited extreme distance
    Unit *unit;
    if ( ( drawer.action.parent = _Universe->AccessCockpit()->GetParent() ) != NULL )
        drawer.action.parenttarget = drawer.action.parent->Target();
    for (un_iter iter = this->GravitationalUnits.createIterator(); (unit = *iter); ++iter) {
        float distance = ( drawstartpos-unit->Position() ).Magnitude()-unit->rSize();
        if (distance < game_options.precull_dist)
            drawer.action.grav_acquire( unit );
        else
            drawer.action.draw( unit );
    }
    //Need to get iterator to approx camera position
    CollideMap::iterator parent = collidemap[Unit::UNIT_ONLY]->lower_bound( key_iterator );
    findObjectsFromPosition( this->collidemap[Unit::UNIT_ONLY], parent, &drawer, drawstartpos, 0, true );
    drawer.action.drawParents();     //draw units targeted by camera
    //FIXME  maybe we could do bolts & units instead of unit only--and avoid bolt drawing step

#if 0
    for (unsigned int sim_counter = 0; sim_counter <= SIM_QUEUE_SIZE; ++sim_counter) {
        double tmp    = queryTime();
        Unit  *unit;
        UnitCollection::UnitIterator iter = physics_buffer[sim_counter].createIterator();
        float  backup = SIMULATION_ATOM;
        unsigned int cur_sim_frame = _Universe->activeStarSystem()->getCurrentSimFrame();
        while ( ( unit = iter.current() ) != NULL ) {
            interpolation_blend_factor = calc_blend_factor( saved_interpolation_blend_factor,
                                                            unit->sim_atom_multiplier,
                                                            unit->cur_sim_queue_slot,
                                                            cur_sim_frame );
            //if (par&&par->Target()==unit) {
            //printf ("i:%f s:%f m:%d c:%d l:%d\n",interpolation_blend_factor,saved_interpolation_blend_factor,unit->sim_atom_multiplier,sim_counter,current_sim_location);
            //}
            SIMULATION_ATOM = backup*unit->sim_atom_multiplier;
            ( (GameUnit< Unit >*)unit )->Draw();
            iter.advance();
        }
        interpolation_blend_factor = saved_interpolation_blend_factor;
        SIMULATION_ATOM = backup;
        tmp = queryTime()-tmp;
    }
#endif
    drawtime = queryTime()-drawtime;
    WarpTrailDraw();

    GFXFogMode( FOG_OFF );
    
    // At this point, we've set all occluders
    // Mesh::ProcessXMeshes will query it

    GFXColor tmpcol( 0, 0, 0, 1 );
    GFXGetLightContextAmbient( tmpcol );
    double   processmesh = queryTime();
    if (!game_options.draw_near_stars_in_front_of_planets) stars->Draw();
    Mesh::ProcessZFarMeshes();
    if (game_options.draw_near_stars_in_front_of_planets) stars->Draw();
    GFXEnable( DEPTHTEST );
    GFXEnable( DEPTHWRITE );
    //need to wait for lights to finish
    GamePlanet::ProcessTerrains();
    Terrain::RenderAll();
    Mesh::ProcessUndrawnMeshes( true );
    processmesh = queryTime()-processmesh;
    Nebula *neb;

    Matrix  ident;
    Identity( ident );

    GFXPopGlobalEffects();
    GFXLightContextAmbient( tmpcol );
    if ( ( neb = _Universe->AccessCamera()->GetNebula() ) )
        neb->SetFogState();
    Beam::ProcessDrawQueue();
    Bolt::Draw();

    GFXFogMode( FOG_OFF );
    Animation::ProcessDrawQueue();
    Halo::ProcessDrawQueue();
    particleTrail.DrawAndUpdate();
    GameStarSystem::DrawJumpStars();
    ConditionalCursorDraw( false );
    if (DrawCockpit)
        _Universe->AccessCockpit()->Draw();
    MeshAnimation::UpdateFrames();
    
    // And now we're done with the occluder set
    Occlusion::end();
}

extern void update_ani_cache();
void UpdateAnimatedTexture()
{
    AnimatedTexture::UpdateAllPhysics();
    update_ani_cache();
}

void TerrainCollide()
{
    Terrain::CollideAll();
}

void UpdateTerrain()
{
    Terrain::UpdateAll( 64 );
}

void UpdateCameraSnds()
{
    _Universe->AccessCockpit( 0 )->AccessCamera()->UpdateCameraSounds();
}

void NebulaUpdate( StarSystem *ss )
{
    if (_Universe->AccessCockpit()->activeStarSystem == ss) {
        Nebula *neb;
        if ( ( neb = _Universe->AccessCamera()->GetNebula() ) ) {
            if (neb->getFade() <= 0)
                //Update physics should set this
                _Universe->AccessCamera()->SetNebula( NULL );
        }
    }
}

void GameStarSystem::createBackground( StarSystem::StarXML *xml )
{
#ifdef NV_CUBE_MAP
    printf( "using NV_CUBE_MAP\n" );
    LightMap[0] = new Texture( (xml->backgroundname+"_light.cube").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_X,
                              GFXFALSE, game_options.max_cubemap_size );
    if ( LightMap[0]->LoadSuccess() && LightMap[0]->isCube() ) {
        LightMap[1] = LightMap[2] = LightMap[3] = LightMap[4] = LightMap[5] = 0;
    } else {
        delete LightMap[0];
        LightMap[0] = new Texture( (xml->backgroundname+"_right.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_X,
                                  GFXFALSE, game_options.max_cubemap_size );
        LightMap[1] = new Texture( (xml->backgroundname+"_left.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_NEGATIVE_X,
                                  GFXFALSE, game_options.max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                                  LightMap[0] );
        LightMap[2] = new Texture( (xml->backgroundname+"_up.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_Y,
                                  GFXFALSE, game_options.max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                                  LightMap[0] );
        LightMap[3] = new Texture( (xml->backgroundname+"_down.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_NEGATIVE_Y,
                                  GFXFALSE, game_options.max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                                  LightMap[0] );
        LightMap[4] = new Texture( (xml->backgroundname+"_front.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_POSITIVE_Z,
                                  GFXFALSE, game_options.max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                                  LightMap[0] );
        LightMap[5] = new Texture( (xml->backgroundname+"_back.image").c_str(), 1, TRILINEAR, CUBEMAP, CUBEMAP_NEGATIVE_Z,
                                  GFXFALSE, game_options.max_cubemap_size, GFXFALSE, GFXFALSE, DEFAULT_ADDRESS_MODE,
                                  LightMap[0] );
    }
#else
    string  bglight = xml->backgroundname+"_light.image";
    string  bgfile  = xml->backgroundname+"_light.image";
    VSFile  f;
    VSError err     = f.OpenReadOnly( bgfile, VSFileSystem::TextureFile );
    if (err > Ok)
        EnvironmentMapGeneratorMain( xml->backgroundname.c_str(), bglight.c_str(), 0, xml->reflectivity, 1 );
    else
        f.Close();
    LightMap[0] = new Texture( bgfile.c_str(), 1, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE );
#endif

    bg = new Background( 
        xml->backgroundname.c_str(), 
        xml->numstars, 
        g_game.zfar*.9, 
        filename,
        xml->backgroundColor,
        xml->backgroundDegamma );
    stars = new Stars( xml->numnearstars, xml->starsp );
    stars->SetBlend( game_options.starblend, game_options.starblend );
}

