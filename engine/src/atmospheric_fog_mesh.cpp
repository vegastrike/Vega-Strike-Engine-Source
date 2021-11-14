#include "atmospheric_fog_mesh.h"



AtmosphericFogMesh::AtmosphericFogMesh()
{
    er              = eg = eb = ea = .5;
    dr              = dg = db = da = .5;
    meshname        = "sphereatm.bfxm";
    scale           = 1.05;
    focus           = .5;
    concavity       = 0;
    tail_mode_start = -1;
    tail_mode_end   = -1;
    min_alpha       = 0;
    max_alpha       = 255;
}
