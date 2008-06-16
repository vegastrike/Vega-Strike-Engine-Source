//
// C++ Interface: Audio::Scene
//
#ifndef __AUDIO_SCENE_H__INCLUDED__
#define __AUDIO_SCENE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"

namespace Audio {

    // Forward declarations
    class Source;
    class Listener;
    

    /**
     * Scene abstract class
     *
     * @remarks This class represents a scene and the interface to manipulating them.
     *      A scene is a collection of sources and ONE listener. The ability to have
     *      multiple scenes (active or not) is of great use, so the possibility of
     *      a scene manager containing multiple scenes is given.
     *      @par Implementations of the Scene interface is required for any given
     *      SceneManager. In fact, it is expected that each SceneManager will provide
     *      its own Scene implementation. Therefore, a concrete Scene class can be
     *      thought of as part of a SceneManager's implementation.
     *
     */
    class Scene
    {
        std::string name;
    
    protected:
        /** Internal constructor used by derived classes */
        Scene(const std::string &name) throw();
        
    public:
        virtual ~Scene();
        
        const std::string& getName() const throw() { return name; }
        
        /** Attach a source to this scene.
         * @remarks The must be stopped. Adding a playing source is an error.
         */
        virtual void add(SharedPtr<Source> source) throw(Exception) = 0;
        
        /** Detach a source from this scene
         * @remarks The source is implicitly stopped.
         */
        virtual void remove(SharedPtr<Source> source) throw(NotFoundException) = 0;
        
        /** Get the scene's listener */
        virtual Listener& getListener() throw() = 0;
    };

};

#endif//__AUDIO_SCENE_H__INCLUDED__
