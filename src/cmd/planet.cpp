#include <math.h>
#include "vegastrike.h"
#include "planet.h"
#include "unit_factory.h"
#include "gfxlib.h"
#include "gfx/sphere.h"
#include "collection.h"
#include "gfx/bsp.h"
#include "ai/order.h"
#include "gfxlib_struct.h"
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>
#include "cont_terrain.h"
#include "atmosphere.h"
#include "gfx/planetary_transform.h"
#include "collide/rapcol.h"
#include "images.h"
#include "gfx/halo.h"
#include "gfx/animation.h"
#include "cmd/script/flightgroup.h"
PlanetaryOrbit:: PlanetaryOrbit(Unit *p, double velocity, double initpos, const QVector &x_axis, const QVector &y_axis, const QVector & centre, Unit * targetunit) : Order(MOVEMENT,0), parent(p), velocity(velocity), theta(initpos), x_size(x_axis), y_size(y_axis) { 
  parent->SetResolveForces(false);
    double delta = x_size.Magnitude() - y_size.Magnitude();
    if(delta == 0) {
      focus = QVector(0,0,0);
    }
    else if(delta>0) {
      focus = x_size*(delta/x_size.Magnitude());
    } else {
      focus = y_size*(-delta/y_size.Magnitude());
    }
    if (targetunit) {
      type = (MOVEMENT);subtype=( SSELF);
      AttachSelfOrder (targetunit);
    } else {
      type = (MOVEMENT);subtype =(SLOCATION);
      AttachOrder (centre);
    }
}
PlanetaryOrbit::~PlanetaryOrbit () {
  parent->SetResolveForces (true);
}
void PlanetaryOrbit::Execute() {
  if (done) 
    return;
  QVector x_offset = cos(theta) * x_size;
  QVector y_offset = sin(theta) * y_size;
  QVector origin (targetlocation);
  if (subtype&SSELF) {
      Unit * tmp = group.GetUnit();
      if (tmp) {
	origin = tmp->Position();
      }else {
	done = true;
	return;
      }
  }
  //unuseddouble radius =  sqrt((x_offset - focus).MagnitudeSquared() + (y_offset - focus).MagnitudeSquared());
  theta+=velocity*SIMULATION_ATOM;
  parent->Velocity = ((origin - focus + x_offset+y_offset-parent->LocalPosition())*(1./SIMULATION_ATOM)).Cast();
  const int Unreasonable_value=(int)(100000/SIMULATION_ATOM);
  if (parent->Velocity.Dot (parent->Velocity)>Unreasonable_value*Unreasonable_value) {
    parent->Velocity.Set (0,0,0);
    parent->SetCurPosition (origin-focus+x_offset+y_offset);
  }
}


void Planet::endElement() {  
}
Planet * Planet::GetTopPlanet (int level) {
  if (level>2) {
    UnitCollection::UnitIterator satiterator = satellites.createIterator();
	  assert(satiterator.current()!=NULL);
	  if (satiterator.current()->isUnit()==PLANETPTR) {
	    return ((Planet *)satiterator.current())->GetTopPlanet (level-1);
	  } else {
	    fprintf (stderr,"Planets are unable to orbit around units");
	    return NULL;
	  }

  } else {
    return this;
  }
  
}
void Planet::AddSatellite (Unit * orbiter) {
	satellites.prepend (orbiter);
	orbiter->SetOwner (this);
}
extern Flightgroup * getStaticBaseFlightgroup(int faction);
void Planet::beginElement(QVector x,QVector y,float vely, const Vector & rotvel, float pos,float gravity,float radius,const char * filename,const char * citylights,BLENDFUNC blendSrc, BLENDFUNC blendDst, vector<char *> dest,int level,  const GFXMaterial & ourmat, const vector <GFXLightLocal>& ligh, bool isunit, int faction,string fullname, bool inside_out){
  //this function is OBSOLETE
  if (level>2) {
    UnitCollection::UnitIterator satiterator = satellites.createIterator();
	  assert(satiterator.current()!=NULL);
	  if (satiterator.current()->isUnit()==PLANETPTR) {
		((Planet *)satiterator.current())->beginElement(x,y,vely,rotvel, pos,gravity,radius,filename,citylights,blendSrc,blendDst,dest,level-1,ourmat,ligh, isunit, faction,fullname,inside_out);
	  } else {
	    fprintf (stderr,"Planets are unable to orbit around units");
	  }
  } else {
    if (isunit==true) {
      Unit *sat_unit=NULL;
      Flightgroup *fg = getStaticBaseFlightgroup(faction);
      satellites.prepend(sat_unit=UnitFactory::createUnit (filename, false, faction,"",fg,fg->nr_ships-1));
      sat_unit->setFullname(fullname);
      un_iter satiterator (satellites.createIterator());
      satiterator.current()->SetAI (new PlanetaryOrbit (satiterator.current(),vely,pos,x,y, QVector (0,0,0), this)) ;
      satiterator.current()->SetOwner (this);
    }else {
      Planet * p;
      satellites.prepend(p=UnitFactory::createPlanet(x,y,vely,rotvel,pos,gravity,radius,filename,citylights,blendSrc,blendDst,dest, QVector (0,0,0), this, ourmat, ligh, faction,fullname,inside_out));
      p->SetOwner (this);
    }
  }
}

