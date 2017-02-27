#include "../filter.h"


void filter::init_constants()
{
}

void filter::do_it()
{
    filter_one_texel flt_one_tex( source_, target_, shininess_ );
    fvector v;
    for( size_t s = eLeft; s < eBack; ++s )
    {
        for( size_t i = 0; i < target.size(); ++i )
        {
            for( size_t j = 0; j < target.size(); ++j )
            {
                mem_texture & tmt = target.get_buffer();
                icoords ic( i, j );
                fRGBAcolor *pt = tmt.pTexel( s, ic );
                ccoords cc = ccoords( side_and_coords( s, scoords( ucoords( ic ) ) ) );
                v = fvector( cc );
                *pt = flt_one_tex( v );
            }
        }
    }
}
