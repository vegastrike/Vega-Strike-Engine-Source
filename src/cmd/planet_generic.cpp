#include "planet_generic.h"
#include "unit_factory.h"
#include "gfx/mesh.h"
#include <sys/types.h>
#include <sys/stat.h>


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
  bool done =this->done;
  this->Order::Execute();
  this->done=done;//we ain't done till the cows come home
  if (done) 
    return;
  QVector x_offset = cos(theta) * x_size;
  QVector y_offset = sin(theta) * y_size;
  QVector origin (targetlocation);
  if (subtype&SSELF) {
      Unit * tmp = group.GetUnit();
      if (tmp) {
		  origin+= tmp->Position();
      }else {
		  done = true;
		  parent->SetResolveForces(true);
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
string GetElMeshName (string name, string faction,char direction)
{
	using namespace VSFileSystem;
		char strdir[2]={direction,0};
		string elxmesh=string(strdir)+"_elevator.xmesh";
		string elevator_mesh =name+"_"+faction+elxmesh;
		VSFile f;
		VSError err = f.OpenReadOnly( elevator_mesh, MeshFile);
		if( err>Ok)
			f.Close();
		else elevator_mesh=name+elxmesh;
		return elevator_mesh;
}

Vector Planet::AddSpaceElevator (const std::string &name, const std::string & faction, char direction) {
	Vector dir,scale;
	
	switch (direction){
	case 'u':
		dir.Set(0,1,0);
//		scale.Set(1,len,1);
		break;
	case 'd':
		dir.Set(0,-1,0);
//		scale.Set(1,len,1);		
		break;
	case 'l':
		dir.Set(-1,0,0);
//		scale.Set(len,1,1);		
		break;
	case 'r':
		dir.Set(1,0,0);
//		scale.Set(len,1,1);		
		break;
	case 'b':
		dir.Set(0,0,-1);
//		scale.Set(1,1,len);				
		break;
	default:
		dir.Set(0,0,1);
//		scale.Set(1,1,len);						
		break;
	}
	Matrix ElevatorLoc(Vector(dir.j,dir.k,dir.i),dir,Vector(dir.k,dir.i,dir.j));
	scale = dir * radius +Vector(1,1,1)-dir;
	if (meshdata.empty()) meshdata.push_back(NULL);
	Mesh * shield = meshdata.back();
	string elevator_mesh=GetElMeshName(name,faction,direction);//filename	
	Mesh * tmp = meshdata.back()=new Mesh (elevator_mesh.c_str(),
										   scale,
										   FactionUtil::
										   GetFaction(faction.c_str()),
										   NULL);
	
	meshdata.push_back(shield);
	{//subunit computations
		Vector mn(tmp->corner_min());
		Vector mx(tmp->corner_max());
		if (dir.Dot (Vector(1,1,1))>0) {
			ElevatorLoc.p.Set (dir.i*mx.i,dir.j*mx.j,dir.k*mx.k);
		}else {
			ElevatorLoc.p.Set (-dir.i*mn.i,-dir.j*mn.j,-dir.k*mn.k);
		}
		
		Unit * un=UnitFactory::createUnit (name.c_str(),true,FactionUtil::GetFactionIndex(faction),"",NULL);
		if (image->dockingports.back().pos.MagnitudeSquared()<10)
			image->dockingports.clear();
		image->dockingports.push_back (DockingPorts(ElevatorLoc.p,un->rSize()*1.5,true));
		un->SetRecursiveOwner(this);
		un->SetOrientation(ElevatorLoc.getQ(),ElevatorLoc.getR());
		un->SetPosition(ElevatorLoc.p);
		SubUnits.prepend(un);
		
	}
	return dir;
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
	    VSFileSystem::vs_fprintf (stderr,"Planets are unable to orbit around units");
	    return NULL;
	  }

  } else {
    return this;
  }
  
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
}
void Planet::AddSatellite (Unit * orbiter) {
	satellites.prepend (orbiter);
	orbiter->SetOwner (this);
}
extern float ScaleJumpRadius(float);
extern Flightgroup * getStaticBaseFlightgroup(int faction);
Unit * Planet::beginElement(QVector x,QVector y,float vely, const Vector & rotvel, float pos,float gravity,float radius,const char * filename,BLENDFUNC blendSrc, BLENDFUNC blendDst, vector<char *> dest,int level,  const GFXMaterial & ourmat, const vector <GFXLightLocal>& ligh, bool isunit, int faction,string fullname, bool inside_out){
  //this function is OBSOLETE
  Unit * un=NULL;
  if (level>2) {
    UnitCollection::UnitIterator satiterator = satellites.createIterator();
	  assert(satiterator.current()!=NULL);
	  if (satiterator.current()->isUnit()==PLANETPTR) {
		un =((Planet *)satiterator.current())->beginElement(x,y,vely,rotvel, pos,gravity,radius,filename,blendSrc,blendDst,dest,level-1,ourmat,ligh, isunit, faction,fullname,inside_out);
	  } else {
	    VSFileSystem::vs_fprintf (stderr,"Planets are unable to orbit around units");
	  }
  } else {
    if (isunit==true) {
      Unit *sat_unit=NULL;
      Flightgroup *fg = getStaticBaseFlightgroup(faction);
      satellites.prepend(sat_unit=UnitFactory::createUnit (filename, false, faction,"",fg,fg->nr_ships-1));
      sat_unit->setFullname(fullname);
      un = sat_unit;
      un_iter satiterator (satellites.createIterator());
      satiterator.current()->SetAI (new PlanetaryOrbit (satiterator.current(),vely,pos,x,y, QVector (0,0,0), this)) ;
      satiterator.current()->SetOwner (this);
    }else {
      Planet * p;
      if (dest.size()!=0) {
	radius = ScaleJumpRadius(radius);
      }
      satellites.prepend(p=UnitFactory::createPlanet(x,y,vely,rotvel,pos,gravity,radius,filename,blendSrc,blendDst,dest, QVector (0,0,0), this, ourmat, ligh, faction,fullname,inside_out));
      un = p;
      p->SetOwner (this);
    }
  }
  return un;
}

