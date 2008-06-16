//
// C++ Implementation: Audio::SceneManager
//

#include "SceneManager.h"
#include "config.h"

#include "Renderer.h"
#include "SourceTemplate.h"
#include "SimpleSource.h"
#include "SimpleScene.h"

template<> Audio::SceneManager* Singleton<Audio::SceneManager>::_singletonInstance = 0;

namespace Audio {

    namespace __impl {
    
        struct SceneManagerData
        {
            // The many required indexes
            typedef std::map<std::string, SharedPtr<Scene> > SceneMap;
            typedef std::set<SharedPtr<Source> > SourceSet;
            
            SceneMap activeScenes;
            SceneMap inactiveScenes;
            
            // Being rendered, they need frequent updates
            SourceSet activeSources; 
            
            SharedPtr<Renderer> renderer;
            
            unsigned int maxSources;
        };
    
    };
    
    using namespace __impl;

    SceneManager::SceneManager() throw() :
        data(new SceneManagerData)
    {
    }
    
    SceneManager::~SceneManager()
    {
    }
    
    SharedPtr<Source> SceneManager::createSource(SharedPtr<Sound> sound, bool looping) 
        throw(Exception)
    {
        throw Exception("not implemented");
    }
    
    SharedPtr<Source> SceneManager::createSource(SharedPtr<SourceTemplate> tpl) 
        throw(Exception)
    {
        SharedPtr<Source> source = createSource(
            getRenderer()->getSound(
                tpl->getSoundName(),
                tpl->getSoundType(),
                tpl->isStreaming() ), 
            tpl->isLooping() );
         
        source->setCosAngleRange( tpl->getCosAngleRange() );
        source->setPerFrequencyRadiusRatios( tpl->getPerFrequencyRadiusRatios() );
        source->setReferenceFreqs( tpl->getReferenceFreqs() );
        source->setGain( tpl->getGain() );
        
        return source;
    }
    
    void SceneManager::destroySource(SharedPtr<Source> source) 
        throw()
    {
        // By simply unreferencing, it should get destroyed when all references are released.
        // Which is good for multithreading - never destroy something that is being referenced.
        
        // We cannot have playing sources without references within the manager
        // Since it's stopped, it will eventually be removed from the active list if there.
        if (source->isPlaying())
            source->stopPlaying();
        
        // Remove all references to it within every scene
        SceneManagerData::SceneMap::iterator it;
        for (it = data->activeScenes.begin(); it != data->activeScenes.end(); ++it)
            it->second->remove(source);
        for (it = data->inactiveScenes.begin(); it != data->inactiveScenes.end(); ++it)
            it->second->remove(source);
    }
    
    void SceneManager::addScene(SharedPtr<Scene> scene) 
        throw(DuplicateObjectException)
    {
        if (   data->activeScenes.count(scene->getName()) 
            || data->inactiveScenes.count(scene->getName()) )
            throw(DuplicateObjectException(scene->getName()));
        
        data->inactiveScenes[scene->getName()] = scene;
    }
    
    SharedPtr<Scene> SceneManager::createScene(const std::string &name) 
        throw(DuplicateObjectException)
    {
        SharedPtr<Scene> scenePtr(new SimpleScene(name));
        addScene(scenePtr);
        return scenePtr;
    }
    
    SharedPtr<Scene> SceneManager::getScene(const std::string &name) const 
        throw(NotFoundException)
    {
        SceneManagerData::SceneMap::const_iterator it;
        
        it = data->activeScenes.find(name);
        if (it != data->activeScenes.end())
            return it->second;
        
        it = data->inactiveScenes.find(name);
        if (it != data->inactiveScenes.end())
            return it->second;
        
        throw(NotFoundException(name));
    }
    
    void SceneManager::destroyScene(const std::string &name) const 
        throw(NotFoundException)
    {
        // By simply unreferencing, it should get destroyed when all references are released.
        // Which is good for multithreading - never destroy something that is being referenced.
        // Any active sources will get deactivated in the next update since there aren't any active scenes
        // containing them.
        data->activeScenes.erase(name);
        data->inactiveScenes.erase(name);
    }
    
    void SceneManager::setSceneActive(const std::string &name, bool active) 
        throw(NotFoundException)
    {
        // Simply move the pointer from one map to the other.
        // The next update will take care of activating sources as necessary.
        SharedPtr<Scene> scene = getScene(name);
        if (active) {
            data->inactiveScenes.erase(name);
            data->activeScenes[name] = scene;
        } else {
            data->activeScenes.erase(name);
            data->inactiveScenes[name] = scene;
        }
    }
    
    bool SceneManager::getSceneActive(const std::string &name) 
        throw(NotFoundException)
    {
        return data->activeScenes.count(name) > 0;
    }
    
    void SceneManager::setRenderer(SharedPtr<Renderer> renderer) 
        throw(Exception)
    {
        SharedPtr<Renderer> curRenderer = getRenderer();
    
        // Detach all active sources
        for (SceneManagerData::SourceSet::const_iterator it = data->activeSources.begin(); it != data->activeSources.end(); ++it)
            curRenderer->detach(*it);
        
        // Swap renderers
        data->renderer.swap(renderer);
    }
    
    SharedPtr<Renderer> SceneManager::getRenderer() const 
        throw()
    {
        return data->renderer;
    }
    
    unsigned int SceneManager::getMaxSources() const 
        throw()
    {
        return data->maxSources;
    }
    
    void SceneManager::setMaxSources(unsigned int n) 
        throw(Exception)
    {
        data->maxSources = n;
    }

    void SceneManager::playSource(
            SharedPtr<SourceTemplate> tpl, 
            const std::string &sceneName,
            LVector3 position,
            Vector3 direction,
            Vector3 velocity,
            Scalar radius) throw(Exception)
    {
        if (tpl->isLooping())
            throw(Exception("Cannot fire a looping source and forget!"));
        
        SharedPtr<Source> src = createSource(tpl);
        
        src->setPosition(position);
        src->setDirection(direction);
        src->setVelocity(velocity);
        src->setRadius(radius);
        
        src->startPlaying();
    }
    
};

