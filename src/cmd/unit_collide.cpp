#include "vegastrike.h"
//#include "unit.h"
#include "beam.h"

#include "bolt.h"
#include "gfx/mesh.h"
#include "unit_collide.h"
#include "physics.h"
#include "gfx/bsp.h"

#include "collide2/CSopcodecollider.h"
#include "collide2/csgeom2/optransfrm.h"
#include "collide2/basecollider.h"

#include "hashtable.h"

#include <string>
#include "vs_globals.h"
#include "configxml.h"
#include "collide.h"
static bool operator == (const Collidable &a,const Collidable &b)
{
	return memcmp(&a,&b,sizeof(Collidable))==0;
}


void Unit::RemoveFromSystem()
{
	for (unsigned int locind=0;locind<NUM_COLLIDE_MAPS;++locind) {
		if (!is_null(this->location[locind])) {
			if (activeStarSystem==NULL) {
				printf ("NONFATAL NULL activeStarSystem detected...please fix\n");
				activeStarSystem=_Universe->activeStarSystem();
			}
			static bool collidemap_sanity_check = XMLSupport::parse_bool(vs_config->getVariable("physics","collidemap_sanity_check","false"));
			if (collidemap_sanity_check) {
				if (0) {		 //activeStarSystem->collidemap->find(*this->location)==activeStarSystem->collidemap->end()){
					CollideMap::iterator i;
					CollideMap::iterator j=activeStarSystem->collidemap[locind]->begin();

					bool found=false;
					for (i=activeStarSystem->collidemap[locind]->begin();
					i!=activeStarSystem->collidemap[locind]->end();++i) {
						if (i==this->location[locind]) {
							printf ("hussah %d\n",*i==*this->location[locind]);
							found=true;
						}
						if(**i<**j) {
							printf ("(%f %f %f) and (%f %f %f) %f < %f %d!!!",
								(**i).GetPosition().i,
								(**i).GetPosition().j,
								(**i).GetPosition().k,
								(**j).GetPosition().i,
								(**j).GetPosition().j,
								(**j).GetPosition().k,
								(**i).GetPosition().MagnitudeSquared(),
								(**j).GetPosition().MagnitudeSquared(),
								(**i).GetPosition().MagnitudeSquared()<
								(**j).GetPosition().MagnitudeSquared());

						}
						j=i;
					}
					printf ("fin %d %d ",*(int*)&i,found);
					activeStarSystem->collidemap[locind]->checkSet();
					assert(0);
				}
			}
			activeStarSystem->collidemap[locind]->erase(this->location[locind]);
			set_null(this->location[locind]);
		}
	}
	int j;
	for (j=0;j<GetNumMounts();j++) {
		if (mounts[j].type->type==weapon_info::BEAM) {
			if (mounts[j].ref.gun) {
				mounts[j].ref.gun->RemoveFromSystem(true);
			}
		}
	}
	activeStarSystem=NULL;
}


void Unit::UpdateCollideQueue (StarSystem * ss, CollideMap::iterator hint[NUM_COLLIDE_MAPS])
{
	if (activeStarSystem==NULL) {
		activeStarSystem = ss;

	}
	else {
		assert (activeStarSystem==ss);
	}
	for (unsigned int locind=0;locind<NUM_COLLIDE_MAPS;++locind) {
		if (is_null(location[locind])) {
			assert (!isSubUnit());
			if (!isSubUnit()) {
				location[locind]=ss->collidemap[locind]->insert(Collidable(this),hint[locind]);
			}
		}
	}

}


extern bool usehuge_table();
void Unit::CollideAll()
{
	static bool noUnitCollisions=XMLSupport::parse_bool(vs_config->getVariable("physics","no_unit_collisions","false"));

	if (isSubUnit()||killed||noUnitCollisions)
		return;
	static bool newUnitCollisions=XMLSupport::parse_bool(vs_config->getVariable("physics","new_collisions","true"));
	if (newUnitCollisions) {
		for (unsigned int locind=0;locind<NUM_COLLIDE_MAPS;++locind) {
			if (is_null(this->location[locind])) {
				this->location[locind]=this->getStarSystem()->collidemap[locind]->insert(Collidable(this));
			}
		}
		CollideMap *cm=this->getStarSystem()->collidemap[Unit::UNIT_BOLT];
		cm->CheckCollisions(this,*this->location[Unit::UNIT_BOLT]);
	}
}


