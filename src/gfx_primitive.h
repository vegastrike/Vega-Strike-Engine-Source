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
#ifndef _Primitive_H
#define _Primitive_H
#include <stdio.h>
#include "quaternion.h"
#include "gfx_aux.h"
#include "gfx_transform.h"

inline float readf (FILE *fp) {float temp;	fread (&temp,sizeof(float),1,fp);return temp;}
inline short reads (FILE *fp) {short temp;	fread (&temp,sizeof(short),1,fp);return temp;}
inline int readi (FILE * fp) {int i; fread (&i,sizeof(int),1,fp); return i;}
inline unsigned char readc (FILE *fp) {unsigned char temp;	fread (&temp,sizeof(char),1,fp);return temp;}

class Primitive {
protected:
	//Texture *force;
	//Texture *squad;
  
  Transformation local_transformation;
  Transformation cumulative_transformation;
  Matrix cumulative_transformation_matrix;
  //  scalar_t prevtime;
  //	LONGLONG prevtime;

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

	virtual void Draw(const Transformation &quat = identity_transformation, const Matrix m = identity_matrix) = 0;//GL_T2F_C4F_N3F_V3F

	Vector &Position();
        void SetPosition (float, float, float);
        void SetPosition (const Vector &);
	void SetOrientation(const Vector &p, const Vector &q, const Vector &r);

	const char *get_name() const { return name; }
};
#endif
