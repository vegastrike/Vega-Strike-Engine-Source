#include "briefing.h"
#include "unit.h"
#include "gfx/mesh.h"
Briefing::Ship::Ship (const char * filename, int faction, const Vector & position) {
  Unit * tmp  = new Unit (filename,true,faction);
  meshdata = tmp->StealMeshes();
  tmp->Kill();
  SetPosition(position);
}
bool UpdatePosition (Vector & res, Vector cur, Vector fin, float speed) {
  Vector direction (fin-cur);
    float dirmag = direction.Magnitude();
    bool ret=true;
    if (dirmag>0&&dirmag<speed*SIMULATION_ATOM) {
      direction=direction* (speed*SIMULATION_ATOM/dirmag);
      ret=false;
    }  
    res= direction+cur;
    return ret;
}
void SetDirection (Matrix mat, Vector start, Vector end, const Matrix cam) {
  end = end-start;
  if (end.MagnitudeSquared()>.000001) {
    Vector p;
    Vector q (-cam[8],-cam[9],-cam[10]);
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
    VectorToMatrix (mat,p,q,r);
  }
}
extern double interpolation_blend_factor;
void Briefing::Render() {
  for (unsigned int i=0;i<starships.size();i++) {
    starships[i]->Render(identity_matrix,interpolation_blend_factor);
  }
}
void Briefing::Ship::Render (const Matrix cam, double interpol) {
  Matrix final;
  Identity(final);
  if (!orders.empty()) {
    Vector pos;
    UpdatePosition(pos,Position(),orders.front().vec,orders.front().speed*interpol);
    SetDirection (final,Position(),orders.front().vec,cam);
    final[12]=pos.i;
    final[13]=pos.j;
    final[14]=pos.k;
  }
  Matrix camfinal;
  MultMatrix (camfinal,cam,final);
  for (unsigned int i=0;i<meshdata.size();i++) {
    meshdata[i]->Draw(1,camfinal,1,short(cloak*32767));
  }
}
Briefing::Ship::~Ship() {
  Destroy();
}
void Briefing::Ship::Destroy() {
  for (unsigned int i=0;i<meshdata.size();i++) {
    delete meshdata[i];
  }
  meshdata.clear();  
}

Briefing::~Briefing() {
  for (unsigned int i=0;i<starships.size();i++) {
    delete starships[i];
  }
  starships.clear();
}


void Briefing::RemoveStarship (int which) {
  if (which<(int)starships.size()&&which>0) {
    starships[which]->Destroy();
  }
}
void Briefing::EnqueueOrder (int which, const Vector & dest, float time) {
  if (which<(int)starships.size()&&which>0) {
    starships[which]->EnqueueOrder (dest,time);
  }
}
void Briefing::OverrideOrder (int which, const Vector & dest, float time) {
  if (which<(int)starships.size()&&which>0) {
    starships[which]->OverrideOrder (dest,time);
  }
}
void Briefing::SetPosition (int which,const Vector &Position) {
  if (which<(int)starships.size()&&which>0) {
    starships[which]->SetPosition (Position);
  }
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
  orders.push_back (Order (destination,(destination-Position()).Magnitude()/time));
}
