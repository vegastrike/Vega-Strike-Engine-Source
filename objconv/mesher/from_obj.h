#include "mesh_io.h"
#include "to_BFXM.h"
#include <vector>
void ObjToXMESH (FILE* obj, FILE * mtl, std::vector<XML> &xml, bool forcenormals);
void ObjToBFXM (FILE *, FILE *, FILE *,bool forcenormals);
string ObjGetMtl (FILE *, string);
