#include <stdio.h>
#include <stdlib.h>
#ifndef PROPHECY
#include "gfx_mesh.h"
#include "cmd_unit.h"
#else
#include <vector>
using std::vector;
struct bsp_vector {
  float x,y,z;
};

struct bsp_polygon {
  vector <bsp_vector> v;
};

#endif
#define TRUE 1
#define FALSE 0

#define BACK -1
#define FRONT +1
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
#define BSPG_THRESHOLD .00001
enum INTERSECT_TYPE {
    BSPG_BACK =-1,
    BSPG_INTERSECT =0,
    BSPG_FRONT =1,
    BSPG_COPLANAR =2
};



typedef struct bsp_vector VECTOR;


struct bsp_tree {
    float a,b,c,d;
    vector <bsp_polygon> tri;
    vector <bsp_tree> triplane;
    struct bsp_tree * left;
    struct bsp_tree * right;
  bsp_tree(): tri(), triplane() {left = right = NULL;}
};


static bool Cross (const bsp_polygon &x, bsp_tree &result) {
  float size =0;
  
  for (unsigned int i=2;(!size)&&i<x.v.size();i++) {
    bsp_vector v1;
    v1.x = x.v[i].x-x.v[0].x;
    v1.y = x.v[i].y-x.v[0].y;
    v1.z = x.v[i].z-x.v[0].z;
    bsp_vector v2; 
    v2.x = x.v[1].x-x.v[0].x;
    v2.y = x.v[1].y-x.v[0].y;
    v2.z = x.v[1].z-x.v[0].z;
    result.a = v1.y * v2.z - v1.z * v2.y;
    result.b = v1.z * v2.x - v1.x * v2.z;
    result.c = v1.x * v2.y  - v1.y * v2.x;     
    size = result.a*result.a+result.b*result.b+result.c*result.c;
  }
  if (size)
      size = ((float)1)/sqrtf (size);
  else 
    return false;
  result.a *=RIGHT_HANDED*(size);
  result.b *=RIGHT_HANDED*(size);
  result.c *=RIGHT_HANDED*(size);
  return true;
}

float Dot (const bsp_vector & A, const bsp_vector & B) {
    return A.x *B.x + A.y*B.y+A.z*B.z;
}

static FILE * o;
static int highestlevel=0;
//ax + by + cz =0;  A.x + (B.x - A.x)k = x;A.y + (B.y - A.y)k = y;A.z + (B.z - A.z)k = z;
// x*A.x + b*B.y + c*C.z + d + k*(a*B.x - a*A.x + b*B.y - b&A.y + c*B.z - c*A.z) = 0;
// k = (A * n + d) / (A * n - B * n) 
//
bool intersectionPoint (const bsp_tree &n, const bsp_vector & A, const bsp_vector & B, bsp_vector & res) {
    float inter = A.x*n.a + A.y*n.b+A.z*n.c;
    float k=(inter - (B.x * n.a + B.y * n.b + B.z * n.c)); 
    if (!k)
      return false;
    k = (inter + n.d ) / k; 
    //assume magnitude (n.a,n.b,n.c) == 1
    res.x = A.x + k*(B.x - A.x);
    res.y = A.y + k*(B.y - A.y);
    res.z = A.z + k*(B.z - A.z);
    return true;
}

enum INTERSECT_TYPE whereIs (const VECTOR & v, const bsp_tree & temp_node) {
     float tmp = ((temp_node.a)*(v.x))+((temp_node.b)*(v.y))+((temp_node.c)*(v.z))+(temp_node.d);
     if (tmp < 0) {
	 return BSPG_BACK;
     }else if (tmp >0) {
	 return BSPG_FRONT;
     }else return BSPG_INTERSECT;
}

static enum INTERSECT_TYPE where_is_poly(const bsp_tree &temp_node,const bsp_polygon &temp_poly3);
static void bsp_stats (bsp_tree * tree);
static void display_bsp_tree(bsp_tree * tree);
static void write_bsp_tree (bsp_tree *tree,int level=0);//assume open file

//can divide 3 or 4 sized planes
void dividePlane (const bsp_polygon & tri, const bsp_tree &unificator, bsp_polygon &back, bsp_polygon &front) {
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
	      if (intersectionPoint (unificator, tri.v[i-1], tri.v[i], int_point)){
		front.v.push_back (int_point);
		back.v.push_back (int_point);
	      }
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
      if (intersectionPoint (unificator, tri.v[tri.v.size()-1], tri.v[0], int_point)){
	front.v.push_back (int_point);
	back.v.push_back (int_point);
      }
    }
}

