#include <math.h>
#include "vegastrike.h"
#include "unit_factory.h"
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
#include "cont_terrain.h"
#include "atmosphere.h"
#ifdef FIX_TERRAIN
#include "gfx/planetary_transform.h"
#endif
#include "collide/rapcol.h"
#include "images.h"
#include "gfx/halo.h"
#include "gfx/animation.h"
#include "cmd/script/flightgroup.h"
#include "gfx/ring.h"
#include "alphacurve.h"
#include "gfx/vsimage.h"
extern string getCargoUnitName (const char *name);

GamePlanet::GamePlanet()
    : GameUnit<Planet>( 0 )
{
  atmosphere = NULL;
  terrain = NULL;
  radius = 0.0;
  shine = NULL;
  inside=false;
  Init();
  terraintrans = NULL;
  SetAI(new Order()); // no behavior
}

static void SetFogMaterialColor (Mesh * thus, const GFXColor&color, const GFXColor & dcolor) {
		GFXMaterial m;m.ar=m.ag=m.ab=m.aa=m.sr=m.sg=m.sb=m.sa=0;m.power=0;
		static float emm=XMLSupport::parse_float (vs_config->getVariable("graphics","atmosphere_emmissive","1"));
		static float diff=XMLSupport::parse_float (vs_config->getVariable("graphics","atmosphere_diffuse","1"));
		m.er= emm*color.r;
		m.eg= emm*color.g;
		m.eb= emm*color.b;
		m.ea= emm*color.a;
		m.dr= diff*dcolor.r;
		m.dg= diff*dcolor.g;
		m.db= diff*dcolor.b;
		m.da= diff*dcolor.a;
		thus->SetMaterial(m);
}
Mesh * MakeFogMesh (const AtmosphericFogMesh & f, float radius) {
  static int count=0;
  count++;
  string nam = f.meshname+XMLSupport::tostring(count)+".png";
  if (f.min_alpha!=0||f.max_alpha!=255||f.concavity!=0 || f.focus!=.5 || f.tail_mode_start!=-1 || f.tail_mode_end!=-1) {
    static int rez=XMLSupport::parse_int (vs_config->getVariable("graphics","atmosphere_texture_resolution","512"));
    unsigned char * tex= (unsigned char *) malloc (sizeof(char) *rez*4);
    for (int i=0;i<rez;++i) {
      tex[i*4]=255;
      tex[i*4+1]=255;
      tex[i*4+2]=255;
      tex[i*4+3]=get_alpha(i,rez,f.min_alpha,f.max_alpha,f.focus,f.concavity,f.tail_mode_start,f.tail_mode_end);
    }
    // Writing in the homedir texture directory
    ::VSImage image;
    image.WriteImage( (char *)nam.c_str(), &tex[0], PngImage, rez, 1, true, 8, TextureFile);
    
  }  
  vector<string> override;
  override.push_back(nam);
  Mesh * ret=Mesh::LoadMesh(f.meshname.c_str(),Vector(f.scale*radius,f.scale*radius,f.scale*radius),0,NULL,override);
  ret->setConvex(true);
  SetFogMaterialColor(ret,GFXColor (f.er,f.eg,f.eb,f.ea),GFXColor(f.dr,f.dg,f.db,f.da));
  return ret;
}
/*
class FogMesh : public Mesh {
public:
	void SetMaterialColor (const GFXColor &color,const GFXColor & dcolor) {
		GFXMaterial m;m.ar=m.ag=m.ab=m.aa=m.sr=m.sg=m.sb=m.sa=0;m.power=0;
		static float emm=XMLSupport::parse_float (vs_config->getVariable("graphics","atmosphere_emmissive","1"));
		static float diff=XMLSupport::parse_float (vs_config->getVariable("graphics","atmosphere_diffuse","1"));
		m.er= emm*color.r;
		m.eg= emm*color.g;
		m.eb= emm*color.b;
		m.ea= emm*color.a;
		m.dr= diff*dcolor.r;
		m.dg= diff*dcolor.g;
		m.db= diff*dcolor.b;
		m.da= diff*dcolor.a;
		this->SetMaterial(m);
		
	}
	FogMesh (const AtmosphericFogMesh & f, float radius):Mesh(f.meshname.c_str(),Vector(f.scale*radius,f.scale*radius,f.scale*radius),0,NULL,true) {
		SetMaterialColor(GFXColor (f.er,f.eg,f.eb,f.ea),GFXColor(f.dr,f.dg,f.db,f.da));
		if (Decal.size()) {
			if (Decal[0]) {
				if (f.min_alpha!=0||f.max_alpha!=255||f.concavity!=0 || f.focus!=.5 || f.tail_mode_start!=-1 || f.tail_mode_end!=-1) {
					int rez=XMLSupport::parse_int (vs_config->getVariable("graphics","atmosphere_texture_resolution","512"));
					unsigned char * tex= (unsigned char *) malloc (sizeof(char) *rez*4);
					for (int i=0;i<rez;++i) {
						tex[i*4]=255;
						tex[i*4+1]=255;
						tex[i*4+2]=255;
						tex[i*4+3]=get_alpha(i,rez,f.min_alpha,f.max_alpha,f.focus,f.concavity,f.tail_mode_start,f.tail_mode_end);
					}
					static int count=0;
					count++;
					string nam = f.meshname+XMLSupport::tostring(count)+".png";
					// Writing in the homedir texture directory
					VSImage image;
					image.WriteImage( (char *)nam.c_str(), &tex[0], PngImage, rez, 1, true, 8, TextureFile);
                                        if (!orig) {
                                          delete Decal[0];
                                          Decal[0]= new Texture(nam.c_str(),nam.c_str());
                                        }
				}
			}
		}
		
	}
	virtual ~FogMesh (){}
};
*/
class AtmosphereHalo:public GameUnit<Unit> {
public:
	float planetRadius;
	AtmosphereHalo (float radiusOfPlanet,vector<Mesh*> & meshes, int faction):
			GameUnit<Unit>(meshes,true,faction){
		planetRadius= radiusOfPlanet;
	}
	virtual void Draw(const Transformation & quat=identity_transformation, const Matrix & m = identity_matrix) {
		QVector dirtocam = _Universe->AccessCamera()->GetPosition()-m.p;
		Transformation qua=quat;
		Matrix mat = m;
		float distance = dirtocam.Magnitude();
		
		float MyDistanceRadiusFactor=planetRadius/distance;
		//float AngleA = asin (MyDistanceRadiusFactor);
		//float AngleC = 3.1415926536/2-AngleA;
		//float HorizonHeight = sin(AngleC)*planetRadius; // these three lines are equiv to next 1 line (and much slower);
		float HorizonHeight = sqrt(1-MyDistanceRadiusFactor*MyDistanceRadiusFactor)*planetRadius;
		
		float zscale;
		float xyscale = zscale =HorizonHeight/planetRadius;
		zscale=0;
		dirtocam.Normalize();
		mat.p+=sqrt(planetRadius*planetRadius-HorizonHeight*HorizonHeight)*dirtocam;
		ScaleMatrix(mat,Vector(xyscale,xyscale,zscale));
		qua.position=mat.p;
		GameUnit<Unit>::Draw(qua,mat);
	}
};
void GamePlanet::AddFog (const std::vector <AtmosphericFogMesh> & v, bool opticalillusion) {
	if(meshdata.empty()) meshdata.push_back(NULL);
#ifdef MESHONLY
	Mesh *shield = meshdata.back(); meshdata.pop_back();
#endif
	std::vector <Mesh * > fogs;
	for (unsigned int i=0;i<v.size();++i) {
//		static float radiusmult = XMLSupport::parse_float (vs_config->getVariable("graphics","atmosphere_size","1.01"));

		Mesh *fog=MakeFogMesh (v[i],rSize());
		fogs.push_back(fog);
	}
	Unit* fawg;
	if (opticalillusion) {
		fawg=new AtmosphereHalo(this->rSize(),fogs,0);
	}else {
		fawg=UnitFactory::createUnit(fogs,true,0);	
	}
	fawg->setFaceCamera();
	getSubUnits().preinsert (fawg);
	fawg->hull/=fawg->GetHullPercent();
#ifdef MESHONLY	
	meshdata.push_back(shield);
#endif
	
}
void GamePlanet::AddCity (const std::string &texture,float radius,int numwrapx, int numwrapy, BLENDFUNC blendSrc, BLENDFUNC blendDst, bool inside_out, bool reverse_normals){
  if (meshdata.empty()) {
    meshdata.push_back(NULL);
  }
  Mesh * shield = meshdata.back();
  meshdata.pop_back();
  GFXMaterial m;
  m.ar=m.ag=m.ab=m.aa=0.0;
  static float materialweight=XMLSupport::parse_float (vs_config->getVariable("graphics","city_light_strength","10"));
  static float daymaterialweight=XMLSupport::parse_float (vs_config->getVariable("graphics","day_city_light_strength","0"));
  m.dr=m.dg=m.db=m.da=materialweight;
  m.sr=m.sg=m.sb=m.sa=0.0;
  m.er=m.eg=m.eb=m.ea=daymaterialweight;
  m.power=0.0;
  static int stacks=XMLSupport::parse_int(vs_config->getVariable ("graphics","planet_detail","24"));
  meshdata.push_back(new CityLights (radius,stacks,stacks, texture.c_str(), numwrapx, numwrapy, inside_out,ONE, ONE,false,0,M_PI,0.0,2*M_PI,reverse_normals));
  meshdata.back()->setEnvMap (GFXFALSE);
  meshdata.back()->SetMaterial (m);


  meshdata.push_back(shield);
}


