#include <assert.h>
#include "star_system.h"
#include "planet.h"
#include "cmd_unit.h"
#include "UnitCollection.h"
#include "gfx_click_list.h"
#include "gfx_hud.h"
#include "cmd_input_dfa.h"
#include "lin_time.h"
#include "cmd_beam.h"
#include "gfx_sphere.h"
#include "cmd_collide.h"
#include "gfx_halo.h"
#include "gfx_background.h"
#include <expat.h>
extern Vector mouseline;

vector<Vector> perplines;
//static SphereMesh *foo;
//static Unit *earth;

StarSystem::StarSystem(char * filename) : 
//  primaries(primaries), 
  drawList(new UnitCollection(true)),//what the hell is this...maybe FALSE FIXME
  units(new UnitCollection(true)), 
  missiles(new UnitCollection(true)), tp(new TextPlane("9x12.fon")) {
  
  currentcamera = 0;	
  systemInputDFA = new InputDFA (this);
  LoadXML(filename);
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

  tp->SetPosition(0.5,0.5,1);

  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  //foo = new SphereMesh(1,5,5,"moon.bmp");
  cam[1].SetProjectionType(Camera::PARALLEL);
  cam[1].SetZoom(10.0);
  cam[1].SetPosition(Vector(0,5,0));
  cam[1].LookAt(Vector(0,0,0), Vector(0,0,1));
  //cam[1].SetPosition(Vector(0,5,-2.5));
  cam[1].SetSubwindow(0,0,0.10,0.10);

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
  delete [] name;
  delete systemInputDFA;
  for (int i=0;i<numprimaries;i++) {
	delete primaries[i];
  }
  delete [] primaries;
}

ClickList *StarSystem::getClickList() {
  return new ClickList (this, drawList);

}

void StarSystem::modelGravity() {
	for (int i=0;i<numprimaries;i++) {
       if (primaries[i]->isUnit ()==PLANETPTR) 
		   ((Planet *)primaries[i])->gravitate(units);
	}
}

void StarSystem::AddUnit(Unit *unit) {
  units->prepend(unit);
  drawList->prepend(unit);
}

void StarSystem::RemoveUnit(Unit *unit) {
  assert(0);
}

void StarSystem::Draw() {
  //primaries->Draw();
  //    systemInputDFA->Draw(); return;
  currentcamera=0;

  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->Draw();
    iter->advance();
  }
  SetViewport();//camera wielding unit is now drawn
  GFXDisable (LIGHTING);
  bg->Draw();

  delete iter;
  GFXDisable(LIGHTING);
  Mesh::ProcessUndrawnMeshes();
  //  Halo::ProcessDrawQueue();
  systemInputDFA->Draw();
}

extern double interpolation_blend_factor;

void StarSystem::Update() {
  Unit *unit;
  UpdateTime();
  time += GetElapsedTime();
  //clog << "time: " << time << "\n";
  //time = SIMULATION_ATOM+SIMULATION_ATOM/2.0;
  bool firstframe = true;
  if(time/SIMULATION_ATOM>=1.0) {
    while(time/SIMULATION_ATOM >= 1.0) { // Chew up all SIMULATION_ATOMs that have elapsed since last update
      // Handle AI in pass 2 to maintain consistency

      ClearCollideQueue();
      modelGravity();
      Iterator *iter = drawList->createIterator();
      while((unit = iter->current())!=NULL) {
	unit->ExecuteAI(); // must execute AI afterwards, since position might update (and ResolveLast=true saves the 2nd to last position for proper interpolation)
	iter->advance();
      }
      delete iter;

      iter = units->createIterator();
      while((unit = iter->current())!=NULL) {
	unit->ResolveForces(identity_transformation,identity_matrix,firstframe);
	iter->advance();
      }
      delete iter;
      iter = drawList->createIterator();
      while((unit = iter->current())!=NULL) {
	unit->CollideAll(); //Goes in an endless loop...
	iter->advance();
      }
      delete iter;
		  
      time -= SIMULATION_ATOM;
      firstframe = false;
    }
    UpdateTime();
  }
  interpolation_blend_factor = time/SIMULATION_ATOM;
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