Planet::Planet()
    : Unit( 0 )
    , radius(0.0f), satellites()
{
  inside=false;
  // Not needed as Unit default constructor is called and already does Init
  //Init();
  terraintrans = NULL;
  atmospheric = false;
}

void Planet::InitPlanet(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename, vector<char *> dest, const QVector &orbitcent, Unit * parent, int faction,string fullname, bool inside_out, unsigned int lights_num)
{
  atmosphere = NULL;
  terrain = NULL;
  static float bodyradius = XMLSupport::parse_float(vs_config->getVariable ("graphics","star_body_radius",".33"));
  if (lights_num){
    radius*=bodyradius;
    VSFileSystem::vs_fprintf (stderr,"scaling %s",filename);
  }
  inside =false;
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
  name=fullname;
  this->fullname=name;
  this->radius=radius;
  this->gravity=gravity;
  static  float densityOfRock = XMLSupport::parse_float(vs_config->getVariable("physics","density_of_jump_point",".01"));
  static  float densityOfJumpPoint = XMLSupport::parse_float(vs_config->getVariable("physics","density_of_jump_point","100000"));
  static  float massofplanet = XMLSupport::parse_float(vs_config->getVariable("physics","mass_of_planet","10000000"));
  hull = (4./3)*M_PI*radius*radius*radius*(dest.empty()?densityOfRock:densityOfJumpPoint);
  mass = massofplanet;
  SetAI(new PlanetaryOrbit(this, vely, pos, x, y, orbitcent, parent)); // behavior
  terraintrans=NULL;

  colTrees= NULL;
  SetAngularVelocity (rotvel);
  static int numdock = XMLSupport::parse_int(vs_config->getVariable ("physics","num_planet_docking_port","4"));
  static float planetdockportsize= XMLSupport::parse_float(vs_config->getVariable ("physics","planet_port_size","1.2"));
  static float planetdockportminsize= XMLSupport::parse_float(vs_config->getVariable ("physics","planet_port_min_size","300"));
  if ((!atmospheric) && dest.size()==0) {
    for (int pdp=0;pdp<numdock;pdp++) {
      float dock = radius*planetdockportsize;
      if (dock-radius<planetdockportminsize) {
	dock = radius+planetdockportminsize;
      }
      image->dockingports.push_back (DockingPorts (Vector(0,0,0),dock,true));
    }
  }
  string tempname = (::getCargoUnitName (filename));
  setFullname(tempname);
 
  Unit * un = UnitFactory::createUnit (tempname.c_str(),true,FactionUtil::GetFaction("planets"));
  if (un->name!=string("LOAD_FAILED")) {
    image->cargo=un->GetImageInformation().cargo;
    image->cargo_volume=un->GetImageInformation().cargo_volume;
  }
  un->Kill();
}

