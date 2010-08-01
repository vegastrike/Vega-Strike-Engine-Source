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
        
        Format() {}
        
        Format(unsigned int freq, unsigned char bps, unsigned char nch) :
            sampleFrequency(freq),
            bitsPerSample(bps),
            channels(nch),
            signedSamples((bps>=16)?1:0),
            nativeOrder(1)
        {
        }
        
        unsigned int frameSize() const
        {
            return (bitsPerSample * channels + 7) / 8;
        }
        
        unsigned int bytesPerSecond() const
        {
            return frameSize() * sampleFrequency;
        }
        
        bool operator==(const Format &o) const
        {
            return (sampleFrequency == o.sampleFrequency)
                && (bitsPerSample == o.bitsPerSample)
                && (channels == o.channels)
                && (signedSamples == o.signedSamples)
                && (nativeOrder == o.nativeOrder);
        }
        
        bool operator!=(const Format &o) const
        {
            return !(*this == o);
        }
    };

};

#endif//__AUDIO_FORMAT_H__INCLUDED__
