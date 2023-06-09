/**
 * Format.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_FORMAT_H
#define VEGA_STRIKE_ENGINE_AUDIO_FORMAT_H
//
// C++ Interface: Audio::Codec
//

namespace Audio {

/**
 * Audio format information class
 *
 */
struct Format {
    unsigned int sampleFrequency;
    unsigned char bitsPerSample;
    unsigned char channels;
    int signedSamples: 1;
    int nativeOrder: 1;

    Format() {
    }

    Format(unsigned int freq, unsigned char bps, unsigned char nch) :
            sampleFrequency(freq),
            bitsPerSample(bps),
            channels(nch),
            signedSamples((bps >= 16) ? 1 : 0),
            nativeOrder(1) {
    }

    unsigned int frameSize() const {
        return (bitsPerSample * channels + 7) / 8;
    }

    unsigned int bytesPerSecond() const {
        return frameSize() * sampleFrequency;
    }

    bool operator==(const Format &o) const {
        return (sampleFrequency == o.sampleFrequency)
                && (bitsPerSample == o.bitsPerSample)
                && (channels == o.channels)
                && (signedSamples == o.signedSamples)
                && (nativeOrder == o.nativeOrder);
    }

    bool operator!=(const Format &o) const {
        return !(*this == o);
    }
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_FORMAT_H