Planet::Planet(QVector x,QVector y,float vely,const Vector & rotvel, float pos,float gravity,float radius,const char * filename, vector<char *> dest, const QVector &orbitcent, Unit * parent, int faction,string fullname, bool inside_out, unsigned int lights_num)
{
	this->InitPlanet( x, y, vely, rotvel, pos, gravity, radius, filename, dest, orbitcent, parent, faction, fullname, inside_out, lights_num);
  for (unsigned int i=0;i<lights_num;i++) {
    int l=-1;
    lights.push_back (l);
  }
}

static void beginPlanetElement (void *userDataVoid, const XML_Char *name, const XML_Char **attr) {
	std::map<std::string, std::string> *userData = (std::map<std::string, std::string> *)userDataVoid;
	std::string namestr((const char*)name);
	for (int i=0;i<namestr.size();i++) {
		namestr[i]=tolower(namestr[i]);
	}
	if (namestr.size()>=6&&memcmp(namestr.c_str(),"planet",6)==0) {
		if (namestr.size()==6) {
			if ((*userData)["\n"]!="\n") {
				fprintf(stderr, "Warning: No surrounding planet tag specified.  This will probably result in missing data.\n");
			}
			std::string planetname;
			std::string planettype;
			// Name is "Planet"
			for (int i=0; attr[i]!=NULL; i+=2) {
				if (strcmp(attr[i], "type")==0) {
					planettype=attr[i+1];
				}
				if (strcmp(attr[i], "name")==0) {
					planetname=attr[i+1];
				}
			}
			if (planettype.size()&&planetname.size()) {
				(*userData)[planettype]=planetname;
			} else {
				fprintf(stderr, "Warning: \"type\" or \"name\" atributes missing... Ignoring planet type...\n");
			}
		} else {
			(*userData)["\n"]="\n";
		}
	} else {
		fprintf(stderr, "Warning: XML tag in planet types not valid: %s\n", name);
	}
}
static void endPlanetElement (void *userData, const XML_Char *name) {
}

static std::map<std::string, std::string> readPlanetTypes(std::string filename) {
	std::map<std::string, std::string> planetTypes;
	VSFile f;
	VSError err = f.OpenReadOnly( filename, UniverseFile);
	if (err>Ok) {
		return planetTypes;
	}
	int len = f.Size();
	XML_Parser parser=XML_ParserCreate(NULL);
	void *buff = XML_GetBuffer(parser, len);
	if (buff == NULL) {
		fprintf(stderr, "Fatal error: out of memory for planet name file\n");
		return planetTypes;
	}
	len=f.Read(buff, len);
	XML_SetElementHandler(parser, &beginPlanetElement, &endPlanetElement);
	XML_SetUserData(parser, &planetTypes);
	XML_ParseBuffer(parser, len, 1);
	XML_ParserFree(parser);
	return planetTypes;
}

string Planet::getHumanReadablePlanetType () const{
	  static std::map<std::string, std::string> planetTypes (readPlanetTypes("planet_types.xml"));
  	  string temp =getCargoUnitName();
	  return planetTypes[temp];
}
Planet::~Planet() { 
	if (terraintrans) {
	  Matrix *tmp = new Matrix ();
	  *tmp=cumulative_transformation_matrix;
	  //terraintrans->SetTransformation (tmp);
	}
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
	satellites.clear();
	insiders.clear();
	Unit::Kill(erasefromsave);
}
