/*
 * CodecRegistry.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_CODECREGISTRY_H
#define VEGA_STRIKE_ENGINE_AUDIO_CODECREGISTRY_H

//
// C++ Interface: Audio::CodecRegistry
//

#include <string>
#include <map>
#include <set>
#include "Exceptions.h"
#include "Singleton.h"

#include "vegadisk/vsfilesystem.h"

namespace Audio {

// The registry needs not know what it's managing... right?
// At least not its interface.
class Codec;
class Stream;

/**
 * Codec registry class.
 *
 * @remarks Use it to manage codec implementations.
 *      @par It handles codec factories and manages resolving file-to-codec associations.
 * @see Codec
 *
 */
class CodecRegistry : public Singleton<CodecRegistry> {
private:
    // The many required indexes
    typedef std::set<Codec *> CodecSet;
    typedef std::map<Codec *, int> CodecPriority;
    typedef std::map<std::string, CodecSet> ExtensionCodecs;
    typedef std::map<std::string, Codec *> NameCodec;

    CodecPriority codecPriority;
    ExtensionCodecs extensionCodecs;
    CodecSet universalCodecs;
    NameCodec nameCodec;

public:
    /** Construct an empty registry
     * @remarks End-users of the class shouldn't be using this. Singletons need it*/
    CodecRegistry();

    ~CodecRegistry();

    /**
     * Add a codec to the registry
     * @remarks You may add a codec multiple times if you wish. The class guarantees
     *      that any further call will be a no-op. Previos priority will still apply.
     * @param codec The codec to be added to the registry.
     */
    void add(Codec *codec, int priority = 0);

    /**
     * Remove a codec from the registry
     * @remarks If the codec had already been removed, this is a no-op.
     * @param codec The codec to be removed from the registry.
     */
    void remove(Codec *codec);

    /**
     * Find a codec by its name
     * @returns A codec instance named by the specified name, never null.
     * @remarks If there's more than one codec with the same name, any one of them is returned.
     *      Not even necessarily the same each time. So avoid adding conflicting codecs.
     *      @par Instead of returning null, if a codec of such characteristics cannot be found,
     *      a CodecNotFound exception is risen.
     */
    Codec *findByName(const std::string &name) const;

    /**
     * Find a codec that can handle the file.
     * @returns A codec instance that can handle the specified file (ie: canHandle() returns true).
     *      It will never return null.
     * @remarks If there's more than one codec that can handle the file, the one with the higher
     *      priority will be returned. Notice that it can't be assured that the codec will be able
     *      to open the file, so be prepared to handle failures further down the road.
     *      @par Instead of returning null, if a codec of such characteristics cannot be found,
     *      an CodecNotFound exception is risen.
     */
    Codec *findByFile(const std::string &path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) const;

    /**
     * Open the specified file with a suitable codec.
     * @see findByFile
     */
    Stream *open(const std::string &path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) const;

};

class CodecRegistration {
    Codec *codec;
public:
    CodecRegistration(Codec *codec, int priority = 0);
    ~CodecRegistration();
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_CODECREGISTRY_H
