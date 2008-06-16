//
// C++ Interface: Audio::Source
//
#ifndef __AUDIO_SOURCE_H__INCLUDED__
#define __AUDIO_SOURCE_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"
#include "RenderableSource.h"

namespace Audio {

    // Forward declarations
    
    class Sound;
    

    /**
     * Source abstract class
     *
     * @remarks This is the interface to all kinds of audio sources.
     *      This base class implements the majority of the functions required
     *      for sources, specific implementations need only provide a handful
     *      of functions (impl functions) that keep their associated scene managers
     *      up to speed.
     *      @par Renderer-specific implementation goes through RenderableSource s
     *      attached to the abstract Source. Renderers are responsible for attaching
     *      RenderableSource s to abstract Sources as necessary. Many implementation
     *      functions (as play/stop/getPlayingTime) go through those renderer-specific 
     *      interfaces.
     *
     */
    class Source
    {
    private:
        SharedPtr<Sound> soundPtr;
        SharedPtr<RenderableSource> rendererDataPtr;
        SharedPtr<UserData> userDataPtr;
        
    protected:
        LVector3 position;
        Vector3 direction;
        Vector3 velocity;
        
        Range<Scalar> cosAngleRange;
        
        Scalar radius;
        PerFrequency<Scalar> pfRadiusRatios;
        PerFrequency<Scalar> referenceFreqs;
        
        Scalar gain;
        
        bool looping;
        
        Timestamp lastKnownPlayingTime;
        Timestamp lastKnownPlayingTimeTime;
        
        /** Notify implementations after position and/or attributes changes
         * @remarks Implementations use the "dirty" member struct to precisely know and 
         *      track what changed and what didn't. Call reset() when synchronized.
         */
        struct {
            /** position, velocity & direction */
            int location : 1;
            
            /** min/max angle, radius, pf radius ratios, reference freqs */
            int attributes : 1;
            
            /** gain */
            int gain : 1;
            
            /** soundPtr */
            int soundPtr : 1;
            
            /** looping */
            int soundAttributes : 1;
            
            void reset()
            {
                location = 0;
                attributes = 0;
                gain = 0;
                soundPtr = 0;
                soundAttributes = 0;
            }
        } dirty;
        
    protected:
        /** Internal constructor used by derived classes */
        Source(SharedPtr<Sound> sound, bool looping = false) throw();
        
    public:
        virtual ~Source();
        
        /** Return the source's central position in 3D space */
        LVector3 getPosition() const throw() { return position; }
        
        /** Set the source's central position in 3D space */
        void setPosition(LVector3 x) throw() { position = x; dirty.location = 1; }
        
        /** Return the source's main propagation direction */
        Vector3 getDirection() const throw() { return direction; }
        
        /** Set the source's main propagation direction */
        void setDirection(Vector3 x) throw() { direction = x; dirty.location = 1; }
        
        /** Return the source's velocity */
        Vector3 getVelocity() const throw() { return velocity; }
        
        /** Set the source's velocity */
        void setVelocity(Vector3 x) throw() { velocity = x; dirty.location = 1; }
        
        /** Return the source's minimum/maximum propagation angle
         * @remarks Sound will fully propagate in directions within minimum
         *      directional drift. Further drift will attenuate the sound
         *      until it (practically) disappears by the maximum propagation angle.
         */
        Range<Scalar> getAngleRange() const throw();
        
        /** @see getAngleRange */
        void setAngleRange(Range<Scalar> r) throw();
        
        /** @see getAngleRange @remarks This version returns cosine-angles rather than radians, much quicker */
        Range<Scalar> getCosAngleRange() const throw() { return cosAngleRange; }
        
        /** @see getAngleRange @remarks This version takes cosine-angles rather than radians, much quicker */
        void setCosAngleRange(Range<Scalar> r) throw() { cosAngleRange = r; dirty.attributes = 1; }
        
        /** Get the source's radius */
        Scalar getRadius() const throw() { return radius; }
        
        /** Set the source's radius */
        void setRadius(Scalar r) throw() { radius = r; dirty.attributes = 1; }
        
