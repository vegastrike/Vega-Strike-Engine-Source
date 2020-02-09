//
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_BORROWED_OPENAL_RENDERER_H__INCLUDED__
#define __AUDIO_BORROWED_OPENAL_RENDERER_H__INCLUDED__

#include "OpenALRenderer.h"
#include "al.h"

namespace Audio {

    /**
     * Hooks up the renderer to an already-existing OpenAL context
     *
     * @remarks If you're already using OpenAL in a standard fashion, you can
     *      still benefit from the scene graph provided by the Audio library
     *      by initializing the OpenAL renderer through this method.
     *          The default constructor will assume the current OpenAL context 
     *      has been previously initialized and hook up to that context.
     */
    class BorrowedOpenALRenderer : public OpenALRenderer
    {
    public:
        /**
         * Hooks up the renderer to an already-existing OpenAL context
         *
         * @remarks If you're already using OpenAL in a standard fashion, you can
         *      still benefit from the scene graph provided by the Audio library
         *      by initializing the OpenAL renderer through this method.
         *          If any parameter was passed as null, it will be retrieved
         *      from the current OpenAL context (which must have been previously
         *      initialized)
         *
         * @param device The OpenAL device associated to this context
         * @param context The OpenAL context associated to this renderer
         */
        BorrowedOpenALRenderer(ALCdevice *device = 0, ALCcontext *context = 0) throw(Exception);
        
        ~BorrowedOpenALRenderer();
        
        
        virtual void setOutputFormat(const Format &format) throw(Exception);
        
    protected:
        virtual void checkContext() throw(Exception);
    };

};


#endif//__AUDIO_BORROWED_OPENAL_RENDERER_H__INCLUDED__
