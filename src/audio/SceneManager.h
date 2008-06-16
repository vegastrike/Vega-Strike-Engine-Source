//
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_SCENEMANAGER_H__INCLUDED__
#define __AUDIO_SCENEMANAGER_H__INCLUDED__

#include <string>
#include <map>
#include <set>
#include "Exceptions.h"
#include "Singleton.h"
#include "Types.h"


namespace Audio {

    // Some forwards, we don't need to know the details of those classes
    class Renderer;
    class Scene;
    class SourceTemplate;
    class Source;
    class Sound;
    
    namespace __impl {
        
        // Forward declaration of internal template manager data
        struct SceneManagerData;
        
    };

    /**
     * Audio Scene Manager.
     *
     * @remarks Use it to create and manage sources, listeners, instantiate source templates,
     *      scenes, and other various scene management tasks. 
     *      @par SceneManager s will handle the complex task of prioritizing sources, managing
     *      source channels, volume levels, and doing all kinds of culling.
     *      @par Another responsability of scene managers is the manipulation and analysis of
     *      environments to feed environmental information to capable renderers.
     *      @par Inactive sources without external references are automatically destroyed when
     *      they become permanently inactive (notice this requirement is merely approximate -
     *      implementations are free to make reasonable approximations for the determination
     *      of such condition as "permanently inactive"). This allows for "fire & forget" sources,
     *      where the source is activated (by calling startPlaying), and then immediately 
     *      unreferenced. The SceneManager will keep track of that source, as it would any other,
     *      until it stops playing (make sure though it's not a looping source!), and will remove
     *      it when that happens, freeing any associated resources.
     *      @par In fact this is so common as to make it part of the API: playSource does exactly
     *      that, it creates a source, plays it, and discards the reference.
     *      @par Thus, SceneManager s are the hub of all activity relating sound. If you're doing
     *      anything bypassing the SceneManager, you're doing it wrong.
     *      @par This class is NOT abstract, it provides a very basic implementation that does
     *      nothing special.
     * @note You must set a renderer or you won't hear anything ;)
     *
     */
    class SceneManager : public Singleton<SceneManager>
    {
    private:
        AutoPtr<__impl::SceneManagerData> data;
    
    public:
        /** Construct an empty registry 
         * @remarks End-users of the class shouldn't be using this. Singletons need it.
         *      Instead, end-users should use the Root class to find manager plugins they
         *      like.
         */
        SceneManager() throw();
        
        virtual ~SceneManager();
        
        /** Create a new source based on the speicified sound
         * @remarks All the attributes will hold unspecified values, so you have to fill them in.}
         * @note The sound must be associated to the correct renderer, or bad things will happen.
         */
        virtual SharedPtr<Source> createSource(SharedPtr<Sound> sound, bool looping = false) throw(Exception);
        
        /** Create a new source based on the specified template
         * @remarks All location information will hold unspecified values, so you have to fill them in.
         */
        SharedPtr<Source> createSource(SharedPtr<SourceTemplate> tpl) throw(Exception);
        
        /** Destroy a source created with this manager */
        virtual void destroySource(SharedPtr<Source> source) throw();
        
        
        
        /** Convenience API to play a source once and forget. 
         * @param tpl The source template from which a source should be instanced
         * @param sceneName The name of the scene to which the source should be attached
         * @param position The initial position of the source
         * @param direction The direction of the (if directional) source
         * @param velocity The movement velocity of the source
         * @param radius The base radius of the source
         * @remarks The source should not be looping, and an exception will be thrown if it is.
         */
        void playSource(
            SharedPtr<SourceTemplate> tpl, 
            const std::string &sceneName,
            LVector3 position,
            Vector3 direction,
            Vector3 velocity,
            Scalar radius) throw(Exception);
        
        /** Create a new named scene */
        virtual SharedPtr<Scene> createScene(const std::string &name) throw(DuplicateObjectException);
        
        /** Get an existing scene by its name */
        virtual SharedPtr<Scene> getScene(const std::string &name) const throw(NotFoundException);
        
        /** Destroy an existing scene by its name */
        virtual void destroyScene(const std::string &name) const throw(NotFoundException);
        
        /** Sets the active state of a scene */
        virtual void setSceneActive(const std::string &name, bool active) throw(NotFoundException);
        
        /** Get the active state of a scene */
        virtual bool getSceneActive(const std::string &name) throw(NotFoundException);
        
        
        /** Set a new renderer
         * @param renderer A new renderer to be used.
         * @remarks Setting the renderer should be done at the very beginning. It is possible
         *      to switch renderers at any point, but the operation is rather costly: all
         *      sounds must be unloaded and recreated, all active sources must be detached and
         *      reattached.
         */
        virtual void setRenderer(SharedPtr<Renderer> renderer) throw(Exception);
        
        /** Get the current renderer */
        SharedPtr<Renderer> getRenderer() const throw();
        
        
        /** Get the maximum number of simultaneous sources that can be playing at a time
         * @remarks This value may be approximate, and it refers to the maximum number
         *      of Source class instances that can effectively be in playing state.
         *      Some sources, when environmental effects are being applied, can count
         *      more than once towards the effective limit, and the manager will try
         *      to compensate this when setting the "MaxSources" attribute. However,
         *      this compensation may not be perfect.
         */
        virtual unsigned int getMaxSources() const throw();
        
        /** Set the maximum number of simultaneous sources thta can be playing at a time
         * @param n The maximum number of simultaneous playing sources desired.
         * @remarks This is not guaranteed to success. If failure arises, either no change
         *      or a seemingly approximate change will be made (ie: if the specified number
         *      is too high, the closest possible one will be set instead).
         * @see getMaxSources
         */
        virtual void setMaxSources(unsigned int n) throw(Exception);
    
    protected:
        /** Add a new scene @see createScene */
        void addScene(SharedPtr<Scene> scene) throw(DuplicateObjectException);
    };

};

#endif//__AUDIO_SCENEMANAGER_H__INCLUDED__
