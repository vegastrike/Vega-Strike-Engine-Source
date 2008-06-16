//
// C++ Implementation: Audio::SimpleScene
//

#include "SimpleScene.h"
#include "SimpleSource.h"
#include "config.h"

namespace Audio {

    SimpleScene::SimpleScene(const std::string &name) throw() :
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
        for (it = inactiveSources.begin(); it != inactiveSources.end(); ++it)
            detach(dynamic_cast<SimpleSource*>(it->get()));
    }

    void SimpleScene::add(SharedPtr<Source> source) 
        throw(Exception)
    {
        attach(dynamic_cast<SimpleSource*>(source.get()));
    }
    
    void SimpleScene::remove(SharedPtr<Source> source) 
        throw(NotFoundException)
    {
        detach(dynamic_cast<SimpleSource*>(source.get()));
    }
    
    Listener& SimpleScene::getListener() 
        throw()
    {
        return listener;
    }
    
    void SimpleScene::notifySourcePlaying(SharedPtr<Source> source, bool playing) 
        throw(Exception)
    {
        if (playing) {
            inactiveSources.erase(source);
            activeSources.insert(source);
        } else {
            activeSources.erase(source);
            inactiveSources.insert(source);
        }
    }
    
    void SimpleScene::attach(SimpleSource *source) 
        throw()
    {
        source->notifySceneAttached(this);
    }
    
    void SimpleScene::detach(SimpleSource *source) 
        throw()
    {
        source->notifySceneAttached(0);
    }
    
};
