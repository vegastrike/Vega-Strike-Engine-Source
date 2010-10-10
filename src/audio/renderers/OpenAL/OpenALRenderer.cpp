//
// C++ Implementation: Audio::Renderer
//

#include "OpenALRenderer.h"
#include "BorrowedOpenALRenderer.h"
#include "OpenALHelpers.h"
#include "config.h"

#include "al.h"

#include "../../Sound.h"
#include "../../Source.h"
#include "../../Listener.h"

#include "OpenALSimpleSound.h"
#include "OpenALStreamingSound.h"

#include "OpenALRenderableListener.h"
#include "OpenALRenderableSource.h"
#include "OpenALRenderableStreamingSource.h"

namespace Audio {

    namespace __impl {
    
        namespace OpenAL {
            
            struct RendererData
            {
                // The many required indexes
                ALCdevice *alDevice;
                ALCcontext *alContext;
                
                class SoundKey {
                    VSFileSystem::VSFileType type;
                    size_t nameLen;
                    std::string name;
                public:
                    SoundKey() : 
                        type(VSFileSystem::SoundFile), 
                        nameLen(0) 
                    {}
                    SoundKey(VSFileSystem::VSFileType _type, const std::string &_name) :
                        type(_type),
                        nameLen(_name.length()), 
                        name(_name)
                    {}
                    
                    bool isNull() const { return nameLen == 0; }
                    
                    bool operator<(const SoundKey &other) const
                    {
                        return (type < other.type) ||
                               ((type == other.type) && (
                                  (nameLen < other.nameLen) ||
                                  ((nameLen == other.nameLen) && (
                                      name < other.name
                                  ))
                               ));
                    }
                    
                    bool operator==(const SoundKey &other) const
                    {
                        return (type == other.type) &&
                               (nameLen == other.nameLen) &&
                               (name == other.name);
                    }
                };
                
                typedef std::map<SoundKey, SharedPtr<Sound> > SoundMap;
                typedef std::map<SharedPtr<Sound>, SoundKey > ReverseSoundMap;
                
                SoundMap loadedSounds;
                ReverseSoundMap loadedSoundsReverse;
                
                struct {
                    int meterDistance : 1;
                    int dopplerFactor : 1;
                } dirty;
                
                
                SharedPtr<Sound> lookupSound(VSFileSystem::VSFileType type, const std::string &name) const
                {
                    SoundKey key(type,name);
                    SoundMap::const_iterator it = loadedSounds.find(key);
                    if (it != loadedSounds.end())
                        return it->second;
                    else
                        return SharedPtr<Sound>();
                }
                
                SoundKey lookupSound(const SharedPtr<Sound> &sound) const
                {
                    ReverseSoundMap::const_iterator it = loadedSoundsReverse.find(sound);
                    if (it != loadedSoundsReverse.end())
                        return it->second;
                    else
                        return SoundKey();
                }
                
                void addSound(VSFileSystem::VSFileType type, const std::string &name, SharedPtr<Sound> sound)
                {
                    SoundKey key(type,name);
                    loadedSounds[key] = sound;
                    loadedSoundsReverse[sound] = key;
                }
                
                void unloadSound(const SharedPtr<Sound> &sound)
                {
                    SoundKey key = lookupSound(sound);
                    if (!key.isNull()) {
                        loadedSounds.erase(key);
                        loadedSoundsReverse.erase(sound);
                    }
                }
                
                void unloadSounds()
                {
                    for (SoundMap::iterator it=loadedSounds.begin(); it != loadedSounds.end(); ++it)
                        it->second->unload();
                }
                
