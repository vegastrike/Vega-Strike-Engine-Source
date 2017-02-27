//
// C++ Interface: Audio::OpenALRenderableListener
//
#ifndef __AUDIO_OPENALRENDERABLELISTENER_H__INCLUDED__
#define __AUDIO_OPENALRENDERABLELISTENER_H__INCLUDED__

#include "../../RenderableListener.h"

#include "../../Exceptions.h"
#include "../../Types.h"

namespace Audio {

    /**
     * OpenAL Renderable Listener class
     *
     * @remarks This class implements the RenderableListener interface for the
     *      OpenAL renderer.
     *
     */
    class OpenALRenderableListener : public RenderableListener
    {
    public:
        OpenALRenderableListener(Listener *listener);
        
        virtual ~OpenALRenderableListener();
    
    protected:
        /** @see RenderableListener::update. */
        virtual void updateImpl(int flags) throw(Exception);
    };

};

#endif//__AUDIO_OPENALRENDERABLELISTENER_H__INCLUDED__
