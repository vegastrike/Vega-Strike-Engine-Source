/** Source for CubicSpline class **/

#include <math.h>
#include <assert.h>
#include "cubicsplines.h"
CubicSpline::CubicSpline() {
#ifdef SPLINE_METHOD2
	A=B=C=E=F=G=I=J=K=0;
	D=H=L=1.0;
#else
	memset(this,0,sizeof(CubicSpline));
#endif
}
#ifdef SPLINE_METHOD2
void	CubicSpline::createSpline( QVector P0, QVector P1, QVector P2, QVector P3)
{
	this->A = P3.i - 3*P2.i + 3*P1.i - P0.i;
	this->B = 3*P2.i - 6*P1.i + 3*P0.i;
	this->C = 3*P1.i - 3*P0.i;
	this->D = P0.i;

	this->E = P3.j - 3*P2.j + 3*P1.j - P0.j;
	this->F = 3*P2.j - 6*P1.j + 3*P0.j;
	this->G = 3*P1.j - 3*P0.j;
	this->H = P0.j;

	this->I = P3.k - 3*P2.k + 3*P1.k - P0.k;
	this->J = 3*P2.k - 6*P1.k + 3*P0.k;
	this->K = 3*P1.k - 3*P0.k;
	this->L = P0.k;
}

QVector CubicSpline::computePoint(double t) const
{
	QVector res( (A*t*t*t + B*t*t + C*t + D), (E*t*t*t + F*t*t + G*t + H), (I*t*t*t + J*t*t + K*t + L));
	return res;
}

CubicSpline::~CubicSpline()
{}
#endif

#ifdef SPLINE_METHOD1
// VECCholeskyTriDiagResol
//
double * VECCholeskyTriDiagResol(double * B, int nb)
{
   double * Y;
   double * LDiag;
   double * LssDiag;
   double * Result;
   int I, K, Debut, Fin;

   Debut=0;
   Fin=nb-1;
   //assert(Assigned(B));
   LDiag = new double[nb];
   LssDiag = new double[(nb-1)];
   // try
      LDiag[Debut]=1.4142135; // = sqrt(2)
      LssDiag[Debut]=1.0/1.4142135;
      for( K=Debut+1; K<=Fin-1; K++)
		{
         LDiag[K]=sqrt(4-LssDiag[K-1]*LssDiag[K-1]);
         LssDiag[K]=1.0/LDiag[K];
		}
      LDiag[Fin]=sqrt(2-LssDiag[Fin-1]*LssDiag[Fin-1]);
      Y = new double[nb];
      //try
         Y[Debut]=B[Debut]/LDiag[Debut];
         for(I=Debut+1; I<=Fin; I++)
            Y[I]=(B[I]-Y[I-1]*LssDiag[I-1])/LDiag[I];
         Result = new double[nb];
		 assert( LDiag[Fin]!=0);
         Result[Fin]=Y[Fin]/LDiag[Fin];
         for(I=Fin-1; I>=Debut; I--)
			{
			assert( LDiag[I]!=0);
            Result[I]=(Y[I]-Result[I+1]*LssDiag[I])/LDiag[I];
			}
      //finally
         delete Y;
      //end;
   //finally
      delete LDiag;
      delete LssDiag;
   //end;
//end;
	return Result;
}

// MATInterpolationHermite
//
// double ** as return type ????????????????
double ** MATInterpolationHermite( double * ordonnees, int nb)
{
   double a, b, c, d;
   double ** m;
   int i, n;
   double * bb;
   double * deriv;
   double * fa;

   m=0;
   if( ordonnees!=0 && (nb>0))
	{
      n=nb-1;
      bb = new double [nb];
      //try
         bb[0]=3*(ordonnees[1]-ordonnees[0]);
         bb[n]=3*(ordonnees[n]-ordonnees[n-1]);
         for(i=1; i<=n-1; i++)
            bb[i]=3*(ordonnees[i+1]-ordonnees[i-1]);
         deriv=VECCholeskyTriDiagResol(bb, nb);
         //try
            m = new double*[n];
            for(i=0;i<=n-1;i++)
			{
               m[i] = new double[4];
               a=ordonnees[i];
               b=deriv[i];
               c=3*(ordonnees[i+1]-ordonnees[i])-2*deriv[i]-deriv[i+1];
               d=-2*(ordonnees[i+1]-ordonnees[i])+deriv[i]+deriv[i+1];
               fa=m[i];
               fa[3]=a+i*(i*(c-i*d)-b);
               fa[2]=b+i*(3*i*d-2*c);
               fa[1]=c-3*i*d;
               fa[0]=d;
            }
         //finally
            delete deriv;
         //end;
      //finally
         delete bb;
      //end;
	}
    return m;
}

