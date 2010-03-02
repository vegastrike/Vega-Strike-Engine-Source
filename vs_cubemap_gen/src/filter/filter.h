#ifndef __FILTER_H__
#define __FILTER_H__

class mem_cubemap;
#include "../units/radians.h"
#include "../units/steradians.h"


class filter
{
    mem_cubemap const & source_;
    mem_cubemap       & target_;
    radians             radius_;
    void init_constants();
    steradians          solid_angle_;
    float               shininess_;
    //etceteras...
    double acc_red_;
    double acc_grn_;
    double acc_blu_;
    double acc_weight_;
    size_t count_;
public:
    virtual ~filter();
    filter
    (
        mem_cubemap const & source
      , mem_cubemap const & target
      , radians const & radius
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


