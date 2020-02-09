//
// C++ Interface: Audio::RenderableListener
//
#ifndef __AUDIO_RENDERABLELISTENER_H__INCLUDED__
#define __AUDIO_RENDERABLELISTENER_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"

namespace Audio {

    // Forward declarations
    
    class Listener;
    

    /**
     * Renderable Listener abstract class
     *
     * @remarks This is the interface to renderer-specific listeners. 
     *      Listeners attached to a renderer receive one such instance
     *      that allows listener-specific commands to be given to the
     *      renderer, like requesting position updates and such.
     * @note Since this mutual attachment would create circular references,
     *      it is implemented on this side with raw pointers. No problem should arise since
     *      the smart pointer used in abstract listener already handles everything correctly,
     *      but care must be had not to have detached renderable listeners around.
     *
     */
    class RenderableListener : public UserData
    {
    private:
        Listener *listener;
        
    protected:
        /** Internal constructor used by derived classes */
        RenderableListener(Listener *listener) throw();
        
    public:
        virtual ~RenderableListener();
        
        enum UpdateFlags {
            UPDATE_ALL          = 0x0F,
            UPDATE_LOCATION     = 0x01,
            UPDATE_ATTRIBUTES   = 0x02,
            UPDATE_EFFECTS      = 0x04,
            UPDATE_GAIN         = 0x08
        };
        
        /** Get the attached listener */
        Listener* getListener() const throw() { return listener; }
        
        /** Update the underlying API with dirty attributes 
         * @param flags You may specify which attributes to update. Not all attributes are
         *      equally costly, so you'll want to ease up on some, pump up some others.
         *      You can or-combine flags.
         * @remarks Although the implementation may throw exceptions, the interface will
         *      ignore them (just log them or something like that). Updates are non-critical
         *      and may fail silently.
         */
        void update(int flags) throw();
        
        // The following section contains all the virtual functions that need be implemented
        // by a concrete class. All are protected, so the interface is independent
        // of implementations.
    protected:
        
        /** @see update. */
        virtual void updateImpl(int flags) throw(Exception) = 0;
    };

};

#endif//__AUDIO_RENDERABLELISTENER_H__INCLUDED__
