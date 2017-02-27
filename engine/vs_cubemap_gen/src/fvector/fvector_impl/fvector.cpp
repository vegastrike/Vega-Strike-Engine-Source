#include "../../first.h"
#include "../fvector.h"


fvector::fvector( ccoords const & cc )
{
    float inv_magnitude = 1.0f
       / ::sqrtf( cc.x_*cc.x_ + cc.y_*cc.y_ + cc.z_*cc.z_ );
    x_ = cc.x_ * inv_magnitude;
    y_ = cc.y_ * inv_magnitude;
    z_ = cc.z_ * inv_magnitude;
}
