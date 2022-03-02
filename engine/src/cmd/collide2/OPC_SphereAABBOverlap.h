///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Updated by Stephen G. Tuggy 2021-07-03
 * Updated by Stephen G. Tuggy 2022-01-06
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Sphere-AABB overlap test, based on Jim Arvo's code.
 *	\param		center		[in] box center
 *	\param		extents		[in] box extents
 *	\return		TRUE on overlap
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline_ bool SphereCollider::SphereAABBOverlap(const Point &center, const Point &extents) {
    // Stats
    mNbVolumeBVTests++;

    float d = 0.0f;

    //find the square of the distance
    //from the sphere to the box

//	float tmp = mCenter.x - center.x;
//	float s = tmp + extents.x;

    float tmp, s;

    tmp = mCenter.x - center.x;
    s = tmp + extents.x;

    if (s < 0.0f) {
        d += s * s;
        if (d > mRadius2) {
            return FALSE;
        }
    } else {
        s = tmp - extents.x;
        if (s > 0.0f) {
            d += s * s;
            if (d > mRadius2) {
                return FALSE;
            }
        }
    }

    tmp = mCenter.y - center.y;
    s = tmp + extents.y;

    if (s < 0.0f) {
        d += s * s;
        if (d > mRadius2) {
            return FALSE;
        }
    } else {
        s = tmp - extents.y;
        if (s > 0.0f) {
            d += s * s;
            if (d > mRadius2) {
                return FALSE;
            }
        }
    }

    tmp = mCenter.z - center.z;
    s = tmp + extents.z;

    if (s < 0.0f) {
        d += s * s;
        if (d > mRadius2) {
            return FALSE;
        }
    } else {
        s = tmp - extents.z;
        if (s > 0.0f) {
            d += s * s;
            if (d > mRadius2) {
                return FALSE;
            }
        }
    }
    return d <= mRadius2;
}
