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

void SphereMesh::ProcessDrawQueue(int whichpass,int whichdrawqueue) {
  static GFXColor spherecol (getSphereColor ());
  if (blendSrc!=ONE||blendDst!=ZERO) {
    GFXPolygonOffset (0,-2);
  }
  GFXColor tmpcol (0,0,0,1);
  GFXGetLightContextAmbient(tmpcol);
  GFXLightContextAmbient(spherecol);
  Mesh::ProcessDrawQueue (whichpass,whichdrawqueue);
  GFXLightContextAmbient(tmpcol);
  GFXPolygonOffset (0,0);
    

}
void CityLights::SelectCullFace (int whichdrawqueue) {
  GFXEnable(CULLFACE);
}

void CityLights::ProcessDrawQueue(int whichpass,int whichdrawqueue) {
  GFXPolygonOffset (0,-1);
  const GFXColor citycol (1,1,1,1);
  GFXColor tmpcol (0,0,0,1);
  GFXGetLightContextAmbient(tmpcol);
  GFXLightContextAmbient(citycol);
  Mesh::ProcessDrawQueue (whichpass,whichdrawqueue);
  GFXLightContextAmbient(tmpcol);
  GFXPolygonOffset (0,0);
}
