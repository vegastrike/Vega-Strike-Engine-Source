#include <stdio.h>


#define TRUE 1
#define FALSE 0

#define BACK -1
#define FRONT +1
#define INTERSECT 0


int unknown=0;

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


struct bsp_tree
        {
        float a,b,c,d;
        struct bsp_tree * left;
        struct bsp_tree * right;
        };

typedef struct bsp_tree BSP_TREE;

struct vector
        {
        float x,y,z;
        struct vector * next;
        };

typedef struct vector VECTOR;


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


struct face
        {
        long normal_number;
        long unknown1;
        long texture;
        long face_number;
        long number_of_vertices;
        long unknown2;
        long end;
        struct face * next;
        };

typedef struct face FACE;

struct fvrt
        {
        long vert;
        long light;
        float x;
        float y;
        struct fvrt * next;
        };

typedef struct fvrt FVRT;

void load (VECTOR ** vertices, VECTOR ** normals, FACE ** _face, FVRT ** _fvrt);
void add_to_vector_list (VECTOR ** vertices, float x,float y,float z);
void add_to_face_list (FACE ** _face,long normal_number,long  unknown1,long  texture,long face_number,long number_of_vertices,long unknown2,long end);
void add_to_fvrt_list (FVRT ** _fvrt,long vert,long light,float fx,float fy);
void build_bsp_tree(BSP_TREE ** bsp,VECTOR * vertices,VECTOR * normals,FACE * _face,FVRT * _fvrt);
BSP_TREE * put_plane_in_tree3(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON3 * temp_poly3);
BSP_TREE * put_plane_in_tree4(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON4 * temp_poly4);
int where_is_poly3(BSP_TREE * temp_node,POLYGON3 * temp_poly3);
int where_is_poly4(BSP_TREE * temp_node,POLYGON4 * temp_poly4);
void display_bsp_tree(BSP_TREE * tree);

void main (void)

{
BSP_TREE * bsp=NULL;
VECTOR * vertices=NULL;
VECTOR * normals=NULL;
FACE * _face=NULL;
FVRT * _fvrt=NULL;

printf ("Wing Commander Prophecy - TREE Builder - Version 0.1 by Mario \"HCl\" Brito\n");

load(&vertices, &normals, &_face, &_fvrt);

build_bsp_tree(&bsp,vertices,normals,_face,_fvrt);

printf ("TREE calculated. Displaying...\n");

display_bsp_tree(bsp);
printf ("counter = %d\n",unknown);
}

void load (VECTOR ** vertices, VECTOR ** normals, FACE ** _face, FVRT ** _fvrt)

{
FILE *f;
long size;
float x,y,z;
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
if (f == NULL)
        {
        printf ("Cannot open file\n");
        //getch();
        }

for (i=0;i<size;i+=12)
        {
        fread (&x,4,1,f);
        fread (&y,4,1,f);
        fread (&z,4,1,f);
        
        add_to_vector_list (vertices,x,y,z);
        }

fclose (f);
// Loading VTNM ...

size = getsize ("vtnm.wcp");

f=fopen ("vtnm.wcp","rb");
if (f == NULL)
        {
        printf ("Cannot open file\n");
        //getch();
        }

for (i=0;i<size;i+=12)
        {
        fread (&x,4,1,f);
        fread (&y,4,1,f);
        fread (&z,4,1,f);
        
        add_to_vector_list (normals,x,y,z);
        }
fclose (f);
// Loading FACE ...

size = getsize ("face.wcp");

f=fopen ("face.wcp","rb");
if (f == NULL)
        {
        printf ("Cannot open file\n");
        //getch();
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

        add_to_face_list (_face,normal_number, unknown1, texture,face_number,number_of_vertices,unknown2,end);
        }

fclose (f);
// Loading FVRT ...

size = getsize ("fvrt.wcp");

f=fopen ("fvrt.wcp","rb");
if (f == NULL)
        {
        printf ("Cannot open file\n");
        //getch();
        }

for (i=0;i<size;i+=16)
        {
        fread (&vert,4,1,f);
        fread (&light,4,1,f);
        fread (&fx,4,1,f);
        fread (&fy,4,1,f);
        
        add_to_fvrt_list (_fvrt,vert,light,fx,fy);
        }

fclose (f);

}

