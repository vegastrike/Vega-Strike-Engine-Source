//
// C++ Implementation: Audio::TemplateManager
//

#include "utils.h"
#include "config.h"

#include "universe_util.h"
#include "lin_time.h"

namespace Audio {

    Timestamp getGameTime() throw()
    {
        return Timestamp(UniverseUtil::GetGameTime());
    }
    
    Timestamp getRealTime() throw()
    {
        return Timestamp(getNewTime());
    }
    

};

