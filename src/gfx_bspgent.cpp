#include <stdio.h>
#include <stdlib.h>
#include "gfx_mesh.h"
#define TRUE 1
#define FALSE 0

#define BACK -1
#define FRONT +1
#define INTERSECT 0


/*
Thanks a lot, it looks great! :)) I think the Y axis is inverted but i
can fix that with no trouble at all :) 

About the collisions, here's how they are defined in WCP:

Fighter/fighter: Sphere collision
Capship/capship: Sphere collision
Capship/asteroid(or corvette): Not defined! That's why you see that
asteroid going through the Kraken in that WCP recon mission:)
Fighter/capship: BSP Tree

The whole concept of a BSP Tree is that any plane (since it has an
infinite surface) divides the space in two parts: "in front" of the
plane and "behind" the plane (in fact BSP stands for Binary Space
Partition). So a BSP Tree's node, which has the values A,B,C and D of
the plane equation (Ax+By+Cz+d=0), a flag ("in front","behind" or
"undetermined" and two pointers (for the two sub-trees), the left
pointer and the right pointer. TIP: I always use the face normals as the
plane normal.. this way you know the plane equation always has the
correct value:)

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


struct bsp_tree
        {
        float a,b,c,d;
        struct bsp_tree * left;
        struct bsp_tree * right;
        };

typedef struct bsp_tree BSP_TREE;

struct bsp_vector
        {
        float x,y,z;
        struct bsp_vector * next;
        };

typedef struct bsp_vector VECTOR;

struct polygon3
        {
        VECTOR a,b,c;
        };

typedef struct polygon3 POLYGON3;

struct polygon4
        {
        VECTOR a,b,c,d;
        };

typedef struct polygon4 POLYGON4;

static void Cross (const polygon3 &x, BSP_TREE &result) {
  Vector v1 (x.c.x-x.a.x,x.c.y-x.a.y,x.c.z-x.a.z);
  Vector v2 (x.b.x-x.a.x,x.b.y-x.a.y,x.b.z-x.a.z);
    result.a = v1.j * v2.k - v1.k * v2.j;
    result.b = v1.k * v2.i - v1.i * v2.k;
    result.c = v1.i * v2.j  - v1.j * v2.i;     
    float size = result.a*result.a+result.b*result.b+result.c*result.c;
    size = ((float)1)/sqrtf (size);
    result.a *=size;
    result.b *=size;
    result.c *=size;
}
static void Cross (const polygon4 &x, BSP_TREE & result) {
  polygon3 tmp;
  memcpy (&tmp,&x,sizeof (polygon3));
  Cross (tmp,result);
}



static FILE * o;
int highestlevel=0;
static BSP_TREE * put_plane_in_tree3(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON3 * temp_poly3);
static BSP_TREE * put_plane_in_tree4(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON4 * temp_poly4);
static int where_is_poly3(BSP_TREE * temp_node,POLYGON3 * temp_poly3);
static int where_is_poly4(BSP_TREE * temp_node,POLYGON4 * temp_poly4);
static void display_bsp_tree(BSP_TREE * tree);
static void write_bsp_tree (BSP_TREE *tree,int level=0);//assume open file



void FreeBSP (BSP_TREE ** tree) {
  if ((*tree)->right)
    FreeBSP(&(*tree)->right);
  if ((*tree)->left)
    FreeBSP(&(*tree)->left);
  free ( (*tree));
  *tree = NULL;
}


void Mesh::BuildBSPTree(const char *filename)

{
  o = fopen (filename, "w+b");
  BSP_TREE * bsp=NULL;
unsigned int i;

BSP_TREE temp_node;

POLYGON3 temp_poly3;
POLYGON4 temp_poly4;
 vector <int> tris;
 vector <int> quads;
 for (i=0;i<xml->triind.size();i++) {
   tris.push_back (xml->triind[i]);
 }
 for (i=0;i<xml->nrmltristrip.size();i++) {
   tris.push_back(xml->nrmltristrip[i]);
 }
 for (i=0;i<xml->nrmltrifan.size();i++) {
   tris.push_back(xml->nrmltrifan[i]);
 }
 for (i=0;i<xml->quadind.size();i++) {
   quads.push_back (xml->quadind[i]);
 }
 for (i=0;i<xml->nrmlquadstrip.size();i++) {
   quads.push_back(xml->nrmlquadstrip[i]);
 }
 
 for (i=0;i<tris.size();i+=3) 
                {
                temp_node.a=0;  // clean temp values...
                temp_node.b=0;
                temp_node.c=0;
                temp_node.d=0;

                temp_poly3.a.x = xml->vertices[tris[i]].x;
                temp_poly3.a.y = xml->vertices[tris[i]].y;
                temp_poly3.a.z = xml->vertices[tris[i]].z;

                temp_poly3.b.x = xml->vertices[tris[i+1]].x;
                temp_poly3.b.y = xml->vertices[tris[i+1]].y;
                temp_poly3.b.z = xml->vertices[tris[i+1]].z;

                temp_poly3.c.x = xml->vertices[tris[i+2]].x;
                temp_poly3.c.y = xml->vertices[tris[i+2]].y;
                temp_poly3.c.z = xml->vertices[tris[i+2]].z;

		Cross (temp_poly3,temp_node);
                                              // Calculate 'd'
                temp_node.d = (float) ((temp_node.a*temp_poly3.a.x)+(temp_node.b*temp_poly3.a.y)+(temp_node.c*temp_poly3.a.z));
                temp_node.d*=-1.0;

                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3);

                }
 for (i=0;i<quads.size();i+=4) 
                {
                temp_node.a=0;  // clean temp values...
                temp_node.b=0;
                temp_node.c=0;
                temp_node.d=0;

                temp_poly4.a.x = xml->vertices[quads[i]].x;
                temp_poly4.a.y = xml->vertices[quads[i]].y;
                temp_poly4.a.z = xml->vertices[quads[i]].z;

                temp_poly4.b.x = xml->vertices[quads[i+1]].x;
                temp_poly4.b.y = xml->vertices[quads[i+1]].y;
                temp_poly4.b.z = xml->vertices[quads[i+1]].z;

                temp_poly4.c.x = xml->vertices[quads[i+2]].x;
                temp_poly4.c.y = xml->vertices[quads[i+2]].y;
                temp_poly4.c.z = xml->vertices[quads[i+2]].z;

                temp_poly4.d.x = xml->vertices[quads[i+3]].x;
                temp_poly4.d.y = xml->vertices[quads[i+3]].y;
                temp_poly4.d.z = xml->vertices[quads[i+3]].z;


		Cross (temp_poly4,temp_node);
                                              // Calculate 'd'
                temp_node.d = (float)((temp_node.a*temp_poly4.a.x)+(temp_node.b*temp_poly4.a.y)+(temp_node.c*temp_poly4.a.z));
                temp_node.d*=-1.0;

                bsp=put_plane_in_tree4(bsp,&temp_node,&temp_poly4);

                }
 write_bsp_tree(bsp,0);
 fclose (o);
 FreeBSP (&bsp);
 fprintf (stderr,"HighestLevel, BSP Tree %d",highestlevel);
}

static BSP_TREE * put_plane_in_tree3(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON3 * temp_poly3)
{
int flag;
BSP_TREE * aux, *temp;

aux = bsp;

if (aux == NULL)
        {
        temp = (BSP_TREE *) malloc (sizeof (BSP_TREE));
        temp->a=temp_node->a;
        temp->b=temp_node->b;
        temp->c=temp_node->c;
        temp->d=temp_node->d;
        temp->left=NULL;
        temp->right=NULL;

        return temp;
        }
else
        {
        flag = where_is_poly3(aux,temp_poly3);
        if (flag == BACK)
                {
                aux->left = put_plane_in_tree3(aux->left,temp_node,temp_poly3);
                return aux;
                }
        else if (flag == FRONT)
                {
                aux->right = put_plane_in_tree3(aux->right,temp_node,temp_poly3);
                return aux;
                }
        else if (flag == INTERSECT)
                {
                aux->left = put_plane_in_tree3(aux->left,temp_node,temp_poly3);
                aux->right = put_plane_in_tree3(aux->right,temp_node,temp_poly3);
                return aux;
                }
	return NULL;
        }
}


static BSP_TREE * put_plane_in_tree4(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON4 * temp_poly4)
{
int flag;
BSP_TREE * aux, *temp;

aux = bsp;

if (aux == NULL)
        {
        temp = (BSP_TREE *) malloc (sizeof (BSP_TREE));
        temp->a=temp_node->a;
        temp->b=temp_node->b;
        temp->c=temp_node->c;
        temp->d=temp_node->d;
        temp->left=NULL;
        temp->right=NULL;
        
        return temp;
        }
else
        {
        flag = where_is_poly4(aux,temp_poly4);
        if (flag == BACK)
                {
                aux->left = put_plane_in_tree4(aux->left,temp_node,temp_poly4);
                return aux;
                }
        else if (flag == FRONT)
                {
                aux->right = put_plane_in_tree4(aux->right,temp_node,temp_poly4);
                return aux;
                }
        else if (flag == INTERSECT)
                {
                aux->left = put_plane_in_tree4(aux->left,temp_node,temp_poly4);
                aux->right = put_plane_in_tree4(aux->right,temp_node,temp_poly4);
                return aux;
                }
	return NULL;
        }
}


static int where_is_poly3(BSP_TREE * temp_node,POLYGON3 * temp_poly3)

{
int flag[3]={0,0,0};

flag[0] = (int)((temp_node->a)*(temp_poly3->a.x))+((temp_node->b)*(temp_poly3->a.y))+((temp_node->c)*(temp_poly3->a.z))+(temp_node->d);
flag[1] = (int)((temp_node->a)*(temp_poly3->b.x))+((temp_node->b)*(temp_poly3->b.y))+((temp_node->c)*(temp_poly3->b.z))+(temp_node->d);
flag[2] = (int)((temp_node->a)*(temp_poly3->c.x))+((temp_node->b)*(temp_poly3->c.y))+((temp_node->c)*(temp_poly3->c.z))+(temp_node->d);

if (flag[0] <=0 && flag[1] <=0 && flag[2] <=0)
        return BACK;
else if (flag[0] >=0 && flag[1] >=0 && flag[2] >=0)
        return FRONT;
else
        return INTERSECT;

}

static int where_is_poly4(BSP_TREE * temp_node,POLYGON4 * temp_poly4)

{
int flag[4]={0,0,0,0};

flag[0] = (int)((temp_node->a)*(temp_poly4->a.x))+((temp_node->b)*(temp_poly4->a.y))+((temp_node->c)*(temp_poly4->a.z))+(temp_node->d);
flag[1] = (int)((temp_node->a)*(temp_poly4->b.x))+((temp_node->b)*(temp_poly4->b.y))+((temp_node->c)*(temp_poly4->b.z))+(temp_node->d);
flag[2] = (int)((temp_node->a)*(temp_poly4->c.x))+((temp_node->b)*(temp_poly4->c.y))+((temp_node->c)*(temp_poly4->c.z))+(temp_node->d);
flag[3] = (int)((temp_node->a)*(temp_poly4->d.x))+((temp_node->b)*(temp_poly4->d.y))+((temp_node->c)*(temp_poly4->d.z))+(temp_node->d);

if (flag[0] <=0 && flag[1] <=0 && flag[2] <=0 && flag[3]<=0)
        return BACK;
else if (flag[0] >=0 && flag[1] >=0 && flag[2] >=0 && flag[3] >=0)
        return FRONT;
else
        return INTERSECT;

}

static void display_bsp_tree(BSP_TREE * tree)

{
if (tree !=NULL)
        {
        printf ("CHNK \"DATA\"\n");
        printf ("{\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        
        printf ("FLOAT %f\n",tree->a);
        printf ("FLOAT %f\n",tree->b);
        printf ("FLOAT %f\n",tree->c);
        printf ("FLOAT %f\n",tree->d);
        
        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("}\n");

printf ("FORM \"BACK\"\n{\n");
if (tree->left != NULL ) display_bsp_tree (tree->left);
else
        {
        printf ("CHNK \"DATA\"\n{\n");
        printf ("BYTE 1\nBYTE 0\nBYTE 0\nBYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n}\n");/*

        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("BYTE 255\n}\n");*/

        }
