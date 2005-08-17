

#include <assert.h>
#include "star_system.h"
#include "cmd/planet.h"
#include "cmd/unit.h"
#include "cmd/unit_collide.h"
#include "cmd/collection.h"
#include "cmd/click_list.h"
#include "cmd/ai/input_dfa.h"
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
extern Music *muzak;
extern Vector mouseline;
#include "vsfilesystem.h"
//static SphereMesh *foo;
//static Unit *earth;




extern string RemoveDotSystem (const char *input);
/*
GameStarSystem::GameStarSystem(): StarSystem()
{
  _Universe->pushActiveStarSystem (this);
  GFXCreateLightContext (lightcontext);
  bolts = new bolt_draw;
  collidetable = new CollideTable(this);
}
*/
GameStarSystem::GameStarSystem(const char * filename, const Vector & centr,const float timeofyear) {

  no_collision_time=0;//(int)(1+2.000/SIMULATION_ATOM);
  ///adds to jumping table;
  name = NULL;
  _Universe->pushActiveStarSystem (this);
  GFXCreateLightContext (lightcontext);
  bolts = new bolt_draw;
  collidetable = new CollideTable(this);
  //  cout << "origin: " << centr.i << " " << centr.j << " " << centr.k << " " << planetname << endl;

  current_stage=MISSION_SIMULATION;

  systemInputDFA = new InputDFA (this);

  LoadXML(filename,centr,timeofyear);
  if (!name)
    name =strdup (filename);
  AddStarsystemToUniverse(filename);
//  primaries[0]->SetPosition(0,0,0);

  //iter = primaries->createIterator();
  //iter->advance();
  //earth=iter->current();
  //delete iter;



  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  //foo = new SphereMesh(1,5,5,"moon.bmp");
  //cam[1].SetProjectionType(Camera::PARALLEL);
  //cam[1].SetZoom(1);
  //cam[1].SetPosition(Vector(0,0,0));
  //cam[1].LookAt(Vector(0,0,0), Vector(0,0,1));
  //cam[1].SetPosition(Vector(0,5,-2.5));
  //cam[1].SetSubwindow(0,0,1,1);

  //cam[2].SetProjectionType(Camera::PARALLEL);
  //cam[2].SetZoom(10.0);
  //cam[2].SetPosition(Vector(5,0,0));
  //cam[2].LookAt(Vector(0,0,0), Vector(0,-1,0));
  //cam[2].SetPosition(Vector(5,0,-2.5));
  //cam[2].SetSubwindow(0.10,0,0.10,0.10);
  UpdateTime();
  time = 0;


  Atmosphere::Parameters params;

  params.radius = 40000;



  params.low_color[0] = GFXColor(0,0.5,0.0);

  params.low_color[1] = GFXColor(0,1.0,0.0);

  params.low_ambient_color[0] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);

  params.low_ambient_color[1] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);

  params.high_color[0] = GFXColor(0.5,0.0,0.0);

  params.high_color[1] = GFXColor(1.0,0.0,0.0);

  params.high_ambient_color[0] = GFXColor(0,0,0);

  params.high_ambient_color[1] = GFXColor(0,0,0);

  /*

  params.low_color[0] = GFXColor(241.0/255.0,123.0/255.0,67.0/255.0);

  params.low_color[1] = GFXColor(253.0/255.0,65.0/255.0,55.0/255.0);

  params.low_ambient_color[0] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);

  params.low_ambient_color[1] = GFXColor(0.0/255.0,0.0/255.0,0.0/255.0);

  params.high_color[0] = GFXColor(60.0/255.0,102.0/255.0,249.0/255.0);

  params.high_color[1] = GFXColor(57.0/255.0,188.0/255.0,251.0/255.0);

  params.high_ambient_color[0] = GFXColor(0,0,0);

  params.high_ambient_color[1] = GFXColor(0,0,0);

  */

  params.scattering = 5;

  _Universe->popActiveStarSystem ();

}
void GameStarSystem::activateLightMap() {
  GFXActiveTexture (1);
#ifdef NV_CUBE_MAP
  LightMap[0]->MakeActive();
  LightMap[1]->MakeActive();
  LightMap[2]->MakeActive();
  LightMap[3]->MakeActive();
  LightMap[4]->MakeActive();
  LightMap[5]->MakeActive();

#else
    LightMap[0]->MakeActive();
#endif
  GFXActiveTexture (0);
}

GameStarSystem::~GameStarSystem() {
  _Universe->pushActiveStarSystem(this);
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
  delete systemInputDFA;
  delete bolts;
  //  delete collidetable;//BAD BAD BAD we need this to happen later!


  //I *think* this is legacy 012603  GFXDeleteLightContext (lightcontext);

  _Universe->popActiveStarSystem();
  RemoveStarsystemFromUniverse();
  
}


