/**
 * alphacurve.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "alphacurve.h"

//GET ALPHA CURVE
//***************************************************
//parametric Bezier curve spline for 3 points, 2 segments
//X(T) Y(T), where X(0) = X0, and X(1) = X1. <same for y>, hence no need for X calculations concernign Y return. but here just for kicks.
//***************************************************
int get_alpha(int _query,
        int _maxrez_x,
        int _min,
        int _max,
        double _focus,
        double _concavity,
        int _tail_mode_start,
        int _tail_mode_end) {
    //INPUT
    int query = _query;
    int min = _min;
    int max = _max;
    double focus = _focus;
    double concavity = _concavity;
    int maxrez_x = _maxrez_x;
    int tail_mode_start = _tail_mode_start;
    int tail_mode_end = _tail_mode_end;
    //INPUT
    //FIX DATA
    if (min < 0) {
        min = 0;
    }                                               //error-test
    if (max > 255) {
        max = 255;
    }                                   //error-test
    if (max < min) {
        min = 0;
        max = 255;
    }                                                            //error-test
    if (concavity > 1.0) {
        concavity = 1;
    }                 //error-test
    if (concavity < -1.0) {
        concavity = -1;
    }               //error-test
    if (query > maxrez_x) {
        query = maxrez_x;
    }             //error-test
    if (query < 0) {
        query = 0;
    }                                   //error-test
    if (focus < .2) {
        focus = .2;
    }                                 //error-test
    if (focus > .8) {
        focus = .8;
    }                                 //error-test
    //TAIL MODES CAN BE NEGATIVE BECAUSE THAT IS IGNORED
    //EXCESSIVE POSITICE VALUES WILL MAKE 255|0 RESULTS.
    //DEPENDING ON THE RESOLUTION AND LIMITS, DIFFERENT SLOPES WILL HIT THE RANGE AT DIFFERNT SETTINGS
    //I LEAVE THIS UNCAPPED, AND UP TO AN INTELLIGENT PERSON TO UNDERSTAND WHY IT BEHAVES SO IF THEY ENTER 10000000000
    int half = int(maxrez_x * focus);                              //half-the-work-point
    double _t = 0.0;
    if (query > half) {
        _t = double(query - half) / double(maxrez_x - half);                    //set parameter to second half
    } else {
        _t = double(query)
                / double(half);
    }                                                                       //set parameter to first half
    int center_y = int(((double(max - min) / double(maxrez_x)) * (half)) + min);
    int delta = 0;                                                        //difference from linear
    if (concavity < 0) {
        delta = max - center_y;            //go down by concavity
    } else {
        delta = center_y - min;
    }                                  //go up by convexity (-concavity)
    //FIX DATA

    //POINTS
//double x0               = 0;		//	start point X
    double y0 = min;                            //start point Y
//double x1               = half;		//	mid point X
    double y1 = center_y - (concavity * delta);
//double x2               = maxrez_x;	//	end point X
    double y2 = max;                            //end point Y
    if (y1 > max) {
        y1 = max;
    }             //error-test
    if (y1 < min) {
        y1 = min;
    }             //error-test
    //POINTS

    //SLOPES
//double vx0                              = double(x1-x0);				//	slope/T X
    double vy0 = double(y1 - y0);                                                                 //slope/T Y
//double vx1                              = double(x2-x0);				//	mid point slope (flat if at top/bottom, so there is no clipping, due to this, range capped to .2 -> .8 instead of 0 -> 1)
    if (concavity < 0) {
        concavity *= double(-1.0);
    }                       //concavity ABSOLUTE
    double vy1 = (double(y2 - y0) / double(2)) * (double(1.0) - concavity);
//double vx2                              = double(x2-x1);				//	end point slope X
    double vy2 = double(y2 - y1);                                                                 //end point slope Y
    if (!(tail_mode_start < 0)) {
        vy0 = tail_mode_start;
    }                //for over riding slopes 0 = flat, 100 or so = vertical
    if (!(tail_mode_end < 0)) {
        vy2 = tail_mode_end;
    }                    //for over riding slopes
    //SLOPES

    //INTERPOLATE
//int xt = 0;
    int yt = 0;
    if (query <= half) {
        double _t0 = _t;
//xt = x0 +	(vx0 * _t0)
//+	( ((3*(x1-x0)) - ((2*vx0) + vx1) ) * pow(_t0,2) )
//+	( ((2*(x0-x1))+(vx0+vx1) ) * pow(_t0,3));

        yt = int(y0 + (vy0 * _t0)
                + (((3 * (y1 - y0)) - ((2 * vy0) + vy1)) * pow(_t0, 2))
                + (((2 * (y0 - y1)) + (vy0 + vy1)) * pow(_t0, 3)));
    } else {
        double _t1 = _t;
//xt = x1	+	(vx1 * _t1)
//+	(( (3*(x2-x1)) - ((2*vx1) + vx2) ) * pow(_t1,2))
//+	(( (2*(x1-x2))+(vx1+vx2) ) * pow(_t1,3));
        yt = int(y1 + (vy1 * _t1)
                + (((3 * (y2 - y1)) - ((2 * vy1) + vy2)) * pow(_t1, 2))
                + (((2 * (y1 - y2)) + (vy1 + vy2)) * pow(_t1, 3)));
    }
    int return_alpha = yt;
    if (return_alpha < min) {
        return_alpha = min;
    }                 //error-test
    if (return_alpha > max) {
        return_alpha = max;
    }                 //error-test
    //INTERPOLATE

    return return_alpha;
}
//***************************************************

