/*
 * Source.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_AUDIO_SOURCE_H
#define VEGA_STRIKE_ENGINE_AUDIO_SOURCE_H

//
// C++ Interface: Audio::Source
//

#include "Exceptions.h"
#include "Types.h"
#include "Format.h"
#include "RenderableSource.h"

namespace Audio {

// Forward declarations

class Sound;
class Listener;
class SourceListener;

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
class Source {
private:
    SharedPtr<Sound> soundPtr;
    SharedPtr<RenderableSource> rendererDataPtr;
    SharedPtr<UserData> userDataPtr;
    long userDataLong;
    SharedPtr<SourceListener> sourceListenerPtr;

protected:
    LVector3 position;
    Vector3 direction;
    Vector3 velocity;

    Range<Scalar> cosAngleRange;

    Scalar radius;
    PerFrequency<Scalar> pfRadiusRatios;
    PerFrequency<Scalar> referenceFreqs;

    Scalar gain;

    struct {
        int looping: 1;
        int attenuated: 1;
        int relative: 1;
    } flags;

    Timestamp lastKnownPlayingTime;
    Timestamp lastKnownPlayingTimeTime;

    /** Notify implementations after position and/or attributes changes
     * @remarks Implementations use the "dirty" member struct to precisely know and
     *      track what changed and what didn't. Call reset() when synchronized.
     */
    struct {
        /** position, velocity & direction */
        int location: 1;

        /** min/max angle, radius, pf radius ratios, reference freqs, attenuation */
        int attributes: 1;

        /** gain */
        int gain: 1;

        /** soundPtr */
        int soundPtr: 1;

        /** looping */
        int soundAttributes: 1;

        void reset() {
            location = 0;
            attributes = 0;
            gain = 0;
            soundPtr = 0;
            soundAttributes = 0;
        }

        void setAll() {
            location = 1;
            attributes = 1;
            gain = 1;
            soundAttributes = 1;
        }
    } dirty;

protected:
    /** Internal constructor used by derived classes */
    Source(SharedPtr<Sound> sound, bool looping = false);