const float densityOfRock = .01; // 1 cm of durasteel equiv per cubic meter
const float densityOfJumpPoint = 100000;
Planet::Planet()
    : Unit( 0 )
    ,  atmosphere (NULL), terrain (NULL), radius(0.0f), satellites()
{
  inside=false;
  Init();
  terraintrans = NULL;
  SetAI(new Order()); // no behavior
}
char * getnoslash (char * inp) {
  char * tmp=inp;
  for (unsigned int i=0;inp[i]!='\0';i++) {
    if (inp[i]=='/'||inp[i]=='\\') {
      tmp=inp+i+1;
    }
  }  
  return tmp;
}
string getCargoUnitName (const char * textname) {
  char * tmp2 = strdup (textname);
  char * tmp = getnoslash(tmp2);
  unsigned int i;
  for (i=0;tmp[i]!='\0'&&(isalpha(tmp[i])||tmp[i]=='_');i++) {
    
  }
  if (tmp[i]!='\0') {
    tmp[i]='\0';
  }
  string retval(tmp);
  free(tmp2);
  return retval;
}



extern vector <char *> ParseDestinations (const string &value);
Planet::Planet(QVector x,QVector y,float vely, const Vector & rotvel, float pos,float gravity,float radius,const char * textname,const char * citylights,BLENDFUNC blendSrc, BLENDFUNC blendDst, vector <char *> dest, const QVector & orbitcent, Unit * parent, const GFXMaterial & ourmat, const std::vector <GFXLightLocal> &ligh, int faction,string fgid, bool inside_out)
    : Unit( 0 )
    , atmosphere(NULL), terrain(NULL), radius(0.0f),  satellites(),shine(NULL)
{
  static float bodyradius = XMLSupport::parse_float(vs_config->getVariable ("graphics","star_body_radius",".5"));
  radius*=bodyradius;
  inside =false;
  for (unsigned int i=0;i<ligh.size();i++) {
    int l;
    GFXCreateLight (l,ligh[i].ligh,!ligh[i].islocal);
    lights.push_back (l);
  }
  curr_physical_state.position = prev_physical_state.position=cumulative_transformation.position=orbitcent+x;
  Init();

  this->faction = faction;
  killed=false;
  while (!dest.empty()) {
    AddDestination(dest.back());
    dest.pop_back();
  }
  //name = "Planet - ";
  //name += textname;
  name=fgid;
  fullname=fgid;
  this->radius=radius;
  this->gravity=gravity;
  hull = (4./3)*M_PI*radius*radius*radius*(dest.empty()?densityOfRock:densityOfJumpPoint);
  SetAI(new PlanetaryOrbit(this, vely, pos, x, y, orbitcent, parent)); // behavior
  terraintrans=NULL;

  static int stacks=XMLSupport::parse_int(vs_config->getVariable ("graphics","planet_detail","24"));
  //  BLENDFUNC blendSrc=SRCALPHA;
  //  BLENDFUNC blendDst=INVSRCALPHA;
  atmospheric=!(blendSrc==ONE&&blendDst==ZERO);
  if ((!citylights)?true:(citylights[0]=='\0')) {
    meshdata = new Mesh*[2];
    nummesh = 1;
  }else {
    meshdata = new Mesh *[3];
    GFXMaterial m;
    m.ar=m.ag=m.ab=m.aa=1.0;
    m.dr=m.dg=m.db=m.da=0.0;
    m.sr=m.sg=m.sb=m.sa=0.0;
    m.er=m.eg=m.eb=m.ea=0.0;
    meshdata[1]= new CityLights (radius,stacks,stacks, citylights, NULL, inside_out,ONE, ONE);
    meshdata[1]->setEnvMap (GFXFALSE);
    meshdata[1]->SetMaterial (m);
    nummesh = 2;
  }
  meshdata[0] = new SphereMesh(radius, stacks, stacks, textname, NULL,inside_out,blendSrc,blendDst);
  meshdata[0]->setEnvMap(GFXFALSE);
  meshdata[0]->SetMaterial (ourmat);


  calculate_extent(false);

  /*stupid Sphere BSP when intersection should do
  string tmpname ("sphere");
  char temp [64];
  sprintf (temp, "%f", radius);
  tmpname +=temp;
  tmpname +=".bsp";
  
  FILE * fp = fopen (tmpname.c_str(), "rb");
  if (!fp) {
  */
#ifdef RAPIDCOLLIDEPLANET
  meshdata[nummesh]= new SphereMesh (radius,8,8,textname, alpha);
  std::vector <bsp_polygon> spherepolys;
  meshdata[nummesh]->GetPolys (spherepolys);
  colTree = new csRapidCollider (spherepolys);
  //BuildBSPTree (tmpname.c_str(),true,meshdata[1]);
  delete meshdata[nummesh];
#else
  colTrees= NULL;
#endif
  meshdata[nummesh]=NULL;
    /*
      } else {
      fclose (fp);
      }
      bspTree = new BSPTree (tmpname.c_str());
  */

  SetAngularVelocity (rotvel);
  static int numdock = XMLSupport::parse_int(vs_config->getVariable ("physics","num_planet_docking_port","4"));
  static float planetdockportsize= XMLSupport::parse_float(vs_config->getVariable ("physics","planet_port_size","1.2"));
  static float planetdockportminsize= XMLSupport::parse_float(vs_config->getVariable ("physics","planet_port_min_size","300"));
  if (!atmospheric) {
    for (int pdp=0;pdp<numdock;pdp++) {
      float dock = radius*planetdockportsize;
      if (dock-radius<planetdockportminsize) {
	dock = radius+planetdockportminsize;
      }
      image->dockingports.push_back (DockingPorts (Vector(0,0,0),dock,true));
    }
  }
  if (ligh.size()>0) {
    static bool drawglow = XMLSupport::parse_bool(vs_config->getVariable ("graphics","draw_star_glow","true"));

    static bool drawstar = XMLSupport::parse_bool(vs_config->getVariable ("graphics","draw_star_body","true"));
    static float glowradius = XMLSupport::parse_float(vs_config->getVariable ("graphics","star_glow_radius","1.33"))/bodyradius;
    static bool far_shine = XMLSupport::parse_bool(vs_config->getVariable ("graphics","draw_star_glow_halo","false"));
    if (drawglow) {
      GFXColor c(ourmat.er,ourmat.eg,ourmat.eb,ourmat.ea);
      static bool spec = XMLSupport::parse_bool(vs_config->getVariable ("graphics","glow_ambient_star_light","false"));
      static bool diff = XMLSupport::parse_bool(vs_config->getVariable ("graphics","glow_diffuse_star_light","false"));
      if (diff)
	c= ligh[0].ligh.GetProperties(DIFFUSE);
      if (spec)
	c= ligh[0].ligh.GetProperties(AMBIENT);

	
      static vector <char *> shines = ParseDestinations (vs_config->getVariable("graphics","star_shine","shine.ani"));
      if (shines.empty()) {
	shines.push_back("shine.ani");
      }
      shine = new Animation (shines[rand()%shines.size()],true,.1,MIPMAP,true,true,c);//GFXColor(ourmat.er,ourmat.eg,ourmat.eb,ourmat.ea));
      shine->SetDimensions ( glowradius*radius,glowradius*radius);
    
      if (!drawstar) {
	delete meshdata[0];
	meshdata[0]=NULL;
	nummesh=0;
      }
    }
  }
  cargounitname =::getCargoUnitName (textname);
 
  Unit * un = UnitFactory::createUnit (cargounitname.c_str(),true,_Universe->GetFaction("planets"));
  if (un->name!=string("LOAD_FAILED")) {
    image->cargo=un->GetImageInformation().cargo;
    image->cargo_volume=un->GetImageInformation().cargo_volume;
  }
  un->Kill();
}
extern bool shouldfog;