Vector Vabs (const Vector &in)
{
	return Vector (in.i>=0?in.i:-in.i,
		in.j>=0?in.j:-in.j,
		in.k>=0?in.k:-in.k);
}


Matrix WarpMatrixForCollisions (Unit * un, const Matrix& ctm)
{
	if (un->GetWarpVelocity().MagnitudeSquared()*SIMULATION_ATOM*SIMULATION_ATOM<un->rSize()*un->rSize()) {
		return ctm;
	}
	else {
		Matrix k(ctm);
		const Vector v(Vector(1,1,1)+Vabs(ctm.getR()*ctm.getR().Dot(un->GetWarpVelocity().Scale(100*SIMULATION_ATOM/un->rSize()))));
		k.r[0]*=v.i;
		k.r[1]*=v.j;
		k.r[2]*=v.k;

		k.r[3]*=v.i;
		k.r[4]*=v.j;
		k.r[5]*=v.k;

		k.r[6]*=v.i;
		k.r[7]*=v.j;
		k.r[8]*=v.k;
		return k;
	}
}


		 //do each of these bubbled subunits collide with the other unit?
bool Unit::Inside (const QVector &target, const float radius, Vector & normal, float &dist)
{
	if (!querySphere(target,radius)) {
		return false;;
	}
	normal=(target-Position()).Cast();
	::Normalize(normal);

							 // if its' in the sphre, that's enough
	if (isPlanet()==true||queryBSP(target, radius, normal,dist,false)) {
		return true;
	}

	return false;
}


static float tmpmax (float a, float b)
{
	return a>b?a:b;
}


