//
// C++ Interface: Audio::SimpleScene
//
#ifndef __AUDIO_SIMPLESCENE_H__INCLUDED__
#define __AUDIO_SIMPLESCENE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Scene.h"
#include "Listener.h"

#include <set>


namespace Audio {

    // Forwards
    class Source;
    class SimpleSource;

    /**
     * SimpleScene, basic implementation of the Scene interface
     *
     * @remarks This class implements the scene interface for a basic Scene manager.
     *
     */
    class SimpleScene : public Scene, public SharedFromThis<SimpleScene>
    {
        typedef std::set<SharedPtr<Source> > SourceSet;
        
        Listener listener;
        
        SourceSet activeSources;
        
    public:
        typedef SourceSet::iterator SourceIterator;
    
    public:
    
        //
        // Standard Scene interface
        //
    
        /** Construct a new, empty scene */
        SimpleScene(const std::string &name) throw();
        
        virtual ~SimpleScene();
        
        /** @copydoc Scene::add 
         * @remarks source MUST be a SimpleSource
         */
        virtual void add(SharedPtr<Source> source) throw(Exception);
        
        /** @copydoc Scene::remove 
         * @remarks source MUST be a SimpleSource
         */
        virtual void remove(SharedPtr<Source> source) throw(NotFoundException);
        
        /** @copydoc Scene::getListener */
        virtual Listener& getListener() throw();
        
        
        //
        // SimpleScene-specific interface
        //
        
        /** Notify the scene of a source that starts or stops playing. */
        virtual void notifySourcePlaying(SharedPtr<Source> source, bool playing) throw(Exception);
        
        /** Gets an iterator over active sources */ 
        SourceIterator getActiveSources() throw();
        
        /** Gets the ending iterator of active sources */
        SourceIterator getActiveSourcesEnd() throw();
        
    protected:
        void attach(SimpleSource *source) throw();
        void detach(SimpleSource *source) throw();
    };

};

#endif//__AUDIO_SIMPLESCENE_H__INCLUDED__