void FreeBSP (bsp_tree ** tree) {
  if ((*tree)->right)
    FreeBSP(&(*tree)->right);
  if ((*tree)->left)
    FreeBSP(&(*tree)->left);
  free ( (*tree));
  *tree = NULL;
}

static bsp_tree * buildbsp(bsp_tree * bsp,vector <bsp_polygon>&, vector <bsp_tree>&, char Vplane);
#ifndef PROPHECY
void Unit::BuildBSPTree(const char *filename, bool vplane, Mesh * hull) {
  bsp_tree * bsp=NULL;
  unsigned int i;
  bsp_tree temp_node;
  vector <bsp_polygon> tri;
  vector <bsp_tree> triplane;
  if (hull!=NULL) {
    hull->GetPolys (tri);
  } else {
    for (i=0;i<nummesh;i++) {
      meshdata[i]->GetPolys(tri);
    }
  }	
  for (i=0;i<tri.size();i++) {
    if (!Cross (tri[i],temp_node)) {
      vector <bsp_polygon>::iterator ee = tri.begin();
      ee+=i;
      tri.erase(ee);
      i--;
      continue;
    }	
    // Calculate 'd'
    temp_node.d = (float) ((temp_node.a*tri[i].v[0].x)+(temp_node.b*tri[i].v[0].y)+(temp_node.c*tri[i].v[0].z));
    temp_node.d*=-1.0;
    triplane.push_back(temp_node);
    //                bsp=put_plane_in_tree3(bsp,&temp_node,&temp_poly3); 
 }
 
 bsp = buildbsp (bsp,tri,triplane, vplane?VPLANE_ALL:0);
 if (bsp) {
   o = fopen (filename, "w+b");
   write_bsp_tree(bsp,0);
   fclose (o);
   bsp_stats (bsp);
   FreeBSP (&bsp);
 }	
 fprintf (stderr,"HighestLevel, BSP Tree %d",highestlevel);
}
#else
long getsize (char * name)

{
  FILE *f;
  long size;
  f=fopen (name,"rb");
  fseek (f,0L,SEEK_END);
  size= ftell (f);
  fclose (f);
  return size;
}