// MATValeurSpline
//
double MATValeurSpline(double ** spline, double x, int nb)
{
   int i;
   double *sa;
   double res;

   if( spline!=0)
	{
      if( x<=0)
         i=0;
      else if(x>nb-1)
         i=nb-1;
      else i= (int) floor(x);
      // TODO : the following line looks like a bug...
      if(i==(nb-1)) i--;
      sa=spline[i];
      res=((sa[0]*x+sa[1])*x+sa[2])*x+sa[3];
	}
   else res=0;
   return res;
}

// MATValeurSplineSlope
//
double MATValeurSplineSlope( double ** spline, double x, int nb)
{
   int i;
   double * sa;
   double res;

   if( spline!=0)
	{
      if( x<=0)
         i=0;
      else if( x>nb-1)
         i=nb-1;
      else i= (int) floor(x);
      // TODO : the following line looks like a bug...
      if(i==(nb-1)) i--;
      sa=spline[i];
      res=(3*sa[0]*x+2*sa[1])*x+sa[2];
	}
	else res=0;
	return res;
}

// ------------------
// ------------------ TCubicSpline ------------------
// ------------------

// Create
//
void CubicSpline::createSpline( QVector P0, QVector P1, QVector P2, QVector P3)
{
	double *X = new double[4];
	double *Y = new double[4];
	double *Z = new double[4];
	X[0] = P0.i;
	X[1] = P1.i;
	X[2] = P2.i;
	X[3] = P3.i;
	Y[0] = P0.j;
	Y[1] = P1.j;
	Y[2] = P2.j;
	Y[3] = P3.j;
	Z[0] = P0.k;
	Z[1] = P1.k;
	Z[2] = P2.k;
	Z[3] = P3.k;
   FNb=4;
   MatX=MATInterpolationHermite(X, FNb);
   MatY=MATInterpolationHermite(Y, FNb);
   MatZ=MATInterpolationHermite(Z, FNb);
   MatW = 0;

   delete X;
   delete Y;
   delete Z;
}

// Destroy
//
CubicSpline::~CubicSpline()
{
	// Loop through all 4 matrices to delete elements
	for( int i=0; i<FNb-1; i++)
	{
		delete MatX[i];
		delete MatY[i];
		delete MatZ[i];
		if( MatW!=0)
			delete MatW[i];
	}
   delete MatX;
   delete MatY;
   delete MatZ;
   if( MatW!=0)
		delete MatW;
}

// SplineAffineVector
//
QVector CubicSpline::computePoint(double t) const
{
	return QVector( MATValeurSpline(MatX, t, FNb), MATValeurSpline(MatY, t, FNb), MATValeurSpline(MatZ, t, FNb));
}

#endif


#ifdef SPLINE_METHOD3

/********************** SPLINE STUFF ************************/
/*** THIS ONE IS NOT IMPLEMENTED, THIS IS JUST RAW SOURCE ***/
/*** THIS ONE IS NOT IMPLEMENTED, THIS IS JUST RAW SOURCE ***/
/********************** SPLINE STUFF ************************/

