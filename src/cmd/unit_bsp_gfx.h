#include "unit_bsp.h"
template<class UnitType>
void GameUnit<UnitType>::BuildBSPTree(const char *filename, bool vplane, Mesh * hull) {
  bsp_tree * bsp=NULL;
  bsp_tree temp_node;
  vector <bsp_polygon> tri;
  vector <bsp_tree> triplane;
  if (hull!=NULL) {
    hull->GetPolys (tri);
  } else {
    for (int j=0;j<nummesh();j++) {
      meshdata[j]->GetPolys(tri);
    }
  }	
  for (unsigned int i=0;i<tri.size();i++) {
    if (!Cross (tri[i],temp_node)) {
      vector <bsp_polygon>::iterator ee = tri.begin();
      ee+=i;
      tri.erase(ee);
      i--;
      continue;
    }	
    // Calculate 'd'
    temp_node.d = (double) ((temp_node.a*tri[i].v[0].i)+(temp_node.b*tri[i].v[0].j)+(temp_node.c*tri[i].v[0].k));
    temp_node.d*=-1.0;
    triplane.push_back(temp_node);
    //                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3); 
 }
 
 bsp = buildbsp (bsp,tri,triplane, vplane?VPLANE_ALL:0);
 if (bsp) {
   VSFileSystem::changehome();
   VSFileSystem::vs_chdir ("generatedbsp");
   o = VSFileSystem::vs_open (filename, "wb");
   VSFileSystem::vs_cdup();
   VSFileSystem::returnfromhome();
   if (o) {
     write_bsp_tree(bsp,0);
     VSFileSystem::vs_close (o);
     bsp_stats (bsp);
     FreeBSP (&bsp);
   }
 }	

}