bool Unit::InsideCollideTree (Unit * smaller, QVector & bigpos, Vector &bigNormal, QVector & smallpos, Vector & smallNormal, bool bigasteroid, bool smallasteroid)
{
	if (smaller->colTrees==NULL||this->colTrees==NULL)
		return false;
	if (hull<0) return false;
	if (smaller->colTrees->usingColTree()==false||this->colTrees->usingColTree()==false)
		return false;
	csOPCODECollider::ResetCollisionPairs();
	//    printf ("Col %s %s\n",name.c_str(),smaller->name.c_str());
	Unit * bigger =this;

	csReversibleTransform bigtransform (/*WarpMatrixForCollisions(bigger)*/bigger->cumulative_transformation_matrix);
	csReversibleTransform smalltransform (/*WarpMatrixForCollisions(smaller)*/smaller->cumulative_transformation_matrix);
	smalltransform.SetO2TTranslation(csVector3(smaller->cumulative_transformation_matrix.p-bigger->cumulative_transformation_matrix.p));
	bigtransform.SetO2TTranslation(csVector3(0,0,0));
	//we're only gonna lerp the positions for speed here... gahh!
	if (smaller->colTrees->colTree(smaller,bigger->GetWarpVelocity())->Collide (*bigger->colTrees->colTree(bigger,smaller->GetWarpVelocity()),
		&smalltransform,
	&bigtransform)) {
		//static int crashcount=0;
		//      VSFileSystem::vs_fprintf (stderr,"%s Crashez to %s %d\n", bigger->name.c_str(), smaller->name.c_str(),crashcount++);
		csCollisionPair *mycollide = csOPCODECollider::GetCollisions();
		unsigned int numHits = csOPCODECollider::GetCollisionPairCount();
		// maybe we should access the collider directly.
		if (numHits) {
//			printf ("%s hit %s\n",smaller->name.c_str(),bigger->name.c_str());
			smallpos.Set((mycollide[0].a1.x+mycollide[0].b1.x+mycollide[0].c1.x)/3,
				(mycollide[0].a1.y+mycollide[0].b1.y+mycollide[0].c1.y)/3,
				(mycollide[0].a1.z+mycollide[0].b1.z+mycollide[0].c1.z)/3);
			smallpos = Transform (smaller->cumulative_transformation_matrix,smallpos);
			bigpos.Set((mycollide[0].a2.x+mycollide[0].b2.x+mycollide[0].c2.x)/3,
				(mycollide[0].a2.y+mycollide[0].b2.y+mycollide[0].c2.y)/3,
				(mycollide[0].a2.z+mycollide[0].b2.z+mycollide[0].c2.z)/3);
			bigpos = Transform (bigger->cumulative_transformation_matrix,bigpos);
			csVector3 sn, bn;
			sn.Cross (mycollide[0].b1-mycollide[0].a1,mycollide[0].c1-mycollide[0].a1);
			bn.Cross (mycollide[0].b2-mycollide[0].a2,mycollide[0].c2-mycollide[0].a2);
			sn.Normalize();
			bn.Normalize();
			smallNormal.Set (sn.x,sn.y,sn.z);
			bigNormal.Set (bn.x,bn.y,bn.z);
			smallNormal = TransformNormal (smaller->cumulative_transformation_matrix,smallNormal);
			bigNormal = TransformNormal (bigger->cumulative_transformation_matrix,bigNormal);
			return true;
		}
	}
	un_iter i;
	static float rsizelim = XMLSupport::parse_float (vs_config->getVariable ("physics","smallest_subunit_to_collide",".2"));
	clsptr bigtype=bigasteroid?ASTEROIDPTR:bigger->isUnit();
	clsptr smalltype=smallasteroid?ASTEROIDPTR:smaller->isUnit();
	if (bigger->SubUnits.empty()==false&&(bigger->graphicOptions.RecurseIntoSubUnitsOnCollision==true||bigtype==ASTEROIDPTR)) {
		i=bigger->getSubUnits();
		float rad=smaller->rSize();
		for (Unit * un;un=*i;++i) {
			float subrad=un->rSize();
			if ((bigtype!=ASTEROIDPTR)&&(subrad/bigger->rSize()<rsizelim)) {
				break;
			}
			else {
				//	  printf ("s:%f",un->rSize()/bigger->rSize());
			}
			if ((un->Position()-smaller->Position()).Magnitude()<=subrad+rad) {
				if ((un->InsideCollideTree(smaller,bigpos, bigNormal,smallpos,smallNormal,bigtype==ASTEROIDPTR,smalltype==ASTEROIDPTR))) {
					return true;
				}
			}
		}
	}
	if (smaller->SubUnits.empty()==false&&(smaller->graphicOptions.RecurseIntoSubUnitsOnCollision==true||smalltype==ASTEROIDPTR)) {
		i=smaller->getSubUnits();
		float rad=bigger->rSize();
		for (Unit * un;un=*i;++i) {
			float subrad=un->rSize();
			if ((smalltype!=ASTEROIDPTR)&&(subrad/smaller->rSize()<rsizelim)) {
				//	  printf ("s:%f",un->rSize()/smaller->rSize());
				break;

			}
			if ((un->Position()-bigger->Position()).Magnitude()<=subrad+rad) {
				if ((bigger->InsideCollideTree(un,bigpos, bigNormal,smallpos,smallNormal,bigtype==ASTEROIDPTR,smalltype==ASTEROIDPTR))) {
					return true;
				}
			}
		}
	}
	//FIXME
	//doesn't check all i*j options of subunits vs subunits
	return false;
}


