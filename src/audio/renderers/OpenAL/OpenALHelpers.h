#ifndef __OPENAL_HELPERS_H__INCLUDED__
#define __OPENAL_HELPERS_H__INCLUDED__

#include "../../Exceptions.h"
#include "al.h"

#define checkAlErrorCode(error) Audio::__impl::OpenAL::_checkAlErrorAt((error), __FILE__, __LINE__)
#define checkAlError() Audio::__impl::OpenAL::_checkAlErrorAt(alGetError(), __FILE__, __LINE__)
#define clearAlError() Audio::__impl::OpenAL::_clearAlError();

#ifndef AL_SEC_OFFSET
/* Supported on Windows, but the headers might be out of date. */
#define AL_SEC_OFFSET 0x1024
#endif
#ifndef ALC_INVALID_ENUM
#define ALC_INVALID_ENUM ALC_ILLEGAL_ENUM
#endif

namespace Audio {
    namespace __impl {
        namespace OpenAL {
            
            void _checkAlErrorAt(ALenum error, const char *filename, int lineno) throw (Exception);
            void _clearAlError() throw();
            ALenum asALFormat(const Format &format) throw(Exception);
            
        }
    }
}

#endif//__OPENAL_HELPERS_H__INCLUDED__
