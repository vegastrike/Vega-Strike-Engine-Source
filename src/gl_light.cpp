/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <GL/gl.h>
#include "gfxlib.h"
#include "vegastrike.h"
#include <list>

#define GFX_DIFFUSE 1
#define GFX_SPECULAR 2
#define GFX_AMBIENT 4
#define GFX_SPOTLIGHT 8;
#define GFX_ATTENUATED 32+64;


#define GFX_MAX_LIGHTS 8
//for OpenGL

struct gfx_light_loc {///yay 32 bytes! 1 cache line
  float vect[4];//last is w for positional, otherwise 3 for spec
  float attenuate[3];
  float intensity;///<=0 if disabled
  //  bool enabled;
  gfx_light_loc() {
    vect[0]=vect[1]=vect[2]=vect[3]=0;
    attenuate[0]=attenuate[1]=attenuate[2]=0;
  }
};

struct gfx_light_data {
  int target;
  bool enabled;
  float spot[3];
  float exp;
  float diffuse [4];
  int options;//diffuse? spec? amb? spot? atten?
  float specular[4];
  float ambient[4];
  gfx_light_data() {
    diffuse[0]=diffuse[1]=diffuse[2]=0;
    specular[0]=specular[1]=specular[2]=0;
    options = ambient[0]=ambient[1]=ambient[2]=0;
    spot[0]=spot[1]=spot[2]=0;
    exp =diffuse[3]=specular[3]=ambient[3]=1;
    enabled=false;
    target =-1;//physical GL light its saved in
  }
};
BOOL GFXNewLight (int &light) {
  //  diffuse[0]=diffuse[1]=diffuse[2]=diffuse[3]=diffuse[4]=diffuse[5]=0;
  //push bakc shit
  return TRUE;
}



float cutoff=.05;
int currentContex=0;
int GLLights[GFX_MAX_LIGHTS];
bool GLLightState[GFX_MAX_LIGHTS];
vector <vector <gfx_light_loc> > _global_lights_loc;
vector <vector <gfx_light_data> > _global_lights_dat;
vector <vector <gfx_light_loc> > _local_lights_loc;
vector <vector <gfx_light_data> > _local_lights_dat;

