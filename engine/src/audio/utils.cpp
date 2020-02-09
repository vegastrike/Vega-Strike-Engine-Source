//
// C++ Implementation: Audio::TemplateManager
//

#include "utils.h"
#include "config.h"

#include "universe_util.h"
#include "lin_time.h"

#include "Source.h"
#include "Listener.h"

namespace Audio {

    Timestamp getGameTime() throw()
    {
        return Timestamp(UniverseUtil::GetGameTime());
    }
    
    Timestamp getRealTime() throw()
    {
        return Timestamp(realTime());
    }
    

    Scalar estimateGain(const Source &src, const Listener &listener) throw()
    {
        // Base priority is source gain
        Scalar gain = src.getGain();
        
        // Account for distance attenuation
        LScalar distance = listener.getPosition().distance(src.getPosition())
                         - listener.getRadius()
                         - src.getRadius();
        LScalar ref = listener.getRadius();
        LScalar rolloff = listener.getRadius() / src.getRadius();
        gain *= (distance <= 0) ? 1.f : float(ref / (ref + rolloff * distance));
        
        // Account for dispersion/sensing angle limitations
        Scalar cosangle = listener.getAtDirection().dot( src.getDirection() );
        if (cosangle < listener.getCosAngleRange().min)
            gain *= listener.getCosAngleRange().phase(cosangle);
        if (cosangle < src.getCosAngleRange().min)
            gain *= src.getCosAngleRange().phase(cosangle);
        
        return gain;
    }
    
    void sleep(unsigned int ms)
    {
        micro_sleep(ms * 1000);
    }
};

