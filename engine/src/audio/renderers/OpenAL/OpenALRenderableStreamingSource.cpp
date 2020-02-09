//
// C++ Implementation: Audio::OpenALRenderableListener
//
#include "OpenALRenderableStreamingSource.h"
#include "OpenALStreamingSound.h"
#include "OpenALHelpers.h"
#include "config.h"

#include "al.h"

#include "../../Source.h"
#include "../../Listener.h"

#include "vs_math.h"

namespace Audio {

    static inline void alSource3f(ALuint source, ALenum param, const Vector3 &v)
    {
        ::alSource3f(source, param, ALfloat(v.x), ALfloat(v.y), ALfloat(v.z));
    }

    static inline void alSource3f(ALuint source, ALenum param, const LVector3 &v)
    {
        ::alSource3f(source, param, ALfloat(v.x), ALfloat(v.y), ALfloat(v.z));
    }

    OpenALRenderableStreamingSource::OpenALRenderableStreamingSource(Source *source)
        : RenderableSource(source)
        , alSource(0)
        , atEos(false)
        , shouldPlay(false)
        , startedPlaying(false)
        , buffering(false)
    {
        alGenSources(1,&alSource);
    }
    
    OpenALRenderableStreamingSource::~OpenALRenderableStreamingSource()
    {
        alDeleteSources(1,&alSource);
    }
    
    void OpenALRenderableStreamingSource::startPlayingImpl(Timestamp start) 
        throw(Exception)
    {
        if (!isPlayingImpl()) {
            SharedPtr<Sound> sound = getSource()->getSound();
            
            assert(sound->isStreaming() && "OpenALRenderableStreamingSource can only handle streaming sounds");
            
            if (!sound->isLoaded())
                sound->load();
            else if (!buffering)
                dynamic_cast<OpenALStreamingSound*>(sound.get())->flushBuffers();
            
            // Seek the stream to the specified position
            atEos = false;
            shouldPlay = true;
            dynamic_cast<OpenALStreamingSound*>(sound.get())->seek(start);
            
            // Make sure we have some starting buffers queued
            queueALBuffers();
            
            // Tell the AL to start playing
            clearAlError();
            ALuint als = getALSource();
            alSourcePlay(als);
            startedPlaying = true;
            checkAlError();
        }
    }
    
    void OpenALRenderableStreamingSource::stopPlayingImpl() 
        throw(Exception)
    {
        shouldPlay = false;
        startedPlaying = false;
        buffering = false;
        alSourceStop(alSource);
    }
    