void load (vector <bsp_polygon> &tri) {
  FILE *f;
  long size;
  float x,y,z;
  vector <bsp_vector> vec;
  vector <int> numberof;
  vector <int> vertexnum;

  long normal_number;
  long unknown1;
  long texture;
  long face_number;
  long number_of_vertices;
  long unknown2;
  long end;
  
  long vert,light;
  float fx,fy;
  
  long i;
  // Loading VERT ...
  
  size = getsize ("vert.wcp");

  f=fopen ("vert.wcp","rb");
  if (f == NULL) {
    printf ("Cannot open file\n");
    return;
  }
  bsp_vector tmpvec;
  for (i=0;i<size;i+=12) {
    fread (&tmpvec.x,4,1,f);
    fread (&tmpvec.y,4,1,f);
    fread (&tmpvec.z,4,1,f);
    vec.push_back (tmpvec);
  }
  fclose (f);
  // Loading VTNM ...

  size = getsize ("vtnm.wcp");

  f=fopen ("vtnm.wcp","rb");
  if (f == NULL) {
    printf ("Cannot open file\n");
    return;
  }
  for (i=0;i<size;i+=12) {
    fread (&x,4,1,f);
    fread (&y,4,1,f);
    fread (&z,4,1,f);
    //    add_to_vector_list (normals,x,y,z);
  }
  fclose (f);
// Loading FACE ...

  size = getsize ("face.wcp");

  f=fopen ("face.wcp","rb");
  if (f == NULL)
    {
      printf ("Cannot open file\n");
      return;
    }
  
  for (i=0;i<size;i+=28)
        {

        long normal_number;
        long unknown1;
        long texture;
        long face_number;
        long number_of_vertices;
        long unknown2;
        long end;

        fread (&normal_number,4,1,f);
        fread (&unknown1,4,1,f);
        fread (&texture,4,1,f);
        fread (&face_number,4,1,f);
        fread (&number_of_vertices,4,1,f);
        fread (&unknown2,4,1,f);
        fread (&end,4,1,f);

	numberof.push_back (number_of_vertices);
        }

fclose (f);
// Loading FVRT ...

size = getsize ("fvrt.wcp");

f=fopen ("fvrt.wcp","rb");
if (f == NULL)
        {
        printf ("Cannot open file\n");
        return;
        }

for (i=0;i<size;i+=16)
        {
        fread (&vert,4,1,f);
        fread (&light,4,1,f);
        fread (&fx,4,1,f);
        fread (&fy,4,1,f);
	vertexnum.push_back (vert);
        }

 fclose (f);
 bsp_vector thevec;;
 int targetvec=0;
 for (int i=0;i<numberof.size();i++) {
   bsp_polygon tmppoly;
   tmppoly.v = vector <bsp_vector>();
   for (int j=0;j<numberof[i];j++) {
     tmppoly.v.push_back (vec[vertexnum[targetvec]]);
     targetvec++;
   }
   tri.push_back (tmppoly);
 } 

}
int main(int argc, char * argv) {
  printf ("Wing Commander Prophecy - TREE Builder - Version 0.1 by Mario \"HCl\" Brito, Rewritten by Daniel Horn");
  bsp_tree * bsp=NULL;
  unsigned int i;
  bsp_tree temp_node;
 vector <bsp_polygon> tri;
 vector <bsp_tree> triplane;
 load (tri);
 for (i=0;i<tri.size();i++) {
     if (!Cross (tri[i],temp_node)) {
	 vector <bsp_polygon>::iterator ee = tri.begin();
	 ee+=i;
	 tri.erase(ee);
	 i--;
	 continue;
     }
     // Calculate 'd'
     temp_node.d = (float) ((temp_node.a*tri[i].v[0].x)+(temp_node.b*tri[i].v[0].y)+(temp_node.c*tri[i].v[0].z));
     temp_node.d*=-1.0;
     triplane.push_back(temp_node);
 }
 bsp = buildbsp (bsp,tri,triplane, 0);
 if (bsp) {
   o = fopen ("output.bsp", "w+b");
   write_bsp_tree(bsp,0);
   fclose (o);
   bsp_stats (bsp);
   FreeBSP (&bsp);
 }	
}
#endif
static int select_plane (const vector <bsp_polygon> &tri, const vector <bsp_tree> &triplane);
static bsp_tree * buildbsp(bsp_tree * bsp,vector <bsp_polygon> &tri, vector <bsp_tree> &triplane, char vplane) {
  assert (tri.size()==triplane.size());
  bool VP = vplane!=0;
  if (tri.size()==0) {
    return NULL;
  }
  bsp_tree * temp;
  vector <bsp_polygon> trileft;
  vector <bsp_tree> triplaneleft;
  vector <bsp_polygon> triright;
  vector <bsp_tree> triplaneright;
  bsp_polygon left_int;
  bsp_polygon right_int;
  unsigned int select= tri.size();
  temp = (bsp_tree *) malloc (sizeof (bsp_tree));
  if (!(vplane&VPLANE_ALL)) {
    select = select_plane (tri, triplane);
    if (select >=tri.size()) {
      fprintf (stderr,"Error Selecting tri for splittage");
      return NULL;
    }	
    temp->a=triplane[select].a;
    temp->b=triplane[select].b;
    temp->c=triplane[select].c;
    temp->d=triplane[select].d;
  } else if (vplane & VPLANE_X) {
    temp->a=1;
    temp->b=0;
    temp->c=0;
    temp->d=0;
    vplane &= (~VPLANE_X);
  }else if (vplane & VPLANE_Y) {
    temp->a=0;
    temp->b=1;
    temp->c=0;
    temp->d=0;
    vplane &= (~VPLANE_Y);
  } else if (vplane & VPLANE_Z) {
    temp->a=0;
    temp->b=0;
    temp->c=1;
    temp->d=0;
    vplane &= (~VPLANE_Z);
  }
    
  unsigned int i;
  for (i=0;i<tri.size();i++) {
    if (i==select) 
      continue;//don't add plane that's there...though it should fall into bspg_coplanar since mag > 1
    enum INTERSECT_TYPE flag = where_is_poly(*temp,tri[i]);    
    switch (flag) {
    case BSPG_BACK:
	trileft.push_back (tri[i]);
	triplaneleft.push_back(triplane[i]);
        break;
    case BSPG_FRONT:
	triright.push_back (tri[i]);
	triplaneright.push_back(triplane[i]);
        break;
    case BSPG_COPLANAR:
      if (triplane[i].a*temp->a+ triplane[i].b*temp->b+triplane[i].c*temp->c <0) {//if they face opposite directions, then it is behind the thing, otherwise, ignored!
	trileft.push_back (tri[i]);
	triplaneleft.push_back (triplane[i]);
      }
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
  if (VP&&(trileft.size()==0||triright.size()==0)) {
    //if the VPLANE doesn't really split any polygons, why add it to tree.. would cause false hits/misses
    free (temp);
    trileft.clear();
    triright.clear();
    triplaneleft.clear();
    triplaneright.clear();
    return buildbsp (NULL,tri,triplane,vplane);
 } else {
    assert (triplane.size()==tri.size()); 
    tri.clear();
    triplane.clear();
    temp->left = buildbsp (NULL,trileft,triplaneleft,vplane);
    temp->right= buildbsp (NULL,triright,triplaneright,vplane);
    return temp;
  }
}


static int select_plane (const vector <bsp_polygon> &tri, const vector <bsp_tree> &triplane) {
  assert (triplane.size()==tri.size());
  float splits=0;
  int front = 0;
  int back = 0;
  unsigned int retval= 0;
  float least_penalty = 10000000000.;
  const float size_factor = .3333333;
  const float balance_factor = 1;
  const float  split_factor = .1;
  const unsigned int toobig=100;
  const int RANDOM_BSP = 0;
  const unsigned int samplesize = 10;
  unsigned int n = RANDOM_BSP?((samplesize>tri.size())?tri.size():samplesize):tri.size();
  unsigned int jj;
  for (unsigned int i=0;i<triplane.size();i++) {
    splits = front = back =0;
    splits = size_factor*(-((float)tri[i].v.size())+3);//start off somewhat negative
    if (tri[i].v.size()>toobig) {
      return i;
    }
    for (unsigned int j=0;j<n;j++) {
      jj = RANDOM_BSP?rand()%tri.size():j;
      if (jj==i) continue;
      switch (where_is_poly(triplane[i],tri[jj])) {
      case BSPG_COPLANAR:
	break;
      case BSPG_BACK:
	back++;
	break;
      case BSPG_FRONT:
	front++;
	break;
      case BSPG_INTERSECT:
	splits+=size_factor*tri[jj].v.size();//more penalty for splitting bigger polygons 
	break;
      }    
    
    }
    float balance_penalty = ((float)fabs (front+splits-back))/(float)(n);
    // split_penalty is 0 for a very good tree with regards to splitting
    // and 1 for a very bad one.
    float split_penalty = (float)splits/(float)n;
    // Total penalty is a combination of both penalties. 0 is very good,
    float penalty = balance_factor * balance_penalty + split_factor * split_penalty;
    if (penalty < least_penalty) {
      least_penalty = penalty;
      retval = i;
    }
  }
  return retval;
}

enum INTERSECT_TYPE where_is_poly(const bsp_tree & temp_node,const bsp_polygon & temp_poly3)

{
    INTERSECT_TYPE last = BSPG_INTERSECT;
    float cur;
    INTERSECT_TYPE icur;
    for (unsigned int i=0;i<temp_poly3.v.size();i++) {
	cur = ((temp_node.a)*(temp_poly3.v[i].x))+((temp_node.b)*(temp_poly3.v[i].y))+((temp_node.c)*(temp_poly3.v[i].z))+(temp_node.d);
	if (cur>0)//??
	    icur = BSPG_FRONT;
	else if (cur <0) 
	    icur = BSPG_BACK;
	else {
	    icur = BSPG_INTERSECT;//no effect
	    if (i==temp_poly3.v.size()-1) {
		return BSPG_COPLANAR;  //don't intersect if the planes are on top of each other
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
static unsigned int maxheight;
static unsigned int minheight;
static double average_height;
//static float almost_av_height;
static unsigned int numends;
static unsigned int numnodes;

static void explore (bsp_tree * tree, unsigned int hgt) {
  numnodes++;
  if (tree->left==NULL&&tree->right==NULL) {
    average_height +=hgt;
    numends +=1;
    if (hgt>maxheight) {
      maxheight = hgt;
    } else {
      if (minheight > hgt) {
	minheight = hgt;
      }else {
	//	almost_av_height +=hgt;
      }
    }
  }

  if (tree->left==NULL) {
  } else {
    explore (tree->left,hgt+1);
  }
  if (tree->right==NULL) {
  } else {
    explore (tree->right,hgt+1);
  }


}

static void bsp_stats (bsp_tree * tree) {
  average_height = numends = numnodes = maxheight= 0;
  minheight = 10000000;// 0xffffffffffffffffffffffffff
  if (tree!=NULL) {
    explore (tree,0);
    if (numends>0)
      average_height /=numends;
  }
  fprintf (stderr,"Num Nodes: %d, NumEnds: %d\n", numnodes,numends);
  fprintf (stderr,"Min Height: %d, Max Height: %d\n",minheight, maxheight);
  fprintf (stderr,"Average Height %f\n", average_height);
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
        flag = where_is_poly(*aux,*temp_poly3);
	switch (flag) {
	case BSPG_COPLANAR:
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
