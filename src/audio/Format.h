//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_FORMAT_H__INCLUDED__
#define __AUDIO_FORMAT_H__INCLUDED__

namespace Audio {

    /**
     * Audio format information class
     *
     */
    struct Format {
        unsigned int sampleFrequency;
        unsigned char bitsPerSample;
        unsigned char channels;
        int signedSamples : 1;
        int nativeOrder : 1;
    };

};

#endif//__AUDIO_STREAM_H__INCLUDED__
