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

#include "vegastrike.h"
#include "nmrcl.h"

const double ode23_time_step_mat[] = { 0., 1./2., 3./4., 1. };
const double ode23_coeff_mat[][4] = {
    { 0., 1./2.,    0.,  2./9. } ,
    { 0.,    0., 3./4.,  1./3. } ,
    { 0.,    0.,    0.,  4./9. } ,
    { 0.,    0.,    0.,     0. } 
};
const double ode23_error_mat[] = { -5./72., 1./12., 1./9., -1./8. };
const double ode23_time_step_exp = 1./3.;

const double ode45_time_step_mat[] = 
{ 0., 1./5., 3./10., 4./5., 8./9., 1., 1. };
const double ode45_coeff_mat[][7] = {
    { 0, 1./5., 3./40.,  44./45.,  19372./6561.,   9017./3168.,     35./384. },
    { 0, 0.,    9./40., -56./15., -25360./2187.,     -355./33.,           0. },
    { 0, 0.,         0,   32./9.,  64448./6561.,  46732./5247.,   500./1113. },
    { 0, 0.,         0,        0,    -212./729.,      49./176.,    125./192. },
    { 0, 0.,         0,        0,             0, -5103./18656., -2187./6784. },
    { 0, 0.,         0,        0,             0,             0,      11./84. },
    { 0, 0.,         0,        0,             0,             0,           0. }
};

const double ode45_error_mat[] = 
{ 71./57600., 0., -71./16695., 71./1920., -17253./339200., 22./525., -1./40.};
const double ode45_time_step_exp = 1./5.;

ode_data_t* euler_new_ode_data()
{
    euler_data_t *data;
    data = (euler_data_t*)malloc(sizeof(euler_data_t));
    return (ode_data_t*) data;
}

int euler_num_estimates()
{
    return 2;
}

void euler_init_ode_data( ode_data_t *p, double init_val, double h )
{
    euler_data_t *data = (euler_data_t*)p;
    data->init_val = init_val;
    data->h = h;
}

double euler_next_time(ode_data_t *p, int step)
{
    euler_data_t *data = (euler_data_t*)p;
    check_assertion( 0 <= step && step < 2, 
		     "invalid step number in euler ode solver" );
    return step * data->h;
}

double euler_next_val( ode_data_t *p, int step )
{
    euler_data_t *data = (euler_data_t*)p;
    double val = data->init_val;

    if ( step == 1 ) 
	val += data->k[0];

    return val;
}

void euler_update_estimate(ode_data_t *p, int step, double val)
{
    euler_data_t *data = (euler_data_t*)p;
    data->k[step] = data->h * val;
}

double euler_final_estimate(ode_data_t *p)
{
    euler_data_t *data = (euler_data_t*)p;
    double val = data->init_val;

    val += 0.5 * (data->k[0] + data->k[1] );
    return val;
}

ode_solver_t new_euler_solver()
{
    ode_solver_t s; 
    s.new_ode_data = euler_new_ode_data;
    s.num_estimates = euler_num_estimates;
    s.init_ode_data = euler_init_ode_data;
    s.next_time = euler_next_time;
    s.next_val = euler_next_val;
    s.update_estimate = euler_update_estimate;
    s.final_estimate = euler_final_estimate;
    s.estimate_error = NULL;
    s.time_step_exponent = NULL;
    return s;
}


ode_data_t* ode23_new_ode_data()
{
    ode23_data_t *data;
    data = (ode23_data_t*)malloc(sizeof(ode23_data_t));
    return (ode_data_t*) data;
}

int ode23_num_estimates()
{
    return 4;
}

void ode23_init_ode_data( ode_data_t *p, double init_val, double h )
{
    ode23_data_t *data = (ode23_data_t*)p;
    data->init_val = init_val;
    data->h = h;
}

double ode23_next_time(ode_data_t *p, int step)
{
    ode23_data_t *data = (ode23_data_t*)p;
    check_assertion( 0 <= step && step < 4, "invalid step for ode23 solver" );
    return ode23_time_step_mat[step] * data->h;
}

double ode23_next_val( ode_data_t *p, int step )
{
    ode23_data_t *data = (ode23_data_t*)p;
    double val = data->init_val;
    int i;

    for ( i=0; i<step; i++ ) {
	val += ode23_coeff_mat[i][step] * data->k[i];
    }
    return val;
}

