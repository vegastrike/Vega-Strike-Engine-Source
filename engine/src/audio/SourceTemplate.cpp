//
// C++ Implementation: Audio::SourceTemplate
//

#include "SourceTemplate.h"

#include <math.h>

namespace Audio {

    SourceTemplate::SourceTemplate(const std::string &sound, VSFileSystem::VSFileType type, bool _looping) :
        soundName(sound),
        soundType(type),
        cosAngleRange(-1,-1),
        pfRadiusRatios(1,1),
        referenceFreqs(250,5000),
        gain(1)
    {
        setLooping(_looping);
        setStreaming(false);
        setRelative(false);
        setAttenuated(true);
    }
    
    SourceTemplate::~SourceTemplate()
    {
    }

    Range<Scalar> SourceTemplate::getAngleRange() const
    { 
        return Range<Scalar>(Scalar(acos(cosAngleRange.min)), 
                             Scalar(acos(cosAngleRange.max))); 
    }
    
    void SourceTemplate::setAngleRange(Range<Scalar> r)
    { 
        cosAngleRange.min = Scalar(cos(r.min)); 
        cosAngleRange.max = Scalar(cos(r.max));
    }
    

};