Unit * Unit::BeamInsideCollideTree (const QVector & start,const QVector & end, QVector & pos, Vector &norm, double &distance)
{
	QVector r (end-start);
	double mag = r.Magnitude();
	if (mag>0) {
		r = r*(1./mag);
	}
	{
		bool temp=true;
		if (this->colTrees==NULL) {
			temp=true;
		}
		else if (this->colTrees->colTree(this,Vector(0,0,0))==NULL) {
			temp=true;
		}
		if (temp) {
			float dis=distance;
			Unit * ret= queryBSP(start,end,norm,dis);
			distance=dis;
			pos=start+r*distance;
			return ret;
		}
	}
	QVector p(-r.k,r.i,-r.j);
	QVector q;
	ScaledCrossProduct(r,p,q);
	ScaledCrossProduct(q,r,p);
	csOPCODECollider::ResetCollisionPairs();
	//    printf ("Col %s %s\n",name.c_str(),smaller->name.c_str());
	const csReversibleTransform bigtransform (cumulative_transformation_matrix);
	Matrix smallerMat(p.Cast(),q.Cast(),r.Cast());
	smallerMat.p = start;
	const csReversibleTransform smalltransform (smallerMat);
	bsp_polygon tri;
	tri.v.push_back(Vector(-mag/1024,0,0));
	tri.v.push_back(Vector(-mag/1024,0,mag));
	tri.v.push_back(Vector(mag/1024,0,mag));
	tri.v.push_back(Vector(mag/1024,0,0));
	vector <bsp_polygon> mesh;
	mesh.push_back(tri);
	csOPCODECollider smallColTree(mesh);
	if (smallColTree.Collide (*(this->colTrees)->colTree(this,Vector(0,0,0)),
		&smalltransform,
	&bigtransform)) {
		static int crashcount=0;

		//            VSFileSystem::vs_fprintf (stderr,"%s Beam Crashez %d\n", name.get().c_str(),crashcount++);
		csCollisionPair  *mycollide = csOPCODECollider::GetCollisions();
		int numHits = csOPCODECollider::GetCollisionPairCount();
		if (numHits) {
			//	printf ("num hits %d",numHits);
			/*
			pos.Set((mycollide[0].a1.x+mycollide[0].b1.x+mycollide[0].c1.x)/3,
					 (mycollide[0].a1.y+mycollide[0].b1.y+mycollide[0].c1.y)/3,
					 (mycollide[0].a1.z+mycollide[0].b1.z+mycollide[0].c1.z)/3);
			pos = Transform (smaller->cumulative_transformation_matrix,smallpos);
			*/
			pos.Set((mycollide[0].a2.x+mycollide[0].b2.x+mycollide[0].c2.x)/3,
				(mycollide[0].a2.y+mycollide[0].b2.y+mycollide[0].c2.y)/3,
				(mycollide[0].a2.z+mycollide[0].b2.z+mycollide[0].c2.z)/3);
			pos = Transform (cumulative_transformation_matrix,pos);
			csVector3 sn, bn;
			sn.Cross (mycollide[0].b1-mycollide[0].a1,mycollide[0].c1-mycollide[0].a1);
			bn.Cross (mycollide[0].b2-mycollide[0].a2,mycollide[0].c2-mycollide[0].a2);
			sn.Normalize();
			bn.Normalize();
			//	smallNormal.Set (sn.x,sn.y,sn.z);
			norm.Set (bn.x,bn.y,bn.z);
			//smallNormal = TransformNormal (smaller->cumulative_transformation_matrix,smallNormal);
			norm = TransformNormal (cumulative_transformation_matrix,norm);
			distance = (pos-start).Magnitude();
			return this;
		}
	}
	//FIXME
	//doesn't check all i*j options of subunits vs subunits
	return false;
}


inline float mysqr(float a) { return a*a; }

