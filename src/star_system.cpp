#include <assert.h>
#include "star_system.h"
#include "planet.h"
#include "cmd_unit.h"
#include "UnitCollection.h"
#include "gfx_click_list.h"
#include "gfx_hud.h"
#include "cmd_input_dfa.h"

#include "gfx_sphere.h"
extern Vector mouseline;
static SphereMesh *foo;
static Unit *earth;

StarSystem::StarSystem(Planet *primaries) : 
  primaries(primaries), 
  units(new UnitCollection()), 
  drawList(new UnitCollection()),
  missiles(new UnitCollection()), tp(new TextPlane("9x12.fon")) {
  currentcamera = 0;	
  systemInputDFA = new InputDFA (this);
  primaries->SetPosition(0,0,5);
  Iterator *iter = primaries->createIterator();
  drawList->prepend(iter);

  delete iter;
  iter = primaries->createIterator();
  iter->advance();
  earth=iter->current();
  delete iter;

  tp->SetPosition(0.5,0.5,1);

  // Calculate movement arcs; set behavior of primaries to follow these arcs
  //Iterator *primary_iterator = primaries->createIterator(); 
  //primaries->SetPosition(0,0,5);
  foo = new SphereMesh(1,5,5,"moon.bmp");

  cam[1].SetProjectionType(Camera::PARALLEL);
  cam[1].SetZoom(10.0);
  cam[1].SetPosition(Vector(0,5,0));
  cam[1].LookAt(Vector(0,0,0), Vector(0,0,1));
  //cam[1].SetPosition(Vector(0,5,-2.5));
  cam[1].SetSubwindow(0,0,0.25,0.25);

  cam[2].SetProjectionType(Camera::PARALLEL);
  cam[2].SetZoom(10.0);
  cam[2].SetPosition(Vector(5,0,0));
  cam[2].LookAt(Vector(0,0,0), Vector(0,-1,0));
  //cam[2].SetPosition(Vector(5,0,-2.5));
  cam[2].SetSubwindow(0.25,0,0.25,0.25);
}

StarSystem::~StarSystem() {
  delete systemInputDFA;
  delete primaries;
}

ClickList *StarSystem::getClickList() {
  return new ClickList (this, drawList);

}

void StarSystem::modelGravity() {
  primaries->gravitate(units);
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
  SetViewport();
  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->TDraw();
    iter->advance();
  }
  delete iter;

  Mesh::ProcessUndrawnMeshes();
  systemInputDFA->Draw();

  Vector p,q,r,pos;
  pos = cam[0].GetPosition();
  cam[0].GetOrientation(p,q,r);
  /*
  cam[1].SetPosition(q);
  cam[1].LookAt(Vector(0,0,0), Vector(0,0,-1));
  */

  currentcamera=1;
  SetViewport();
  GFXClear();
  {
  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->TDraw();
    iter->advance();
  }
  delete iter;
  }
  Mesh::ProcessUndrawnMeshes();

  glActiveTextureARB(GL_TEXTURE0_ARB);
  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glDisable(GL_TEXTURE_2D);

  GFXBlendMode(ONE,ZERO);
  GFXDisable(DEPTHTEST);
  GFXDisable(CULLFACE);
  GFXLoadIdentity(MODEL);
  glBegin(GL_POINTS);
  glColor4f(1,0,0,1);
  glVertex3f(pos.i,pos.j,pos.k);
  glVertex3f(pos.i+0.1,pos.j,pos.k);
  glVertex3f(pos.i+0.1,pos.j,pos.k+0.1);
  glVertex3f(pos.i,pos.j,pos.k+0.1);
  glEnd();

  glBegin(GL_LINES);
  glColor4f(0,1,0,1);
  glVertex3f(pos.i,pos.j,pos.k);
  glVertex3f(pos.i+mouseline.i*10,pos.j+mouseline.j*10,pos.k+mouseline.k*10);
  glEnd();

  GFXEnable(DEPTHTEST);
  GFXEnable(CULLFACE);

  currentcamera=2;
  SetViewport();
  /*  cam[2].SetPosition(p);
  cam[2].LookAt(Vector(0,0,0), Vector(0,0,-1));
  */
  GFXClear();
  {
  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
    unit->TDraw();
    iter->advance();
  }
  delete iter;
  }

  Mesh::ProcessUndrawnMeshes(); 

  glActiveTextureARB(GL_TEXTURE0_ARB);
  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB(GL_TEXTURE1_ARB);
  glDisable(GL_TEXTURE_2D);

  GFXBlendMode(ONE,ZERO);
  GFXDisable(DEPTHTEST);
  GFXDisable(CULLFACE);
  GFXLoadIdentity(MODEL);
  glBegin(GL_POINTS);
  glColor4f(1,0,0,1);
  glVertex3f(pos.i,pos.j,pos.k);
  glVertex3f(pos.i,pos.j+0.1,pos.k);
  glVertex3f(pos.i,pos.j+0.1,pos.k+0.1);
  glVertex3f(pos.i,pos.j,pos.k+0.1);
  glEnd();

  glBegin(GL_LINES);
  glColor4f(0,1,0,1);
  glVertex3f(pos.i,pos.j,pos.k);
  glVertex3f(pos.i+mouseline.i*10,pos.j+mouseline.j*10,pos.k+mouseline.k*10);
  glEnd();

  GFXEnable(DEPTHTEST);
  GFXEnable(CULLFACE);

  currentcamera=0;
  SetViewport();
}

void StarSystem::Update() {
  modelGravity();

  Iterator *iter = drawList->createIterator();
  Unit *unit;
  while((unit = iter->current())!=NULL) {
        unit->ResolveForces();
    // Do something with AI state here eventually
    unit->ExecuteAI();
    iter->advance();
  }
  delete iter;
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
