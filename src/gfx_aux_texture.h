#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "gfxlib.h"
//#include <gl/gl.h>
#ifdef __cplusplus
#include <string>
#else
#include <string.h>
#endif
using namespace std;

struct Texture{
	unsigned int sizeX;
	unsigned int sizeY;
	unsigned char *data;
	enum {_8BIT, _24BIT, _24BITRGBA} mode;
	unsigned char palette[256*4+1];

	int name;
	int stage;

	Texture *original;
	int refcount;

	BOOL checkold(const string &s);
	void setold();

public:

	Texture(char *,char *, int stage = 0);
	Texture(char * FileName, int stage = 0);
	~Texture()
	{
		if(original == NULL)
		{
			if(data != NULL)
			{
				delete [] data;
				GFXDeleteTexture(name);
				//glDeleteTextures(1, &name);
			}
		}
		else
		{
			original->refcount--;
			if(original->refcount == 0)
				delete original;
		}
	}

	int Bind();
	void Transfer();
	void MakeActive(int stage = 0);

	//void Filter();
	//void NoFilter();
};
Texture * LoadAlphaMap (char *FileName);
Texture * LoadTexture (char * FileName);
Texture * LoadRGBATexture (char * FileNameRGB, char *FileNameA);

void DelTexDat (Texture *); //note does not remove from OpenGL
#endif