void add_to_vector_list (VECTOR ** vertices,float x,float y,float z)

{
VECTOR * temp, *aux;

temp = (VECTOR *) malloc (sizeof(VECTOR));

temp->x=x;
temp->y=y;
temp->z=z;
temp->next = NULL;

if ((*vertices) == NULL)
        {
        *vertices = temp;
        return;
        }
else
        {
        aux = (*vertices);
        while (1)
                {
                if (aux->next == NULL)
                        {
                        aux->next = temp;
                        return;
                        }
                else
                        aux=aux->next;
                }

        }

}

void add_to_face_list (FACE ** _face,long normal_number,long  unknown1,long  texture,long face_number,long number_of_vertices,long unknown2,long end)
{
FACE * temp, *aux;

temp = (FACE *) malloc (sizeof(FACE));

temp->normal_number=normal_number;
temp->unknown1=unknown1;
temp->texture = texture;
temp->face_number=face_number;
temp->number_of_vertices=number_of_vertices;
temp->unknown2=unknown2;
temp->end=end;
temp->next = NULL;

if ((*_face) == NULL)
        {
        *_face = temp;
        return;
        }
else
        {
        aux = (*_face);
        while (1)
                {
                if (aux->next == NULL)
                        {
                        aux->next = temp;
                        return;
                        }
                else
                        aux=aux->next;
                }

        }

}


void add_to_fvrt_list (FVRT ** _fvrt,long vert,long light,float fx,float fy)

{
FVRT * temp, *aux;

temp = (FVRT *) malloc (sizeof(FVRT));

temp->vert = vert;
temp->light = light;
temp->x=fx;
temp->y=fy;
temp->next = NULL;

if ((*_fvrt) == NULL)
        {
        *_fvrt = temp;
        return;
        }
else
        {
        aux = (*_fvrt);
        while (1)
                {
                if (aux->next == NULL)
                        {
                        aux->next = temp;
                        return;
                        }
                else
                        aux=aux->next;
                }

        }

}


void build_bsp_tree(BSP_TREE ** bsp,VECTOR * vertices,VECTOR * normals,FACE * _face,FVRT * _fvrt)

{

long i;
long normal;
long counter; //debug
BSP_TREE temp_node;

POLYGON3 temp_poly3;
POLYGON4 temp_poly4;

VECTOR * vert_aux, * vtnm_aux;
FACE * face_aux;
FVRT * fvrt_aux;

vert_aux = vertices;
vtnm_aux = normals;
face_aux = _face;
fvrt_aux = _fvrt;

while (face_aux != NULL)
        {
        if (face_aux->number_of_vertices == 3)
                {
                temp_node.a=0;  // clean temp values...
                temp_node.b=0;
                temp_node.c=0;
                temp_node.d=0;

                normal = face_aux->normal_number;
                for (i=0;i<normal;i++)          // search for normals
                        {
                        vtnm_aux=vtnm_aux->next;
                        }
                temp_node.a=vtnm_aux->x;     
                temp_node.b=vtnm_aux->y;
                temp_node.c=vtnm_aux->z;

                vtnm_aux = normals;
                                              // Find vertices
                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly3.a.x = vert_aux->x;
                temp_poly3.a.y = vert_aux->y;
                temp_poly3.a.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;

                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly3.b.x = vert_aux->x;
                temp_poly3.b.y = vert_aux->y;
                temp_poly3.b.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;

                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly3.c.x = vert_aux->x;
                temp_poly3.c.y = vert_aux->y;
                temp_poly3.c.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;



                                              // Calculate 'd'

                temp_node.d = (float) ((temp_node.a*temp_poly3.a.x)+(temp_node.b*temp_poly3.a.y)+(temp_node.c*temp_poly3.a.z));
                temp_node.d*=-1.0;

                //printf ("3-Plane calculated (%ld)\n",counter);
                (*bsp)=put_plane_in_tree3((*bsp),&temp_node,&temp_poly3);

                }

        else if (face_aux->number_of_vertices == 4)
                {
                temp_node.a=0;  // clean temp values...
                temp_node.b=0;
                temp_node.c=0;
                temp_node.d=0;

                normal = face_aux->normal_number;
                for (i=0;i<normal;i++)          // search for normals
                        {
                        vtnm_aux=vtnm_aux->next;
                        }
                temp_node.a=vtnm_aux->x;     
                temp_node.b=vtnm_aux->y;
                temp_node.c=vtnm_aux->z;

                vtnm_aux = normals;
                                              // Find vertices
                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly4.a.x = vert_aux->x;
                temp_poly4.a.y = vert_aux->y;
                temp_poly4.a.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;

                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly4.b.x = vert_aux->x;
                temp_poly4.b.y = vert_aux->y;
                temp_poly4.b.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;

                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly4.c.x = vert_aux->x;
                temp_poly4.c.y = vert_aux->y;
                temp_poly4.c.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;

                for (i=0;i<fvrt_aux->vert;i++)
                        {
                        vert_aux=vert_aux->next;
                        }
                temp_poly4.d.x = vert_aux->x;
                temp_poly4.d.y = vert_aux->y;
                temp_poly4.d.z = vert_aux->z;

                vert_aux = vertices;
                fvrt_aux = fvrt_aux->next;


                                              // Calculate 'd'
                temp_node.d = (float)((temp_node.a*temp_poly4.a.x)+(temp_node.b*temp_poly4.a.y)+(temp_node.c*temp_poly4.a.z));
                temp_node.d*=-1.0;

                //printf ("4-Plane calculated (%ld)\n",counter);
                (*bsp)=put_plane_in_tree4((*bsp),&temp_node,&temp_poly4);

                }
        else
                {
                printf ("ERROR: Can only deal with 3 or 4 sided polys\n");
                //getch();
                }
        face_aux=face_aux->next;
        counter++;
        }
}

