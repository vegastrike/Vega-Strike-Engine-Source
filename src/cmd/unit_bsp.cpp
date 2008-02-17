#include "endianness.h"
#include <assert.h>
#include "unit_bsp.h"
#include "vsfilesystem.h"

using namespace VSFileSystem;

using std::vector;

bool Cross (const bsp_polygon &x, bsp_tree &result) {
  double size =0;
  
  for (unsigned int i=2;(!size)&&i<x.v.size();i++) {
    Vector v1;
    v1.i = x.v[i].i-x.v[0].i;
    v1.j = x.v[i].j-x.v[0].j;
    v1.k = x.v[i].k-x.v[0].k;
    Vector v2; 
    v2.i = x.v[1].i-x.v[0].i;
    v2.j = x.v[1].j-x.v[0].j;
    v2.k = x.v[1].k-x.v[0].k;
    result.a = v1.j * v2.k - v1.k * v2.j;
    result.b = v1.k * v2.i - v1.i * v2.k;
    result.c = v1.i * v2.j  - v1.j * v2.i;     
    size = result.a*result.a+result.b*result.b+result.c*result.c;
  }
  if (size)
      size = ((double)1)/sqrtf (size);
  else 
    return false;
  result.a *=RIGHT_HANDED*(size);
  result.b *=RIGHT_HANDED*(size);
  result.c *=RIGHT_HANDED*(size);
  return true;
}
/*
double Dot (const bsp_vector & A, const bsp_vector & B) {
    return A.i *B.i + A.j*B.j+A.k*B.k;
}
*/
//FILE * o;
VSFile fo;
//ax + by + cz =0;  A.i + (B.i - A.i)k = x;A.j + (B.j - A.j)k = y;A.k + (B.k - A.k)k = z;
// x*A.i + b*B.j + c*C.k + d + k*(a*B.i - a*A.i + b*B.j - b&A.j + c*B.k - c*A.k) = 0;
// k = (A * n + d) / (A * n - B * n) 
//
bool intersectionPoint (const bsp_tree &n, const Vector & A, const Vector & B, Vector & res) {
    double inter = A.i*n.a + A.j*n.b+A.k*n.c;
    double k=(inter - (B.i * n.a + B.j * n.b + B.k * n.c)); 
    if (!k)
      return false;
    k = (inter + n.d ) / k; 
    //assume magnitude (n.a,n.b,n.c) == 1
    res.i = A.i + k*(B.i - A.i);
    res.j = A.j + k*(B.j - A.j);
    res.k = A.k + k*(B.k - A.k);
    return true;
}

enum INTERSECT_TYPE whereIs (const TVECTOR & v, const bsp_tree & temp_node) {
     double tmp = ((temp_node.a)*(v.i))+((temp_node.b)*(v.j))+((temp_node.c)*(v.k))+(temp_node.d);
     if (tmp < 0) {
	 return BSPG_BACK;
     }else if (tmp >0) {
	 return BSPG_FRONT;
     }else return BSPG_INTERSECT;
}

enum INTERSECT_TYPE where_is_poly(const bsp_tree &temp_node,const bsp_polygon &temp_poly3);
static void display_bsp_tree(bsp_tree * tree);

