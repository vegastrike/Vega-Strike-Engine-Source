#ifndef __RADIANS_H__
#define __RADIANS_H__

#include "number.h"

namespace units
{
//

class radians
{
    float val_;
public:
    radians(): val_(0.0f);
    float get() const { return val_; }
    radians const & from_deg() const;
};

number fsin( radians const & );
number fcos( radians const & );
number ftan( radians const & );

//
}

#endif