BSP_TREE * put_plane_in_tree3(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON3 * temp_poly3)
{
int flag;
BSP_TREE * aux, *temp, ** aux2;

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
        }
}


BSP_TREE * put_plane_in_tree4(BSP_TREE * bsp,BSP_TREE * temp_node,POLYGON4 * temp_poly4)
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
        }
}


int where_is_poly3(BSP_TREE * temp_node,POLYGON3 * temp_poly3)

{
int flag[3]={0,0,0};
int eval;
int i;

flag[0] = ((temp_node->a)*(temp_poly3->a.x))+((temp_node->b)*(temp_poly3->a.y))+((temp_node->c)*(temp_poly3->a.z))+(temp_node->d);
flag[1] = ((temp_node->a)*(temp_poly3->b.x))+((temp_node->b)*(temp_poly3->b.y))+((temp_node->c)*(temp_poly3->b.z))+(temp_node->d);
flag[2] = ((temp_node->a)*(temp_poly3->c.x))+((temp_node->b)*(temp_poly3->c.y))+((temp_node->c)*(temp_poly3->c.z))+(temp_node->d);

if (flag[0] <=0 && flag[1] <=0 && flag[2] <=0)
        return BACK;
else if (flag[0] >=0 && flag[1] >=0 && flag[2] >=0)
        return FRONT;
else
        return INTERSECT;

}

int where_is_poly4(BSP_TREE * temp_node,POLYGON4 * temp_poly4)

{
int flag[4]={0,0,0,0};
int eval;
int i;

flag[0] = ((temp_node->a)*(temp_poly4->a.x))+((temp_node->b)*(temp_poly4->a.y))+((temp_node->c)*(temp_poly4->a.z))+(temp_node->d);
flag[1] = ((temp_node->a)*(temp_poly4->b.x))+((temp_node->b)*(temp_poly4->b.y))+((temp_node->c)*(temp_poly4->b.z))+(temp_node->d);
flag[2] = ((temp_node->a)*(temp_poly4->c.x))+((temp_node->b)*(temp_poly4->c.y))+((temp_node->c)*(temp_poly4->c.z))+(temp_node->d);
flag[3] = ((temp_node->a)*(temp_poly4->d.x))+((temp_node->b)*(temp_poly4->d.y))+((temp_node->c)*(temp_poly4->d.z))+(temp_node->d);

if (flag[0] <=0 && flag[1] <=0 && flag[2] <=0 && flag[3]<=0)
        return BACK;
else if (flag[0] >=0 && flag[1] >=0 && flag[2] >=0 && flag[3] >=0)
        return FRONT;
else
        return INTERSECT;

}

void display_bsp_tree(BSP_TREE * tree)

{
float x;
char * c;
int i;
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
