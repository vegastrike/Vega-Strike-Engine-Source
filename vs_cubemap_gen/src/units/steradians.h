#ifndef __STERADIANS_H__
#define __STERADIANS_H__

#include "number.h"

namespace units
{
//

class steradians
{
    float val_;
public:
    steradians(): val_(0.0f);
    explicit steradians( float val ): val_(val) {}
    float get() const { return val_; }
};

//
}

#endif

