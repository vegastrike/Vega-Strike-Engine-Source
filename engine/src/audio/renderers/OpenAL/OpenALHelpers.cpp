/**
 * OpenALHelpers.cpp
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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

//
// C++ Implementation: OpenAL helper functions
//

#include "OpenALHelpers.h"

#include "../../Types.h"
#include "../../Exceptions.h"

#include "al.h"

#include <string>
#include <stdio.h>
#include "vs_logging.h"

namespace Audio {
    namespace __impl {
        namespace OpenAL {

            void _checkAlErrorAt(ALenum error, const char *filename, int lineno)
            {
                switch(error) {
                case AL_NO_ERROR : return;
                default:
                    {
                        const char* errdesc = (const char *)alGetString(error);
                        char s_lineno[32];
                        char s_errcode_buf[32];
                        const char *s_errcode;
                        sprintf(s_lineno, "%d", lineno);

                        switch (error) {
                        case ALC_INVALID_DEVICE: s_errcode = "(Invalid device)";      break;
                        case ALC_INVALID_CONTEXT:s_errcode = "(Invalid context id)";  break;
                        case ALC_INVALID_ENUM:   s_errcode = "(Bad enum)";            break;
                        case ALC_INVALID_VALUE:  s_errcode = "(Bad value)";           break;
                        case ALC_OUT_OF_MEMORY:  s_errcode = "(Out of memory)";       break;
                        default:
                            sprintf(s_errcode_buf, "(0x%x)", error);
                            s_errcode = s_errcode_buf;
                        };

                        std::string error("OpenAL error: ");
                        error += std::string(errdesc ? errdesc : "unknown") + s_errcode + " at " + filename + ":" + s_lineno;
                        VS_LOG_AND_FLUSH(error, (boost::format("%1%") % error));
                        throw Exception(error);
                    }
                }
            }

            void _clearAlError()
            {
                alGetError();
            }

            ALenum asALFormat(const Format &format)
            {
                ALenum alformat;
                switch(format.bitsPerSample) {
                case 8:
                    switch(format.channels) {
                    case 1:  alformat = AL_FORMAT_MONO8; break;
                    case 2:  alformat = AL_FORMAT_STEREO8; break;
                    default: throw Exception("internal error - unexpected number of channels");
                    };
                    break;
                case 16:
                    switch(format.channels) {
                    case 1:  alformat = AL_FORMAT_MONO16; break;
                    case 2:  alformat = AL_FORMAT_STEREO16; break;
                    default: throw Exception("internal error - unexpected number of channels");
                    };
                    break;
                default:
                    throw Exception("internal error - unexpected bit depth");
                };

                return alformat;
            }

        }
    }
}
