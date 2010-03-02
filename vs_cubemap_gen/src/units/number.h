#ifndef __NUMBER_H__
#define __NUMBER_H__

namespace units
{
//

class number
{
    float val_;
public:
    number(): val(0.0f) {}
    explicit number( float val ): val_(val) {}
    float get() const { return val_; }
};

//
}

#endif


