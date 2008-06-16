//
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_RENDERER_H__INCLUDED__
#define __AUDIO_RENDERER_H__INCLUDED__

#include <string>
#include <map>
#include <set>
#include "Exceptions.h"
#include "Singleton.h"
#include "Types.h"

#include "vsfilesystem.h"

namespace Audio {

    // Some forwards
    class Source;
    class Sound;
    class Listener;


    /**
     * Audio Renderer interface.
     *
     * @remarks Audio renderer implementations will implement this interface to communicate
     *      with SceneManager s.
     *      @par Renderers are also the factories for sound resources, since most APIs already
     *      handle sample storage and other sound resource aspects.
     *      @par The interface is rather simple since most operations are done through 
     *      RenderableSource proxies.
     *
     */
    class Renderer 
    {
    private:
        Scalar meterDistance;
    
    public:
        /** Initialize the renderer with default or config-driven settings.
         * @remarks End-users might want to use specific constructors of specific renderers.
         */
        Renderer() throw(Exception);
        
        virtual ~Renderer();
        
        /** Create a sound, from the stream of the specified name.
         * @param name The path of the soundfile.
         * @param type The file type (needed by the filesystem).
         * @param straeming If true, requests for a streaming sound - one that is not loaded to
         *      memory for playback, but rather read on-demand during playback.
         * @remarks The name is the path of a soundstream as it would be passed to the CodecRegistry.
         *      @par Streaming sounds can also be created, which depending on the renderer implementation
         *      may or may not result in a different kind of sound resource. Most notably, the difference
         *      may depend on runtime-dependant state. For instance, it is quite reasonable that a
         *      streaming sound won't be created if the same sound file exists in a non-streaming
         *      form.
         * @see CodecRegistry
         */
        virtual SharedPtr<Sound> getSound(
            const std::string &name, 
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile, 
            bool streaming = false) throw(Exception) = 0;
        
        /** Attach a source to this renderer
         * @remarks A source may only be attached to one renderer. If the source was attached already,
         *      an exception will be thrown.
         *      @par Attachment may mean resource allocation. Either immediate or deferred. So it may
         *      fail if resources are scarce.
         */
        virtual void attach(SharedPtr<Source> source) throw(Exception) = 0;
        
        /** Attach a listener to this renderer
         * @remarks A listener may only be attached to one renderer. If the listener was attached already,
         *      an exception will be thrown.
         *      @par Attachment may mean resource allocation. Either immediate or deferred. So it may
         *      fail if resources are scarce.
         */
        virtual void attach(SharedPtr<Listener> listener) throw(Exception) = 0;
        
        /** Detach a source from this renderer.
         * @remarks Immediately frees any allocated resources.
         */
        virtual void detach(SharedPtr<Source> source) throw() = 0;
        
        /** Detach a listener from this renderer.
         * @remarks Immediately frees any allocated resources.
         */
        virtual void detach(SharedPtr<Listener> source) throw() = 0;
        
        
        
        /** Sets the distance in world units that represents one meter.
         * @remarks This reference distance is required by environmental effect processing
         *      to accurately account for distance factors beyond simple gain falloff.
         */
        virtual void setMeterDistance(Scalar distance) throw();
        
        /** Gets the distance in world units that represents one meter.
         * @see setMeterDistance
         */
        virtual Scalar getMeterDistance() const throw();
        
    };

};

#endif//__AUDIO_RENDERER_H__INCLUDED__
