#ifndef _Primitive_H
#define _Primitive_H
#include <stdio.h>
//#include <gl\gl.h>
//#include <gl\glaux.h>
#include "gfx_aux.h"
#include "gfx_transform.h"
//class Primitive;

inline float readf (FILE *fp) {float temp;	fread (&temp,sizeof(float),1,fp);return temp;}
inline short reads (FILE *fp) {short temp;	fread (&temp,sizeof(short),1,fp);return temp;}
inline int readi (FILE * fp) {int i; fread (&i,sizeof(int),1,fp); return i;}
inline unsigned char readc (FILE *fp) {unsigned char temp;	fread (&temp,sizeof(char),1,fp);return temp;}

class Primitive {
protected:
	//Texture *force;
	//Texture *squad;

	Vector pos;

  //  scalar_t prevtime;
  //	LONGLONG prevtime;

	Matrix orientation;
	Matrix translation;
	Matrix transformation;
	Matrix stackstate;

	void InitPrimitive();

	char name[64];

	Logo *forcelogos;
	int numforcelogo;

	Logo *squadlogos;
	int numsquadlogo;

	//void UpdateGL();
public:
	Primitive ();
	virtual ~Primitive();

	virtual void Draw();//GL_T2F_C4F_N3F_V3F
	virtual void Draw(const Vector &pp, const Vector &pq, const Vector &pr, const Vector &ppos){
		Draw();
	}

	Vector &Position();

};
#endif
