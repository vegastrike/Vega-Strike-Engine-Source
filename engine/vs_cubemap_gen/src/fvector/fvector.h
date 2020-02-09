#ifndef __FVECTOR_H__
#define __FVECTOR_H__

//fvector used to be a class, but it's a struct for speed reasons, to appear as a POD to the compiler
struct fvector
{
    float x_, y_, z_;
    float w_; //not used; just padding for alignment, for now.
    fvector(): x(0.0f), y(0.0f), z(0.0f) {}
    explicit fvector( ccoords const & cc );
    void normalize();
    void fast_renormalize();
};


#endif