bool Unit::Collide (Unit * target)
{
	//now first OF ALL make sure they're within bubbles of each other...
	if ((Position()-target->Position()).MagnitudeSquared()>mysqr(radial_size+target->radial_size))
		return false;

	clsptr targetisUnit=target->isUnit();
	clsptr thisisUnit=this->isUnit();
	static float NEBULA_SPACE_DRAG=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_space_drag","0.01"));
	if (targetisUnit==NEBULAPTR)
								 // why? why not?
		this->Velocity *= (1 - NEBULA_SPACE_DRAG);
	if (target==this||((targetisUnit!=NEBULAPTR&&thisisUnit!=NEBULAPTR)&&(owner==target||target->owner==this||(owner!=NULL&&target->owner==owner)))||(Network!=NULL&&_Universe->isPlayerStarship(target)==NULL&&_Universe->isPlayerStarship(this)==NULL))
		return false;
	if (targetisUnit==ASTEROIDPTR&&thisisUnit==ASTEROIDPTR)
		return false;
	std::multimap<Unit*,Unit*>* last_collisions=&_Universe->activeStarSystem()->last_collisions;
	std::multimap<Unit*,Unit*>::iterator iter;
	iter=last_collisions->find(target);
	for (;iter!=last_collisions->end()&&iter->first==target;++iter) {
		if (iter->second==this) {
			//printf ("No double collision\n"); (litters debug output, and I bet it's not needed now)
			return false;
		}
	}
	last_collisions->insert(std::pair<Unit*,Unit*>(this,target));
	//unit v unit? use point sampling?
	if ((this->DockedOrDocking()&(DOCKED_INSIDE|DOCKED))||(target->DockedOrDocking()&(DOCKED_INSIDE|DOCKED))) {
		return false;
	}
	//now do some serious checks
	Vector normal(-1,-1,-1);
	float dist;
	Unit * bigger;
	Unit * smaller;
	if (radial_size<target->radial_size) {
		bigger = target;
		smaller = this;
	}
	else {
		bigger = this;
		smaller = target;
	}
	bool usecoltree =(this->colTrees&&target->colTrees)
		?this->colTrees->colTree(this,Vector(0,0,0))&&target->colTrees->colTree(this,Vector(0,0,0))
		: false;
	if (usecoltree) {
		QVector bigpos,smallpos;
		Vector bigNormal,smallNormal;
		if (bigger->InsideCollideTree (smaller,bigpos,bigNormal,smallpos,smallNormal)) {
			if (!bigger->isDocked(smaller)&&!smaller->isDocked(bigger)) {
				bigger->reactToCollision (smaller,bigpos, bigNormal,smallpos,smallNormal, 10   );
			} else return false;
		} else return false;
	}
	else {
		if (bigger->Inside(smaller->Position(),smaller->rSize(),normal,dist)) {
			if (!bigger->isDocked(smaller)&&!smaller->isDocked(bigger)) {
				bigger->reactToCollision (smaller,bigger->Position(), normal,smaller->Position(), -normal, dist);
			}else return false;
		}
		else {
			return false;
		}
	}
	//UNUSED BUT GOOD  float elast = .5*(smallcsReversibleTransform (cumulative_transformation_matrix),er->GetElasticity()+bigger->GetElasticity());
	//BAD  float speedagainst = (normal.Dot (smaller->GetVelocity()-bigger->GetVelocity()));
	//BADF  smaller->ApplyForce (normal * fabs(elast*speedagainst)/SIMULATION_ATOM);
	//BAD  bigger->ApplyForce (normal * -fabs((elast+1)*speedagainst*smaller->GetMass()/bigger->GetMass())/SIMULATION_ATOM);
	//deal damage similarly to beam damage!!  Apply some sort of repel force

	//NOT USED BUT GOOD  Vector farce = normal*smaller->GetMass()*fabs(normal.Dot ((smaller->GetVelocity()-bigger->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM));
	return true;
}


float globQueryShell (QVector st, QVector dir, float radius)
{
	double temp1 = radius;
	double a,b,c;
	c = st.Dot (st);
	c = c - temp1*temp1;
	b = 2 * (dir.Dot (st));
	a = dir.Dot(dir);
	//b^2-4ac
	c = b*b - 4*a*c;
	if (c<0||a==0)
		return 0;
	a *=2;

	float tmp = (-b + sqrt (c))/a;
	c = (-b - sqrt (c))/a;
	if (tmp>0&&tmp<=1) {
		return (c>0&&c<tmp) ? c : tmp;
	}
	else if (c>0&&c<=1) {
		return c;
	}
	return 0;
}


float globQuerySphere (QVector start, QVector end, QVector pos, float radius)
{

	QVector st = start-pos;
	if (st.MagnitudeSquared()<radius*radius)
		return 1.0e-6;
	return globQueryShell(st,end-start,radius);
}


