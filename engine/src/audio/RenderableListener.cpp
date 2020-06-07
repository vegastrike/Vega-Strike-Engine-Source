//
// C++ Implementation: Audio::RenderableListener
//

#include "RenderableListener.h"

namespace Audio {

    RenderableListener::RenderableListener(Listener *_listener) :
        listener(_listener)
    {
    }
    
    RenderableListener::~RenderableListener()
    {
        // Just in case.
        listener = 0;
    }
    
    void RenderableListener::update(int flags) 
    {
        try {
            updateImpl(flags);
        } catch(const Exception& e) {}
    }

};
