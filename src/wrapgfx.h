#ifndef _WrapGFX_H
#define _WrapGFX_H

#define NUM_CAM		10
#define NUM_LIGHT	8
#define NUM_FORCES	64

#include "gfx_camera.h"
#include "gfx_primitive.h"
#include "in_main.h"

struct Light{
	Vector direction;

	float r,g,b;
	BOOL enabled;
	Light()
	{
		r = 0;
		g = 0;
		b = 0;
		enabled = FALSE;
	}
	Light(Vector dir, float r, float g, float b)
	{
		this->direction = dir;
		this->r = r;
		this->g = g;
		this->b = b;
	}
};

class WrapGFX {
protected:

/*
	static int _NumLightSources; //Number of light sources
	static float _OneOverNumberLight; //calculate this as 1/_NumLightSources to increase light rendering speed a LOT
	static Light *_LightSources; //Dynamically allocated array of light sources
	static float _CamTransConst;

	static HGLRC glRC;
	static HPALETTE hPalette;
	static BOOL  done;

	static Vector _CamCoord, _CamP, _CamQ, _CamR;
*/
	//int _NumLightSources; //Number of light sources
	//float _OneOverNumberLight; //calculate this as 1/_NumLightSources to increase light rendering speed a LOT
	//Light *_LightSources; //Dynamically allocated array of light sources
	//float _CamTransConst;

	char * hPalette;
	BOOL  done;

	//Vector _CamCoord, _CamP, _CamQ, _CamR;
	
	//Matrix view;
	//Matrix modelview[16];
	Camera cam[NUM_CAM];
	int currentcamera;

	BOOL StartGL();
	BOOL active;
	
	Light lights[NUM_LIGHT];
	int numlights;
	//Mouse *mouse;
        Texture *LightMap; //type Texture
        Texture *ForceLog;
        Texture *SquadLog;
	
public:
	//move back to private!!!
	Primitive *topobject; // the top object of the object list
  Texture *getLightMap() {return LightMap;};
  Texture * getForceLog () {return ForceLog;};
  Texture * getSquadLog () {return SquadLog;};
	WrapGFX(int argc, char **argv);
	~WrapGFX();

	BOOL StartGFX();

	void StartDraw();
	void EndDraw();

	void SelectCamera(int cam)
	{
		if(cam<NUM_CAM&&cam>=0)
			currentcamera = cam;
	}

	Camera *AccessCamera(int num)
	{
		if(num<NUM_CAM&&num>=0)
			return &cam[num];
		else
			return NULL;
	}

	Camera *AccessCamera()
	{
		return &cam[currentcamera];
	}

	void SetViewport()
	{
		cam[currentcamera].UpdateGFX(); //sets the cam to the current matrix
	}

	void Continue()
	{
		
	}
	void Switch();

	void SetLight(int num);
	void SetLight(int num, Light &newparams);
	void EnableLight(int num);
	void DisableLight(int num);
};

#endif
