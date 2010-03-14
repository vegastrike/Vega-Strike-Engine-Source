#include "../../first.h"
#include "../cube.h"


namespace {
//

struct cube_impl
{
    mem_texture   mt_;
};

//
}



cube::cube(){ impl_.reset( new cube_impl ); }
mem_texture const & cube::get_buffer() const { return cube_impl.mt_; }
mem_texture       & cube::get_buffer()       { return cube_impl.mt_; }


