/*
 * CodecRegistry.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
// C++ Implementation: Audio::CodecRegistry, Audio::CodecRegistration
//

#include "CodecRegistry.h"
#include "codecs/Codec.h"

#include <algorithm>
#include <iostream>

template<> Audio::CodecRegistry *Singleton<Audio::CodecRegistry>::_singletonInstance = 0;

namespace Audio {
CodecRegistry::CodecRegistry() {
}

CodecRegistry::~CodecRegistry() {
    nameCodec.clear();
    universalCodecs.clear();
    extensionCodecs.clear();

    for (CodecPriority::const_iterator it = codecPriority.begin(); it != codecPriority.end(); ++it) {
        delete it->first;
    }
    codecPriority.clear();
}

void CodecRegistry::add(Codec *codec, int priority) {
    if (codecPriority.find(codec) == codecPriority.end()) {
        codecPriority[codec] = priority;
        nameCodec[codec->getName()] = codec;

        // static initializer ordering issue between boost logger and other code?
        VS_LOG(trace, (boost::format("Registering codec %1%") % codec->getName().c_str()));

        const Codec::Extensions *ext = codec->getExtensions();
        if (ext) {
            for (Codec::Extensions::const_iterator it = ext->begin(); it != ext->end(); ++it) {
                VS_LOG(trace, (boost::format(" %1%") % it->c_str()));
                extensionCodecs[*it].insert(codec);
            }
        } else {
            VS_LOG(trace, " as universal");
            universalCodecs.insert(codec);
        }

        VS_LOG(trace, ".");
    } else {
        VS_LOG(trace, (boost::format("Codec %1% already registered") % codec->getName().c_str()));
    }
}

void CodecRegistry::remove(Codec *codec) {
    if (codecPriority.find(codec) != codecPriority.end()) {
        codecPriority.erase(codec);
        nameCodec.erase(codec->getName());
        const Codec::Extensions *ext = codec->getExtensions();
        if (ext) {
            for (Codec::Extensions::const_iterator it = ext->begin(); it != ext->end(); ++it) {
                extensionCodecs[*it].erase(codec);
            }
        } else {
            universalCodecs.erase(codec);
        }
    }
}

Codec *CodecRegistry::findByName(const std::string &name) const {
    NameCodec::const_iterator it = nameCodec.find(name);
    if (it != nameCodec.end()) {
        return it->second;
    } else {
        throw CodecNotFoundException(
                std::string("No codec with name \"") + name + "\" has been registered");
    }
}

template<typename INDEX, typename T>
class MappedComparator {
    const INDEX &_index;
public:
    MappedComparator(const INDEX &index) : _index(index) {
    }

    bool operator()(const T &a, const T &b) const {
        typename INDEX::const_iterator ait = _index.find(a);
        typename INDEX::const_iterator bit = _index.find(b);
        if (ait == _index.end()) {
            return (bit != _index.end());
        } else {
            if (bit == _index.end()) {
                return false;
            } else {
                return (*ait < *bit);
            }
        }
    }
};

Codec *CodecRegistry::findByFile(const std::string &path, VSFileSystem::VSFileType type) const {
    std::vector<Codec *> candidates;

    // NOTE: we'll assume that if they are including the specified extension, they'll return
    //      true if asked if they can handle the file with !canOpen.
    size_t sep = path.find_last_of('.');
    if (sep != std::string::npos && (sep + 1) < path.length()) {
        ExtensionCodecs::const_iterator eit = extensionCodecs.find(
                path.substr(sep + 1, std::string::npos));
        if (eit != extensionCodecs.end()) {
            for (CodecSet::const_iterator esit = eit->second.begin(); esit != eit->second.end(); ++esit) {
                candidates.push_back(*esit);
            }
        }
    }

    for (CodecSet::const_iterator uit = universalCodecs.begin(); uit != universalCodecs.end(); ++uit) {
        if ((*uit)->canHandle(path, false, type)) {
            candidates.push_back(*uit);
        }
    }

    // Now we'll sort candidates by priority, and return the first codec that can actually handle the file.
    if (candidates.begin() != candidates.end()) {
        std::sort(
                candidates.begin(),
                candidates.end(),
                MappedComparator<CodecPriority, Codec *>(codecPriority));

        // Why do we need an explicit cast *to* const?
        // See http://www.mpi-inf.mpg.de/~hitoshi/otherprojects/tips/cpp-memo.shtml
        for (std::vector<Codec *>::const_reverse_iterator it = candidates.rbegin();
                it != reinterpret_cast<const std::vector<Codec *> &>(candidates).rend(); ++it) {
            if ((*it)->canHandle(path, true, type)) {
                return *it;
            }
        }
    }

    // No candidate is able to handle this one!
    throw CodecNotFoundException(
            std::string("No registered codec can handle the file \"") + path + "\"");
}

Stream *CodecRegistry::open(const std::string &path, VSFileSystem::VSFileType type) const {
    Codec *codec = findByFile(path, type);
    return codec->open(path, type);
}

CodecRegistration::CodecRegistration(Codec *_codec, int priority)
        : codec(_codec) {
    if (!CodecRegistry::getSingleton()) {
        new CodecRegistry();
    }
    CodecRegistry::getSingleton()->add(codec);
}

CodecRegistration::~CodecRegistration() {
    CodecRegistry::getSingleton()->remove(codec);
}

};
