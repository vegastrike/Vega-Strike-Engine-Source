//
// C++ Interface: Audio::SceneManager
//
#ifndef __AUDIO_OPENAL_RENDERER_H__INCLUDED__
#define __AUDIO_OPENAL_RENDERER_H__INCLUDED__

#include "../../Exceptions.h"
#include "../../Types.h"
#include "../../Renderer.h"
#include "../../Format.h"

namespace Audio {

    namespace __impl {
        
        namespace OpenAL {
            // Forward declaration of internal renderer data
            struct RendererData;
        };
        
    };

    /**
     * OpenAL Renderer implementation
     *
     * @remarks Audio renderer implementation based on OpenAL.
     *
     */
    class OpenALRenderer : public Renderer
    {
    protected:
        AutoPtr<__impl::OpenAL::RendererData> data;
        
    public:
        /** Initialize the renderer with default or config-driven settings. */
        OpenALRenderer() throw(Exception);
        
        virtual ~OpenALRenderer();
        
        /** @copydoc Renderer::getSound */
        virtual SharedPtr<Sound> getSound(
            const std::string &name, 
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile, 
            bool streaming = false) throw(Exception);
        
        /** @copydoc Renderer::owns */
        virtual bool owns(SharedPtr<Sound> sound);
        
        /** @copydoc Renderer::attach(SharedPtr<Source>) */
        virtual void attach(SharedPtr<Source> source) throw(Exception);
        
        /** @copydoc Renderer::attach(SharedPtr<Listener>) */
        virtual void attach(SharedPtr<Listener> listener) throw(Exception);
        
        /** @copydoc Renderer::detach(SharedPtr<Source>) */
        virtual void detach(SharedPtr<Source> source) throw();
        
        /** @copydoc Renderer::detach(SharedPtr<Listener>) */
        virtual void detach(SharedPtr<Listener> listener) throw();
        
        /** @copydoc Renderer::setMeterDistance */
        virtual void setMeterDistance(Scalar distance) throw();
        
        /** @copydoc Renderer::setDopplerFactor */
        virtual void setDopplerFactor(Scalar factor) throw();
        
        /** @copydoc Renderer::setOutputFormat */
        virtual void setOutputFormat(const Format &format) throw(Exception);
        
        /** @copydoc Renderer::beginTransaction */
        virtual void beginTransaction() throw(Exception);
        
        /** @copydoc Renderer::commitTransaction */
        virtual void commitTransaction() throw(Exception);
    protected:
    
        /** Makes sure the AL context is valid, creating one if necessary */
        virtual void checkContext() throw(Exception);
        
        /** Sets expected defaults into the context */
        virtual void initContext() throw(Exception);
        
        /** Sets doppler effect globals into the context */
        void setupDopplerEffect() throw(Exception);
    };
    
};


#endif//__AUDIO_OPENAL_RENDERER_H__INCLUDED__
