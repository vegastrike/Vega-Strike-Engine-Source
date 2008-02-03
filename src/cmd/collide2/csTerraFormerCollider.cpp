#include "cssysdef.h"
#include "iutil/strset.h"
#include "iutil/objreg.h"
#include "csgeom/box.h"
#include "csTerraFormerCollider.h"
#include "OPC_TreeBuilders.h"


using namespace Opcode;

csTerraFormerCollider::csTerraFormerCollider (iTerraFormer* terraformer, 
                                              iObjectRegistry* object_reg)
{
  csTerraFormerCollider::object_reg = object_reg;
  former = terraformer;
  // Get the shared string repository
  csRef<iStringSet> strings = csQueryRegistryTagInterface<iStringSet> (
    object_reg, "crystalspace.shared.stringset");
  stringHeights = strings->Request ("heights");
  stringVertices = strings->Request ("vertices");

  opcMeshInt.SetCallback (&MeshCallback, this); 

  transform.m[0][0] = 1;
  transform.m[1][0] = 0;
  transform.m[2][0] = 0;
  transform.m[3][0] = 0;  

  transform.m[0][1] = 0;
  transform.m[1][1] = 1;
  transform.m[2][1] = 0;
  transform.m[3][1] = 0;  

  transform.m[0][2] = 0;
  transform.m[1][2] = 0;
  transform.m[2][2] = 1;
  transform.m[3][2] = 0;  

  transform.m[0][3] = 0;
  transform.m[1][3] = 0;
  transform.m[2][3] = 0;
  transform.m[3][3] = 1; 

  resolution = 4;
  indexholder = 0;

  opcode_model = 0;

  InitOPCODEModel ();
  UpdateOPCODEModel (csVector3 (0), 4);
}
csTerraFormerCollider::~csTerraFormerCollider ()
{
  if (opcode_model)
  {
    delete opcode_model;
    opcode_model = 0;
  }

  delete[] indexholder;
}

void csTerraFormerCollider::UpdateOPCODEModel (const csVector3 &other_pos, float res)
{
  if (ceil (res) > resolution)
  {
    resolution = (unsigned int)ceil (res);
    InitOPCODEModel ();
  }
  csRef<iTerraSampler> sampler = former->GetSampler (
    csBox2 (other_pos.x - resolution, other_pos.z - resolution,
    other_pos.x + resolution, other_pos.z + resolution), resolution , resolution);

  const csVector3 *v = sampler->SampleVector3 (stringVertices);

  for (unsigned int y = 0 ; y < resolution ; y++)
  {
    for (unsigned int x = 0 ; x < resolution ; x++)
    {
      int index = y*resolution + x;
      vertices[index].Set (v[index].x,v[index].y,v[index].z);
    }
  }
  
  int i = 0;
  for (unsigned int y = 0 ; y < resolution-1 ; y++)
  {
    int yr = y * resolution;
    for (unsigned int x = 0 ; x < resolution-1 ; x++)
    {
      indexholder[i++] = yr + x;
      indexholder[i++] = yr+resolution + x;
      indexholder[i++] = yr + x+1;
      indexholder[i++] = yr + x+1;
      indexholder[i++] = yr+resolution + x;
      indexholder[i++] = yr+resolution + x+1;
    }
  }
  opcode_model->Build (OPCC);
}

void csTerraFormerCollider::InitOPCODEModel ()
{
  delete indexholder;
  delete opcode_model;
  indexholder = new unsigned int[3* 2 * (resolution-1) * (resolution-1)];
  vertices.SetSize (resolution*resolution);

  opcode_model = new Opcode::Model;

  opcMeshInt.SetNbTriangles (2 * (resolution-1) * (resolution-1));
  opcMeshInt.SetNbVertices((udword)vertices.GetSize());

  // Mesh data
  OPCC.mIMesh = &opcMeshInt;
  OPCC.mSettings.mRules = SPLIT_SPLATTER_POINTS | SPLIT_GEOM_CENTER;
  OPCC.mNoLeaf = true;
  OPCC.mQuantized = true;
  OPCC.mKeepOriginal = false;
  OPCC.mCanRemap = true;
}

void csTerraFormerCollider::MeshCallback (udword triangle_index, 
                          Opcode::VertexPointers& triangle, void* user_data)
{
  csTerraFormerCollider* collider = (csTerraFormerCollider*)user_data;
  udword *tri_array = collider->indexholder;
  Point *vertholder = collider->vertices.GetArray ();
  int index = 3 * triangle_index;
  triangle.Vertex[0] = &vertholder [tri_array[index]] ;
  triangle.Vertex[1] = &vertholder [tri_array[index + 1]];
  triangle.Vertex[2] = &vertholder [tri_array[index + 2]];
}

float csTerraFormerCollider::SampleFloat (float x, float z)
{
  float y;
  former->SampleFloat (stringHeights, x, z, y);
  return y;
}