                void openDevice(const char *deviceSpecifier)
                    throw (Exception)
                {
                    if (alDevice)
                        throw Exception("Trying to open a device without closing the previous one first");
                        
                    clearAlError();
                    
                    if (deviceSpecifier == NULL) {
                        #ifdef _WIN32
                            deviceSpecifier = "DirectSound3D";
                        #else
                            #ifdef __APPLE__
                                deviceSpecifier = "sdl";
                            #endif
                        #endif
                    }
                    
                    alDevice = alcOpenDevice((ALCstring)(deviceSpecifier));
                    
                    if (!alDevice)
                        checkAlError();
                    else
                        clearAlError();
                }
                
                void closeDevice()
                    throw (Exception)
                {
                    if (alContext)
                        throw Exception("Trying to close device without closing the previous one first");
                    if (alDevice) {
                        unloadSounds();
                        alcCloseDevice(alDevice);
                        alDevice = NULL;
                        clearAlError();
                    }
                }
                
                void openContext(const Format &format)
                    throw (Exception)
                {
                    if (alContext)
                        throw Exception("Trying to open context without closing the previous one first");
                    if (!alDevice)
                        throw Exception("Trying to open context without opening a device first");
                    
                    clearAlError();
                    
                    ALCint params[] = {
                        ALC_FREQUENCY, format.sampleFrequency,
                        0
                    };
                    
                    alContext = alcCreateContext(alDevice, params);
                    if (!alContext)
                        checkAlError();
                    else
                        clearAlError();
                    
                    alcMakeContextCurrent(alContext);
                    checkAlError();
                }
                
                void commit() 
                    throw (Exception)
                {
                    alcProcessContext(alContext);
                    checkAlError();
                }
                
                void suspend()
                    throw (Exception)
                {
                    // FIXME: There's a residual error here on Windows. Can't track down where it's from.
                    alGetError();
                    checkAlError();
                    alcMakeContextCurrent(alContext);
                    alcSuspendContext(alContext);
                    checkAlError();
                }
                
                void closeContext()
                    throw (Exception)
                {
                    if (alContext) {
                        alcMakeContextCurrent(NULL);
                        alcDestroyContext(alContext);
                        clearAlError();
                        alContext = NULL;
                    }
                }
                
                RendererData() throw() :
                    alDevice(NULL),
                    alContext(NULL)
                {
                }
                
                ~RendererData()
                {
                    unloadSounds();
                    closeContext();
                    closeDevice();
                }
            };
        };
    };

    using namespace __impl::OpenAL;

    OpenALRenderer::OpenALRenderer() 
        throw(Exception) :
        data(new RendererData)
    {
    }
    
    OpenALRenderer::~OpenALRenderer()
    {
    }
    
    SharedPtr<Sound> OpenALRenderer::getSound(
            const std::string &name, 
            VSFileSystem::VSFileType type, 
            bool streaming) 
        throw(Exception)
    {
        checkContext();
        SharedPtr<Sound> sound = data->lookupSound(type,name);
        if (!sound.get() || streaming) {
            if (streaming) {
                // Streaming sounds cannot be cached, so if a streaming sound
                // is in the cache, it must be evicted and re-created
                sound.reset();
                data->addSound(
                    type,
                    name,
                    sound = SharedPtr<Sound>(new OpenALStreamingSound(name,type))
                );
            } else {
                data->addSound(
                    type,
                    name,
                    sound = SharedPtr<Sound>(new OpenALSimpleSound(name,type))
                );
            }
        }
        return sound;
    }
    
    bool OpenALRenderer::owns(SharedPtr<Sound> sound)
    {
        return !data->lookupSound(sound).isNull();
    }
    
    void OpenALRenderer::attach(SharedPtr<Source> source) 
        throw(Exception)
    {
        checkContext();
        source->setRenderable( SharedPtr<RenderableSource>(
                source->getSound()->isStreaming() 
                    ? (RenderableSource*)new OpenALRenderableStreamingSource(source.get())
                    : (RenderableSource*)new OpenALRenderableSource(source.get())
            ) 
        );
    }
    
    void OpenALRenderer::attach(SharedPtr<Listener> listener) 
        throw(Exception)
    {
        checkContext();
        listener->setRenderable( SharedPtr<RenderableListener>(
            new OpenALRenderableListener(listener.get()) ) );
    }
    
