#ifndef GFXVERTEX_H
#define GFXVERTEX_H

struct glVertex {
	float s;
	float t;
	float i;
	float j;
	float k;
	float x;
	float y;
	float z;
	
	//glVertex(){};
	glVertex &SetTexCoord(float s, float t) {this->s = s; this->t = t; return *this;}
	glVertex &SetNormal(const Vector &norm) {i = norm.i; j = norm.j; k = norm.k; return *this;}
	glVertex &SetVertex(const Vector &vert) {x = vert.i; y = vert.j; z = vert.k; return *this;}
};

#endif