vector <UnitContainer *> PlanetTerrainDrawQueue;
void Planet::Draw(const Transformation & quat, const Matrix &m) {
  //Do lighting fx
  // if cam inside don't draw?
  //  if(!inside) {
  Unit::Draw(quat,m);
  //  }
    QVector t (_Universe->AccessCamera()->GetPosition()-Position());
    static int counter=0;
    if (counter ++>100)
      if (t.Magnitude()<corner_max.i) {
	inside=true;
      } else {
	//if ((terrain&&t.Dot (TerrainH)>corner_max.i)||(!terrain&t.Dot(t)>corner_max.i*corner_max.i)) {
	inside=false;
	///somehow warp unit to reasonable place outisde of planet
	if (terrain) {
#ifdef PLANETARYTRANSFORM
	  terrain->DisableUpdate();
#endif
	  
	}
      }

 GFXLoadIdentity (MODEL);
 for (unsigned int i=0;i<lights.size();i++) {
   GFXSetLight (lights[i], POSITION,GFXColor (cumulative_transformation.position.Cast()));
 }

 if (inside&&terrain) {
   PlanetTerrainDrawQueue.push_back (new UnitContainer (this));
   //DrawTerrain();
 }
 if (shine){
   Vector p,q,r;
   QVector c;
   MatrixToVectors (cumulative_transformation_matrix,p,r,q,c);
   shine->SetOrientation (p,q,r);
   shine->SetPosition (c);
   shine->Draw ();
 }
}
void Planet::ProcessTerrains () {
  _Universe->AccessCamera()->SetPlanetaryTransform (NULL);
  while (!PlanetTerrainDrawQueue.empty()) {
    Planet * pl = (Planet *)PlanetTerrainDrawQueue.back()->GetUnit();
    pl->DrawTerrain();
    PlanetTerrainDrawQueue.back()->SetUnit(NULL);
    delete PlanetTerrainDrawQueue.back();
    PlanetTerrainDrawQueue.pop_back();
  }
}

