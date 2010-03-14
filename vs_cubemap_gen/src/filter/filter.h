#ifndef __FILTER_H__
#define __FILTER_H__

class mem_cubemap;
#include "../units/radians.h"
#include "../units/steradians.h"


class filter
{
    mem_cubemap const & source_;
    mem_cubemap       & target_;
    Radians             radius_;
    void init_constants();
    Steradians          solid_angle_;
    float               shininess_;
    //etceteras...
public:
    virtual ~filter();
    filter
    (
        mem_cubemap const & source
      , mem_cubemap const & target
      , Radians const & radius
    )
    : source_(source)
    , target_(target)
    , radius_(radius)
    {
        init_constants();
    }
    void do_it();
};


#endif


