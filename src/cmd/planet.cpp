#include <math.h>
#include "vegastrike.h"
#include "planet.h"
#include "gfxlib.h"
#include "gfx/sphere.h"
#include "collection.h"
#include "gfx/bsp.h"
#include "ai/order.h"
#include "gfxlib_struct.h"
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>
PlanetaryOrbit:: PlanetaryOrbit(Unit *p, double velocity, double initpos, const Vector &x_axis, const Vector &y_axis, const Vector & centre, Unit * targetunit) : Order(MOVEMENT), parent(p), velocity(velocity), theta(initpos), x_size(x_axis), y_size(y_axis) { 
  parent->SetResolveForces(false);
    double delta = x_size.Magnitude() - y_size.Magnitude();
    if(delta == 0) {
      focus = Vector(0,0,0);
    }
    else if(delta>0) {
      focus = x_size*(delta/x_size.Magnitude());
    } else {
      focus = y_size*(-delta/y_size.Magnitude());
    }
    if (targetunit) {
      type = (MOVEMENT | TARGET);
      UnitCollection tmpcoll;
      tmpcoll.prepend (targetunit);
      AttachOrder (&tmpcoll);
    } else {
      type = (MOVEMENT | LOCATION);
      AttachOrder (centre);
    }
}
PlanetaryOrbit::~PlanetaryOrbit () {
  parent->SetResolveForces (true);
}
void PlanetaryOrbit::Execute() {
  if (done) 
    return;
  Vector x_offset = cos(theta) * x_size;
  Vector y_offset = sin(theta) * y_size;
  Vector origin (targetlocation);
  if (type&TARGET) {
    if (targets) {
      UnitCollection::UnitIterator * tmp = targets->createIterator();
      if (tmp->current()) {
	origin = tmp->current()->Position();
	delete tmp;
      }else {
	delete tmp;
	done = true;
	return;
      }
    }
  }
  double radius =  sqrt((x_offset - focus).MagnitudeSquared() + (y_offset - focus).MagnitudeSquared());
  theta+=velocity/(radius?radius:1) *SIMULATION_ATOM;
  parent->Velocity = (origin - focus + x_offset+y_offset-parent->curr_physical_state.position)/SIMULATION_ATOM;
}


void Planet::endElement() {  
}

void Planet::beginElement(Vector x,Vector y,float vely,float pos,float gravity,float radius,char * filename,char * alpha,vector<char *> dest,int level,  const GFXMaterial & ourmat, const vector <GFXLightLocal>& ligh, bool isunit, int faction){
  UnitCollection::UnitIterator * satiterator =NULL;
  if (level>2) {
    UnitCollection::UnitIterator * satiterator = satellites.createIterator();
	  assert(satiterator->current()!=NULL);
	  if (satiterator->current()->isUnit()==PLANETPTR) {
		((Planet *)satiterator->current())->beginElement(x,y,vely,pos,gravity,radius,filename,alpha,dest,level-1,ourmat,ligh, isunit, faction);
	  } else {
	    fprintf (stderr,"Planets are unable to orbit around units");
	  }
  } else {
    if (isunit==true) {
      satellites.prepend(new Unit (filename, true, false, faction));
      satiterator = satellites.createIterator();
      satiterator->current()->SetAI (new PlanetaryOrbit (satiterator->current(),vely,pos,x,y, Vector (0,0,0), this)) ;
      satiterator->current()->SetOwner (this);
    }else {
      satellites.prepend(new Planet(x,y,vely,pos,gravity,radius,filename,alpha,dest, Vector (0,0,0), this, ourmat, ligh, faction));
    }
  }
  delete satiterator;
}

const float densityOfRock = .01; // 1 cm of durasteel equiv per cubic meter

Planet::Planet()  : Unit(), radius(0.0f), satellites() {
  Init();
  SetAI(new Order()); // no behavior
}

