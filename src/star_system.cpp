#include <assert.h>
#include "star_system.h"
#include "cmd/planet.h"
#include "cmd/unit.h"
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
#include "gfx/quadtree.h"
extern Music *muzak;
extern Vector mouseline;
extern QuadTree * qt;
vector<Vector> perplines;
//static SphereMesh *foo;
//static Unit *earth;

StarSystem::StarSystem(char * filename, const Vector & centr,const string planetname) : 
//  primaries(primaries), 
  drawList(new UnitCollection),//what the hell is this...maybe FALSE FIXME
  units(new UnitCollection), 
  missiles(new UnitCollection) {

  cout << "origin: " << centr.i << " " << centr.j << " " << centr.k << " " << planetname << endl;

  current_stage=PHY_AI;
  currentcamera = 0;	
  systemInputDFA = new InputDFA (this);
  numprimaries=0;
  LoadXML(filename,centr);
//  primaries[0]->SetPosition(0,0,0);
  int i;
  Iterator * iter;
  for (i =0;i<numprimaries;i++) {
	  if (primaries[i]->isUnit()==PLANETPTR) {
		iter = ((Planet*)primaries[i])->createIterator();
		drawList->prepend(iter);
		delete iter;
	  } else {
		drawList->prepend (primaries[i]);
	  }
  }
  //iter = primaries->createIterator();
  //iter->advance();
  //earth=iter->current();
  //delete iter;



  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  //foo = new SphereMesh(1,5,5,"moon.bmp");
  cam[1].SetProjectionType(Camera::PARALLEL);
  cam[1].SetZoom(1);
  cam[1].SetPosition(Vector(0,0,0));
  cam[1].LookAt(Vector(0,0,0), Vector(0,0,1));
  //cam[1].SetPosition(Vector(0,5,-2.5));
  cam[1].SetSubwindow(0,0,1,1);

  cam[2].SetProjectionType(Camera::PARALLEL);
  cam[2].SetZoom(10.0);
  cam[2].SetPosition(Vector(5,0,0));
  cam[2].LookAt(Vector(0,0,0), Vector(0,-1,0));
  //cam[2].SetPosition(Vector(5,0,-2.5));
  cam[2].SetSubwindow(0.10,0,0.10,0.10);
  UpdateTime();
  time = 0;

}
void StarSystem::activateLightMap() {
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
}

StarSystem::~StarSystem() {
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
  delete [] name;
  delete systemInputDFA;
  for (int i=0;i<numprimaries;i++) {
	delete primaries[i];
  }
  delete [] primaries;
  Bolt::Cleanup();
}

UnitCollection * StarSystem::getUnitList () {
  return drawList;
}

ClickList *StarSystem::getClickList() {
  return new ClickList (this, drawList);

}
/**OBSOLETE!
void StarSystem::modelGravity(bool lastframe) {
  for (int i=0;i<numprimaries;i++) {
    primaries[i]->UpdatePhysics (identity_transformation,identity_matrix,lastframe,units)
  }
}	
*/
void StarSystem::AddUnit(Unit *unit) {
  units->prepend(unit);
  drawList->prepend(unit);
}

void StarSystem::RemoveUnit(Unit *unit) {
  assert(0);
}

void StarSystem::Draw() {

  GFXDisable (LIGHTING);
  bg->Draw();

  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->Draw();
    Vector norm;
    float t;
    if ((t=qt->GetHeight (unit->Position(),norm))<0) {
      fprintf (stderr,"Unit %s crashed at height %f <%f,%f,%f>", unit->name.c_str(),t,norm.i,norm.j,norm.k);
    }
    iter->advance();
  }
  delete iter;
  _Universe->AccessCockpit()->SetupViewPort(true);///this is the final, smoothly calculated cam
  
  //  SetViewport();//camera wielding unit is now drawn  Note: Background is one frame behind...big fat hairy deal
  GFXColor tmpcol (0,0,0,1);
  GFXGetLightContextAmbient(tmpcol);
  Mesh::ProcessZFarMeshes();
  qt->Render();
  Mesh::ProcessUndrawnMeshes(true);
  GFXPopGlobalEffects();
  GFXLightContextAmbient(tmpcol);
  Halo::ProcessDrawQueue();
  Beam::ProcessDrawQueue();
  Animation::ProcessDrawQueue();
  Bolt::Draw();

  stars->Draw();
  static bool doInputDFA = XMLSupport::parse_bool (vs_config->getVariable ("graphics","MouseCursor","false"));
  _Universe->AccessCockpit()->Draw();
  if (doInputDFA) {
    GFXHudMode (true);
    systemInputDFA->Draw();
    GFXHudMode (false);
  }

}

extern double interpolation_blend_factor;

void StarSystem::Update() {
  static int numframes;
  Unit *unit;


  
  UpdateTime();
  time += GetElapsedTime();
  //clog << "time: " << time << "\n";
  //time = SIMULATION_ATOM+SIMULATION_ATOM/2.0;
  bool firstframe = true;
  if(time/SIMULATION_ATOM>=.33333333) {
    while(time/SIMULATION_ATOM >= .333333333) { // Chew up all SIMULATION_ATOMs that have elapsed since last update
      // Handle AI in pass 2 to maintain consistency

      Iterator *iter;
      if (current_stage==PHY_AI) {
      iter = drawList->createIterator();
      if (firstframe&&rand()%2) {
	AUDRefreshSounds();
      }

      while((unit = iter->current())!=NULL) {
	unit->ExecuteAI(); // must execute AI afterwards, since position might update (and ResolveLast=true saves the 2nd to last position for proper interpolation)
	unit->ResetThreatLevel();
	iter->advance();
      }
      delete iter;
      current_stage=PHY_COLLIDE;
      } else
      if (current_stage==PHY_COLLIDE) {
	static int stage=0;
	qt->Update(64,((stage++)%64));
	numframes++;//don't resolve physics until 2 seconds
	if (numframes>2/(SIMULATION_ATOM)) {
	  iter = drawList->createIterator();
	  while((unit = iter->current())!=NULL) {
	    unit->CollideAll();
	    iter->advance();
	  }
	  delete iter;
	}
	current_stage=PHY_RESOLV;
      } else
      if (current_stage==PHY_RESOLV) {
      iter = drawList->createIterator();
      AccessCamera()->UpdateCameraSounds();
	  muzak->Listen();
      while((unit = iter->current())!=NULL) {

	unit->UpdatePhysics(identity_transformation,identity_matrix,Vector (0,0,0),firstframe,units);
	iter->advance();
      }
      delete iter;
      
      
      Bolt::UpdatePhysics();
      current_stage=PHY_AI;
      firstframe = false;
      }
      time -= .3333333333*SIMULATION_ATOM;

    }
    //    UpdateTime();
  }
  interpolation_blend_factor = .333333*((3*time)/SIMULATION_ATOM+current_stage);
  //  fprintf (stderr,"bf:%lf",interpolation_blend_factor);
  //clog << "blend factor: " << interpolation_blend_factor << "\n";
}


void StarSystem::SelectCamera(int cam){
    if(cam<NUM_CAM&&cam>=0)
      currentcamera = cam;
}
Camera* StarSystem::AccessCamera(int num){
  if(num<NUM_CAM&&num>=0)
    return &cam[num];
  else
    return NULL;
}