ClickList *GameStarSystem::getClickList() {
  return new ClickList (this, &drawList);

}
/**OBSOLETE!
void GameStarSystem::modelGravity(bool lastframe) {
  for (int i=0;i<numprimaries;i++) {
    primaries[i]->UpdatePhysics (identity_transformation,identity_matrix,lastframe,units)
  }
}	
*/
void ConditionalCursorDraw(bool tf) {
	static bool hardware_cursor = XMLSupport::parse_bool (vs_config->getVariable("graphics","hardware_cursor","false"));
	if (hardware_cursor) {
		winsys_show_cursor(tf);
	}
}
void GameStarSystem::SwapIn () {
  GFXSetLightContext (lightcontext);

  /*
  for (un_iter un_i=drawList.createIterator();NULL!= (un=*un_i);++un_i) {
    un->SwapInHalos();

  }
  */
  /*
  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    if (unit->isUnit()==PLANETPTR) {
      ((Planet *)unit)->EnableLights();
    }
    iter->advance();
  }
  delete iter;  
  */
  unsigned int i;
  for (i=0;i<terrains.size();i++) {
    //gotta push this shit somehow
    //terrains[i]->EnableDraw();
  }
  for (i=0;i<contterrains.size();i++) {
    //contterrains[i]->EnableDraw();
  }
}

void GameStarSystem::SwapOut () {
  //AUDStopAllSounds();
  /*
  for (un_iter un_i=drawList.createIterator();NULL!= (un=*un_i);++un_i) {
    un->SwapOutHalos();
    Halo::ProcessDrawQueue();

  }
  */
  /*
  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    if (unit->isUnit()==PLANETPTR) {
      ((Planet *)unit)->DisableLights();
    }
    iter->advance();
  }
  delete iter;
  */
  unsigned int i;
  for (i=0;i<terrains.size();i++) {
    //terrains[i]->DisableDraw();
  }
  for (i=0;i<contterrains.size();i++) {
    //contterrains[i]->DisableDraw();
  }

}
static double calc_blend_factor(double frac, int priority, int when_it_will_be_simulated, int cur_simulation_frame) {
	bool is_at_end=when_it_will_be_simulated==SIM_QUEUE_SIZE;
  if (cur_simulation_frame>when_it_will_be_simulated) {
    when_it_will_be_simulated+=SIM_QUEUE_SIZE;
  }
  double distance = when_it_will_be_simulated-cur_simulation_frame;//number between for next SIM_FRAME and SIM_QUEUE_SIZE-1
  double when_it_was_simulated=when_it_will_be_simulated-(double)priority;
  double fraction_of_physics_frame=(cur_simulation_frame-when_it_was_simulated+frac-1)/priority;
  if (is_at_end) {
	  return 1;
  }else {
	  return fraction_of_physics_frame;
  }
}
extern double interpolation_blend_factor;
//#define UPDATEDEBUG  //for hard to track down bugs
void GameStarSystem::Draw(bool DrawCockpit) {
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);

  double saved_interpolation_blend_factor=interpolation_blend_factor = (1./PHY_NUM)*((PHY_NUM*time)/SIMULATION_ATOM+current_stage);
  GFXColor4f(1,1,1,1);
  if (DrawCockpit) {
    AnimatedTexture::UpdateAllFrame();
  }
  for (unsigned int i=0;i<contterrains.size();i++) {
    contterrains[i]->AdjustTerrain(this);
  }

  GFXDisable (LIGHTING);
  bg->Draw();

  //  VSFileSystem::Fprintf (stderr,"|t%f i%lf|",GetElapsedTime(),interpolation_blend_factor);
  Unit * par;
  bool alreadysetviewport=false;
  if ((par=_Universe->AccessCockpit()->GetParent())==NULL) {
    _Universe->AccessCamera()->UpdateGFX (GFXTRUE);
  }else {
    if (!par->isSubUnit()) {
      //now we can assume world is topps
      par-> cumulative_transformation = linear_interpolate (par->prev_physical_state,par->curr_physical_state,interpolation_blend_factor);
      Unit * targ = par->Target();
      if (targ) {
	targ-> cumulative_transformation = linear_interpolate (targ->prev_physical_state,targ->curr_physical_state,interpolation_blend_factor);
      }
      _Universe->AccessCockpit()->SetupViewPort(true);
      alreadysetviewport=true;
    }

  }

  Unit *unit;
  for (unsigned int sim_counter=0;sim_counter<=SIM_QUEUE_SIZE;++sim_counter) {
    UnitCollection::UnitIterator iter = physics_buffer[sim_counter].createIterator();    
    while((unit = iter.current())!=NULL) {
      interpolation_blend_factor=calc_blend_factor(interpolation_blend_factor,unit->sim_atom_multiplier,sim_counter,current_sim_location);
	  //if (par&&par->Target()==unit) {
		  //printf ("i:%f s:%f m:%d c:%d l:%d\n",interpolation_blend_factor,saved_interpolation_blend_factor,unit->sim_atom_multiplier,sim_counter,current_sim_location);
	  //}
      ((GameUnit<Unit> *)unit)->Draw();
      interpolation_blend_factor=saved_interpolation_blend_factor;
      iter.advance();
    }
  }
  WarpTrailDraw();

  GFXFogMode (FOG_OFF);


  static bool always_make_smooth=XMLSupport::parse_bool(vs_config->getVariable("graphics","always_make_smooth_cam","false"));
  bool whichview=  _Universe->AccessCockpit()->GetView()==CP_CHASE;//||_Universe->AccessCockpit()->GetView()==CP_VIEWTARGET;
  if (always_make_smooth||whichview||!alreadysetviewport)
    _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  //  SetViewport();//camera wielding unit is now drawn  Note: Background is one frame behind...big fat hairy deal
  GFXColor tmpcol (0,0,0,1);
  GFXGetLightContextAmbient(tmpcol);
  static bool DrawNearStarsLast =XMLSupport::parse_bool(vs_config->getVariable("graphics","draw_near_stars_in_front_of_planets","false"));
  if (!DrawNearStarsLast) {
	  stars->Draw();
  }
  Mesh::ProcessZFarMeshes();
  if (DrawNearStarsLast) {
	  stars->Draw();
  }
  
  GFXEnable (DEPTHTEST);
  GFXEnable (DEPTHWRITE);
  //need to wait for lights to finish
  GamePlanet::ProcessTerrains();
  Terrain::RenderAll();
  Mesh::ProcessUndrawnMeshes(true);
  Nebula * neb;

  Matrix ident;

  Identity(ident);

  //Atmosphere::ProcessDrawQueue();



  GFXPopGlobalEffects();



  GFXLightContextAmbient(tmpcol);


  

  if ((neb = _Universe->AccessCamera()->GetNebula())) {
    neb->SetFogState();
  }
  Beam::ProcessDrawQueue();

  Bolt::Draw();

  //  if (_Universe->AccessCamera()->GetNebula()!=NULL)
  GFXFogMode (FOG_OFF);
  Animation::ProcessDrawQueue();
  Halo::ProcessDrawQueue();
  particleTrail.DrawAndUpdate();
  GameStarSystem::DrawJumpStars();
  ConditionalCursorDraw(false);
  //  static bool doInputDFA = XMLSupport::parse_bool (vs_config->getVariable ("graphics","MouseCursor","false"));
  if (DrawCockpit) {
    _Universe->AccessCockpit()->Draw();
    //    if (doInputDFA) {
    //      GFXHudMode (true);
    //      systemInputDFA->Draw();
    //      GFXHudMode (false);
    //    }
  }
}
extern void update_ani_cache();
void	UpdateAnimatedTexture()
{ AnimatedTexture::UpdateAllPhysics();		update_ani_cache();}
void	TerrainCollide()
{ Terrain::CollideAll();}
void	UpdateTerrain()
{ Terrain::UpdateAll(64);}
void	UpdateCameraSnds()
{ _Universe->AccessCockpit(0)->AccessCamera()->UpdateCameraSounds();}
void	NebulaUpdate( StarSystem * ss)
{
	if (_Universe->AccessCockpit()->activeStarSystem==ss){
	  Nebula * neb;
	  if ((neb=_Universe->AccessCamera()->GetNebula())) {
	    if (neb->getFade()<=0) {
	      _Universe->AccessCamera()->SetNebula(NULL);//Update physics should set this
	    }
	  }
	}
}
extern Music *muzak;
void	TestMusic()
{
	if (muzak)
		  muzak->Listen();
}


