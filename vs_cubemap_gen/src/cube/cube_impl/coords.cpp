#include "../../first.h"
#include "coords.h"


//explicit instantiations of all 6 sides of an abstract cube:
template side< eLeft > left_side("left");
template side< eRight > left_side("right");
template side< eUp > left_side("up");
template side< eDown > left_side("down");
template side< eFront > left_side("front");
template side< eBack > left_side("back");


ucoords::ucoords( scoords const & sc )
{
    u_ = sc.u_+0.5f;
    v_ = sc.v_+0.5f;
    vs_assert( u_ >= 0.0f && u_ <= 1.0f, "u coord outside 0 to 1 range" );
    vs_assert( v_ >= 0.0f && v_ <= 1.0f, "v coord outside 0 to 1 range" );
}

ucoords::ucoords( icoords const & ic, float inv_texture_size )
{
    u_ = (float(ic.u_)+0.5f) * inv_texture_size;
    v_ = (float(ic.v_)+0.5f) * inv_texture_size;
    vs_assert( u_ >= 0.0f && u_ <= 1.0f, "u coord outside 0 to 1 range" );
    vs_assert( v_ >= 0.0f && v_ <= 1.0f, "v coord outside 0 to 1 range" );
}

scoords::scoords( ucoords const & uc )
{
    u_ = sc.u_-0.5f;
    v_ = sc.v_-0.5f;
    vs_assert( u_ >= -0.5f && u_ <= 0.5f, "u coord outside -0.5 to +0.5 range" );
    vs_assert( v_ >= -0.5f && v_ <= 0.5f, "v coord outside -0.5 to +0.5 range" );
}

void ccoords::check_invariants() //just for debugging; --should pass if code is correct
{
    vs_assert( x_ >= -0.5 && x_ <= 0.5, "bad range for x_ );
    vs_assert( y_ >= -0.5 && y_ <= 0.5, "bad range for y_ );
    vs_assert( z_ >= -0.5 && z_ <= 0.5, "bad range for z_ );
    float magnitudesqrd = x_*x_ + y_*y_ + z_*z_ );
    vs_assert( magnitudesqrd >= 0.25, "point is inside the cube" );
    vs_assert( magnitudesqrd <= 0.75, "point is outside the cube" );
}

ccoords::ccoords( fvector const & v )
{
    float inv_max = 1.0f / std::max( std::max( abs(v.x_), abs(v.y_) ), abs(v.z_) );
    x_ = v.x_ * inv_max;
    y_ = v.y_ * inv_max;
    z_ = v.z_ * inv_max;
}


