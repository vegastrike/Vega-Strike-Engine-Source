/**
 * Codec.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_CODEC_H__INCLUDED__
#define __AUDIO_CODEC_H__INCLUDED__

#include <string>
#include <vector>
#include "../Exceptions.h"

#include "vsfilesystem.h"

namespace Audio {

// Forward Stream so that Codec headers don't depend on them.
// Implementations will surely depend on them, though...
// ...but this way Codec users may be oblivious to them.
class Stream;

/**
 * Codec factory class.
 *
 * @remarks Use it to create Stream instances attached to a file.
 * @see CodecRegistry to create Codec instances.
 *
 */
class Codec {
private:
    std::string name;

protected:
    /** Internal constructor */
    Codec(const std::string &name);

public:
    /** Back-appendable collection of std::string, optimally for static readonly data */
    typedef std::vector<std::string> Extensions;

public:
    virtual ~Codec();

    /** Return the descriptive name of the codec. */
    const std::string &getName() const;

    /** Return a list of supported extensions.
     * @remarks
     *      If it returns something, it is expected that files handled by this codec
     *      be named by appending one of the returned extensions as a suffix.
     *      @par Though supported extensions are a big help to the codec registry
     *      when asked to suggest codecs for a file, a null value can be returned
     *      asking the registry to always try this codec. This is useful if the codec
     *      can handle many (perhaps unknown) file types, or if there is no associated
     *      extension to the type handled.
     *      @par The base implementation does just that - returns null.
     *      @par It is expected that files named with returned extensions will pass the
     *      tests for canHandle(path,false). Though this may not always be true. So
     *      the final test canHandle(path,true) should never be skipped, even if the file
     *      includes an extension within the returned set.
     */
    virtual const Extensions *getExtensions() const;

    /**
     * Returns whether this codec instanc can handle the specified file.
     * @param path the file to be tested
     * @param canOpen whether opening the file to take a peek is allowed. If not, the
     *      implementation should rely on the filename alone to do its task
     *      (ie: look for an extension). See the remarks for more details.
     *
     * @remarks This member will return true when it thinks the codec can decode
     *      the specified file, and false otherwise. No certainty is required. However,
     *      an assumption is made that codec implementations will make a reasonable
     *      effort to guess the outcome of an "open" call, given the allowed operations.
     *      If canOpen is true, the implementation can open the file to inspect its
     *      format and accurately decide that. The operation is still expected to be
     *      rather quick. If canOpen is false, the operation is expected to be even
     *      quicker and the implementation may not open the file. Rather, filename
     *      patterns (or any other method that doesn't open the file) are the only
     *      viable methods of implementation, and thus it is expected that the returned
     *      value will be less accurate. Whenever in doubt, the function will return
     *      success (true).
     */
    virtual bool canHandle(const std::string &path,
                           bool canOpen,
                           VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) = 0;

    /**
     * Returns an instance of the Stream class attached to this codec and the specified
     * file.
     *
     * @param path the file to be attached to the returned object.
     * @param type (optional) the type of file, used by resource management APIs.
     * @remarks Don't even bother to call it if canHandle returns false. However,
     *      if it returns true, this can still fail. Be prepared to catch exceptions
     *      and deal with them accordingly.
     */
    virtual Stream *open(const std::string &path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) = 0;
};

};

#endif//__AUDIO_CODEC_H__INCLUDED__