public:
    virtual ~Source();

    /** Return the source's central position in 3D space */
    LVector3 getPosition() const {
        return position;
    }

    /** Set the source's central position in 3D space */
    void setPosition(LVector3 x) {
        position = x;
        dirty.location = 1;
    }

    /** Return the source's main propagation direction */
    Vector3 getDirection() const {
        return direction;
    }

    /** Set the source's main propagation direction */
    void setDirection(Vector3 x) {
        direction = x;
        dirty.location = 1;
    }

    /** Return the source's velocity */
    Vector3 getVelocity() const {
        return velocity;
    }

    /** Set the source's velocity */
    void setVelocity(Vector3 x) {
        velocity = x;
        dirty.location = 1;
    }

    /** Return the source's minimum/maximum propagation angle
     * @remarks Sound will fully propagate in directions within minimum
     *      directional drift. Further drift will attenuate the sound
     *      until it (practically) disappears by the maximum propagation angle.
     */
    Range<Scalar> getAngleRange() const;

    /** @see getAngleRange */
    void setAngleRange(Range<Scalar> r);

    /** @see getAngleRange @remarks This version returns cosine-angles rather than radians, much quicker */
    Range<Scalar> getCosAngleRange() const {
        return cosAngleRange;
    }

    /** @see getAngleRange @remarks This version takes cosine-angles rather than radians, much quicker */
    void setCosAngleRange(Range<Scalar> r) {
        cosAngleRange = r;
        dirty.attributes = 1;
    }

    /** Get the source's radius */
    Scalar getRadius() const {
        return radius;
    }

    /** Set the source's radius */
    void setRadius(Scalar r) {
        radius = r;
        dirty.attributes = 1;
    }

    /** Get the source's frequency-dependant radius ratios
     * @remarks Sound propagation goes different for low and high frequencies than
     *      for others. Sound creation does too - this specifies the ratio of a source's
     *      volume that is generating high/low frequency vibrations. This will affect
     *      propagation of those frequencies over distance.
     */
    PerFrequency<Scalar> getPerFrequencyRadiusRatios() const {
        return pfRadiusRatios;
    }

    /** Set the source's frequency-dependant radius ratios
     * @see getRadiusRatios
     */
    void setPerFrequencyRadiusRatios(PerFrequency<Scalar> val) {
        pfRadiusRatios = val;
        dirty.attributes = 1;
    }

    /** Get the source's refernece frequencies */
    PerFrequency<Scalar> getReferenceFreqs() const {
        return referenceFreqs;
    }

    /** Set the source's reference frequencies */
    void setReferenceFreqs(PerFrequency<Scalar> val) {
        referenceFreqs = val;
        dirty.attributes = 1;
    }

    /** Get the source's main gain */
    Scalar getGain() const {
        return gain;
    }

    /** Set the source's main gain */
    void setGain(Scalar g) {
        gain = g;
        dirty.gain = 1;
    }

    /** Is the source in looping mode? */
    bool isLooping() const {
        return flags.looping != 0;
    }

    /** Set the source's looping mode */
    void setLooping(bool loop) {
        flags.looping = loop ? 1 : 0;
        dirty.soundAttributes = 1;
    }

    /** Is the source using distance attenuation? */
    bool isAttenuated() const {
        return flags.attenuated != 0;
    }

    /** Set whether the source uses distance attenuation */
    void setAttenuated(bool attenuated) {
        flags.attenuated = attenuated ? 1 : 0;
        dirty.attributes = 1;
    }

    /** Is the source's position always relative to the root listener?
     * @remarks Relative sources are useful for interphase sounds, music,
     *      comm streams, and all those sources which are not anchored
     *      to a real 3D object, but rather to the listener itself.
     */
    bool isRelative() const {
        return flags.relative != 0;
    }

    /** Set whether the source's position is always relative to the root listener. */
    void setRelative(bool relative) {
        flags.relative = relative ? 1 : 0;
        dirty.attributes = 1;
    }

    /** Play the source
     * @param start an optional timestamp to start playing from.
     * @remarks Rewind and play from the beginning. If the source is playing, it is reset.
     *      May not take effect immediately.
     */
    void startPlaying(Timestamp start = 0);

    /** Stop a playing source
     * @remarks If the source is playing, stop it. Otherwise, do nothing.
     *      @par Remembers the playing time so that a call to getWouldbePlayingTime can
     *      return the correct time.
     */
    void stopPlaying();

    /** Pause a playing source
     * @remarks If the source is active, make it inactive. Otherwise, do nothing.
     *      Playing state is not changed, but the attached renderable is stopped. This is useful
     *      to free up resources. A later call to continuePlaying() undoes this, and playing resumes
     *      as if the source hadn't been stopped (at getWouldBePlayingTime).
     * @see getWouldBePlayingTime
     */
    void pausePlaying();

    /** Continue playing a source
     * @remarks If the source is playing but inactive, continue playing. Otherwise, do nothing.
     * @see pausePlaying
     */
    void continuePlaying();

    /** Is the source still playing? */
    bool isPlaying() const;

    /** Is the attached renderable playing this source? */
    bool isActive() const;

    /** Get the playing position of a playing or paused source */
    Timestamp getPlayingTime() const;

    /** Get the playing position of a playing or paused source. For a paused source, extrapolate
     * from the last known playing time and elapsed time since the measurement was done.
     */
    Timestamp getWouldbePlayingTime() const;

    /** Get renderer-specific data associated (and destroyed) with this sound source */
    SharedPtr<RenderableSource> getRenderable() const {
        return rendererDataPtr;
    }

    /** Set renderer-specific data to be associated (and destroyed) with this sound source */
    void setRenderable(SharedPtr<RenderableSource> ptr);

    /** Get user-specific data associated (and destroyed) with this sound source */
    SharedPtr<UserData> getUserDataPtr() const {
        return userDataPtr;
    }

    /** Set user-specific data to be associated (and destroyed) with this sound source */
    void setUserDataLong(SharedPtr<UserData> ptr) {
        userDataPtr = ptr;
    }

    /** Get user-specific data associated with this sound source */
    long getUserDataLong() const {
        return userDataLong;
    }

    /** Get user-specific data associated with this sound source */
    void setUserDataLong(long data) {
        userDataLong = data;
    }

    /** Get an event listener associated with this sound source */
    SharedPtr<SourceListener> getSourceListener() const {
        return sourceListenerPtr;
    }

    /** Set an event listener to be associated with this sound source */
    void setSourceListener(SharedPtr<SourceListener> ptr) {
        sourceListenerPtr = ptr;
    }

    /** Get the associated sound stream */
    SharedPtr<Sound> getSound() const {
        return soundPtr;
    }

    /** Set the associated sound stream - Only for SceneManagers to call */
    void setSound(SharedPtr<Sound> ptr) {
        soundPtr = ptr;
        dirty.soundPtr = 1;
    }

    /** Convenience function to update the attached renderable, if present, and active.
     * @param flags see RenderableSource::UpdateFlags
     * @param sceneListener the listener to which this source is associated
     */
    void updateRenderable(int flags, const Listener &sceneListener);

    /** Seek to the specified position
     * @note It may not be supported by the renderer on all sources.
     *      Streaming sources are guaranteed to perform a rough seek on a best effort
     *      basis, meaning the effective time after the seek may be off a bit, and
     *      the process may be costly.
     *       Seeking in non-streaming sources may not be supported at all.
     * @throws EndOfStreamException if you try to seek past the end
     */
    void seek(Timestamp time);

protected:
    /** Set the last known playing time, update the measurement timestamp as well.
     * @return the parameter, for chaining.
     */
    Timestamp setLastKnownPlayingTime(Timestamp timestamp);

    // The following section contains all the virtual functions that need be implemented
    // by a concrete Sound class. All are protected, so the stream interface is independent
    // of implementations.
protected:

    /** @see startPlaying
     * @param start The starting position.
     */
    virtual void startPlayingImpl(Timestamp start) = 0;

    /** @see stopPlaying.*/
    virtual void stopPlayingImpl() = 0;

    /** @see isPlaying.*/
    virtual bool isPlayingImpl() const = 0;

    // The following section contains package-private stuff
    // They're intended for plugin developers, and not end users
public:

    /**
     * Notifies source listeners (if any) that the source
     * has begun actually playing.
     *
     * @remarks Usually, listener notifications are automatic. In the
     *      case of rendering start (actual playing), it's not as
     *      easy, since the renderer and scene manager have ultimate
     *      control over it. It's the renderer's responsability, thus,
     *      to fire this notification event when it's begun rendering.
     *          All other events take place immediately, so they're
     *      automatically handled by Source's implementation.
     */
    void _notifyPlaying();
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_SOURCE_H
