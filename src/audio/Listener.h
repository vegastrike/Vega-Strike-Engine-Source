//
// C++ Interface: Audio::Scene
//
#ifndef __AUDIO_LISTENER_H__INCLUDED__
#define __AUDIO_LISTENER_H__INCLUDED__

#include "Exceptions.h"
#include "Types.h"

namespace Audio {

    /**
     * Listener class
     *
     * @remarks This class represents a scene listener. Its interface is quite simple.
     *      It's possible that scene managers (scenes) will provide their own implementation.
     *      @par This class is not abstract, its interface is simple enough so that Listener
     *      acts both as interface and basic implementation.
     */
    class Listener
    {
        LVector3 position;
        Vector3 atDirection;
        Vector3 upDirection;
        Vector3 velocity;
        
        Scalar radius;
        
        Range<Scalar> cosAngleRange;
        
        SharedPtr<UserData> userDataPtr;
        SharedPtr<UserData> rendererDataPtr;
        
        /** Notify implementations after position and/or attributes changes
         * @remarks Implementations use the "dirty" member struct to precisely know and 
         *      track what changed and what didn't. Call reset() when synchronized.
         */
        struct {
            /** position, velocity & direction */
            int location : 1;
            
            /** min/max angle, radius, pf radius ratios, reference freqs */
            int attributes : 1;
            
            void reset()
            {
                location = 0;
                attributes = 0;
            }
        } dirty;
        
    public:
        /** Construct a default listener with default parameters */
        Listener() throw();
        
        virtual ~Listener();
        
        /** Return the listener's central position in 3D space */
        LVector3 getPosition() const throw() { return position; }
        
        /** Set the listener's central position in 3D space */
        void setPosition(LVector3 x) throw() { position = x; dirty.location = 1; }
        
        /** Return the listener's front direction */
        Vector3 getAtDirection() const throw() { return atDirection; }
        
        /** Return the listener's up direction */
        Vector3 getUpDirection() const throw() { return upDirection; }
        
        /** Set the listener's orientation */
        void setOrientation(Vector3 at, Vector3 up) throw() { atDirection = at; upDirection = up; dirty.location = 1; }
        
        /** Return the listener's velocity */
        Vector3 getVelocity() const throw() { return velocity; }
        
        /** Set the listener's velocity */
        void setVelocity(Vector3 x) throw() { velocity = x; dirty.location = 1; }
        
        
        /** Return the listener's minimum/maximum perception angle
         * @remarks A listener's sound perception may be directional or omnidirectional. 
         *      Perception will be full within minimum directional drift. Further drift 
         *      will attenuate the sound until it reaches maximum attenuation by the maximum 
         *      perception angle. Notice that maximum attenuation may not be full silence.
         */
        Range<Scalar> getAngleRange() const throw();
        
        /** @see getAngleRange */
        void setAngleRange(Range<Scalar> r) throw();
        
        /** @see getAngleRange @remarks This version returns cosine-angles rather than radians, much quicker */
        Range<Scalar> getCosAngleRange() const throw() { return cosAngleRange; }
        
        /** @see getAngleRange @remarks This version takes cosine-angles rather than radians, much quicker */
        void setCosAngleRange(Range<Scalar> r) throw() { cosAngleRange = r; dirty.attributes = 1; }
        
        /** Get the listener's radius */
        Scalar getRadius() const throw() { return radius; }
        
        /** Set the listener's radius */
        void setRadius(Scalar r) throw() { radius = r; dirty.attributes = 1; }
        
        
        /** Get renderer-specific data associated (and destroyed) with this sound source */
        SharedPtr<UserData> getRendererData() const throw() { return rendererDataPtr; }
        
        /** Set renderer-specific data to be associated (and destroyed) with this sound source */
        void setRendererData(SharedPtr<UserData> ptr) throw() { rendererDataPtr = ptr; }
        
        /** Get user-specific data associated (and destroyed) with this listener */
        SharedPtr<UserData> getUserData() const throw() { return userDataPtr; }
        
        /** Set user-specific data to be associated (and destroyed) with this listener */
        void setUserData(SharedPtr<UserData> ptr) throw() { userDataPtr = ptr; }
        
        
    };

};

#endif//__AUDIO_LISTENER_H__INCLUDED__
