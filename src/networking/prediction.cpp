#include "client.h"
#include "prediction.h"
#include "gfx/lerp.h"
#include "cmd/unit_generic.h"

/*************************************************************************************/
/**** Prediction virtual class                                                    ****/
/*************************************************************************************/

Prediction::Prediction()
{
}
Prediction::~Prediction()
{
}

void		Prediction::InitInterpolation( Unit * un, double deltatime)
{
	// This function is to call after the state have been updated (which should be after receiving a SNAPSHOT)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	/************* VA IS TO BE UNCOMMENTED ****************/
	//Unit * un = clt->game_unit.GetUnit();
	unsigned int del = deltatime;
	double delay = del;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	A0 = un->old_state.getPosition();
	B = un->curr_physical_state.position;
	VA = un->old_state.getVelocity();
	VB = un->Velocity;
	AA = un->old_state.getAcceleration();
	AB = un->GetAcceleration();
	A1 = A0 + VA;
	// A2 is computed from position B and velocity VB
	A3 = B + VB*delay + AB*delay*delay*0.5;
	// A2 is the predicted position
	A2 = A3 - (VB + AB*delay);
}

Transformation Prediction::Interpolate( Unit * un, double deltatime)
{
	return Transformation( InterpolateOrientation( un, deltatime), InterpolatePosition( un, deltatime));
}

/*************************************************************************************/
/**** NullPrediction class : doesn't do any prediction/interpolation              ****/
/*************************************************************************************/

void		NullPrediction::InitInterpolation( Unit * un, double deltatime)
{
}

QVector		NullPrediction::InterpolatePosition( Unit * un, double deltatime)
{
	return un->curr_physical_state.position;
}

Quaternion	NullPrediction::InterpolateOrientation( Unit * un, double deltatime)
{
	return un->curr_physical_state.orientation;
}

/*************************************************************************************/
/**** LinearPrediction class : based on lerp.c stuff also used in non networking  ****/
/*************************************************************************************/

QVector		LinearPrediction::InterpolatePosition( Unit * un, double deltatime)
{
	const Transformation old_pos( un->curr_physical_state);
	const Transformation new_pos( un->old_state.getOrientation(), A2);
	return (linear_interpolate(  old_pos, new_pos, deltatime)).position;
}

Quaternion	LinearPrediction::InterpolateOrientation( Unit * un, double deltatime)
{
	const Transformation old_pos( un->curr_physical_state);
	const Transformation new_pos( un->old_state.getOrientation(), A2);
	return (linear_interpolate(  old_pos, new_pos, deltatime)).orientation;
}

Transformation LinearPrediction::Interpolate( Unit * un, double deltatime)
{
	const Transformation old_pos( un->curr_physical_state);
	const Transformation new_pos( un->old_state.getOrientation(), A2);
	return linear_interpolate(  old_pos, new_pos, deltatime);
}

/*************************************************************************************/
/**** CubicSplinePrediction class : based on cubic spline interpolation           ****/
/*************************************************************************************/

void		CubicSplinePrediction::InitInterpolation( Unit * un, double deltatime)
{
	Prediction::InitInterpolation( un, deltatime);

	this->createSpline( A0, A1, A2, A3);
}

// Don't know how to do here maybe should have a Spline for each 3 vectors of the rotation quaternion 
// Do at least linear interpolation since we inherit from LinearPrediction
Quaternion	CubicSplinePrediction::InterpolateOrientation( Unit * un, double deltatime)
{
	return un->curr_physical_state.orientation;
}

QVector		CubicSplinePrediction::InterpolatePosition( Unit * un, double deltatime)
{
	// There should be another function called when received a new position update and creating the spline
	QVector pos( this->computePoint( deltatime));
	return pos;
}

/*************************************************************************************/
/**** MixedPrediction class : Linear for orientation, CubicSpline for position    ****/
/*************************************************************************************/

Transformation MixedPrediction::Interpolate( Unit * un, double deltatime)
{
	Transformation linear( LinearPrediction::Interpolate( un, deltatime));
	linear.position = CubicSplinePrediction::InterpolatePosition( un, deltatime);
	return linear;
}

