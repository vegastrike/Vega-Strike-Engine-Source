#include <stdio.h>
#include <stdlib.h>
#include "gfx_mesh.h"
#define TRUE 1
#define FALSE 0

#define BACK -1
#define FRONT +1
#define INTERSECT 0


/*

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



struct bsp_vector
        {
        float x,y,z;
        struct bsp_vector * next;
        };

typedef struct bsp_vector VECTOR;

struct bsp_polygon
        {
        vector <bsp_vector> v;
        };

struct bsp_plane {
  float a,b,c,d;
};
struct bsp_tree {
    float a,b,c,d;
    vector <bsp_polygon> tri;
    vector <bsp_plane> triplane;
    struct bsp_tree * left;
    struct bsp_tree * right;
 bsp_tree operator = (bsp_plane p) {
     a=p.a;
     b = p.b;
     c = p.c;
     d = p.d;
     return *this;
 } 
};






static void Cross (const bsp_polygon &x, bsp_plane &result) {
  Vector v1 (x.v[2].x-x.v[0].x,x.v[2].y-x.v[0].y,x.v[2].z-x.v[0].z);
  Vector v2 (x.v[1].x-x.v[0].x,x.v[1].y-x.v[0].y,x.v[1].z-x.v[0].z);
    result.a = v1.j * v2.k - v1.k * v2.j;
    result.b = v1.k * v2.i - v1.i * v2.k;
    result.c = v1.i * v2.j  - v1.j * v2.i;     
    float size = result.a*result.a+result.b*result.b+result.c*result.c;
    size = ((float)1)/sqrtf (size);
    result.a *=size;
    result.b *=size;
    result.c *=size;
}


float Dot (const bsp_vector & A, const bsp_vector & B) {
    return A.x *B.x + A.y*B.y+A.z*B.z;
}


static FILE * o;
int highestlevel=0;
//ax + by + cz =0;  A.x + (B.x - A.x)k = x;A.y + (B.y - A.y)k = y;A.z + (B.z - A.z)k = z;
// x*A.x + b*B.y + c*C.z + d + k*(a*B.x - a*A.x + b*B.y - b&A.y + c*B.z - c*A.z) = 0;
// k = (A * n + d) / (A * n - B * n) 
//
enum INTERSECT_TYPE {
    BSPG_BACK =-1,
    BSPG_INTERSECT =0,
    BSPG_FRONT =1 
};
bool intersectionPoint (const bsp_plane &n, const bsp_vector & A, const bsp_vector & B, bsp_vector & res) {
    float k = A.x*n.a + A.y*n.b+A.z*n.c;
    k = (k + n.d ) / (k - (B.x * n.a + B.y * n.b + B.z * n.c)); 
    //assume magnitude (n.a,n.b,n.c) == 1
    if (k<0||k>1) {//lies outside the segment
	return false;
    }
    res.x = A.x + k*(B.x - A.x);
    res.y = A.y + k*(B.y - A.y);
    res.z = A.z + k*(B.z - A.z);
    return true;
}

enum INTERSECT_TYPE whereIs (const VECTOR & v, const bsp_plane & temp_node) {
     float tmp = ((temp_node.a)*(v.x))+((temp_node.b)*(v.y))+((temp_node.c)*(v.z))+(temp_node.d);
     if (tmp < 0) {
	 return BSPG_BACK;
     }else if (tmp >0) {
	 return BSPG_FRONT;
     }else return BSPG_INTERSECT;
}

static bsp_tree * put_plane_in_tree(bsp_tree * bsp,bsp_tree * temp_node,bsp_polygon * temp_poly3);
static enum INTERSECT_TYPE where_is_poly(bsp_tree * temp_node,bsp_polygon * temp_poly3);

static void display_bsp_tree(bsp_tree * tree);
static void write_bsp_tree (bsp_tree *tree,int level=0);//assume open file

//can divide 3 or 4 sized planes
void dividePlane (const bsp_polygon & tri, const bsp_plane &unificator, bsp_polygon &back, bsp_polygon &front) {
    enum INTERSECT_TYPE oldflag;
    enum INTERSECT_TYPE flag;
    bsp_vector int_point;
    front.v = vector <VECTOR> ();
    back.v = vector <VECTOR> ();
    for (unsigned int i=0;i<tri.v.size();i++) {
	flag = whereIs (tri.v[i], unificator);
	if (flag==BSPG_INTERSECT) {
	    front.v.push_back (tri.v[i]);
	    back.v.push_back (tri.v[i]);
	} else {
	    if (oldflag!=BSPG_INTERSECT&&i!=0&&flag!=oldflag) {
		//need to add the intersection point in!
		intersectionPoint (unificator, tri.v[i-1], tri.v[i], int_point);
		front.v.push_back (int_point);
		back.v.push_back (int_point);
	    }
	    if (flag==BSPG_FRONT) {
		front.v.push_back (tri.v[i]);
	    }else {
		back.v.push_back (tri.v[i]);
	    }	    
	}
	oldflag = flag;
    }
    flag = whereIs (tri.v[0],unificator);//check the corner case if the intersection point was between last and first points (2/3 the time in triangles)
    if (oldflag!=BSPG_INTERSECT&&flag!=BSPG_INTERSECT&&flag!=oldflag) {
	//need to add the intersection point in!
	intersectionPoint (unificator, tri.v[tri.v.size()-1], tri.v[0], int_point);
	front.v.push_back (int_point);
	back.v.push_back (int_point);
    }
}
void dividePlane (const bsp_polygon & tri, const bsp_tree &unificator, bsp_polygon &back, bsp_polygon &front) {
    bsp_plane tmp;
    tmp.a  = unificator.a;
    tmp.b  = unificator.b;
    tmp.c  = unificator.c;
    tmp.d  = unificator.d;
    dividePlane (tri,tmp,back,front);
}

void FreeBSP (bsp_tree ** tree) {
  if ((*tree)->right)
    FreeBSP(&(*tree)->right);
  if ((*tree)->left)
    FreeBSP(&(*tree)->left);
  free ( (*tree));
  *tree = NULL;
}

static bsp_tree * buildbsp(bsp_tree * bsp,vector <bsp_polygon>&, vector <bsp_plane>&);
void Mesh::BuildBSPTree(const char *filename)

{
  o = fopen (filename, "w+b");
  bsp_tree * bsp=NULL;
unsigned int i;

bsp_plane temp_node;

bsp_polygon temp_poly3;
bsp_polygon temp_poly4;
 vector <int> tris;
 vector <bsp_polygon> tri;
 vector <bsp_plane> triplane;
 int nums;
 vector <int> quads;
 vector <int> * curs;
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
 curs = &tris;
 nums = 3;
 for (int kk=0;kk<2;kk++) {
     for (i=0;i<(*curs).size();i+=nums) {
	 for (int j=0;j<nums;j++) {
	     temp_poly3.v.push_back (bsp_vector());
	     temp_poly3.v[j].x = xml->vertices[(*curs)[i+j]].x;
	     temp_poly3.v[j].y = xml->vertices[(*curs)[i+j]].y;
	     temp_poly3.v[j].z = xml->vertices[(*curs)[i+j]].z;
	 }
	 Cross (temp_poly3,temp_node);
	 // Calculate 'd'
	 temp_node.d = (float) ((temp_node.a*temp_poly3.v[0].x)+(temp_node.b*temp_poly3.v[0].y)+(temp_node.c*temp_poly3.v[0].z));
	 temp_node.d*=-1.0;
	 tri.push_back(temp_poly3);
	 triplane.push_back(temp_node);
	 //                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3);
	 
     }
     nums = 4;
     curs = &quads;
 }
 bsp = buildbsp (bsp,tri,triplane);
 write_bsp_tree(bsp,0);
 fclose (o);
 FreeBSP (&bsp);
 fprintf (stderr,"HighestLevel, BSP Tree %d",highestlevel);
}

static bsp_tree * buildbsp(bsp_tree * bsp,vector <bsp_polygon> &tri, vector <bsp_plane> &triplane) {

  bsp_tree * temp;
  vector <bsp_polygon> trileft;
  vector <bsp_plane> triplaneleft;
  vector <bsp_polygon> triright;
  vector <bsp_plane> triplaneright;
  bsp_polygon left_int;
  bsp_polygon right_int;
  unsigned int select = rand ()%(tri.size());
  if (select >=tri.size()) {
      fprintf (stderr,"Error Selecting tri for splittage");
      return NULL;
  }
  temp = (bsp_tree *) malloc (sizeof (bsp_tree));
  temp->a=triplane[select].a;
  temp->b=triplane[select].b;
  temp->c=triplane[select].c;
  temp->d=triplane[select].d;
  
  unsigned int i;
  for (i=0;i<tri.size();i++) {
    enum INTERSECT_TYPE flag = where_is_poly(temp,&tri[i]);    
    switch (flag) {
    case BSPG_BACK:
	trileft.push_back (tri[i]);
	triplaneleft.push_back(triplane[i]);
        break;
    case BSPG_FRONT:
	triright.push_back (tri[i]);
	triplaneright.push_back(triplane[i]);
        break;
    case BSPG_INTERSECT:
	dividePlane(tri[i],*temp,left_int,right_int);
	if (left_int.v.size()>2) {
	    triplaneleft.push_back (triplane[i]);
	    trileft.push_back (left_int);
	}
	if (right_int.v.size()>2) {
	    triplaneright.push_back (triplane[i]);
	    triright.push_back (right_int);
	}
	break;
    }
  }
 
  tri = vector <bsp_polygon>();
  triplane=vector<bsp_plane>();
  temp->left = buildbsp (NULL,trileft,triplaneleft);
  temp->right= buildbsp (NULL,triright,triplaneright);
  return temp;
}


enum INTERSECT_TYPE where_is_poly(bsp_tree * temp_node,bsp_polygon * temp_poly3)

{
    INTERSECT_TYPE last = BSPG_INTERSECT;
    float cur;
    INTERSECT_TYPE icur;
    for (unsigned int i=0;i<temp_poly3->v.size();i++) {
	cur = ((temp_node->a)*(temp_poly3->v[i].x))+((temp_node->b)*(temp_poly3->v[i].y))+((temp_node->c)*(temp_poly3->v[i].z))+(temp_node->d);
	if (cur>0)
	    icur = BSPG_FRONT;
	else if (cur <0) 
	    icur = BSPG_BACK;
	else {
	    icur = BSPG_INTERSECT;//no effect
	    if (i==temp_poly3->v.size()-1) {
		return BSPG_BACK;  //don't intersect if the planes are on top of each other
	    }
	    continue;
	}
	if (last == BSPG_INTERSECT) {
	    last = icur;
	}else {
	    if (last !=icur ) {
		return BSPG_INTERSECT;
	    }
	}

    }
    return last;
}


static void display_bsp_tree(bsp_tree * tree)

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

static void write_bsp_tree (bsp_tree *tree,int level)//assume open file
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



















////////////UNUSED!
static bsp_tree * put_plane_in_tree(bsp_tree * bsp,bsp_tree * temp_node,bsp_polygon * temp_poly3)
{
    enum INTERSECT_TYPE flag;
    bsp_tree * aux, *temp;
    
    aux = bsp;
    
    if (aux == NULL) {
	temp = (bsp_tree *) malloc (sizeof (bsp_tree));
	temp->a=temp_node->a;
	temp->b=temp_node->b;
	temp->c=temp_node->c;
	temp->d=temp_node->d;
	temp->left=NULL;
	temp->right=NULL;
	
        return temp;
    } else {
        flag = where_is_poly(aux,temp_poly3);
	switch (flag) {
	case BSPG_BACK:
	    aux->left = put_plane_in_tree(aux->left,temp_node,temp_poly3);
	    return aux;
	    
	case BSPG_FRONT:
	    aux->right = put_plane_in_tree(aux->right,temp_node,temp_poly3);
	    return aux;
	    
	case BSPG_INTERSECT:
	    aux->left = put_plane_in_tree(aux->left,temp_node,temp_poly3);
	    aux->right = put_plane_in_tree(aux->right,temp_node,temp_poly3);
	    return aux;
        }
    }
    return temp;
}