Unit * Unit::queryBSP (const QVector &pt, float err, Vector & norm, float &dist, bool ShieldBSP)
{
	int i;
	if ((!SubUnits.empty())&&graphicOptions.RecurseIntoSubUnitsOnCollision) {
		un_fiter i = SubUnits.fastIterator();
		for (Unit * un;un=*i;++i) {
			Unit * retval;
			if ((retval=un->queryBSP(pt,err, norm,dist,ShieldBSP))) {
				return retval;
			}
		}
	}
	QVector st (InvTransform (cumulative_transformation_matrix,pt));
	bool temp=false;
	for (i=0;i<nummesh()&&!temp;i++) {
		temp|=meshdata[i]->queryBoundingBox (st,err);
	}
	if (!temp)
		return NULL;
	BSPTree *const* tmpBsp;
	BSPTree *myNull=NULL;
	if (this->colTrees) {
		tmpBsp = ShieldUp(st.Cast())?&this->colTrees->bspShield:&this->colTrees->bspTree;
		if (this->colTrees->bspTree&&!ShieldBSP) {
			tmpBsp= &this->colTrees->bspTree;
		}
	}
	else {
		tmpBsp=&myNull;
	}
	if (!(*tmpBsp)) {
		dist = (st - meshdata[i-1]->Position().Cast()).Magnitude()-err-meshdata[i-1]->rSize();
		return this;
	}
	if ((*tmpBsp)->intersects (st.Cast(),err,norm,dist)) {

		norm = ToWorldCoordinates (norm);
		norm.Normalize();
		return this;
	}
	return NULL;
}


bool testRayVersusBB(Vector Min, Vector Max, const QVector& start, const Vector& end, Vector& Coord)
{
	const float eps =0.00001f;
	unsigned int i;
	float bbmin[3];
	float bbmax[3];
	float dir[3];
	float finalCoord[3];
	bool Inside = true;
	float origin[3];
	float tmax[3];
	tmax[0]=tmax[1]=tmax[2]=-1.0f;
	origin[0]=start.i;
	origin[1]=start.j;
	origin[2]=start.k;
	bbmin[0]=Min.i;
	bbmin[1]=Min.j;
	bbmin[2]=Min.k;
	bbmax[0]=Max.i;
	bbmax[1]=Max.j;
	bbmax[2]=Max.k;
	dir[0]=end.i-start.i;
	dir[1]=end.j-start.j;
	dir[2]=end.k-start.k;
	for(i=0;i<3;++i) {
		if(origin[i] > bbmax[i]) {
			finalCoord[i] = bbmax[i];
			if(dir[i]!=0.0f)
				tmax[i] = (bbmax[i] - origin[i]) / dir[i];
			Inside      = false;
		}else if(origin[i] < bbmin[i])
		{
			finalCoord[i]   = bbmin[i];
			if(dir[i]!=0.0f)
				tmax[i] = (bbmin[i] - origin[i]) / dir[i];
			Inside      = false;
		}
	}

	if(Inside) {
		Coord = start.Cast();
		return true;
	}

	unsigned int WhichPlane = 0;
	if(tmax[1] > tmax[WhichPlane])  WhichPlane = 1;
	if(tmax[2] > tmax[WhichPlane])  WhichPlane = 2;

	if(tmax[WhichPlane]<0)
		return false;
	for(i=0;i<3;i++) {
		if(i!=WhichPlane) {
			finalCoord[i] = origin[i] + tmax[WhichPlane] * dir[i];
			if(finalCoord[i]+eps < bbmin[i] || finalCoord[i] > bbmax[i] + eps)  return false;
		}
	}
	Coord=Vector(finalCoord[0],finalCoord[1],finalCoord[2]);
	if (tmax[0]>=0&&tmax[0]<=1&&tmax[1]>=0&&tmax[1]<=1&&tmax[2]>=0&&tmax[2]<=1) {
		return true;
	}
	return false;
}


