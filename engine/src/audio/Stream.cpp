//
// C++ Implementation: Audio::Stream
//

#include "Stream.h"

#include <utility>
#include <cstring>

//using namespace std;

namespace Audio {

    using std::min;

    Stream::Stream(const std::string& path)
    {
    }
    
    Stream::~Stream()
    {
    }

    double Stream::getLength()
    {
        return getLengthImpl();
    }

    double Stream::getPosition() const
    {
        return getPositionImpl();
    }

    void Stream::seek(double position)
    {
        seekImpl(position);
    }

    unsigned int Stream::read(void *buffer, unsigned int bufferSize)
    {
        void *rbuffer;
        void *rbufferEnd;
        unsigned int rbufferSize;
        unsigned int rode = 0;
        
        try {
            getBufferImpl(rbuffer, rbufferSize);
        } catch (const NoBufferException&) {
            nextBufferImpl();
            getBufferImpl(rbuffer, rbufferSize);
            curBufferPos = rbuffer;
        }
        rbufferEnd = ((char*)rbuffer) + rbufferSize;
        
        while (bufferSize > 0) {
            if (!((curBufferPos >= rbuffer) && (curBufferPos < rbufferEnd))) {
                nextBufferImpl();
                getBufferImpl(rbuffer, rbufferSize);
                curBufferPos = rbuffer;
                rbufferEnd = ((char*)rbuffer) + rbufferSize;
            }
            
            size_t remaining = min( bufferSize, (unsigned int)((char*)rbufferEnd - (char*)curBufferPos) ); //is there no std::ptrdiff?
            memcpy(buffer, curBufferPos, remaining);
            buffer = (void*)((char*)buffer + remaining);
            curBufferPos = (void*)((char*)curBufferPos + remaining);
            bufferSize -= remaining;
            rode += remaining;
        }
        
        return rode;
    }

};
