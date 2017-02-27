//
// C++ Implementation: Audio::RenderableListener
//

#include "RenderableListener.h"
#include "config.h"

namespace Audio {

    RenderableListener::RenderableListener(Listener *_listener) throw() : 
        listener(_listener)
    {
    }
    
    RenderableListener::~RenderableListener()
    {
        // Just in case.
        listener = 0;
    }
    
    void RenderableListener::update(int flags) 
        throw()
    {
        try {
            updateImpl(flags);
        } catch(Exception e) {}
    }

};