void Planet::DrawTerrain() {


	  _Universe->AccessCamera()->SetPlanetaryTransform (terraintrans);
	  inside =true;
	  if (terrain)
	    terrain->EnableUpdate();
#ifdef PLANETARYTRANSFORM
	  TerrainUp = t;
	  Normalize(TerrainUp);
	  TerrainH = TerrainUp.Cross (Vector (-TerrainUp.i+.25, TerrainUp.j-.24,-TerrainUp.k+.24));
	  Normalize (TerrainH);
#endif
	
    //    shouldfog=true;




 GFXLoadIdentity (MODEL);
  if (inside&&terrain) {
    _Universe->AccessCamera()->UpdatePlanetGFX();
    //    Camera * cc = _Universe->AccessCamera();
    //    VectorAndPositionToMatrix (tmp,cc->P,cc->Q,cc->R,cc->GetPosition()+cc->R*100);
    terrain->SetTransformation (*_Universe->AccessCamera()->GetPlanetGFX());
    terrain->AdjustTerrain(_Universe->activeStarSystem());
    terrain->Draw();
#ifdef PLANETARYTRANSFORM

    terraintrans->GrabPerpendicularOrigin (_Universe->AccessCamera()->GetPosition(),tmp);
    terrain->SetTransformation (tmp);
    terrain->AdjustTerrain(_Universe->activeStarSystem());
    terrain->Draw();
    if (atmosphere) {
      Vector tup (tmp[4],tmp[5],tmp[6]);
      Vector p = (_Universe->AccessCamera()->GetPosition());
      Vector blah = p-Vector (tmp[12],tmp[13],tmp[14]);
      blah = p - (blah.Dot (tup))*tup;
      tmp[12]=blah.i;
      tmp[13]=blah.j;
      tmp[14]=blah.k;      
      atmosphere->SetMatricesAndDraw (_Universe->AccessCamera()->GetPosition(),tmp);
    }
#endif
  }
    
  
}