void	GameStarSystem::createBackground( StarSystem::StarXML * xml)
{
#ifdef NV_CUBE_MAP
  printf("using NV_CUBE_MAP\n");
  LightMap[0]=new Texture ((xml->backgroundname+"_right_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_X);
  LightMap[1]=new Texture ((xml->backgroundname+"_left_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_X);
  LightMap[2]=new Texture ((xml->backgroundname+"_up_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_Y);
  LightMap[3]=new Texture ((xml->backgroundname+"_down_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_Y);
  LightMap[4]=new Texture ((xml->backgroundname+"_front_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_POSITIVE_Z);
  LightMap[5]=new Texture ((xml->backgroundname+"_back_light.bmp").c_str(),1,BILINEAR,CUBEMAP,CUBEMAP_NEGATIVE_Z);
#else
  //string bglight= VSFileSystem::sharedtextures+"/"+xml->backgroundname+"_light.bmp";
  string bglight= xml->backgroundname+"_light.bmp";
  string bgfile = xml->backgroundname+"_light.bmp";
  VSFile f;
  VSError err = f.OpenReadOnly(bgfile,TextureFile);
  if (err>Ok) {
      EnvironmentMapGeneratorMain (xml->backgroundname.c_str(),bglight.c_str(), 0,xml->reflectivity,1);
  }else {
	  f.Close();
  }
  LightMap[0] = new Texture(bgfile.c_str(), 1,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE);
  
#endif
  bg = new Background(xml->backgroundname.c_str(),xml->numstars,g_game.zfar*.9,filename);
  stars = new Stars (xml->numnearstars, xml->starsp);
  stars->SetBlend (XMLSupport::parse_bool(vs_config->getVariable ("graphics","starblend","true")),  
		   XMLSupport::parse_bool(vs_config->getVariable ("graphics","starblend","true")));
}