        /** Get the source's frequency-dependant radius ratios 
         * @remarks Sound propagation goes different for low and high frequencies than
         *      for others. Sound creation does too - this specifies the ratio of a source's
         *      volume that is generating high/low frequency vibrations. This will affect
         *      propagation of those frequencies over distance.
         */
        PerFrequency<Scalar> getPerFrequencyRadiusRatios() const throw() { return pfRadiusRatios; }
        
        /** Set the source's frequency-dependant radius ratios
         * @see getRadiusRatios 
         */
        void setPerFrequencyRadiusRatios(PerFrequency<Scalar> val) throw() { pfRadiusRatios = val; dirty.attributes = 1; }
        
        /** Get the source's refernece frequencies */
        PerFrequency<Scalar> getReferenceFreqs() const throw() { return referenceFreqs; }
        
        /** Set the source's reference frequencies */
        void setReferenceFreqs(PerFrequency<Scalar> val) throw() { referenceFreqs = val; dirty.attributes = 1; }
        
        /** Get the source's main gain */
        Scalar getGain() const throw() { return gain; }
        
        /** Set the source's main gain */
        void setGain(Scalar g) throw() { gain = g; dirty.gain = 1; }
        
        /** Is the source in looping mode? */
        bool isLooping() const throw() { return looping; }
        
        /** Set the source's looping mode */
        void setLooping(bool loop) throw() { looping = loop; dirty.soundAttributes = 1; }
        
        /** Play the source
         * @remarks If the source isn't playing, rewind and play from the beginning.
         *      May not take effect immediately.
         */
        void startPlaying() throw(Exception);
        
        /** Stop a playing source
         * @remarks If the source is playing, stop it. Otherwise, do nothing.
         *      @par Remembers the playing time so that a call to getWouldbePlayingTime can
         *      return the correct time.
         */
        void stopPlaying() throw();
        
        /** Pause a playing source
         * @remarks If the source is active, make it inactive. Otherwise, do nothing.
         *      Playing state is not changed, but the attached renderable is stopped. This is useful
         *      to free up resources. A later call to continuePlaying() undoes this, and playing resumes
         *      as if the source hadn't been stopped (at getWouldBePlayingTime).
         * @see getWouldBePlayingTime
         */
        void pausePlaying() throw();
        
        /** Continue playing a source
         * @remarks If the source is playing but inactive, continue playing. Otherwise, do nothing.
         * @see pausePlaying
         */
        void continuePlaying() throw(Exception);
        
        /** Is the source still playing? */
        bool isPlaying() const throw();
        
        /** Is the attached renderable playing this source? */
        bool isActive() const throw();
        
        /** Get the playing position of a playing or paused source */
        Timestamp getPlayingTime() const throw();
        
        /** Get the playing position of a playing or paused source. For a paused source, extrapolate
         * from the last known playing time and elapsed time since the measurement was done.
         */
        Timestamp getWouldbePlayingTime() const throw();
        
        /** Get renderer-specific data associated (and destroyed) with this sound source */
        SharedPtr<RenderableSource> getRendererData() const throw() { return rendererDataPtr; }
        
        /** Set renderer-specific data to be associated (and destroyed) with this sound source */
        void setRendererData(SharedPtr<RenderableSource> ptr) throw() { rendererDataPtr = ptr; }
        
        /** Get user-specific data associated (and destroyed) with this sound source */
        SharedPtr<UserData> getUserData() const throw() { return userDataPtr; }
        
        /** Set user-specific data to be associated (and destroyed) with this sound source */
        void setUserData(SharedPtr<UserData> ptr) throw() { userDataPtr = ptr; }
        
        /** Get the associated sound stream */
        SharedPtr<Sound> getSound() const throw() { return soundPtr; }
        
        /** Set the associated sound stream - Only for SceneManagers to call */
        void setSound(SharedPtr<Sound> ptr) throw() { soundPtr = ptr; dirty.soundPtr = 1; }
        
        /** Convenience function to update the attached renderable, if present, and active. */
        void updateRenderable(RenderableSource::UpdateFlags flags) const throw();
        
    protected:
        /** Set the last known playing time, update the measurement timestamp as well.
         * @return the parameter, for chaining.
         */
        Timestamp setLastKnownPlayingTime(Timestamp timestamp) throw();
        
        // The following section contains all the virtual functions that need be implemented
        // by a concrete Sound class. All are protected, so the stream interface is independent
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
    };

};

#endif//__AUDIO_SOURCE_H__INCLUDED__
