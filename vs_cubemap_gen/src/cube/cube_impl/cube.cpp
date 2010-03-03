#include "../../first.h"
#include "../cube.h"


//get cube coords (3d) from side and signed side uv's:
void cube::compute_ccoords_from_side_and_scoords( side_and_coords const & snc, ccoords & cc )
{
    #define S ((snc.side_.get_eSide()))
    #define U ((snc.scoords_.u_))
    #define V ((snc.scoords_.v_))
    switch( S )
    {
    //The numbers and axis flips and whatnot below are COMPLETELY bogus for now.
    //I have *no idea* what the standard mapping of texture coords to cube space
    //coordinates are, or where to find the information.
    case eLeft:
        cc = ccoords(  0.5f,    U,    V );
        break;
    case eRight:
        cc = ccoords( -0.5f,   -U,    V );
        break;
    case eUp:
        cc = ccoords(    -U,  0.5,   -V );
        break;
    case eDown:
        cc = ccoords(    -U, -0.5,    V );
        break;
    case eFront:
        cc = ccoords(     U,    V,  0.5 );
        break;
    case eBack:
        cc = ccoords(     U,   -V, -0.5 );
        break;
    default:
        vs_assert( 0, "bad side enum in switch" );
        break;
    }
    #undef V
    #undef U
    #undef S
}

//conversely, get side and signed uv's from cube coords:
void cube::compute_side_and_scoords_from_ccoords( ccoords const & cc, side_and_coords & snc )
{
    #define X ((cc.x_))
    #define Y ((cc.y_))
    #define Z ((cc.z_))
    float X2 = X*X;
    float Y2 = Y*Y;
    float Z2 = Z*Z;
    eSide s;
    float u;
    float v;
    if( X2 > Y2 )
    {
        if( X2 > Z2 )
        {
            //this is either the left side or the right side
            X > 0 ? s = eLeft : s = eRight;
        }
        else
        {
            //this is either the front or the back
            Z > 0 ? s = eFront : s = eBack;
        }
    }
    else
    {
        if( Y2 > Z2 )
        {
            //this is either up or down sides
            Y > 0 ? s = eUp : s = eDown;
        }
        else
        {
            //this is either the front or the back
            Z > 0 ? s = eFront : s = eBack;
        }
    }
    switch( s )
    {
    case eLeft:
        //cc = ccoords(  0.5f,    U,    V );
        u =  Y;  v =  Z;
        break;
    case eRight:
        //cc = ccoords( -0.5f,   -U,    V );
        u = -Y;  v =  Z;
        break;
    case eUp:
        //cc = ccoords(    -U,  0.5,   -V );
        u = -X;  v = -Z;
        break;
    case eDown:
        //cc = ccoords(    -U, -0.5,    V );
        u =  X;  v =  Z;
        break;
    case eFront:
        //cc = ccoords(     U,    V,  0.5 );
        u =  X;  v =  Y;
        break;
    case eBack:
        //cc = ccoords(     U,   -V, -0.5 );
        u =  X;  v = -Y;
        break;
    default:
        vs_assert( 0, "bad side enum in switch" );
        break;
    }
    snc = side_and_coords( s, scoords( u, v ) );
    #undef Z
    #undef Y
    #undef X
}


