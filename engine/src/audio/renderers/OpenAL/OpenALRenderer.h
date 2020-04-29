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
        OpenALRenderer();
        
        virtual ~OpenALRenderer();
        
        /** @copydoc Renderer::getSound */
        virtual SharedPtr<Sound> getSound(
            const std::string &name, 
            VSFileSystem::VSFileType type = VSFileSystem::UnknownFile, 
            bool streaming = false);
        
        /** @copydoc Renderer::owns */
        virtual bool owns(SharedPtr<Sound> sound);
        
        /** @copydoc Renderer::attach(SharedPtr<Source>) */
        virtual void attach(SharedPtr<Source> source);
        
        /** @copydoc Renderer::attach(SharedPtr<Listener>) */
        virtual void attach(SharedPtr<Listener> listener);
        
        /** @copydoc Renderer::detach(SharedPtr<Source>) */
        virtual void detach(SharedPtr<Source> source);
        
        /** @copydoc Renderer::detach(SharedPtr<Listener>) */
        virtual void detach(SharedPtr<Listener> listener);
        
        /** @copydoc Renderer::setMeterDistance */
        virtual void setMeterDistance(Scalar distance);
        
        /** @copydoc Renderer::setDopplerFactor */
        virtual void setDopplerFactor(Scalar factor);
        
        /** @copydoc Renderer::setOutputFormat */
        virtual void setOutputFormat(const Format &format);
        
        /** @copydoc Renderer::beginTransaction */
        virtual void beginTransaction();
        
        /** @copydoc Renderer::commitTransaction */
        virtual void commitTransaction();
    protected:
    
        /** Makes sure the AL context is valid, creating one if necessary */
        virtual void checkContext();
        
        /** Sets expected defaults into the context */
        virtual void initContext();
        
        /** Sets doppler effect globals into the context */
        void setupDopplerEffect();
    };
    
};


#endif//__AUDIO_OPENAL_RENDERER_H__INCLUDED__