Vector GamePlanet::AddSpaceElevator(const std::string &name, const std::string &faction,char direction) {//direction is udrlfb//up down right left front ack
	return Planet::AddSpaceElevator (name,faction,direction);
}
void GamePlanet::AddAtmosphere(const std::string & texture, float radius, BLENDFUNC blendSrc, BLENDFUNC blendDst) {
  if (meshdata.empty()) {
    meshdata.push_back(NULL);
  }
  Mesh * shield = meshdata.back();
  meshdata.pop_back();
  static int stacks=XMLSupport::parse_int(vs_config->getVariable ("graphics","planet_detail","24"));
  meshdata.push_back(new SphereMesh(radius, stacks, stacks, texture.c_str(), NULL,false,blendSrc,blendDst));  
  if (meshdata.back()) {
      //By klauss - this needs to be done for most atmospheres
      GFXMaterial a = {0,0,0,0,
                       1,1,1,1,
                       0,0,0,0,
                       0,0,0,0,
                       0};
      meshdata.back()->SetMaterial(a);
  };
  meshdata.push_back(shield);
}
void GamePlanet::AddRing(const std::string &texture,float iradius,float oradius, const QVector &R,const QVector &S,  int slices, int wrapx, int wrapy, BLENDFUNC blendSrc, BLENDFUNC blendDst) {
  if (meshdata.empty()) {
    meshdata.push_back(NULL);
  }
  Mesh * shield = meshdata.back();
  meshdata.pop_back();
  static int stacks=XMLSupport::parse_int(vs_config->getVariable ("graphics","planet_detail","24"));
  if (slices>0) {
    stacks = stacks;
    if (stacks<3)
      stacks=3;
    for (int i=0;i<slices;i++) {
      meshdata.push_back(new RingMesh(iradius,oradius ,stacks,texture.c_str(),R,S,wrapx, wrapy,  blendSrc,blendDst,false,i*(2*M_PI)/((float)slices),(i+1)*(2*M_PI)/((float)slices)));  
    }
  }
  meshdata.push_back(shield);  
}