void Planet::reactToCollision(Unit * un, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist) {
#ifdef JUMP_DEBUG
  fprintf (stderr,"%s reacting to collision with %s drive %d", name.c_str(),un->name.c_str(), un->GetJumpStatus().drive);
#endif
  if (terrain&&un->isUnit()!=PLANETPTR) {
    un->SetPlanetOrbitData (terraintrans);
    Matrix top;
    Identity(top);
    /*
    Vector posRelToTerrain = terraintrans->InvTransform(un->LocalPosition());
    top[12]=un->Position().i- posRelToTerrain.i;
    top[13]=un->Position().j- posRelToTerrain.j;
    top[14]=un->Position().k- posRelToTerrain.k;
    */
    Vector P,Q,R;
    un->GetOrientation (P,Q,R);
    terraintrans->InvTransformBasis (top,P,Q,R,un->Position());
    Matrix inv,t;

    InvertMatrix (    inv,top);
    VectorAndPositionToMatrix (t,P,Q,R,un->Position());
    MultMatrix (top,t,inv);
#ifdef PLANETARYTRANSFORM

    terraintrans->GrabPerpendicularOrigin(un->Position(),top);
    static int tmp=0;
    /*    if (tmp) {
      terrain->SetTransformation (top);
      terrain->AdjustTerrain (_Universe->activeStarSystem());
      terrain->Collide (un);
      }else {*/

      //    }
#endif
    terrain->Collide (un,top);      
  }
  jumpReactToCollision(un);
  //screws with earth having an atmosphere... blahrgh
  if (!terrain&&GetDestinations().empty()&&!atmospheric) {//no place to go and acts like a ship
    Unit::reactToCollision (un,biglocation,bignormal,smalllocation,smallnormal,dist);
  }

  //nothing happens...you fail to do anythign :-)
  //maybe air reisstance here? or swithc dynamics to atmos mode
}
string Planet::getHumanReadablePlanetType () const{

  	  string temp =getCargoUnitName();
	  if (temp=="m_class") {
	    temp = "Agricultural";
	  }else if (temp=="Dirt"||temp=="newdetroit"||temp=="earth") {
	    temp = "Industrial";
	  }else if (temp=="university") {
	    temp = "University";
	  }else if (temp=="Snow") {
	    temp = "Ice Colony";
	  }else if (temp=="carribean") {
	    temp="Pleasure";
	  }else {
	    temp="";
	  }
	  return temp;
}
void Planet::EnableLights () {
  for (unsigned int i=0;i<lights.size();i++) {
    GFXEnableLight (lights[i]);
  }  
}
void Planet::DisableLights () {
  for (unsigned int i=0;i<lights.size();i++) {
    GFXDisableLight (lights[i]);
  }
}
Planet::~Planet() { 
  if (shine)
    delete shine;
  if (terrain) {
    delete terrain;
  }
  if (atmosphere){
     delete atmosphere;
  }
	if (terraintrans) {
	  Matrix *tmp = new Matrix ();
	  *tmp=cumulative_transformation_matrix;
	  terraintrans->SetTransformation (tmp);
	  //FIXME
	  //We're losing memory here...but alas alas... planets don't die that often
	}
}

PlanetaryTransform *Planet::setTerrain (ContinuousTerrain * t, float ratiox, int numwraps,float scaleatmos) {
  terrain = t;
  terrain->DisableDraw();
  float x,z;
  t->GetTotalSize (x,z);
  terraintrans = new PlanetaryTransform (.8*corner_max.i,x*ratiox,z,numwraps,scaleatmos);
  terraintrans->SetTransformation (&cumulative_transformation_matrix);
  return terraintrans;
}
void Planet::setAtmosphere (Atmosphere *t) {
  atmosphere = t;
}


void Planet::Kill(bool erasefromsave) {
	UnitCollection::UnitIterator iter;
	Unit *tmp;
	for (iter = satellites.createIterator();
	     (tmp = iter.current())!=NULL;
	     iter.advance()) {
	  tmp->SetAI (new Order);
	}
	/* probably not FIXME...right now doesn't work on paged out systems... not a big deal */
	for (unsigned int i=0;i<this->lights.size();i++) {
	  GFXDeleteLight (lights[i]);
	}
	/*	*/
	satellites.clear();
	insiders.clear();
	Unit::Kill(erasefromsave);
}

void Planet::gravitate(UnitCollection *uc) {
  /*
  float *t = cumulative_transformation_matrix;

  
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
