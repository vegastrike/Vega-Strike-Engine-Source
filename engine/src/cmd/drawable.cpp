#include "drawable.h"
#include "vsfilesystem.h"
#include "gfx/mesh.h"
#include "gfx/quaternion.h"
#include "unit_generic.h"

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
        // TODO: search and replace cerr with BOOST_LOG_TRIVIAL(error)
                    cerr << "Animated Mesh: " << path << " loaded - with: " << m->getVertexList()->GetNumVertices() << " vertices." << endl;
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
		std::cerr << "Animation data loaded for unit: " << string(filename) << ", named " << uniqueUnitName << " - with: " << numFrames << " frames." << std::endl;
		return true;
	} else {
		delete meshes;
		return false;
	}
}

void Drawable::AnimationStep()
{
#ifdef DEBUG_MESH_ANI
        std::cerr << "Starting animation step of Unit: " << uniqueUnitName << std::endl;
#endif
        if((!this->isContinuousLoop())&&(loopCount==0))
            return;
        //copy reference to data
        meshdata.at(0) = vecAnimations.at(activeAnimation)->at(activeMesh);

        Draw();

#ifdef DEBUG_MESH_ANI
        std::cerr << "Drawed mesh: " << uniqueUnitName << std::endl;
#endif

        activeMesh = nextactiveMesh;
        nextactiveMesh++;
        if(nextactiveMesh >= vecAnimations.at(activeAnimation)->size())
            nextactiveMesh = 0;

        if(loopCount > 0)
            loopCount--;
#ifdef DEBUG_MESH_ANI
        std::cerr << "Ending animation step of Unit: " << uniqueUnitName << std::endl;
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
