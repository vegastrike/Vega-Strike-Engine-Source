/*
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
#include <stdio.h>
#include "gfx_bsp.h"

//All or's are coded with the assumption that the inside of the object has a much bigger impact than the outside of the object when both need to be analyzed

BSPNode::BSPNode(BSPDiskNode *input) {
	BSPDiskNode *s_input = input; // keep track of current position to allow changes in the read position to propagate up the tree

	isVirtual = input->isVirtual;
	n.i = input->x;
	n.j = input->y;
	n.k = input->z;
	if(input->hasFront) {
		front = new BSPNode(input+1);
	}
	else {
		front = NULL;
	}
	input = s_input;
	if(input->hasBack) {
		back = new BSPNode(input+1);
	}
	else {
		back = NULL;
	}
	s_input++;
}

float BSPNode::intersects(const Vector &start, const Vector &end) const {
	float peq1 = plane_eqn(start);
	float peq2 = plane_eqn(end);

	// n = normal, u = origin, v = direction vector
	if(peq1==0 && peq2==0) { // on the plane; shouldn't collide unless the plane is a virtual plane
	    return ((back!=NULL)?back->intersects(start, end):(start-end).Magnitude()+.000000001)
		|| ((front!=NULL)?front->intersects(start, end):false);		
	}

	if(peq1<=0 && peq2<=0) {
		return (back!=NULL)?back->intersects(start, end):(start-end).Magnitude()+.000000001; // if lies completely within a back leaf, then its inside the object
	}
	else if(peq2>=0 && peq2>=0) {
		return (front!=NULL)?front->intersects(start, end):false; // if lies completely on the outside of a front leaf, then outside object
	}
	else {
		Vector u = start;
		Vector v = end - start;
		float t = ((-d - n.Dot(u))/n.Dot(v)); // cannot be parallel except in exceptionally messed up roundoff errors

		Vector intersection =  start + t * v;
		return  ((back!=NULL)?back->intersects(intersection, end):(start-end).Magnitude()+.00000001)
			|| ((front!=NULL)?front->intersects(start, intersection):false);
	}
}

bool BSPNode::intersects(const Vector &pt, const float err) const {
	float peq = plane_eqn(pt);
	if(peq>0) { 
		return (front!=NULL)?front->intersects(pt,err):false;
	}
	else if(peq==0) { // if on the plane and not virtual, then its not in the object
	    //if(isVirtual) {//daniel080201
			return ((back!=NULL)?back->intersects(pt,err):true)
				|| ((front!=NULL)?front->intersects(pt,err):false);
			//}
			//else {
			//return false; 
			//}
	}
	else {
		return (back!=NULL)?back->intersects(pt,err):true; // if behind and no back children, then there are no more subdivisions and thus this thing is in
	}
}

bool BSPNode::intersects(const BSPTree *t1) const {
	return false;
}

float BSPTree::intersects(const Vector &start, const Vector &end) const {
	return root->intersects(start, end);;
}

bool BSPTree::intersects(const Vector &pt, const float err) const {
	return root->intersects(pt,err);
}

bool BSPTree::intersects(const BSPTree *t1) const {
	return root->intersects(t1);;
}

BSPTree::BSPTree(BSPDiskNode *input) {
	root = new BSPNode(input);
}

BSPTree::BSPTree(const char *filename) {
  FILE *fp = fopen(filename, "rb");

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  int numRecords = size / (sizeof(float)*4+2);
  rewind(fp);
  BSPDiskNode *nodes = new BSPDiskNode[numRecords];
  
  for(int a=0; a<numRecords; a++) {
    fread(&nodes[a].x, sizeof(float), 1, fp);
    fread(&nodes[a].y, sizeof(float), 1, fp);
    fread(&nodes[a].z, sizeof(float), 1, fp);
    fread(&nodes[a].d, sizeof(float), 1, fp);
    nodes[a].isVirtual = false;
    char byte;
    fread(&byte, 1, 1, fp);
    if(byte) nodes[a].hasFront = true;
    else nodes[a].hasFront = false;
    fread(&byte, 1, 1, fp);
    if(byte) nodes[a].hasBack = true;
    else nodes[a].hasBack = false;
  }
  root = new BSPNode(nodes);
  delete [] nodes;
  fclose(fp);
}