Planet::Planet(Vector x,Vector y,float vely, float pos,float gravity,float radius,char * textname,char * alpha,vector <char *> dest, const Vector & orbitcent, Unit * parent, const GFXMaterial & ourmat, const std::vector <GFXLightLocal> &ligh, int faction) : Unit(), radius(0.0f),  satellites() {
  for (unsigned int i=0;i<ligh.size();i++) {
    int l;
    GFXCreateLight (l,ligh[i].ligh,!ligh[i].islocal);
    lights.push_back (l);
  }
  curr_physical_state.position = prev_physical_state.position=cumulative_transformation.position=orbitcent+x;
  destination=dest;
  Init();
  this->faction = faction;
  killed=false;
  name = "Planet - ";
  name += textname;
  this->radius=radius;
  this->gravity=gravity;
  hull = (4./3)*M_PI*radius*radius*radius*densityOfRock;
  SetAI(new PlanetaryOrbit(this, vely, pos, x, y, orbitcent, parent)); // behavior

  meshdata = new Mesh*[2];
  static int stacks=XMLSupport::parse_int(vs_config->getVariable ("graphics","planet_detail","24"));
  meshdata[0] = new SphereMesh(radius, stacks, stacks, textname, alpha);
  meshdata[0]->setEnvMap(GFXFALSE);
  meshdata[0]->SetMaterial (ourmat);
  nummesh = 1;

  calculate_extent();
  /*stupid Sphere BSP when intersection should do
  string tmpname ("sphere");
  char temp [64];
  sprintf (temp, "%f", radius);
  tmpname +=temp;
  tmpname +=".bsp";
  
  FILE * fp = fopen (tmpname.c_str(), "rb");
  if (!fp) {
    meshdata[1]= new SphereMesh (radius,8,8,textname, alpha);
    BuildBSPTree (tmpname.c_str(),true,meshdata[1]);
    delete meshdata[1];
  } else {
    fclose (fp);
  }
  bspTree = new BSPTree (tmpname.c_str());
  */
  meshdata[1]=NULL;
}
void Planet::Draw(const Transformation & quat, const Matrix m) {
  //Do lighting fx
  Unit::Draw(quat,m);
  GFXLoadIdentity (MODEL);
  for (unsigned int i=0;i<lights.size();i++) {
    GFXSetLight (lights[i], POSITION,GFXColor (cumulative_transformation.position));
  }
}
Planet::~Planet() { 
	unsigned int i;
	if (bspTree)
	  delete bspTree;
	for (i=0;i<this->destination.size();i++) {
		delete [] destination[i];
	}
	for (i=0;i<(int)this->lights.size();i++) {
	  GFXDeleteLight (lights[i]);
	}
}

void Planet::Kill() {
	UnitCollection::UnitIterator * iter;
	Unit *tmp;
	for (iter = satellites.createIterator();
	     (tmp = iter->current())!=NULL;
	     iter->advance()) {
	  tmp->SetAI (new Order);
	}
	delete iter;
	Unit::Kill();
}

void Planet::gravitate(UnitCollection *uc) {
  float *t = cumulative_transformation_matrix;

  /*
  if(gravity!=0.0&&uc) {
    Iterator *iterator = uc->createIterator();
    Unit *unit;
    Vector vec(0,0,0);
    
    while((unit = iterator->current())!=NULL) {
      if(unit->type()!=PLANETPTR) {
	Vector r = (unit->Position() - (vec.Transform(t)));
	//      cerr << "Unit (" << unit << "): " << endl;
	//      cerr << "Gravity source: " << vec.Transform(t) << "\nUnit position: " << unit->Position() << "\nDelta: " << r << endl;
	float _r_ = r.Magnitude();
	r = r * (1.0/_r_);
	r =  r * -(gravity/(_r_*_r_));
	//      cerr << "Distance: " << _r_ << "\nGravity force vector: " << r << endl;
      
	if(_r_ > radius) {
	  unit->Accelerate(r);
	}
      }
      iterator->advance();
    }
    delete iterator;
    }*/

  // fake gravity
  /***FIXME 091401 why do we need to traverse satellites??????
  UnitCollection::UnitIterator * iter;
  for (iter = satellites.createIterator();
       iter->current()!=NULL;
       iter->advance()) {
	if (iter->current()->isUnit()==PLANETPTR) 
	    ((Planet *)iter->current())->gravitate(uc);//FIXME 071201
	else { //FIXME...[causes flickering for crashing orbiting units
	
	  //((Unit *)iter->current())->ResolveForces (identity_transformation,identity_matrix,false); 
	  ((Unit *)iter->current())->UpdateCollideQueue();
	}
  }
  delete iter;
  **/
  UpdateCollideQueue();
}