bool testRayInsideBB(const Vector &Min, const Vector &Max, const QVector& start, const Vector& end, Vector& Coord)
{
	if (start.i>Min.i&&start.j>Min.j&&start.k>Min.k&&start.i<Max.i&&start.j<Max.j&&start.k<Max.k) {
		return true;
	}
	return testRayVersusBB(Min,Max,start,end,Coord);
}


Unit * Unit::queryBSP (const QVector &start, const QVector & end, Vector & norm, float &distance, bool ShieldBSP)
{
	Unit * tmp;
	float rad=this->rSize();
	if ((!SubUnits.empty())&&graphicOptions.RecurseIntoSubUnitsOnCollision)
		if (tmp=*SubUnits.fastIterator())
			rad+=tmp->rSize();
	if (!globQuerySphere(start,end,cumulative_transformation_matrix.p,rad))
		return NULL;
	static bool use_bsp_tree = XMLSupport::parse_bool(vs_config->getVariable("physics","beam_bsp","false"));
	if (graphicOptions.RecurseIntoSubUnitsOnCollision)
	if (!SubUnits.empty()) {
		un_fiter i(SubUnits.fastIterator());
		for (Unit * un;un=*i;++i) {
			if ((tmp=un->queryBSP(start,end, norm,distance,ShieldBSP))!=0) {
				return tmp;
			}
		}
	}
	BSPTree *myNull=NULL;
	BSPTree *const* tmpBsp = &myNull;
	QVector st (InvTransform (cumulative_transformation_matrix,start));
	QVector ed (InvTransform (cumulative_transformation_matrix,end));

	if (use_bsp_tree) {
		if (this->colTrees) {
			tmpBsp=ShieldUp(st.Cast())?&this->colTrees->bspShield:&this->colTrees->bspTree;
			if (this->colTrees->bspTree&&!ShieldBSP) {
				tmpBsp= &this->colTrees->bspTree;
			}
			tmpBsp = &this->colTrees->bspTree;
		}
	}
	//for (;tmpBsp!=NULL;tmpBsp=((ShieldUp(st.Cast())&&(tmpBsp!=((this->colTrees?&this->colTrees->bspTree:&myNull))))?((this->colTrees?&this->colTrees->bspTree:&myNull)):NULL)) {
	static bool sphere_test=XMLSupport::parse_bool(vs_config->getVariable("physics","sphere_collision","true"));
	static bool bb_test=XMLSupport::parse_bool(vs_config->getVariable("physics","bounding_box_collision","false"));

	distance = querySphereNoRecurse (start,end);
	if (distance||!sphere_test) {
		if (!(*tmpBsp)) {
			Vector coord;
			int nm=nummesh();
			Unit * retval=NULL;
			if (bb_test) {
				for (unsigned int i=0;i<nm;++i) {
					if(testRayVersusBB(meshdata[i]->corner_min(),meshdata[i]->corner_max(),st,ed,coord)) {
						norm = TransformNormal(cumulative_transformation_matrix,coord);
						distance=(coord-st).Magnitude();
								 //normal points out from center
						norm.Normalize();
						ed=coord.Cast();
						retval=this;
					}
				}
			}
			else {
				norm = (distance * (start-end)).Cast();
				distance = norm.Magnitude();
				norm= (norm.Cast()+start).Cast();
				norm.Normalize();//normal points out from center
				retval=this;
			}

			return retval;
		}
	}else
	return NULL;
	/*bool temp=false;
		for (i=0;i<nummesh()&&!temp;i++) {
	  temp = (1==meshdata[i]->queryBoundingBox (st,ed,0));
	}
	if (!temp) {
	  return NULL;
	}*/
	if ((distance = (*tmpBsp)->intersects (st.Cast(),ed.Cast(),norm))!=0) {
		norm = ToWorldCoordinates (norm);
		return this;
	}
	//}
	return NULL;
}