    bool OpenALRenderableStreamingSource::isPlayingImpl() const 
        throw(Exception)
    {
        // According to the AL, streaming sounds can cease to be 
        // in the playing state because of buffer starvation. However,
        // we want to consider them still playing, so if we haven't
        // reached EOS, they're playing from the framework's POV
        // (ie: just attaching more buffers make them play)
        if (startedPlaying && !atEos)
            return true;
        
        ALint state = 0;
        alGetSourcei(getALSource(), AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }
    
    Timestamp OpenALRenderableStreamingSource::getPlayingTimeImpl() const 
        throw(Exception)
    {
        ALfloat offs = -1.f;
        alGetSourcef(getALSource(), AL_SEC_OFFSET, &offs);
        
        if (offs < 0.f)
            throw NotImplementedException("getPlayingTimeImpl");
        
        Timestamp base = dynamic_cast<OpenALStreamingSound*>(getSource()->getSound().get())
            ->getTimeBase();
        
        return Timestamp(offs) + base;
    }
    
    void OpenALRenderableStreamingSource::seekImpl(Timestamp time) 
        throw(Exception)
    {
        // Seek the stream to the specified position
        atEos = false;
        dynamic_cast<OpenALStreamingSound*>(getSource()->getSound().get())
            ->seek(time);
    }
    
    void OpenALRenderableStreamingSource::updateImpl(int flags, const Listener& sceneListener) 
        throw(Exception)
    {
        Source *source = getSource();
        ALSourceHandle als = getALSource();
        
        // Restart playing in case of a buffer underrun,
        // else just fill buffers
        if (shouldBePlaying() && !isPlayingImpl()) {
            // startPlayingImpl(source->getWouldbePlayingTime());
            // NOTE: Cannot use startPlaying because it stresses the buggy seek method
            //  Must fix that
            
            SharedPtr<Sound> sound = source->getSound();
            
            if (!sound->isLoaded())
                sound->load();
            else if (!buffering)
                dynamic_cast<OpenALStreamingSound*>(sound.get())->flushBuffers();
            
            // Make sure we have some starting buffers queued
            queueALBuffers();
            
            // Tell the AL to start playing
            clearAlError();
            ALuint als = getALSource();
            alSourcePlay(als);
            checkAlError();
            
            startedPlaying = true;
        } else {
            queueALBuffers();
        }
        
        // Update various attributes if required
        clearAlError();
        
        if (flags & UPDATE_ATTRIBUTES) {
            // Distance attenuation
            if (source->isAttenuated()) {
                alSourcef(als, AL_REFERENCE_DISTANCE, source->getRadius());
                alSourcef(als, AL_ROLLOFF_FACTOR, 1.f / source->getRadius());
            } else {
                alSourcef(als, AL_ROLLOFF_FACTOR, 0.f);
            }
            // Cone
            {
                Range<Scalar> angleRange = source->getAngleRange();
                alSourcef(als, AL_CONE_INNER_ANGLE, float(angleRange.min) * M_1_PI * 360.f);
                alSourcef(als, AL_CONE_OUTER_ANGLE, float(angleRange.max) * M_1_PI * 360.f);
                alSourcef(als, AL_CONE_OUTER_GAIN , 0.f);
            }
            // Relativity
            alSourcei(als, AL_SOURCE_RELATIVE, source->isRelative() ? AL_TRUE : AL_FALSE);
            // Looping
            alSourcei(als, AL_LOOPING, source->isLooping() ? AL_TRUE : AL_FALSE);
        }
        if (flags & UPDATE_GAIN) {
            // Gain
            alSourcef(als, AL_GAIN, source->getGain());
        }
        if (flags & UPDATE_LOCATION) {
            if (source->isRelative()) {
                alSource3f(als, AL_POSITION, source->getPosition());
                alSource3f(als, AL_VELOCITY, source->getVelocity());
                alSource3f(als, AL_DIRECTION, source->getDirection());
            } else {
                alSource3f(als, AL_POSITION, 
                    source->getPosition() - sceneListener.getPosition() );
                alSource3f(als, AL_VELOCITY, 
                    sceneListener.toLocalDirection(
                        source->getVelocity() - sceneListener.getVelocity()
                    ) );
                alSource3f(als, AL_DIRECTION, 
                    sceneListener.toLocalDirection(
                        source->getDirection()
                    ) );
            }
        }
        
        checkAlError();
    }
    
    void OpenALRenderableStreamingSource::queueALBuffers()
        throw(Exception)
    {
        SharedPtr<Sound> sound = getSource()->getSound();
        
        if (!sound->isLoaded())
            sound->load();
        
        assert(sound->isStreaming() && "OpenALRenderableStreamingSource can only handle streaming sounds");

        buffering = true;
        
        OpenALStreamingSound *streamingSound = dynamic_cast<OpenALStreamingSound*>(sound.get());
        Source *source = getSource();
        ALSourceHandle als = getALSource();
        ALint buffersProcessed = 0;
        
        // Unqueue any buffers the AL is done with, 
        // returning them to the streaming sound
        alGetSourcei(als, AL_BUFFERS_PROCESSED, &buffersProcessed);
        
        while (buffersProcessed > 0) {
            ALBufferHandle buffers[2];
            ALsizei nbuffers = (buffersProcessed > 2) ? 2 : (ALsizei)buffersProcessed;
            alSourceUnqueueBuffers(als, nbuffers, buffers);
            
            for (ALsizei i=0; i<nbuffers; ++i)
                streamingSound->unqueueBuffer(buffers[i]);
            
            buffersProcessed -= nbuffers;
        }
        
        // Get buffers from the streaming sound and queue them 
        // until the streaming sound says Basta!
        ALBufferHandle buffer;
        do {
            try {
                buffer = streamingSound->readAndFlip();
            } catch(EndOfStreamException e) {
                fprintf(stderr, "EOS!\n");
                if (source->isLooping()) {
                    streamingSound->seek(0);
                    buffer = streamingSound->readAndFlip();
                } else {
                    atEos = true;
                    buffer = AL_NULL_BUFFER;
                }
            }
            if (buffer != AL_NULL_BUFFER)
                alSourceQueueBuffers(als, 1, &buffer);
        } while (buffer != AL_NULL_BUFFER);
    }

};
