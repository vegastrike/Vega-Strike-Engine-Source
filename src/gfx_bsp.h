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
	float d;
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
	~BSPTree() {
		delete root;
	}

	//point and tree clipping
	bool intersects(const Vector &start, const Vector &end) const;
	bool intersects(const Vector &pt) const;
	bool intersects(const BSPTree *t1) const;
};

#endif