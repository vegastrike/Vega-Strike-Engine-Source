/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel Horn
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
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define EPS 1.0e-10

#include "gauss.h"

/*
  prototype declarations
*/

unsigned short order(double *matrix, int n, int pivot);
void elim(double *matrix, int n, int pivot);
void backsb(double *matrix, int n, double *soln);

/*
  gauss function -- takes a two dimensional array as its arg,
  of dimension n x (n+1)
*/

int gauss(double *matrix, int n, double *soln)
{
    int pivot=0;
    unsigned short error=0;


    while ((pivot<(n-1)) && (!error))
    {
	if(!(error = order(matrix,n,pivot)))
	{
	    elim(matrix,n,pivot);
	    pivot++;
	}
    }

    if (error)
    {
	return 1;
    }
    else
    {
	backsb(matrix, n, soln);
    }
	
    return 0;
}


unsigned short order( double *matrix, int n, int pivot)
{
    int row, rmax, k;
    double temp;
    unsigned short error=0;

    rmax = pivot;

    for (row=pivot+1; row<n; row++)
    {
	if (fabs(*(matrix+row*(n+1)+pivot)) > fabs(*(matrix+rmax*(n+1)+pivot)))
	    rmax = row;
    }

    if (fabs(*(matrix+rmax*(n+1)+pivot)) < EPS )
	error = 1;
    else if (rmax != pivot)
    {
	for (k=0; k<(n+1); k++)
	{
	    temp = *(matrix+rmax*(n+1)+k);
	    *(matrix+rmax*(n+1)+k) = *(matrix+pivot*(n+1)+k);
	    *(matrix+pivot*(n+1)+k) = temp;
	}
    }

    return error;

}

void elim(double *matrix, int n, int pivot)
{
    int row, col;
    double factor;

    for (row = pivot+1; row < n; row++)
    {
	factor = (*(matrix+row*(n+1)+pivot))/(*(matrix+pivot*(n+1)+pivot));
	*(matrix+row*(n+1)+pivot)=0.0;
	for (col=pivot+1l; col<n+1; col++)
	{
	    *(matrix+row*(n+1)+col) = *(matrix+row*(n+1)+col) - 
		(*(matrix+pivot*(n+1)+col))*factor;
	}
    }
}


void backsb(double *matrix, int n, double *soln)
{
    int row, col;


    for (row = n-1; row >=0; row--)
    {
	for (col = n-1; col >= row+1; col--)
	{
	    *(matrix+row*(n+1)+(n)) = *(matrix+row*(n+1)+n) - 
		(*(soln+col))*(*(matrix+row*(n+1)+col));
	}
	*(soln+row) = (*(matrix+row*(n+1)+n))/(*(matrix+row*(n+1)+row));
    }

}