extern const vector <string>& ParseDestinations (const string &value);
GamePlanet::GamePlanet(QVector x,QVector y,float vely, const Vector & rotvel, float pos,float gravity,float radius,const char * textname,BLENDFUNC blendSrc, BLENDFUNC blendDst, const vector<string> &dest, const QVector & orbitcent, Unit * parent, const GFXMaterial & ourmat, const std::vector <GFXLightLocal> &ligh, int faction,string fgid, bool inside_out)
    : GameUnit<Planet>( 0 )
{
	  atmosphere = NULL;
  terrain = NULL;

  shine = NULL;
  unsigned int nlights=0;
  if( !ligh.empty())
	  nlights=ligh.size();

  for (unsigned int i=0;i<nlights;i++) {
    int l;
    GFXCreateLight (l,ligh[i].ligh,!ligh[i].islocal);
    lights.push_back (l);
  }
  //  BLENDFUNC blendSrc=SRCALPHA;
  //  BLENDFUNC blendDst=INVSRCALPHA;
  bool wormhole = dest.size()!=0;
  if (wormhole ) {
	  static std::string wormhole_unit = vs_config->getVariable ("graphics","wormhole","wormhole");
	  string stab (".stable");
	  if (rand()>RAND_MAX*.99)
		  stab = ".unstable";
	  string wormholename = wormhole_unit+stab;
	  string wormholeneutralname = wormhole_unit+".neutral"+stab;
	  Unit * jum = UnitFactory::createUnit (wormholename.c_str(),true,faction);
	  int neutralfaction=FactionUtil::GetNeutralFaction();
	  faction = neutralfaction;
	  
	  Unit * neujum = UnitFactory::createUnit (wormholeneutralname.c_str(),true,neutralfaction);
	  Unit * jump=jum;
	  bool anytrue=false;
	  while (jump!=NULL) {
		  if (jump->name!="LOAD_FAILED") {
			  anytrue=true;
			  radius = jump->rSize();
			  while (jump->meshdata.size()) {
				  this->meshdata.push_back(jump->meshdata.back());
				  jump->meshdata.pop_back();
			  }
			  un_iter i;
			  Unit * su;
			  for (i=jump->getSubUnits();(su=*i)!=NULL;++i) {
			  SubUnits.prepend (su);
			  }
			  jump->SubUnits.clear();
/*			  for (i=jump->getSubUnits();(su=*i)!=NULL;) {
				  i.remove();
			  }
*/
		  }
		  jump->Kill();
		  if (jump!=neujum) {
			  jump=neujum;
		  }else{
			  jump=NULL;
		  }
	  }
	  wormhole = anytrue;
  }
  if (!wormhole) {
	  static int stacks=XMLSupport::parse_int(vs_config->getVariable ("graphics","planet_detail","24"));
	  atmospheric=!(blendSrc==ONE&&blendDst==ZERO);
	  meshdata.push_back(new SphereMesh(radius, stacks, stacks, textname, NULL,inside_out,blendSrc,blendDst));
          
	  meshdata.back()->setEnvMap(GFXFALSE);
          /*if (meshdata.back()->numTextures()>1) {
             if (meshdata.back()->texture(1)==0) {                
                meshdata.back()->SetMaterial (ourmat);
             }
          }else {
             meshdata.back()->SetMaterial (ourmat);
          }*/ //By Klauss (Why?)

	  meshdata.back()->SetMaterial(ourmat);
	  meshdata.push_back(NULL);
  }

  calculate_extent(false);
  if (wormhole) {
	  static float radscale=XMLSupport::parse_float(vs_config->getVariable("physics","jump_mesh_radius_scale",".5"));
	  radius*=radscale;
	  corner_min.i=corner_min.j=corner_min.k=-radius;
	  corner_max.i=corner_max.j=corner_max.k=radius;
	  radial_size=radius;
	  if (!meshdata.empty()) {
		  meshdata[0]->setVirtualBoundingBox(corner_min,corner_max,radius);
	  }
	  
  }  

  if (ligh.size()>0) {
	static float bodyradius = XMLSupport::parse_float(vs_config->getVariable ("graphics","star_body_radius",".33"));
    static bool drawglow = XMLSupport::parse_bool(vs_config->getVariable ("graphics","draw_star_glow","true"));

    static bool drawstar = XMLSupport::parse_bool(vs_config->getVariable ("graphics","draw_star_body","true"));
    static float glowradius = XMLSupport::parse_float(vs_config->getVariable ("graphics","star_glow_radius","1.33"))/bodyradius;
    if (drawglow) {
      GFXColor c(ourmat.er,ourmat.eg,ourmat.eb,ourmat.ea);
      static bool spec = XMLSupport::parse_bool(vs_config->getVariable ("graphics","glow_ambient_star_light","false"));
      static bool diff = XMLSupport::parse_bool(vs_config->getVariable ("graphics","glow_diffuse_star_light","false"));
      if (diff)
	c= ligh[0].ligh.GetProperties(DIFFUSE);
      if (spec)
	c= ligh[0].ligh.GetProperties(AMBIENT);

	
      static vector<string> shines = ParseDestinations (vs_config->getVariable("graphics","star_shine","shine.ani"));
	  if (shines.empty()) 
		  shines.push_back("shine.ani");
      shine = new Animation (shines[rand()%shines.size()].c_str(),true,.1,BILINEAR,false,true,c);//GFXColor(ourmat.er,ourmat.eg,ourmat.eb,ourmat.ea));
      shine->SetDimensions ( glowradius*radius,glowradius*radius);
    
      if (!drawstar) {
	delete meshdata[0];
	meshdata.clear();
	meshdata.push_back(NULL);
      }
    }
  }
	this->InitPlanet( x, y, vely, rotvel, pos, gravity, radius, textname, dest, orbitcent, parent, faction, fgid, inside_out, nlights);
}

