#ifndef _ALPHACURVE_H_
#define _ALPHACURVE_H_

#include <math.h>

//	query = requested pixel alpha					any
//	maxrez x = width of the image in pixels				any
//	min = minimum alpha						min 0
//	max = maximum alpha						max 255
//	focus = multiplier of focus center for curve			0-1
//	concavity = multiplier from linear -> terminal(0 or 255)	0-1
//	tail_mode_start = overriding slope value for start		negative = standard, 0=flat, high=vertical
//	tail_mode_end = overriding slope value for start		negative = standard, 0=flat, high=vertical

int get_alpha(int _query, int _maxrez_x, int _min, int _max, double _focus, double _concavity, int _tail_mode_start, int _tail_mode_end);

#endif
