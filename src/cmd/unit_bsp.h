#ifndef __UNIT_BSP_H
#define __UNIT_BSP_H
#include <stdio.h>
#include <stdlib.h>

#ifndef PROPHECY
#include "vsfilesystem.h"
#include "gfx/mesh.h"
//#include "unit.h"
#else
#include <vector>
using std::vector;
struct Vector {
  double i,j,k;
};

struct bsp_polygon {
  vector <bsp_vector> v;
};

#endif
#define TRUE 1
#define FALSE 0

//#define BACK -1
//#define FRONT +1
#define INTERSECT 0
#define VPLANE_Z 4
#define VPLANE_Y 2
#define VPLANE_X 1
#define VPLANE_ALL 7

/*

The whole concept of a BSP Tree is that any plane (since it has an
infinite surface) divides the space in two parts: "in front" of the
plane and "behind" the plane (in fact BSP stands for Binary Space
Partition). So a BSP Tree's node, which has the values A,B,C and D of
the plane equation (Ax+By+Cz+d=0), a flag ("in front","behind" or
"undetermined" and two pointers (for the two sub-trees), the left
pointer and the right pointer. 

To build a BSP tree, you start with an empty tree (NULL). Then calculate
a plane (from a poly) and put it in the tree. After this you must add
the other polys: if they are behind the plane or on the plane, take the
left pointer. If they are in front of the plane, take the right pointer.
If the node has a plane equation, put the flag the value of
"undetermined", otherwise put "behind" or "front" as needed. A very
important note: if a given plane intersects a polygon, you must add that
poly's plane both to the left and right subtree (see the recursive
function called put_plane_in_tree3 to see what i mean..
put_plane_in_tree4 is the same, except it handles quadrilaters(sp?) ).

After the tree is built, the game should test the tree for collisions:
supply the ship's coordinates to the BSP Tree test routine and follow
the appropriate paths (left or right). If you reach a "front" value,
then there is no collision. If you reach the "behind" value, then a
collision just occurred and VS should deal with it appropriately:)

Here's a proposed form for the node:

struct BSP_Node
        {
        char flag;
        float a,b,c,d;
        struct BSP_Node * left;
        struct BSP_Node * right;
        }



I'm sending you a small BMP along with the source code to demonstrate
how should a tree be built and tested. If there's anything you don't
understand, e-mail me back and i'll try to explain things a bit better:)

Seeya!:)
*/
#define RIGHT_HANDED -1

enum INTERSECT_TYPE {
    BSPG_BACK =-1,
    BSPG_INTERSECT =0,
    BSPG_FRONT =1,
    BSPG_COPLANAR =2
};



typedef class Vector TVECTOR;


struct bsp_tree {
    double a,b,c,d;
    vector <bsp_polygon> tri;
    vector <bsp_tree> triplane;
    struct bsp_tree * left;
    struct bsp_tree * right;
  bsp_tree(): tri(), triplane() {left = right = NULL;}
};

using namespace VSFileSystem;
extern VSFile fo;
bool intersectionPoint (const bsp_tree &n, const Vector & A, const Vector & B, Vector & res);
enum INTERSECT_TYPE whereIs (const TVECTOR & v, const bsp_tree & temp_node);
void dividePlane (const bsp_polygon & tri, const bsp_tree &unificator, bsp_polygon &back, bsp_polygon &front);
void FreeBSP (bsp_tree ** tree);

// Were static
bsp_tree * buildbsp(bsp_tree * bsp,vector <bsp_polygon>&, vector <bsp_tree>&, char Vplane);
bool Cross (const bsp_polygon &x, bsp_tree &result);
void bsp_stats (bsp_tree * tree);
void write_bsp_tree (bsp_tree *tree,int level=0);//assume open file
//
#ifdef PROPHECY

long getsize (char * name);
void load (vector <bsp_polygon> &tri);
#endif
enum INTERSECT_TYPE where_is_poly(const bsp_tree & temp_node,const bsp_polygon & temp_poly3);

#ifdef PROPHECY
#include <vector>
#else
#include "unit_bsp.h"
#include "configxml.h"
#include "vs_globals.h"
#endif

#endif

