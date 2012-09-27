//
// C++ Interface: Audio::OpenALRenderableSource
//
#ifndef __AUDIO_OPENALRENDERABLESTREAMINGSOURCE_H__INCLUDED__
#define __AUDIO_OPENALRENDERABLESTREAMINGSOURCE_H__INCLUDED__

#include "al.h"

#include "../../RenderableSource.h"

#include "../../Exceptions.h"
#include "../../Types.h"

namespace Audio {

    /**
     * OpenAL Renderable Source class
     *
     * @remarks This class implements the RenderableSource interface for the
     *      OpenAL renderer.
     *
     */
    class OpenALRenderableStreamingSource : public RenderableSource
    {
        ALuint alSource;
        bool atEos;
        bool shouldPlay;
        bool startedPlaying;
        bool buffering;
        
    public:
        OpenALRenderableStreamingSource(Source *source);
        
        virtual ~OpenALRenderableStreamingSource();
        
        /** 
         * Returns whether this stream should be playing
         * @remarks Streaming sources may halt due to buffer underruns.
         *      By keeping track of the desired state, rather than querying
         *      the underlying AL source, we can overcome this problem.
         */
        bool shouldBePlaying() const { return shouldPlay && !atEos; }
        
    protected:
        /** @see RenderableSource::startPlayingImpl. */
        virtual void startPlayingImpl(Timestamp start) throw(Exception);
        
        /** @see RenderableSource::stopPlayingImpl. */
        virtual void stopPlayingImpl() throw(Exception);
        
        /** @see RenderableSource::isPlayingImpl. */
        virtual bool isPlayingImpl() const throw(Exception);
        
        /** @see RenderableSource::getPlayingTimeImpl. */
        virtual Timestamp getPlayingTimeImpl() const throw(Exception);
        
        /** @see RenderableSource::updateImpl. */
        virtual void updateImpl(int flags, const Listener& sceneListener) throw(Exception);
        
        /** @see RenderableSource::seekImpl. */
        virtual void seekImpl(Timestamp time) throw(Exception);
        
        /** Derived classes may use the underlying AL source handle to set additional attributes */
        ALuint getALSource() const { return alSource; }
        
        /** Queue AL buffers from the source's AL sound stream.
         * @note It will fail with an assertion if the attached sound isn't a streaming OpenAL sound
         * @note It will not throw an EndOfStream exception, even if the sound reaches the end 
         *      and it's not a looping source.
         */
        void queueALBuffers() throw(Exception);
    };

};

#endif//__AUDIO_OPENALRENDERABLESTREAMINGSOURCE_H__INCLUDED__
