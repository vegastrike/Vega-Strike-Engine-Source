#/*
 * Vega Strike
 * Copyright (C) 2001-2002 Alan Shieh
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
#ifndef GFX_BSP
#define GFX_BSP

//With all fairness, front should be outside and back should be inside
//Hopefully, the deal with virtual nodes will actually work

#include "gfx_transform.h"
#include "stdlib.h"

struct BSPDiskNode {
	float x,y,z,d;
	bool isVirtual;
	bool hasFront;
	bool hasBack;
};

class BSPTree;

class BSPNode {
	Vector n;
	float d;// distance AGAINST the direction of Vector n. 
	bool isVirtual;
	BSPNode *front;
	BSPNode *back;
	float plane_eqn(const Vector &vect) const {
		return n.i*vect.i+n.j*vect.j+n.k*vect.k+d;
	}

public:
	BSPNode(BSPDiskNode *input);
	~BSPNode() {
		if(front!=NULL)
			delete front;
		if(back!=NULL)
			delete back;
	}

	bool intersects(const Vector &start, const Vector &end) const;
	bool intersects(const Vector &pt) const;
	bool intersects(const BSPTree *t1) const;
};

class BSPTree {
	BSPNode *root;
public:
	BSPTree(BSPDiskNode *input);
	BSPTree(const char *filename);
	~BSPTree() {
		delete root;
	}

	//point and tree clipping
	bool intersects(const Vector &start, const Vector &end) const;
	bool intersects(const Vector &pt) const;
	bool intersects(const BSPTree *t1) const;
};

#endif