extern bool shouldfog;

vector <UnitContainer *> PlanetTerrainDrawQueue;
void GamePlanet::Draw(const Transformation & quat, const Matrix &m) {
  //Do lighting fx
  // if cam inside don't draw?
  //  if(!inside) {
  GameUnit<Planet>::Draw(quat,m);
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
void GamePlanet::ProcessTerrains () {
  _Universe->AccessCamera()->SetPlanetaryTransform (NULL);
  while (!PlanetTerrainDrawQueue.empty()) {
    Planet * pl = (Planet *)PlanetTerrainDrawQueue.back()->GetUnit();
    pl->DrawTerrain();
    PlanetTerrainDrawQueue.back()->SetUnit(NULL);
    delete PlanetTerrainDrawQueue.back();
    PlanetTerrainDrawQueue.pop_back();
  }
}

void GamePlanet::DrawTerrain() {


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



extern bool CrashForceDock(Unit* thus, Unit* dockingUn, bool force);
extern void abletodock(int dock);
void GamePlanet::reactToCollision(Unit * un, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist) {
#ifdef JUMP_DEBUG
  VSFileSystem::vs_fprintf (stderr,"%s reacting to collision with %s drive %d", name.c_str(),un->name.c_str(), un->GetJumpStatus().drive);
#endif
#ifdef FIX_TERRAIN
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
#endif
  jumpReactToCollision(un);
  //screws with earth having an atmosphere... blahrgh
  if (!terrain&&GetDestinations().empty()&&!atmospheric) {//no place to go and acts like a ship
    GameUnit<Planet>::reactToCollision (un,biglocation,bignormal,smalllocation,smallnormal,dist);
	static bool planet_crash_docks = XMLSupport::parse_bool(vs_config->getVariable("physics","planet_collision_docks","true"));
	if (_Universe->isPlayerStarship(un)&&planet_crash_docks) {
          CrashForceDock(this,un,true);
	}
  }

  //nothing happens...you fail to do anythign :-)
  //maybe air reisstance here? or swithc dynamics to atmos mode
}
void GamePlanet::EnableLights () {
  for (unsigned int i=0;i<lights.size();i++) {
    GFXEnableLight (lights[i]);
  }  
}
void GamePlanet::DisableLights () {
  for (unsigned int i=0;i<lights.size();i++) {
    GFXDisableLight (lights[i]);
  }
}
GamePlanet::~GamePlanet() { 
  if (shine)
    delete shine;
  if (terrain) {
    delete terrain;
  }
  if (atmosphere){
     delete atmosphere;
  }
#ifdef FIX_TERRAIN
	if (terraintrans) {
	  Matrix *tmp = new Matrix ();
	  *tmp=cumulative_transformation_matrix;
	  terraintrans->SetTransformation (tmp);
	  //FIXME
	  //We're losing memory here...but alas alas... planets don't die that often
	}
#endif
}

PlanetaryTransform *GamePlanet::setTerrain (ContinuousTerrain * t, float ratiox, int numwraps,float scaleatmos) {
  terrain = t;
  terrain->DisableDraw();
  float x,z;
  t->GetTotalSize (x,z);
#ifdef FIX_TERRAIN
  terraintrans = new PlanetaryTransform (.8*corner_max.i,x*ratiox,z,numwraps,scaleatmos);
  terraintrans->SetTransformation (&cumulative_transformation_matrix);

  return terraintrans;
#endif
  return NULL;
}

void GamePlanet::setAtmosphere (Atmosphere *t) {
  atmosphere = t;
}


void GamePlanet::Kill(bool erasefromsave) {
	Unit *tmp;
	for (un_iter iter = satellites.createIterator();tmp = *iter;++iter) {
	  tmp->SetAI (new Order);
	}
	/* probably not FIXME...right now doesn't work on paged out systems... not a big deal */
	for (unsigned int i=0;i<this->lights.size();i++) {
	  GFXDeleteLight (lights[i]);
	}
	/*	*/
	satellites.clear();
	insiders.clear();
	GameUnit<Planet>::Kill(erasefromsave);
}

void GamePlanet::gravitate(UnitCollection *uc) {
  // Should put computation only in Planet and GFX/SFX only here if needed
  Planet::gravitate( uc);
  
}