vector <gfx_light_loc> * _glights_loc=NULL;
vector <gfx_light_data> * _glights_dat=NULL;
vector <gfx_light_loc> * _llights_loc=NULL;
vector <gfx_light_data> * _llights_dat=NULL;
BOOL GFXCreateLightContext (int & con_number) {
  con_number = _global_lights_loc.size();
  _global_lights_loc.push_back (vector <gfx_light_loc>());
  _global_lights_dat.push_back (vector <gfx_light_data>());

  _local_lights_loc.push_back (vector <gfx_light_loc>());
  _local_lights_dat.push_back (vector <gfx_light_data>());
  _glights_loc = &_global_lights_loc[con_number];
  _glights_dat = &_global_lights_dat[con_number];
  _llights_loc = &_local_lights_loc[con_number];
  _llights_dat = &_local_lights_dat[con_number];
  return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXDeleteLightContext(int con_number) {
  _global_lights_loc[con_number]=vector <gfx_light_loc>();//don't want to mess up other context numbers
  _global_lights_dat[con_number]=vector <gfx_light_data>();//don't want to mess up other context numbers
  _local_lights_loc[con_number]=vector <gfx_light_loc>();
  _local_lights_dat[con_number]=vector <gfx_light_data>();
  return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXEnableGlobalLight (int light) {
  //  if (light<0||light>=_glights->size()) 
  //    return FALSE;
  (*_glights_dat)[light].enabled=true;
  (*_glights_loc)[light].intensity = fabs ((*_glights_loc)[light].intensity);
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXEnableLight (int light) {
  //  assert (_llights);
  //  if (light<0||light>=_llights->size()) 
  //    return FALSE;
  (*_llights_dat)[light].enabled=true;
  (*_llights_loc)[light].intensity = fabs ((*_llights_loc)[light].intensity);
  return TRUE;
}


BOOL /*GFXDRVAPI*/ GFXDisableGlobalLight (int light) {
  //  if (light<0||light>=_glights->size()) 
  //    return FALSE;
  (*_glights_dat)[light].enabled=false;
  (*_glights_loc)[light].intensity =- fabs ((*_glights_loc)[light].intensity);
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXDisableLight (int light) {
  //  assert (_llights);
  //  if (light<0||light>=_llights->size()) 
  //    return FALSE;
  (*_llights_dat)[light].enabled=false;
  (*_llights_loc)[light].intensity =-fabs ((*_llights_loc)[light].intensity);
  return TRUE;
}


BOOL /*GFXDRVAPI*/ GFXPickLights (const Vector& location) {
  //picks 1-5 lights to use
  return TRUE;
}

//minor differences between d3d and gl direction and position, will be dealt with if need be
BOOL /*GFXDRVAPI*/ GFXSetLightPosition(int light, const Vector &position_)
{
	float position[4];
	position[0] = position_.i;
	position[1] = position_.j;
	position[2] = position_.k;
	position[3] = 1;
	glLightfv(GL_LIGHT0+light, GL_POSITION, position);
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSetLightDirection(int light, const Vector &direction)
{
	float position[4];
	position[0] = direction.i;
	position[1] = direction.j;
	position[2] = direction.k;
	position[3] = 0;
	glLightfv(GL_LIGHT0+light, GL_POSITION, position);
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSetLightProperties(gfx_light_data &curlight,gfx_light_loc &curlightloc, enum LIGHT_TARGET lighttarg, const GFXColor &color) {
  if (color.r==color.g&&color.g==color.b&&color.b==0) {
    curlight.options &= (~lighttarg);
  } else {
    curlight.options |= (lighttarg);
  }
  switch (lighttarg) {
  case DIFFUSE:
    curlight.diffuse[0]=color.r;
    curlight.diffuse[1]=color.g;
    curlight.diffuse[2]=color.b;
    curlight.diffuse[3]=color.a;
    break;
  case SPECULAR:
    curlight.specular[0]=color.r;
    curlight.specular[1]=color.g;
    curlight.specular[2]=color.b;
    curlight.specular[3]=color.a;
    break;
  case AMBIENT:
    curlight.ambient[0]=color.r;
    curlight.ambient[1]=color.g;
    curlight.ambient[2]=color.b;
    curlight.ambient[3]=color.a;
    break;
  case POSITION:
    curlightloc.vect[0]=color.r;
    curlightloc.vect[1]=color.g;
    curlightloc.vect[2]=color.b;
    curlightloc.vect[3]=0; //assume set attenuate AFTER position set
    //////????    curlight.options&=(~(POSITION));
  case DIRECTION:
    curlight.spot[0]=color.r;
    curlight.spot[1]=color.g;
    curlight.spot[2]=color.b;
    curlight.exp = color.a;///exponent!!!!!!
    /////??????    curlight.options&=(~(DIRECTION));//are position and direction mutually exclusive?
    break;
  case ATTENUATE:
    curlightloc.attenuate[0]=color.r;
    curlightloc.attenuate[1]=color.g;
    curlightloc.attenuate[2]=color.b;
    curlightloc.vect[3]=1;
    break;
  }
  if (lighttarg==AMBIENT||lighttarg==SPECULAR||lighttarg==DIFFUSE) {
    curlightloc.intensity=max (curlight.specular[0]+curlight.specular[1]+curlight.specular[2],max (curlight.diffuse[0]+curlight.diffuse[1]+curlight.diffuse[2],curlight.ambient[0]+curlight.ambient[1]+curlight.ambient[2]))*.333333333333333333333333;//calculate new cuttof val for light 'caching'
    if (!curlight.enabled)
      curlightloc.intensity=-curlightloc.intensity;
  }
  return TRUE;
}
BOOL GFXSetGlobalLight(int light, enum LIGHT_TARGET lightarg, const GFXColor & color) {
  return GFXSetLightProperties ((*_glights_dat)[light],(*_glights_loc)[light],lightarg ,color);
}

BOOL GFXSetLight(int light, enum LIGHT_TARGET lightarg, const GFXColor & color) {
  return GFXSetLightProperties ((*_llights_dat)[light],(*_llights_loc)[light], lightarg ,color);
}