bool Unit::querySphere (const QVector &pnt, float err) const
{
	int i;
	const Matrix * tmpo = &cumulative_transformation_matrix;

	Vector TargetPoint (tmpo->getP());
#ifdef VARIABLE_LENGTH_PQR
								 //adjust the ship radius by the scale of local coordinates
	double SizeScaleFactor = sqrt(TargetPoint.Dot(TargetPoint));
#endif
	if (nummesh()<1&&isPlanet()) {
		TargetPoint = (tmpo->p-pnt).Cast();
		if (TargetPoint.Dot (TargetPoint)<
			err*err+
			radial_size*radial_size
#ifdef VARIABLE_LENGTH_PQR
			*SizeScaleFactor*SizeScaleFactor
#endif
			+
#ifdef VARIABLE_LENGTH_PQR
			SizeScaleFactor*
#endif
			2*err*radial_size
			)
			return true;
	}
	else {
		for (i=0;i<nummesh();i++) {
			TargetPoint = (Transform (*tmpo,meshdata[i]->Position().Cast())-pnt).Cast();
			if (TargetPoint.Dot (TargetPoint)<
				err*err+
				meshdata[i]->rSize()*meshdata[i]->rSize()
#ifdef VARIABLE_LENGTH_PQR
				*SizeScaleFactor*SizeScaleFactor
#endif
				+
#ifdef VARIABLE_LENGTH_PQR
				SizeScaleFactor*
#endif
				2*err*meshdata[i]->rSize()
				)
				return true;
		}
	}
	if (graphicOptions.RecurseIntoSubUnitsOnCollision)
	if (!SubUnits.empty()) {
		un_fkiter i=SubUnits.constFastIterator();
		for (const Unit * un;(un=*i);++i) {
			if ((un)->querySphere (pnt,err)) {
				return true;
			}
		}
	}
	return false;
}


float Unit::querySphere (const QVector &start, const QVector &end, float min_radius) const
{
	if (!SubUnits.empty()) {
		un_fkiter i=SubUnits.constFastIterator();
		for (const Unit * un;(un=*i);++i) {
			float tmp;
			if ((tmp=un->querySphere (start,end,min_radius))!=0) {
				return tmp;
			}
		}
	}

	return querySphereNoRecurse (start,end,min_radius);
}


//does not check inside sphere
float Unit::querySphereNoRecurse (const QVector & start, const QVector & end, float min_radius) const
{
	int i;
	double tmp;
	for (i=0;i<nummesh();i++) {

		if ((meshdata[i]->Position().Magnitude()>this->rSize())||(meshdata[i]->rSize()>30+this->rSize())) {
			continue;
		}
		if (isUnit()==PLANETPTR&&i>0)
			break;
		double a, b,c;
		QVector st = start - Transform (cumulative_transformation_matrix,meshdata[i]->Position().Cast());

		QVector dir = end-start; //now start and end are based on mesh's position
		// v.Dot(v) = r*r; //equation for sphere
		// (x0 + (x1 - x0) *t) * (x0 + (x1 - x0) *t) = r*r
		c = st.Dot (st);
		double temp1 = (min_radius+meshdata[i]->rSize());
		// if (st.MagnitudeSquared()<temp1*temp1) //UNCOMMENT if you want inside sphere to count...otherwise...
		//  return 1.0e-6;
		if( min_radius!=-FLT_MAX)
			c = c - temp1*temp1;
		else
			c = temp1;
#ifdef VARIABLE_LENGTH_PQR
		c *= SizeScaleFactor*SizeScaleFactor;
#endif
		b = 2 * (dir.Dot (st));
		a = dir.Dot(dir);
		//b^2-4ac
		if( min_radius!=-FLT_MAX)
			c = b*b - 4*a*c;
		else
			c = FLT_MAX;
		if (c<0||a==0)
			continue;
		a *=2;

		tmp = (-b + sqrt (c))/a;
		c = (-b - sqrt (c))/a;
		if (tmp>0&&tmp<=1) {
			return (c>0&&c<tmp) ? c : tmp;
		}
		else if (c>0&&c<=1) {
			return c;
		}
	}

	return 0;
	//return (bestd==-FLT_MAX)?0:(bestd/sqrt(beammsqr));
}