    void OpenALRenderer::detach(SharedPtr<Source> source) 
        throw()
    {
        // Just clear it... RenderableListener's destructor will handle everything fine.
        source->setRenderable( SharedPtr<RenderableSource>() );
    }
    
    void OpenALRenderer::detach(SharedPtr<Listener> listener) 
        throw()
    {
        // Just clear it... RenderableListener's destructor will handle everything fine.
        listener->setRenderable( SharedPtr<RenderableListener>() );
    }
    
    void OpenALRenderer::setMeterDistance(Scalar distance) 
        throw()
    {
        // ToDo
        // Nothing yet - this is an extension to OpenAL 1.1's specs and in this phase
        // we'll implement only basic functionality.
        Renderer::setMeterDistance(distance);
        
        // meterDistance affects doppler settings (since it affects the speed of sound)
        data->dirty.dopplerFactor = 1;
        data->dirty.meterDistance = 1;
    }
    
    void OpenALRenderer::setDopplerFactor(Scalar factor) 
        throw()
    {
        Renderer::setDopplerFactor(factor);
        
        // Just flag it as dirty so that the next commit reconfigures the doppler effect.
        data->dirty.dopplerFactor = 1;
    }
    
    void OpenALRenderer::setOutputFormat(const Format &format) 
        throw(Exception)
    {
        if (!data->alDevice)
            data->openDevice(NULL);
        data->closeContext();
        data->openContext(format);
        Renderer::setOutputFormat(format);
    }
    
    void OpenALRenderer::checkContext()
        throw(Exception)
    {
        if (!data->alDevice)
            data->openDevice(NULL);
        if (!data->alContext) {
            data->openContext(getOutputFormat());
            initContext();
        }
    }
    
    void OpenALRenderer::beginTransaction() 
        throw(Exception)
    {
        data->suspend();
        
        if (data->dirty.dopplerFactor)
            setupDopplerEffect();
    }
    
    void OpenALRenderer::commitTransaction() 
        throw(Exception)
    {
        data->commit();
    }
    
    void OpenALRenderer::initContext()
        throw(Exception)
    {
        // Set the distance model
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
        
        // Flag everything as dirty
        data->dirty.meterDistance = 1;
        data->dirty.dopplerFactor = 1;
    }
    
    void OpenALRenderer::setupDopplerEffect()
        throw(Exception)
    {
        clearAlError();
        
        // First of all, compute the speed of sound (in world units)
        Scalar speedOfSound = 343.3f * getMeterDistance();
        
        // Set doppler factor and speed of sound
        alDopplerFactor(getDopplerFactor());
#ifdef _WIN32
        alDopplerVelocity(speedOfSound);
#else
        alSpeedOfSound(speedOfSound);
#endif
        
        data->dirty.dopplerFactor = 0;
        
        checkAlError();
    }
    
    BorrowedOpenALRenderer::BorrowedOpenALRenderer(ALCdevice *device, ALCcontext *context) 
        throw(Exception) :
        OpenALRenderer()
    {
        if (device) 
            data->alDevice = device;
        if (context)
            data->alContext = context;
        else
            data->alContext = alcGetCurrentContext();
        if (!device && data->alContext)
            data->alDevice = alcGetContextsDevice(data->alContext);
        
        initContext();
    }
    
    BorrowedOpenALRenderer::~BorrowedOpenALRenderer()
    {
        data->alDevice = NULL;
        data->alContext = NULL;
    }

    void BorrowedOpenALRenderer::setOutputFormat(const Format &format) 
        throw(Exception)
    {
        // No-op... format is given by the borrowed context
        Renderer::setOutputFormat(format);
    }
    
    void BorrowedOpenALRenderer::checkContext()
        throw(Exception)
    {
        // No-op... context has been borrowed
    }

    
};
