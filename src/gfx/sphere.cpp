#include "sphere.h"
#include "ani_texture.h"
#include "vegastrike.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "vsfilesystem.h"
#include "xml_support.h"
#ifndef M_PI
#define M_PI 3.1415926536F
#endif
#include "gfx/camera.h"

using XMLSupport::tostring;

static GFXColor getSphereColor () {
  float color[4];
  vs_config->getColor ("planet_ambient",color);
  GFXColor tmp (color[0],color[1],color[2],color[3]);
  return tmp;
}

#ifdef PARTITIONED_Z_BUFFER
void SphereMesh::ProcessDrawQueue(int whichpass,int whichdrawqueue,float zmin,float zmax) {
#else
void SphereMesh::ProcessDrawQueue(int whichpass,int whichdrawqueue) {
#endif
  if (whichpass==1) return; //Mesh already draws pass 1 in pass 0

  static GFXColor spherecol (getSphereColor ());
  if (blendSrc!=ONE||blendDst!=ZERO) {
    GFXPolygonOffset (0,-1);
  }
  GFXColor tmpcol (0,0,0,1);
  GFXGetLightContextAmbient(tmpcol);
  GFXLightContextAmbient(spherecol);
#ifdef PARTITIONED_Z_BUFFER
  Mesh::ProcessDrawQueue (whichpass,whichdrawqueue,zmin,zmax);
#else
  Mesh::ProcessDrawQueue (whichpass,whichdrawqueue);
#endif
  GFXLightContextAmbient(tmpcol);
  GFXPolygonOffset (0,0);
    

}
void CityLights::SelectCullFace (int whichdrawqueue) {
  GFXEnable(CULLFACE);
}

#ifdef PARTITIONED_Z_BUFFER
void CityLights::ProcessDrawQueue(int whichpass,int whichdrawqueue,float zmin,float zmax) {
#else
void CityLights::ProcessDrawQueue(int whichpass,int whichdrawqueue) {
#endif
  GFXPolygonOffset (0,-1);
  const GFXColor citycol (1,1,1,1);
  GFXColor tmpcol (0,0,0,1);
  GFXGetLightContextAmbient(tmpcol);
  GFXLightContextAmbient(citycol);
#ifdef PARTITIONED_Z_BUFFER
  Mesh::ProcessDrawQueue (whichpass,whichdrawqueue,zmin,zmax);
#else
  Mesh::ProcessDrawQueue (whichpass,whichdrawqueue);
#endif
  GFXLightContextAmbient(tmpcol);
  GFXPolygonOffset (0,0);
}
