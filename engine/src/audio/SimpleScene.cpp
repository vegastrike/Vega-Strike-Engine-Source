//
// C++ Implementation: Audio::SimpleScene
//

#include "SimpleScene.h"
#include "SimpleSource.h"
#include "config.h"

#include "SceneManager.h"

namespace Audio {

    SimpleScene::SimpleScene(const std::string &name) :
        Scene(name)
    {
    }
    
    SimpleScene::~SimpleScene()
    {
        SourceSet::iterator it;
        
        for (it = activeSources.begin(); it != activeSources.end(); ++it) {
            (*it)->stopPlaying();
            detach(dynamic_cast<SimpleSource*>(it->get()));
        }
    }

    void SimpleScene::add(SharedPtr<Source> source) 
    {
        attach(dynamic_cast<SimpleSource*>(source.get()));
    }
    
    void SimpleScene::remove(SharedPtr<Source> source) 
    {
        detach(dynamic_cast<SimpleSource*>(source.get()));
    }
    
    Listener& SimpleScene::getListener() 
    {
        return listener;
    }
    
    void SimpleScene::notifySourcePlaying(SharedPtr<Source> source, bool playing) 
    {
        if (playing) 
            activeSources.insert(source);
        else
            activeSources.erase(source);
        
        SceneManager::getSingleton()->notifySourcePlaying(source, shared_from_this(), playing);
    }
    
    void SimpleScene::attach(SimpleSource *source) 
    {
        source->notifySceneAttached(this);
    }
    
    void SimpleScene::detach(SimpleSource *source) 
    {
        source->notifySceneAttached(0);
    }
    
    /** Gets an iterator over active sources */ 
    SimpleScene::SourceIterator SimpleScene::getActiveSources() 
    {
        return activeSources.begin();
    }
    
    /** Gets the ending iterator of active sources */
    SimpleScene::SourceIterator SimpleScene::getActiveSourcesEnd() 
    {
        return activeSources.end();
    }
    
};
