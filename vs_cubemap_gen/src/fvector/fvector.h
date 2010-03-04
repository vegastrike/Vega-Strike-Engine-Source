#ifndef __FVECTOR_H__
#define __FVECTOR_H__


class fvector
{
   float x_, y_, z_;
public:
    fvector(): x(0.0f), y(0.0f), z(0.0f) {}
    explicit fvector( ccoords const & cc );
    float get_x() const { return x_; }
    float get_y() const { return y_; }
    float get_z() const { return z_; }
};


#endif


