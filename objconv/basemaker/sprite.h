#ifndef SPRITE_H
#define SPRITE_H

#include "gfx/quaternion.h"
#include "gfx/vec.h"
#include "gfx/matrix.h"
#include "gfx/hud.h"
#include "vs_globals.h"

#if defined (__APPLE__) || defined (MACOSX)
    #include <GLUT/glut.h>
    #include <OpenGL/glext.h>
#else
    #include <GL/glut.h>
    #include <GL/glext.h>
#endif

#include <iostream>
//#include <stdlib>

class Texture;

class VSSprite
{
    float    xcenter;
    float    ycenter;
    float    widtho2;
    float    heighto2;
    float    maxs, maxt;
    float    rotation;
    Texture *surface;
public: 
	VSSprite( const char *file, enum FILTER texturefilter = BILINEAR, GFXBOOL force = GFXFALSE );
    ~VSSprite();
    bool LoadSuccess()
    {
        return surface != NULL;
    }
    void Draw();
    void DrawHere( Vector &ll, Vector &lr, Vector &ur, Vector &ul );
    void Rotate( const float &rad )
    {
        rotation += rad;
    }
    void SetST( const float s, const float t );
    void SetPosition( const float &x1, const float &y1 );
    void GetPosition( float &x1, float &y1 );
    void SetSize( float s1, float s2 );
    void GetSize( float &x1, float &y1 );
    void SetRotation( const float &rot );
    void GetRotation( float &rot );
    void ReadTexture( FILE *f );
	//float &Rotation(){return rotation;};
    Texture * getTexture()
    {
        return surface;
    }
};

#endif
