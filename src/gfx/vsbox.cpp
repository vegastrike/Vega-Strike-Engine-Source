#include "vsbox.h"
#include "xml_support.h"


/*
inline ostream &operator<<(ostrstream os, const Vector &obj) {
  return os << "(" << obj.i << "," << obj.j << "," << obj.k << ")";
}
*/
string tostring(const Vector &v) {
	return std::string("(") + XMLSupport::tostring(v.i) + ", " + XMLSupport::tostring(v.j) + ", " + XMLSupport::tostring(v.k) + ")";
}

Box::Box(const Vector &corner1, const Vector &corner2) : corner_min(corner1), corner_max(corner2) {
  InitUnit();
  draw_sequence = 3;
  setEnvMap  (GFXFALSE);
  blendSrc = ONE;
  blendDst=ONE;
  Box *oldmesh;
  string hash_key = string("@@Box") + "#" + tostring(corner1) + "#" + tostring(corner2);
  //  cerr << "hashkey: " << hash_key << endl;
  if(0 != (oldmesh = (Box*)meshHashTable.Get(hash_key)))
    {
      *this = *oldmesh;
      oldmesh->refcount++;
      orig = oldmesh;
      return;
    }
  int a=0;
  GFXVertex *vertices = new GFXVertex[18];
#define VERTEX(ax,ay,az) { vertices[a].x = ax; vertices[a].y = ay; vertices[a].z = az;vertices[a].i=ax;vertices[a].j=ay;vertices[a].k=az;vertices[a].s=0;vertices[a].t=0;a++;}

  VERTEX(corner_max.i,corner_min.j,corner_max.k);
  VERTEX(corner_min.i,corner_min.j,corner_max.k);
  VERTEX(corner_min.i,corner_min.j,corner_min.k);
  VERTEX(corner_max.i,corner_min.j,corner_min.k);

  VERTEX(corner_max.i,corner_max.j,corner_min.k);
  VERTEX(corner_min.i,corner_max.j,corner_min.k);
  VERTEX(corner_min.i,corner_max.j,corner_max.k);
  VERTEX(corner_max.i,corner_max.j,corner_max.k);


  a = 8;

  VERTEX(corner_max.i,corner_min.j,corner_max.k);
  VERTEX(corner_min.i,corner_min.j,corner_max.k);
  VERTEX(corner_min.i,corner_max.j,corner_max.k);
  VERTEX(corner_max.i,corner_max.j,corner_max.k);

  VERTEX(corner_max.i,corner_max.j,corner_min.k);
  VERTEX(corner_min.i,corner_max.j,corner_min.k);

  VERTEX(corner_min.i,corner_min.j,corner_min.k);
  VERTEX(corner_max.i,corner_min.j,corner_min.k);


  VERTEX(corner_max.i,corner_min.j,corner_max.k);
  VERTEX(corner_min.i,corner_min.j,corner_max.k);

  int offsets[2];
  offsets[0]=8;
  offsets[1]=10;
  enum POLYTYPE polys[2];
  polys[0]=GFXQUAD;
  polys[1]=GFXQUADSTRIP;
  vlist = new GFXVertexList(polys,18,vertices,2,offsets);
  //  quadstrips[0] = new GFXVertexList(GFXQUADSTRIP,10,vertices);
  delete [] vertices;

  meshHashTable.Put(hash_key, this);
  orig = this;
  refcount++;
  draw_queue = new vector<MeshDrawContext>;
#undef VERTEX
}

void Box::ProcessDrawQueue(int) {
  if(!draw_queue->size()) return;
  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  GFXColor(0.0,.90,.3,.4);
  GFXDisable(LIGHTING);
  GFXDisable (TEXTURE0);
  GFXDisable (TEXTURE1);
  GFXDisable (DEPTHWRITE);
  GFXDisable (CULLFACE);
  //GFXBlendMode(ONE, ONE);

  while(draw_queue->size()) {


    GFXLoadMatrixModel( draw_queue->back().mat);
    draw_queue->pop_back();

  GFXBegin(GFXQUAD);
  GFXColor4f(0.0,1.0,0.0,0.2);

  GFXVertex3f(corner_max.i,corner_min.j,corner_max.k);
  GFXVertex3f(corner_max.i,corner_max.j,corner_max.k);
  GFXVertex3f(corner_min.i,corner_max.j,corner_max.k);
  GFXVertex3f(corner_min.i,corner_min.j,corner_max.k);

  GFXColor4f(0.0,1.0,0.0,0.2);
  GFXVertex3f(corner_min.i,corner_min.j,corner_min.k);
  GFXVertex3f(corner_min.i,corner_max.j,corner_min.k);
  GFXVertex3f(corner_max.i,corner_max.j,corner_min.k);
  GFXVertex3f(corner_max.i,corner_min.j,corner_min.k);

  GFXColor4f(0.0,.70,0.0,0.2);

  GFXVertex3f(corner_max.i,corner_min.j,corner_max.k);
  GFXVertex3f(corner_min.i,corner_min.j,corner_max.k);
  GFXVertex3f(corner_min.i,corner_min.j,corner_min.k);
  GFXVertex3f(corner_max.i,corner_min.j,corner_min.k);

  GFXColor4f(0.0,.70,0.0,0.2);
  GFXVertex3f(corner_max.i,corner_max.j,corner_min.k);
  GFXVertex3f(corner_min.i,corner_max.j,corner_min.k);
  GFXVertex3f(corner_min.i,corner_max.j,corner_max.k);
  GFXVertex3f(corner_max.i,corner_max.j,corner_max.k);

  GFXColor4f(0.0,.90,.3,0.2);
  GFXVertex3f(corner_max.i,corner_max.j,corner_max.k);
  GFXVertex3f(corner_max.i,corner_min.j,corner_max.k);
  GFXVertex3f(corner_max.i,corner_min.j,corner_min.k);
  GFXVertex3f(corner_max.i,corner_max.j,corner_min.k);

  GFXColor4f(0.0,.90,.3,0.2);
  GFXVertex3f(corner_min.i,corner_max.j,corner_min.k);
  GFXVertex3f(corner_min.i,corner_min.j,corner_min.k);
  GFXVertex3f(corner_min.i,corner_min.j,corner_max.k);
  GFXVertex3f(corner_min.i,corner_max.j,corner_max.k);
  GFXEnd();


  /*
    vlist->Draw();
    if(quadstrips!=NULL) {
      for(int a=0; a<numQuadstrips; a++)
	quadstrips[a]->Draw()
	  ;
	  }
  */

  }
  GFXEnable (DEPTHWRITE);
}