printf ("\n}\nFORM \"FRNT\"\n{\n");
if (tree->right != NULL ) display_bsp_tree (tree->right);
else
        {
        printf ("CHNK \"DATA\"\n{\n");
        printf ("BYTE 2\nBYTE 0\nBYTE 0\nBYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");
        printf ("BYTE 0\n");

        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("BYTE 255\n");
        printf ("BYTE 255\n}\n");

        }
printf ("}\n");

        }

}


static void wrtf (float f) {fwrite (&f,sizeof (float),1,o);}
static void wrtb (bool b) {fwrite (&b,sizeof (bool),1,o);}

static void write_bsp_tree (BSP_TREE *tree,int level)//assume open file
{
	level++;
	wrtf (tree->a);
	wrtf (tree->b);
	wrtf (tree->c);
	wrtf (tree->d);
	if (tree->right)
		wrtb (true);
	else
		wrtb(false);
	if (tree->left)
		wrtb(true);
	else
		wrtb(false);
	if (tree->right)
		write_bsp_tree (tree->right,level);
	else
	{
		if (tree->left) 
		{
			wrtf (0);
			wrtf (0);
			wrtf (0);
			wrtf (0);
			wrtb (false);
			wrtb (false);
		}
	}
	if (tree->left)
		write_bsp_tree (tree->left,level);
	else
	{
		if (tree->right)
		{
			wrtf (0);
			wrtf (0);
			wrtf (0);
			wrtf (0);
			wrtb (false);
			wrtb (false);
		}
	}
	if (level > highestlevel)
		highestlevel = level;
}
