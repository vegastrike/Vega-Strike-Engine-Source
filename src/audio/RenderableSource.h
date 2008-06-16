//
// C++ Interface: Audio::RenderableSource
//
#ifndef __AUDIO_RENDERABLESOURCE_H__INCLUDED__
#define __AUDIO_RENDERABLESOURCE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"

namespace Audio {

    // Forward declarations
    
    class Source;
    

    /**
     * Renderable Source abstract class
     *
     * @remarks This is the interface to renderer-specific sources. All abstract sources
     *      must get attached to a renderable source. And each renderable source is attached to
     *      an abstract source. 
     *      @par All attributes come from the abstract source, so this class only has renderer-specific
     *      state and resources, plus implementation functions. Since it is intended to be attached
     *      to abstract sources as any user data would, it is a subclass of UserData.
     * @note Since this mutual attachment would create circular references,
     *      it is implemented on this side with raw pointers. No problem should arise since
     *      the smart pointer used in abstract sources already handles everything correctly,
     *      but care must be had not to have detached renderable sources around.
     *
     */
    class RenderableSource : public UserData
    {
    private:
        Source *source;
        
    protected:
        /** Internal constructor used by derived classes */
        RenderableSource(Source *source) throw();
        
    public:
        virtual ~RenderableSource();
        
        enum UpdateFlags {
            UPDATE_ALL,
            UPDATE_LOCATION,
            UPDATE_ATTRIBUTES,
            UPDATE_EFFECTS
        };
        
        /** Play the source from the specified timestamp 
         * @param start The starting position. Defaults to the beginning.
         * @remarks It just plays. Will not synchronize attributes with the underlying API.
         *      That must be done through a separate update() call.
         */
        void startPlaying(Timestamp start = 0) throw(Exception);
        
        /** Stop a playing source
         * @remarks If the source is playing, stop it. Otherwise, do nothing.
         */
        void stopPlaying() throw();
        
        /** Is the source still playing? */
        bool isPlaying() const throw();
        
        /** Get the playing position of a playing source 
         * @remarks Will throw if it's not playing!
         */
        Timestamp getPlayingTime() const throw(Exception);
        
        /** Get the attached source */
        Source* getSource() const throw() { return source; }
        
        /** Update the underlying API with dirty attributes 
         * @param flags You may specify which attributes to update. Not all attributes are
         *      equally costly, so you'll want to ease up on some, pump up some others.
         * @remarks Although the implementation may throw exceptions, the interface will
         *      ignore them (just log them or something like that). Updates are non-critical
         *      and may fail silently.
         */
        void update(UpdateFlags flags) throw();
        
        // The following section contains all the virtual functions that need be implemented
        // by a concrete Sound class. All are protected, so the interface is independent
        // of implementations.
    protected:
        
        /** @see startPlaying 
         * @param start The starting position.
         */
        virtual void startPlayingImpl(Timestamp start) throw(Exception) = 0;
        
        /** @see stopPlaying.*/
        virtual void stopPlayingImpl() throw(Exception) = 0;
        
        /** @see isPlaying.*/
        virtual bool isPlayingImpl() const throw(Exception) = 0;
        
        /** @see getPlayingTime.*/
        virtual Timestamp getPlayingTimeImpl() const throw(Exception) = 0;
        
        /** @see update. */
        virtual void updateImpl(UpdateFlags flags) throw(Exception) = 0;
    };

};

#endif//__AUDIO_RENDERABLESOURCE_H__INCLUDED__
