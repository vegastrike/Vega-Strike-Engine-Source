#include "briefing.h"
#include "unit_generic.h"
#include "unit_factory.h"
#include "gfx/mesh.h"
#include "script/mission.h"
#include "gfx/ani_texture.h"
#include "gfx/matrix.h"
Briefing::Ship::Ship (const char * filename, int faction, const Vector & position) {
  VSCONSTRUCT2('s')
  Unit * tmp  = UnitFactory::createUnit(filename,true,faction);
  meshdata = tmp->StealMeshes();
  tmp->Kill();
  cloak=1;
  SetPosition(position);
}
void Briefing::SetCloak(int w, float c) {
  if (w>=0&&w<starships.size()) {
    starships[w]->cloak=c;
  }
}
bool UpdatePosition (Vector & res, Vector cur, Vector fin, float speed) {
  Vector direction (fin-cur);
    float dirmag = direction.Magnitude();
    bool ret=true;
    if (dirmag>0&&dirmag>speed*SIMULATION_ATOM) {
      direction=direction* (speed*SIMULATION_ATOM/dirmag);
      ret=false;
    }  
    res= direction+cur;
    return ret;
}
void SetDirection (Matrix &mat, Vector start, Vector end, const Matrix cam, bool updatepos) {
  end = end-start;
  if (end.MagnitudeSquared()>.000001) {
    Vector p;
    Vector q (-cam.getR());
    Vector r(end);
    Normalize(r);
    q = q- r*(r.Dot (q)/r.MagnitudeSquared());
    if (q.MagnitudeSquared()<.000001) {
      q=r;
      q.Yaw (M_PI/2);
      q.Pitch (M_PI/2);
      q = q-r*(r.Dot(q)/r.MagnitudeSquared());
    }
    Normalize(q);
    ScaledCrossProduct (q,r,p);
    VectorAndPositionToMatrix (mat,p,q,r,QVector(0,0,0));
  }
}
extern double interpolation_blend_factor;
void Briefing::Render() {
	AnimatedTexture::UpdateAllFrame();

  cam.SetPosition(cam.GetPosition(),Vector(0,0,0));
  cam.UpdateGFX(GFXTRUE,GFXFALSE);
  //  glClearColor(1,0,0,1);
  //  GFXClear(GFXTRUE);
  for (unsigned int i=0;i<starships.size();i++) {
    starships[i]->Render(identity_matrix,interpolation_blend_factor);
  }
  GFXEnable(DEPTHTEST);
  GFXEnable (DEPTHWRITE);
  GFXBlendMode (ONE,ZERO);
  GFXDisable(CULLFACE);
  // 
  GFXClear(GFXTRUE);
  Mesh::ProcessUndrawnMeshes();
  _Universe->AccessCamera()->UpdateGFX(GFXTRUE,GFXFALSE);
  //  glClearColor(0,0,0,0);
}
void Briefing::Ship::Render (const Matrix &cam, double interpol) {
  Matrix final;
  Identity(final);
  Vector pos(Position());
  Vector dir = Position()+Vector (1,0,0);
  if (!orders.empty()) {
    UpdatePosition(pos,Position(),orders.front().vec,orders.front().speed*interpol);
    dir = orders.front().vec;
  }
  SetDirection (final,pos,dir,cam,!orders.empty());
  final.p =pos.Cast();
  
  Matrix camfinal;
  MultMatrix (camfinal,cam,final);
  for (unsigned int i=0;i<meshdata.size();i++) {
    short scloak = short (cloak*32767);
    if ((scloak&0x1)==0) {
      scloak+=1;
    }
    meshdata[i]->Draw(1,camfinal,1,cloak>.99?-1:scloak);
  }
}
Briefing::Ship::~Ship() {
  Destroy();
  VSDESTRUCT2
}
void Briefing::Ship::Destroy() {
  for (unsigned int i=0;i<meshdata.size();i++) {
    delete meshdata[i];
  }
  meshdata.clear();  
}
Briefing::Briefing() {
  VSCONSTRUCT2('b')
  cam.SetPosition(QVector(0,0,0),Vector(0,0,0));
  cam.SetOrientation(Vector(1,0,0),Vector(0,1,0),Vector(0,0,1));
  tp.SetPos (-1,1);
  tp.SetSize (1,-.5);
}
Briefing::~Briefing() {
  VSDESTRUCT2
  for (unsigned int i=0;i<starships.size();i++) {
    delete starships[i];
  }
  starships.clear();
}


void Briefing::RemoveStarship (int which) {
  if (which<(int)starships.size()&&which>=0) {
    starships[which]->Destroy();
  }
}
void Briefing::EnqueueOrder (int which, const Vector & dest, float time) {
  if (which<(int)starships.size()&&which>=0) {
    starships[which]->EnqueueOrder (dest,time);
  }
}
void Briefing::OverrideOrder (int which, const Vector & dest, float time) {
  if (which<(int)starships.size()&&which>=0) {
    starships[which]->OverrideOrder (dest,time);
  }
}
void Briefing::SetPosition (int which,const Vector &Position) {
  if (which<(int)starships.size()&&which>=0) {
    starships[which]->SetPosition (Position);
  }
}
Vector Briefing::GetPosition (int which) {
  if (which<(int)starships.size()&&which>=0) {
    return starships[which]->Position ();
  }
  return Vector(0,0,0);
}
int Briefing::AddStarship (const char * fn, int faction, const Vector &pos) {
  Ship * tmp = new Ship (fn,faction,pos);
  if (tmp->LoadFailed()) {
    delete tmp;
    return -1;
  }
  starships.push_back (tmp);
  return starships.size()-1;
}
void Briefing::Update() {
  for (unsigned int i=0;i<starships.size();i++) {
    starships[i]->Update();
  }
  
}
void Briefing::Ship::Update () {
  if (!orders.empty()) {
    Vector finpos;
    if (UpdatePosition(finpos,Position(),orders.front().vec,orders.front().speed)){
      orders.pop_front();
    } else {
      //      SetDirection (mat,Position(),orders.front().vec);
    }
    SetPosition(finpos);    
  }
  
}
void Briefing::Ship::OverrideOrder (const Vector & destination, float time) {
  orders.clear();
  EnqueueOrder (destination,time);
}

void Briefing::Ship::EnqueueOrder (const Vector & destination, float time) {
  if (time<.00001) {
    time =SIMULATION_ATOM;
  }
  orders.push_back (BriefingOrder (destination,(destination-Position()).Magnitude()/time));
}
