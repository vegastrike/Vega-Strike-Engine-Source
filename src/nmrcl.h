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

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _NMRCL_H_
#define _NMRCL_H_

#define EULER 0
#define ODE23 1
#define ODE45 2

typedef char ode_data_t;
typedef ode_data_t* (*new_ode_data_ptr)();
typedef int (*num_estimates_ptr)();
typedef void (*init_ode_data_ptr)(ode_data_t *, double init_val, double h );
typedef double (*next_time_ptr)(ode_data_t *, int step);
typedef double (*next_val_ptr)(ode_data_t *, int step);
typedef void (*update_estimate_ptr)(ode_data_t *, int step, double val );
typedef double (*final_estimate_ptr)(ode_data_t *);
typedef double (*estimate_error_ptr)(ode_data_t *);
typedef double (*time_step_exponent_ptr)();

typedef struct {
    new_ode_data_ptr new_ode_data;
    num_estimates_ptr num_estimates;
    init_ode_data_ptr init_ode_data;
    next_time_ptr next_time;
    next_val_ptr next_val;
    update_estimate_ptr update_estimate;
    final_estimate_ptr final_estimate;
    estimate_error_ptr estimate_error;
    time_step_exponent_ptr time_step_exponent;
} ode_solver_t;

typedef struct {
    double k[4];
    double init_val;
    double h;
} ode23_data_t;

ode_solver_t new_ode23_solver();

typedef struct {
    double k[7];
    double init_val;
    double h;
} ode45_data_t;

ode_solver_t new_ode45_solver();

typedef struct {
    double k[2];
    double init_val;
    double h;
} euler_data_t;

ode_solver_t new_euler_solver();

double lin_interp( const double x[], const double y[], double val, int n );

#endif /* _NMRCL_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