//can divide 3 or 4 sized planes
void dividePlane (const bsp_polygon & tri, const bsp_tree &unificator, bsp_polygon &back, bsp_polygon &front) {
    enum INTERSECT_TYPE oldflag;
    enum INTERSECT_TYPE flag;
    Vector int_point;
    front.v = vector <TVECTOR> ();
    back.v = vector <TVECTOR> ();
    for (unsigned int i=0;i<tri.v.size();i++) {
	flag = whereIs (tri.v[i], unificator);
	if (flag==BSPG_INTERSECT) {
	    front.v.push_back (tri.v[i]);
	    back.v.push_back (tri.v[i]);
	} else {
	    if (i!=0&&oldflag!=BSPG_INTERSECT&&flag!=oldflag) {
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

static int select_plane (const vector <bsp_polygon> &tri, const vector <bsp_tree> &triplane);
bsp_tree * buildbsp(bsp_tree * bsp,vector <bsp_polygon> &tri, vector <bsp_tree> &triplane, char vplane) {
  assert (tri.size()==triplane.size());
  bool VP = vplane!=0;
  if (tri.size()==0) {
    return NULL;
  }
  bsp_tree * temp=NULL;
  vector <bsp_polygon> trileft;
  vector <bsp_tree> triplaneleft;
  vector <bsp_polygon> triright;
  vector <bsp_tree> triplaneright;
  bsp_polygon left_int;
  bsp_polygon right_int;
  unsigned int select= tri.size();
  temp = (bsp_tree *) malloc (sizeof (bsp_tree));
  temp->left=NULL;
  temp->right=NULL;
  temp->a=temp->b=temp->d=0;
  temp->c=1;
  if (!(vplane&VPLANE_ALL)) {
    static bool use_heuristic=XMLSupport::parse_bool (vs_config->getVariable ("graphics","use_bsp_heuristic","false"));
    if (use_heuristic) {
      select = select_plane (tri, triplane);
    }else {
      select = rand()%tri.size();
    }
    if (select >=tri.size()) {
      VSFileSystem::vs_fprintf (stderr,"Error Selecting tri for splittage");
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
  return temp;
}


static int select_plane (const vector <bsp_polygon> &tri, const vector <bsp_tree> &triplane) {
  assert (triplane.size()==tri.size());
  double splits=0;
  int front = 0;
  int back = 0;
  unsigned int retval= 0;
  double least_penalty = 10000000000.;
  const double size_factor = .3333333;
  const double balance_factor = 1;
  const double  split_factor = .1;
  const unsigned int toobig=100;
  const int RANDOM_BSP = 0;
  const unsigned int samplesize = 10;
  unsigned int n = RANDOM_BSP?((samplesize>tri.size())?tri.size():samplesize):tri.size();
  unsigned int jj;
  for (unsigned int i=0;i<triplane.size();i++) {
    splits = front = back =0;
    splits = size_factor*(-((double)tri[i].v.size())+3);//start off somewhat negative
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
    double balance_penalty = ((double)fabs (front+splits-back))/(double)(n);
    // split_penalty is 0 for a very good tree with regards to splitting
    // and 1 for a very bad one.
    double split_penalty = (double)splits/(double)n;
    // Total penalty is a combination of both penalties. 0 is very good,
    double penalty = balance_factor * balance_penalty + split_factor * split_penalty;
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
    double cur;
    INTERSECT_TYPE icur;
    for (unsigned int i=0;i<temp_poly3.v.size();i++) {
	cur = ((temp_node.a)*(temp_poly3.v[i].i))+((temp_node.b)*(temp_poly3.v[i].j))+((temp_node.c)*(temp_poly3.v[i].k))+(temp_node.d);
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
//static double almost_av_height;
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

void bsp_stats (bsp_tree * tree) {
  average_height = numends = numnodes = maxheight= 0;
  minheight = 10000000;// 0xffffffffffffffffffffffffff
  if (tree!=NULL) {
    explore (tree,0);
    if (numends>0)
      average_height /=numends;
  }
  VSFileSystem::vs_fprintf (stderr,"Num Nodes: %d, NumEnds: %d\n", numnodes,numends);
  VSFileSystem::vs_fprintf (stderr,"Min Height: %d, Max Height: %d\n",minheight, maxheight);
  VSFileSystem::vs_fprintf (stderr,"Average Height %f\n", average_height);
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

static void wrtf(float f) {
	union { float fval; unsigned int ival; } t;
	t.fval = f; t.ival = le32_to_cpu(t.ival);
	fo.Write(&t.fval, sizeof t.fval);
}
static void wrtb(const bool b) { fo.Write(&b, sizeof(bool)); }


void write_bsp_tree (bsp_tree *tree,int level)//assume open file
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
