#ifndef __FVECTOR_H__
#define __FVECTOR_H__


class fvector
{
   float x, y, z;
public:
    fvector(): x(0.0f), y(0.0f), z(0.0f) {}
    explicit fvector( ccoords const & cc );
};


#endif


