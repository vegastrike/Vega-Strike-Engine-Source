//
// C++ Implementation: Audio::Stream
//

#include "Stream.h"
#include "config.h"

#include <utility>
#include <cstring>

using namespace std;

namespace Audio {

    Stream::Stream(const std::string& path) throw(Exception)
    {
    }
    
    Stream::~Stream()
    {
    }

    double Stream::getLength() throw(Exception)
    {
        return getLengthImpl();
    }

    double Stream::getPosition() const throw()
    {
        return getPositionImpl();
    }

    void Stream::seek(double position) throw(Exception)
    {
        seekImpl(position);
    }

    unsigned int Stream::read(void *buffer, unsigned int bufferSize) throw(Exception)
    {
        void *rbuffer;
        void *rbufferEnd;
        unsigned int rbufferSize;
        unsigned int rode = 0;
        
        try {
            getBufferImpl(rbuffer, rbufferSize);
        } catch (NoBufferException) {
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
            
            size_t remaining = min( bufferSize, (unsigned int)((char*)rbufferEnd - (char*)curBufferPos) );
            memcpy(buffer, curBufferPos, remaining);
            (char*&)buffer += remaining;
            (char*&)curBufferPos += remaining;
            bufferSize -= remaining;
            rode += remaining;
        }
        
        return rode;
    }

};
