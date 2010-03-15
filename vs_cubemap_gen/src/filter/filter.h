#ifndef __FILTER_H__
#define __FILTER_H__

class mem_cubemap;
#include "../units/radians.h"
#include "../units/steradians.h"


class filter_mip
{
    mem_tex<S> const & source_;
    mem_tex<S>       & target_;
    Radians            radius_;
    void init_constants();
    Steradians         solid_angle_;
    float              shininess_;
    //etceteras...
public:
    virtual ~filter_mip();
    filter
    (
        mem_tex<S> const & source
      , mem_tex<S> const & target
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


