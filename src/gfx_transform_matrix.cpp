#include <stdio.h>
#include <math.h>
/*---------------------------------------------------------------------------*
 | mat_inv: Compute the inverse of a n x n matrix, using the maximum pivot   |
 |          strategy.  n <= MAX1.                                            |
 *---------------------------------------------------------------------------*

   Parameters:
       a        a n x n square matrix
       b        inverse of input a.
       n        dimenstion of matrix a.
*/

    /// Set mat = inverse(_m)
int invert(float b [],float a[])

{
   const int n = 4;
   //int      is, js;      // Was never referenced
   int     i, j, k;
   int r[ 4]={0};
   int c[ 4]={0};
   int row[ 4]={0};
   int col[ 4]={0};
   float m[ 4][ 4*2], pivot, max_m, tmp_m, fac;
           /* Set working matrix */
   for ( i = 0; i < n; i++ )  {
     for ( j = 0; j < n; j++ )
       {
	 m[ i][ j] = a[ i * n + j];
	 m[ i][ j + n] = ( i == j ) ? 1.0 : 0.0 ;
       }
   }

   /* Begin of loop */
   for ( k = 0; k < n; k++ )
     {
       /* Choosing the pivot */
       for ( i = 0, max_m = 0; i < n; i++ )
	 {
	   if ( row[ i]  )      continue;
	   for ( j = 0; j < n; j++ )
	     {
	       if ( col[ j] )          continue;
	       tmp_m = fabs( m[ i][ j]);
	       if ( tmp_m > max_m)
		 {
		   max_m = tmp_m;
		   r[ k] = i;
		   c[ k] = j;
		 }
	     }
	 }
       row[ r[k] ] = col[ c[k] ] = 1;
       pivot = m[ r[ k] ][ c[ k] ];
       
       
       if ( fabs( pivot) <= 1e-20)
	 {
	   fprintf (stderr, "*** pivot = %f in mat_inv. ***\n",pivot);
	   return -1;
	 }
       
       /* Normalization */
       for ( j = 0; j < 2*n; j++ )
	 {
	   if ( j == c[ k] )
	     m[ r[ k]][ j] = 1.0;
	   else
	     m[ r[ k]][ j] /= pivot;
	 }
       
       /* Reduction */
       for ( i = 0; i < n; i++ )
	 {
	   if ( i == r[ k] )
	     continue;
	   
	   for ( j=0, fac = m[ i][ c[k]]; j < 2*n; j++ )
	     {
	       if ( j == c[ k] )
		 m[ i][ j] = 0.0;
	       else
		 m[ i][ j] -= fac * m[ r[k]][ j];
	     }
	 }
     }	
   /* Assign invers to a matrix */
   for ( i = 0; i < n; i++ )
     for ( j = 0; j < n; j++ )
       row[ i] = ( c[ j] == i ) ? r[ j] : row[ i];
   
   for ( i = 0; i < n; i++ )
     for ( j = 0; j < n; j++ )
       b[ i * n +  j] = m[ row[ i]][ j + n];
   
   return 1;   // It worked
}
