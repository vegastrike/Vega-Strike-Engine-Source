/**
 * drawable.cpp
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "drawable.h"
#include "vsfilesystem.h"
#include "gfx/mesh.h"
#include "gfx/quaternion.h"
#include "unit_generic.h"
#include "unit.h"
#include "options.h"
#include "csv.h"
#include "unit_csv.h"
#include "universe_util.h"

#include <boost/algorithm/string.hpp>


Drawable::Drawable() :
                   animatedMesh(true),
                   activeAnimation(0),
                   timeperframe(3.0),
                   done(true),
                   activeMesh(0),
                   nextactiveMesh(1),
                   infiniteLoop(true),
                   loopCount(0),
                   curtime(0.0)

{}

bool Drawable::DrawableInit(const char *filename, int faction,
        Flightgroup *flightgrp, const char *animationExt)
{
    string fnam(filename);
    string::size_type pos = fnam.find('.');
    string anifilename = fnam.substr(0, pos);

    if(animationExt)
        anifilename += string("_") + string(animationExt);

    std::vector< Mesh* > *meshes = new vector<Mesh *>();
    int i = 1;
    char count[30] = "1";
    string dir = anifilename;
    while(true)
    {
            sprintf( count, "%d", i );
            string unit_name = anifilename;
            boost::algorithm::to_lower(unit_name); //toLowerCase(anifilename) + "_";

            if(i < 10)
                    unit_name += "0";
            if(i < 100)
                    unit_name += "0";
            if(i < 1000)
                    unit_name += "0";
            if(i < 10000)
                    unit_name += "0";
            if(i < 100000)
                    unit_name += "0";

            unit_name += count;
            string path = dir + "/" + unit_name + ".bfxm";
            if( VSFileSystem::FileExistsData( path, VSFileSystem::MeshFile ) != -1 )
            {
                    Mesh *m = Mesh::LoadMesh( path.c_str(), Vector(1,1,1), faction, flightgrp );
                    meshes->push_back( m );
    #ifdef DEBUG_MESH_ANI
                    BOOST_LOG_TRIVIAL(debug) << "Animated Mesh: " << path << " loaded - with: " << m->getVertexList()->GetNumVertices() << " vertices.";
    #endif
            }
            else
                    break;
            i++;
    }


	if( meshes->size() != 0 )
	{
		//FIXME: an animation is created only for the first submesh
		string animationName;
		sprintf( count, "%lu", (unsigned long)meshes->size() );
		if(!animationExt)
			animationName = string(count); //if there is no extension given, the animations are called by their load order, 1, 2 ,3 ....10..
		else
			animationName = animationExt;
		addAnimation(meshes, animationName.c_str());

		int numFrames = meshes->size();
		++Drawable::unitCount;
		sprintf( count, "%u", unitCount );
		uniqueUnitName = drawableGetName() + string(count);
		Units[uniqueUnitName] = static_cast<Unit*>(this);
		BOOST_LOG_TRIVIAL(info) << "Animation data loaded for unit: " << string(filename) << ", named " << uniqueUnitName << " - with: " << numFrames << " frames.";
		return true;
	} else {
		delete meshes;
		return false;
	}
}

void Drawable::AnimationStep()
{
#ifdef DEBUG_MESH_ANI
        BOOST_LOG_TRIVIAL(debug) << "Starting animation step of Unit: " << uniqueUnitName;
#endif
        if((!this->isContinuousLoop())&&(loopCount==0))
            return;
        //copy reference to data
        meshdata.at(0) = vecAnimations.at(activeAnimation)->at(activeMesh);

        Draw();

#ifdef DEBUG_MESH_ANI
        BOOST_LOG_TRIVIAL(debug) << "Drawed mesh: " << uniqueUnitName;
#endif

        activeMesh = nextactiveMesh;
        nextactiveMesh++;
        if(nextactiveMesh >= vecAnimations.at(activeAnimation)->size())
            nextactiveMesh = 0;

        if(loopCount > 0)
            loopCount--;
#ifdef DEBUG_MESH_ANI
        BOOST_LOG_TRIVIAL(debug) << "Ending animation step of Unit: " << uniqueUnitName;
#endif
}

void Drawable::UpdateFrames()
{
	std::map< string, Unit * >::iterator pos;
	for(pos = Units.begin(); pos != Units.end(); ++pos)
	{
	pos->second->curtime += GetElapsedTime();
	if (pos->second->curtime >= pos->second->timePerFrame()) {
		pos->second->AnimationStep();
		pos->second->curtime = 0.0;
	}
	}
}

void Drawable::addAnimation( std::vector<Mesh *> *meshes, const char* name )
{
    if( (meshes->size() > 0) && animatedMesh ) {
        vecAnimations.push_back( meshes );
        vecAnimationNames.push_back(string(name));
    }
}

void Drawable::StartAnimation( unsigned int how_many_times, int numAnimation )
{
    if(animationRuns())
        StopAnimation();

    done = false;
}

void Drawable::StopAnimation()
{
    done = true;
}

string Drawable::getAnimationName(unsigned int animationNumber) const
{
        return vecAnimationNames.at(animationNumber);
}

unsigned int Drawable::getAnimationNumber(const char *name) const
{
	string strname(name);
	for(unsigned int i=0;i<vecAnimationNames.size();i++)
		if(strname == vecAnimationNames[i])
			return i;

	return 0; //NOT FOUND!
}

void Drawable::ChangeAnimation( const char *name )
{
        unsigned int AnimNumber = getAnimationNumber(name);
    if( (AnimNumber < numAnimations()) && isAnimatedMesh() )
        activeAnimation = AnimNumber;
}

void Drawable::ChangeAnimation( unsigned int AnimNumber )
{
    if( (AnimNumber < numAnimations()) && isAnimatedMesh() )
        activeAnimation = AnimNumber;
}

bool Drawable::isAnimatedMesh() const
{
    return animatedMesh;
}

double Drawable::framesPerSecond() const
{
    return 1/timeperframe;
}

double Drawable::timePerFrame() const
{
    return timeperframe;
}

unsigned int Drawable::numAnimations()
{
    return vecAnimations.size();
}

void Drawable::ToggleAnimatedMesh( bool on )
{
    animatedMesh = on;
}

bool Drawable::isContinuousLoop() const
{
        return infiniteLoop;
}

void Drawable::SetAniSpeed( float speed )
{
    timeperframe = speed;
}

void Drawable::clear()
{
    StopAnimation();

    for (unsigned int i = 0; i < vecAnimations.size(); i++) {
        for(unsigned int j = 0; j < vecAnimations[i]->size(); j++) {
                delete vecAnimations[i]->at(j);
        }
        delete vecAnimations[i];
                vecAnimations[i]->clear();
    }
    vecAnimations.clear();
    vecAnimationNames.clear();

    Units.erase(uniqueUnitName);
}

bool Drawable::animationRuns() const
{
    return !done;
}


void Drawable::Split( int level )
{
    Unit *unit = static_cast<Unit*>(this);

    if (game_options.split_dead_subunits)
        for (un_iter su = unit->getSubUnits(); *su; ++su)
            (*su)->Split( level );
    Vector PlaneNorm;
    for (unsigned int i = 0; i < nummesh();) {
        if (this->meshdata[i]) {
            if (this->meshdata[i]->getBlendDst() == ONE) {
                delete this->meshdata[i];
                this->meshdata.erase( this->meshdata.begin()+i );
            } else {i++; }} else {this->meshdata.erase( this->meshdata.begin()+i ); }}
    int    nm = this->nummesh();
    string fac = FactionUtil::GetFaction( unit->faction );

    CSVRow unit_stats( LookupUnitRow( unit->name, fac ) );
    unsigned int num_chunks = unit_stats.success() ? atoi( unit_stats["Num_Chunks"].c_str() ) : 0;
    if (nm <= 0 && num_chunks == 0)
        return;
    vector< Mesh* >old = this->meshdata;
    Mesh *shield = old.back();
    old.pop_back();

    vector< unsigned int >meshsizes;
    if ( num_chunks && unit_stats.success() ) {
        size_t i;
        vector< Mesh* >nw;
        unsigned int which_chunk = rand()%num_chunks;
        string  chunkname   = UniverseUtil::LookupUnitStat( unit->name, fac, "Chunk_"+XMLSupport::tostring( which_chunk ) );
        string  dir = UniverseUtil::LookupUnitStat( unit->name, fac, "Directory" );
        VSFileSystem::current_path.push_back( unit_stats.getRoot() );
        VSFileSystem::current_subdirectory.push_back( "/"+dir );
        VSFileSystem::current_type.push_back( UnitFile );
        float randomstartframe   = 0;
        float randomstartseconds = 0;
        string scalestr     = UniverseUtil::LookupUnitStat( unit->name, fac, "Unit_Scale" );
        int   scale = atoi( scalestr.c_str() );
        if (scale == 0) scale = 1;
        AddMeshes( nw, randomstartframe, randomstartseconds, scale, chunkname, unit->faction,
                   unit->getFlightgroup(), &meshsizes );
        VSFileSystem::current_type.pop_back();
        VSFileSystem::current_subdirectory.pop_back();
        VSFileSystem::current_path.pop_back();
        for (i = 0; i < old.size(); ++i)
            delete old[i];
        old.clear();
        old = nw;
    } else {
        for (int split = 0; split < level; split++) {
            vector< Mesh* >nw;
            size_t oldsize = old.size();
            for (size_t i = 0; i < oldsize; i++) {
                PlaneNorm.Set( rand()-RAND_MAX/2, rand()-RAND_MAX/2, rand()-RAND_MAX/2+.5 );
                PlaneNorm.Normalize();
                nw.push_back( NULL );
                nw.push_back( NULL );
                old[i]->Fork( nw[nw.size()-2], nw.back(), PlaneNorm.i, PlaneNorm.j, PlaneNorm.k,
                             -PlaneNorm.Dot( old[i]->Position() ) );                                                                              //splits somehow right down the middle.
                delete old[i];
                old[i] = NULL;
                if (nw[nw.size()-2] == NULL) {
                    nw[nw.size()-2] = nw.back();
                    nw.pop_back();
                }
                if (nw.back() == NULL)
                    nw.pop_back();
            }
            old = nw;
        }
        meshsizes.reserve( old.size() );
        for (size_t i = 0; i < old.size(); ++i)
            meshsizes.push_back( 1 );
    }
    old.push_back( NULL );     //push back shield
    if (shield)
        delete shield;
    nm = old.size()-1;
    unsigned int k = 0;
    vector< Mesh* >tempmeshes;
    for (vector<Mesh *>::size_type i=0;i<meshsizes.size();i++) {
        Unit *splitsub;
        tempmeshes.clear();
        tempmeshes.reserve( meshsizes[i] );
        for (unsigned int j = 0; j < meshsizes[i] && k < old.size(); ++j, ++k)
            tempmeshes.push_back( old[k] );
        unit->SubUnits.prepend( splitsub = new GameUnit( tempmeshes, true, unit->faction ) );
        splitsub->hull->facets[0].health = 1000;
        splitsub->name = "debris";
        splitsub->setMass(game_options.debris_mass*splitsub->getMass()/level);
        splitsub->pImage->timeexplode = .1;
        if (splitsub->meshdata[0]) {
            Vector loc  = splitsub->meshdata[0]->Position();
            float  locm = loc.Magnitude();
            if (locm < .0001)
                locm = 1;
            splitsub->ApplyForce( splitsub->meshdata[0]->rSize()*game_options.explosionforce*10*splitsub->getMass()*loc/locm );
            loc.Set( rand(), rand(), rand()+.1 );
            loc.Normalize();
            splitsub->ApplyLocalTorque( loc*splitsub->GetMoment()*game_options.explosiontorque*( 1+rand()%(int) ( 1+unit->rSize() ) ) );
        }
    }
    old.clear();
    this->meshdata.clear();
    this->meshdata.push_back( NULL );     //the shield
    unit->Mass *= game_options.debris_mass;
}


void Drawable::LightShields( const Vector &pnt, const Vector &normal, float amt, const GFXColor &color )
{
    Unit *unit = static_cast<Unit*>(this);
    // Not sure about shield percentage - more variance for more damage?
    // TODO: figure out the above comment
    meshdata.back()->AddDamageFX( pnt, unit->shieldtight ? unit->shieldtight*normal : Vector( 0, 0, 0 ), std::min( 1.0f, std::max( 0.0f,                                                                                                            amt ) ), color );
}
