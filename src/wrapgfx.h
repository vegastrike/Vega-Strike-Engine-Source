/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
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
#ifndef _WrapGFX_H
#define _WrapGFX_H


#define NUM_LIGHT	8
#define NUM_FORCES	64

#include "gfx_camera.h"
#include "gfx_primitive.h"
#include "in_main.h"
#include "star_system.h"

/*
struct Light{
	Vector direction;

	float r,g,b;
	BOOL enabled;
	Light()
	{
		r = 0;
		g = 0;
		b = 0;
		enabled = GFXFALSE;
	}
	Light(Vector dir, float r, float g, float b)
	{
		this->direction = dir;
		this->r = r;
		this->g = g;
		this->b = b;
	}
};
*/

//extern StarSystem star_system;
class WrapGFX {
protected:

	char * hPalette;
	GFXBOOL  done;

	Camera hud_camera;

	void StartGL();
	GFXBOOL active;
        StarSystem * star_system;
	//	Light lights[NUM_LIGHT];
	int numlights;
	//Mouse *mouse;
        Texture *LightMap[6]; //type Texture
        Texture *ForceLogo;
        Texture *SquadLogo;
	
public:
	//move back to private!!!
	Primitive *topobject; // the top object of the object list
  void activateLightMap();
  Texture * getForceLogo () {return ForceLogo;};
  Texture * getSquadLogo () {return SquadLogo;};
	WrapGFX(int argc, char **argv);
	~WrapGFX();

	void StartGFX();

	void StartDraw();
	void EndDraw();
        void Loop(void main_loop());
  StarSystem* activeStarSystem() {
    return star_system;
  }
	void SelectCamera(int cam)
	{
	  if (star_system!=NULL) {
	    star_system->SelectCamera(cam);
	  }
	  //if(cam<NUM_CAM&&cam>=0)
	  //	currentcamera = cam;
	}

	Camera *AccessCamera(int num)
	{
	  if (star_system!=NULL) {
	    return star_system->AccessCamera(num);
	  } else
	    return NULL;
	}

	Camera *AccessCamera()
	{
	  if (star_system!=NULL) {
		return star_system->AccessCamera();
	  } else
	    return NULL;
	}
	Camera *AccessHudCamera() { return &hud_camera; }

	void SetViewport()
	{
	  	  if (star_system!=NULL) {
		    star_system->SetViewport();
		  }
		  //		cam[currentcamera].UpdateGFX(); //sets the cam to the current matrix
	}

	void Continue()
	{
		
	}
	void Switch();

};

#endif