/*
   This returns the point "output" on the spline curve.
   The parameter "v" indicates the position, it ranges from 0 to n-t+2
   
*/
void SplinePoint(int *u,int n,int t,double v,XYZ *control,XYZ *output)
{
   int k;
   double b;

   output->x = 0;
   output->y = 0;
   output->z = 0;

   for (k=0;k<=n;k++) {
      b = SplineBlend(k,t,u,v);
      output->x += control[k].x * b;
      output->y += control[k].y * b;
      output->z += control[k].z * b;
   }
}

/*
   Calculate the blending value, this is done recursively.
   
   If the numerator and denominator are 0 the expression is 0.
   If the deonimator is 0 the expression is 0
*/
double SplineBlend(int k,int t,int *u,double v)
{
   double value;

   if (t == 1) {
      if ((u[k] <= v) && (v < u[k+1]))
         value = 1;
      else
         value = 0;
   } else {
      if ((u[k+t-1] == u[k]) && (u[k+t] == u[k+1]))
         value = 0;
      else if (u[k+t-1] == u[k]) 
         value = (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
      else if (u[k+t] == u[k+1])
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v);
     else
         value = (v - u[k]) / (u[k+t-1] - u[k]) * SplineBlend(k,t-1,u,v) + 
                 (u[k+t] - v) / (u[k+t] - u[k+1]) * SplineBlend(k+1,t-1,u,v);
   }
   return(value);
}

/*
   The positions of the subintervals of v and breakpoints, the position
   on the curve are called knots. Breakpoints can be uniformly defined
   by setting u[j] = j, a more useful series of breakpoints are defined
   by the function below. This set of breakpoints localises changes to
   the vicinity of the control point being modified.
*/
void SplineKnots(int *u,int n,int t)
{
   int j;

   for (j=0;j<=n+t;j++) {
      if (j < t)
         u[j] = 0;
      else if (j <= n)
         u[j] = j - t + 1;
      else if (j > n)
         u[j] = n - t + 2;	
   }
}

/*-------------------------------------------------------------------------
   Create all the points along a spline curve
   Control points "inp", "n" of them.
   Knots "knots", degree "t".
   Ouput curve "outp", "res" of them.
*/
void SplineCurve(XYZ *inp,int n,int *knots,int t,XYZ *outp,int res)
{
   int i;
   double interval,increment;

   interval = 0;
   increment = (n - t + 2) / (double)(res - 1);
   for (i=0;i<res-1;i++) {
      SplinePoint(knots,n,t,interval,inp,&(outp[i]));
      interval += increment;
   }
   outp[res-1] = inp[n];
}

/*
   Example of how to call the spline functions
	Basically one needs to create the control points, then compute
   the knot positions, then calculate points along the curve.
*/
#define N 3
XYZ inp[N+1] = {0.0,0.0,0.0,   1.0,0.0,3.0,   2.0,0.0,1.0,   4.0,0.0,4.0};
#define T 3
int knots[N+T+1];
#define RESOLUTION 200
XYZ outp[RESOLUTION];

int main(int argc,char **argv)
{
   int i;

   SplineKnots(knots,N,T);
   SplineCurve(inp,N,knots,T,outp,RESOLUTION);

   /* Display the curve, in this case in OOGL format for GeomView */
   printf("LIST\n");
   printf("{ = SKEL\n");
   printf("%d %d\n",RESOLUTION,RESOLUTION-1);
   for (i=0;i<RESOLUTION;i++)
      printf("%g %g %g\n",outp[i].x,outp[i].y,outp[i].z);
   for (i=0;i<RESOLUTION-1;i++)
      printf("2 %d %d 1 1 1 1\n",i,i+1);
   printf("}\n");

   /* The axes */
   printf("{ = SKEL 3 2  0 0 4  0 0 0  4 0 0  2 0 1 0 0 1 1 2 1 2 0 0 1 1 }\n");

   /* Control point polygon */
   printf("{ = SKEL\n");
   printf("%d %d\n",N+1,N);
   for (i=0;i<=N;i++)
      printf("%g %g %g\n",inp[i].x,inp[i].y,inp[i].z);
   for(i=0;i<N;i++)
      printf("2 %d %d 0 1 0 1\n",i,i+1);
   printf("}\n");

}
#endif
