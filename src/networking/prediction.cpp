#include "client.h"
#include "prediction.h"
#include "gfx/lerp.h"
#include "cmd/unit_generic.h"

// OLD NetClient::predict( ObjSerial clientid) function
/*
	// This function is to call after the state have been updated (which should be always the case)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	Unit * un = Clients.get(clientid)->game_unit.GetUnit();
	unsigned int del = Clients.get(clientid)->deltatime;
	double delay = del;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	QVector A( un->old_state.getPosition());
	QVector B( un->curr_physical_state.position);
	Vector  VA( un->old_state.getVelocity());
	Vector  VB( un->Velocity);
	Vector  AA( un->old_state.getAcceleration());
	Vector  AB( un->GetAcceleration());
	QVector A1( A + VA);
	// A2 is computed from position B and velocity VB
	QVector A3( B + VB*delay + AB*delay*delay*0.5);
	QVector A2( A3 - (VB + AB*delay));

	// HERE : Backup the current state ???? --> Not sure
	un->curr_physical_state.position = A3;
*/

// blend = time

/*************************************************************************************/
/**** Prediction virtual class                                                    ****/
/*************************************************************************************/

Prediction::Prediction()
{
}
Prediction::~Prediction()
{
}

Transformation Prediction::Interpolate( ClientPtr clt, double blend)
{
	return Transformation( InterpolateOrientation( clt, blend), InterpolatePosition( clt, blend));
}

/*************************************************************************************/
/**** LinearPrediction class : based on lerp.c stuff also used in non networking  ****/
/*************************************************************************************/

void		LinearPrediction::InitInterpolation( ClientPtr clt)
{
}

QVector		LinearPrediction::InterpolatePosition( ClientPtr clt, double blend)
{
	QVector position;
	return position;
}

Quaternion	LinearPrediction::InterpolateOrientation( ClientPtr clt, double blend)
{
	Quaternion orient;
	return orient;
}

Transformation LinearPrediction::Interpolate( ClientPtr clt, double blend)
{
	Unit * un = clt->game_unit.GetUnit();

	// Only interpolate between old position and current one -> should prolly first extrapolate the next new position
	// and then interpolate between current position and the next new one
	const Transformation old_pos( un->old_state.getOrientation(), un->old_state.getPosition());
	const Transformation cur_pos( un->curr_physical_state.orientation, un->curr_physical_state.position);

	return linear_interpolate(  old_pos, cur_pos, blend);
}

/*************************************************************************************/
/**** CubicSplinePrediction class : based on cubic spline interpolation           ****/
/*************************************************************************************/

void		CubicSplinePrediction::InitInterpolation( ClientPtr clt)
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
	QVector A( un->old_state.getPosition());
	QVector B( un->curr_physical_state.position);
	Vector  VA( un->old_state.getVelocity());
	Vector  VB( un->Velocity);
	Vector  AA( un->old_state.getAcceleration());
	Vector  AB( un->GetAcceleration());
	QVector A1( A + VA);
	// A2 is computed from position B and velocity VB
	QVector A3( B + VB*delay + AB*delay*delay*0.5);
	QVector A2( A3 - (VB + AB*delay));

	//Clients.get(clientid)->spline.createSpline( A, A1, A2, A3);
}

Quaternion	CubicSplinePrediction::InterpolateOrientation( ClientPtr clt, double blend)
{
	Quaternion orient;

	return orient;
}

QVector		CubicSplinePrediction::InterpolatePosition( ClientPtr clt, double blend)
{
	// There should be another function called when received a new position update and creating the spline
	QVector pos( this->computePoint( blend));
	return pos;
}

/*************************************************************************************/
/**** MixedPrediction class : Linear for orientation, CubicSpline for position    ****/
/*************************************************************************************/

Transformation MixedPrediction::Interpolate( ClientPtr clt, double blend)
{
	Transformation linear( LinearPrediction::Interpolate( clt, blend));
	linear.position = CubicSplinePrediction::InterpolatePosition( clt, blend);
	return linear;
}

/*
QVector			MixedPrediction::InterpolatePosition( ClientPtr, double blend)
{ 
	return CubicSplinePrediction::InterpolatePosition( clt, blend);
}

Orientation		MixedPrediction::InterpolateOrientation( ClientPtr clt, double blend)
{
	return LinearPrediction::InterpolateOrientation( clt, blend);
}
*/