void ode23_update_estimate(ode_data_t *p, int step, double val)
{
    ode23_data_t *data = (ode23_data_t*)p;
    data->k[step] = data->h * val;
}

double ode23_final_estimate(ode_data_t *p)
{
    ode23_data_t *data = (ode23_data_t*)p;
    double val = data->init_val;
    int i;

    for ( i=0; i<3; i++ ) {
	val += ode23_coeff_mat[i][3] * data->k[i];
    }
    return val;
}

double ode23_estimate_error(ode_data_t *p)
{
    ode23_data_t *data = (ode23_data_t*)p;
    double err=0.;
    int i;

    for ( i=0; i<4; i++ ) {
	err += ode23_error_mat[i] * data->k[i];
    }
    return fabs(err);
}

double ode23_time_step_exponent()
{
    return ode23_time_step_exp;
}

ode_solver_t new_ode23_solver()
{
    ode_solver_t s; 
    s.new_ode_data = ode23_new_ode_data;
    s.num_estimates = ode23_num_estimates;
    s.init_ode_data = ode23_init_ode_data;
    s.next_time = ode23_next_time;
    s.next_val = ode23_next_val;
    s.update_estimate = ode23_update_estimate;
    s.final_estimate = ode23_final_estimate;
    s.estimate_error = ode23_estimate_error;
    s.time_step_exponent = ode23_time_step_exponent;
    return s;
}

ode_data_t* ode45_new_ode_data()
{
    ode45_data_t *data;
    data = (ode45_data_t*)malloc(sizeof(ode45_data_t));
    return (ode_data_t*) data;
}

int ode45_num_estimates()
{
    return 7;
}

void ode45_init_ode_data( ode_data_t *p, double init_val, double h )
{
    ode45_data_t *data = (ode45_data_t*)p;
    data->init_val = init_val;
    data->h = h;
}

double ode45_next_time(ode_data_t *p, int step)
{
    ode45_data_t *data = (ode45_data_t*)p;
    check_assertion( 0 <= step && step < 7, "invalid step for ode45 solver" );
    return ode45_time_step_mat[step] * data->h;
}

double ode45_next_val( ode_data_t *p, int step )
{
    ode45_data_t *data = (ode45_data_t*)p;
    double val = data->init_val;
    int i;

    for ( i=0; i<step; i++ ) {
	val += ode45_coeff_mat[i][step] * data->k[i];
    }
    return val;
}

void ode45_update_estimate(ode_data_t *p, int step, double val)
{
    ode45_data_t *data = (ode45_data_t*)p;
    data->k[step] = data->h * val;
}

double ode45_final_estimate(ode_data_t *p)
{
    ode45_data_t *data = (ode45_data_t*)p;
    double val = data->init_val;
    int i;

    for ( i=0; i<6; i++ ) {
	val += ode45_coeff_mat[i][6] * data->k[i];
    }
    return val;
}

double ode45_estimate_error(ode_data_t *p)
{
    ode45_data_t *data = (ode45_data_t*)p;
    double err=0.;
    int i;

    for ( i=0; i<7; i++ ) {
	err += ode45_error_mat[i] * data->k[i];
    }
    return fabs(err);
}

double ode45_time_step_exponent()
{
    return ode45_time_step_exp;
}

ode_solver_t new_ode45_solver()
{
    ode_solver_t s; 
    s.new_ode_data = ode45_new_ode_data;
    s.num_estimates = ode45_num_estimates;
    s.init_ode_data = ode45_init_ode_data;
    s.next_time = ode45_next_time;
    s.next_val = ode45_next_val;
    s.update_estimate = ode45_update_estimate;
    s.final_estimate = ode45_final_estimate;
    s.estimate_error = ode45_estimate_error;
    s.time_step_exponent = ode45_time_step_exponent;
    return s;
}

double lin_interp( const double x[], const double y[], double val, int n )
{
    int i;
    double m, b;

    check_assertion( 
	n>=2, "linear interpolation requires at least two data points" );

    if ( val < x[0] ) {
	i = 0;
    } else if ( val >= x[n-1] ) {
	i = n-2;
    } else {
	/* should replace with binary search if we ever use large tables */
	for ( i=0; i<n-1; i++ ) {
	    if ( val < x[i+1] ) break;
	}
    }

    m = ( y[i+1] - y[i] ) / ( x[i+1] - x[i] );
    b =  y[i] - m * x[i];

    return m * val + b;
}
