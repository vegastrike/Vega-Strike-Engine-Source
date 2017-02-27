//
// C++ Implementation: Audio::OpenALRenderableListener
//

#include "OpenALRenderableListener.h"
#include "config.h"

#include "al.h"

#include "../../Listener.h"

namespace Audio {

    OpenALRenderableListener::OpenALRenderableListener(Listener *listener)
        : RenderableListener(listener)
    {
    }
    
    OpenALRenderableListener::~OpenALRenderableListener()
    {
    }
    
    void OpenALRenderableListener::updateImpl(int flags) 
        throw(Exception)
    {
        if (flags & UPDATE_LOCATION) {
            const Vector3 pos(getListener()->getPosition()); // no option but to cast it down to float :(
            const Vector3 vel(getListener()->getVelocity());
            const Vector3 at (getListener()->getAtDirection());
            const Vector3 up (getListener()->getUpDirection());
            ALfloat ori[] = { at.x, at.y, at.z, up.x, up.y, up.z };
            
            alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
            alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
            alListenerfv(AL_ORIENTATION, ori);
        }
        if (flags & UPDATE_ATTRIBUTES) {
            alListenerf (AL_GAIN, getListener()->getGain());
        }
    }

};
