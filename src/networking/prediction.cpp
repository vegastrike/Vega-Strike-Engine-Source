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

void		Prediction::InitInterpolation( ClientPtr clt)
{
	// This function is to call after the state have been updated (which should be always the case)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	/************* VA IS TO BE UNCOMMENTED ****************/
	Unit * un = clt->game_unit.GetUnit();
	unsigned int del = clt->deltatime;
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
	A2 = A3 - (VB + AB*delay);
}

Transformation Prediction::Interpolate( ClientPtr clt)
{
	return Transformation( InterpolateOrientation( clt), InterpolatePosition( clt));
}

/*************************************************************************************/
/**** NullPrediction class : doesn't do any prediction/interpolation              ****/
/*************************************************************************************/

void		NullPrediction::InitInterpolation( ClientPtr clt)
{
}

QVector		NullPrediction::InterpolatePosition( ClientPtr clt)
{
	return clt->game_unit.GetUnit()->curr_physical_state.position;
}

Quaternion	NullPrediction::InterpolateOrientation( ClientPtr clt)
{
	return clt->game_unit.GetUnit()->curr_physical_state.orientation;
}

/*************************************************************************************/
/**** LinearPrediction class : based on lerp.c stuff also used in non networking  ****/
/*************************************************************************************/

QVector		LinearPrediction::InterpolatePosition( ClientPtr clt)
{
	Unit * un = clt->game_unit.GetUnit();
	// USE PREDICTED POSITION
	const Transformation old_pos( un->curr_physical_state);
	const Transformation new_pos( un->old_state.getOrientation(), A2);
	/*
	const Transformation old_pos( un->old_state.getOrientation(), un->old_state.getPosition());
	const Transformation cur_pos( un->curr_physical_state.orientation, un->curr_physical_state.position);
	*/
	return (linear_interpolate(  old_pos, new_pos, clt->deltatime)).position;
}

Quaternion	LinearPrediction::InterpolateOrientation( ClientPtr clt)
{
	Unit * un = clt->game_unit.GetUnit();
	return un->curr_physical_state.orientation;
	//return (linear_interpolate(  old_pos, cur_pos, clt->deltatime)).orientation;
}

Transformation LinearPrediction::Interpolate( ClientPtr clt)
{
	Unit * un = clt->game_unit.GetUnit();

	// USE PREDICTED POSITION
	const Transformation old_pos( un->curr_physical_state);
	const Transformation new_pos( un->old_state.getOrientation(), A2);
	/*
	const Transformation old_pos( un->old_state.getOrientation(), un->old_state.getPosition());
	const Transformation cur_pos( un->curr_physical_state.orientation, un->curr_physical_state.position);
	*/
	return linear_interpolate(  old_pos, new_pos, clt->deltatime);
}

/*************************************************************************************/
/**** CubicSplinePrediction class : based on cubic spline interpolation           ****/
/*************************************************************************************/

void		CubicSplinePrediction::InitInterpolation( ClientPtr clt)
{
	Prediction::InitInterpolation( clt);

	this->createSpline( A0, A1, A2, A3);
}

Quaternion	CubicSplinePrediction::InterpolateOrientation( ClientPtr clt)
{
	return clt->game_unit.GetUnit()->curr_physical_state.orientation;
}

QVector		CubicSplinePrediction::InterpolatePosition( ClientPtr clt)
{
	// There should be another function called when received a new position update and creating the spline
	QVector pos( this->computePoint( clt->deltatime));
	return pos;
}

/*************************************************************************************/
/**** MixedPrediction class : Linear for orientation, CubicSpline for position    ****/
/*************************************************************************************/

Transformation MixedPrediction::Interpolate( ClientPtr clt)
{
	Transformation linear( LinearPrediction::Interpolate( clt));
	linear.position = CubicSplinePrediction::InterpolatePosition( clt);
	return linear;
}

/*
QVector			MixedPrediction::InterpolatePosition( ClientPtr)
{ 
	return CubicSplinePrediction::InterpolatePosition( clt);
}

Orientation		MixedPrediction::InterpolateOrientation( ClientPtr clt)
{
	return LinearPrediction::InterpolateOrientation( clt);
}
*/

