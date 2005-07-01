#include "client.h"
#include "prediction.h"
#include "gfx/lerp.h"
#include "cmd/unit_generic.h"

/*************************************************************************************/
/**** Prediction virtual class                                                    ****/
/*************************************************************************************/

Prediction::Prediction():
		A0(0,0,0),B(0,0,0),A1(0,0,0),A2(0,0,0),A3(0,0,0),VA(0,0,0),VB(0,0,0),AA(0,0,0),AB(0,0,0),OA(),OB() {
}
Prediction::~Prediction()
{
}

void		Prediction::InitInterpolation( Unit * un, const ClientState &last_packet_state, double elapsed_since_last_packet, double deltatime)
{
	// This function is to call after the state have been updated (which should be after receiving a SNAPSHOT)

	// This function computes 4 splines points needed for a spline creation
	//    - compute a point on the current spline using blend as t value
	//    - parameter A and B are old_position and new_position (received in the latest packet)

	/************* VA IS TO BE UNCOMMENTED ****************/
	//Unit * un = clt->game_unit.GetUnit();


// NETFIXME: Why cast to int and then back to double?
//	double delay = (double)(unsigned int)deltatime;

	
	double delay = deltatime;
	this->deltatime=deltatime;
	// A is last known position and B is the position we just received
	// A1 is computed from position A and velocity VA
	A0 = un->old_state.getPosition();
	B = un->curr_physical_state.position;
	OA = un->old_state.getOrientation();
	OB = un->curr_physical_state.orientation;
	VA = un->old_state.getVelocity();
	VB = un->Velocity;
	AA = un->old_state.getAcceleration();
	if (elapsed_since_last_packet>0.) {
		AB = (last_packet_state.getVelocity()-VB)/((float)elapsed_since_last_packet);
	} else {
		AB = un->GetAcceleration();
	}
	// A1 = old_position + old_velocity * 1 sec
	A1 = A0 + VA;
	// A3 is computed from position B and velocity VB
	// A3 = new_position + new_velocity * packet_delay + new_acceleration/2 * packet_delay^2
	A3 = B + VB*delay + AB*delay*delay*0.5;
	// A2 is the predicted position
	// A2 = new_position + new_velocity * (packet_delay - 1 sec) + new_acceleration * (packet_delay - 1 sec)^2
	A2 = A3 - (VB + AB*delay);
}

Transformation Prediction::Interpolate( Unit * un, double deltatime) const
{
	return Transformation( InterpolateOrientation( un, deltatime), InterpolatePosition( un, deltatime));
}

/*************************************************************************************/
/**** NullPrediction class : doesn't do any prediction/interpolation              ****/
/*************************************************************************************/

void		NullPrediction::InitInterpolation( Unit * un, const ClientState &last_packet_state, double elapsed_since_last_packet, double deltatime)
{
}

QVector		NullPrediction::InterpolatePosition( Unit * un, double deltatime) const
{
	return un->curr_physical_state.position;
}

Quaternion	NullPrediction::InterpolateOrientation( Unit * un, double deltatime) const
{
	return un->curr_physical_state.orientation;
}

/*************************************************************************************/
/**** LinearPrediction class : based on lerp.c stuff also used in non networking  ****/
/*************************************************************************************/

QVector		LinearPrediction::InterpolatePosition( Unit * un, double deltatime) const
{
	return Interpolate(un,deltatime).position;
}

Quaternion	LinearPrediction::InterpolateOrientation( Unit * un, double deltatime) const
{
	return Interpolate(un,deltatime).orientation;
}

Transformation LinearPrediction::Interpolate( Unit * un, double deltatime) const
{
	const Transformation old_pos( OA, A0);//un->curr_physical_state);
	const Transformation new_pos( OB, A2);
	if (deltatime>this->deltatime||this->deltatime==0) {
		return un->curr_physical_state;
	} else {
		return (linear_interpolate_uncapped(  old_pos, new_pos, deltatime/this->deltatime));
	}
}

/*************************************************************************************/
/**** CubicSplinePrediction class : based on cubic spline interpolation           ****/
/*************************************************************************************/

void		CubicSplinePrediction::InitInterpolation( Unit * un, const ClientState &last_packet_state, double elapsed_since_last_packet, double deltatime)
{
	Prediction::InitInterpolation( un, last_packet_state, elapsed_since_last_packet, deltatime);

	interpolation_spline.createSpline( A0, A1, A2, A3);
}

// Don't know how to do here maybe should have a Spline for each 3 vectors of the rotation quaternion 
// Do at least linear interpolation since we inherit from LinearPrediction
Quaternion	CubicSplinePrediction::InterpolateOrientation( Unit * un, double deltatime) const
{
	return un->curr_physical_state.orientation;
}

QVector		CubicSplinePrediction::InterpolatePosition( Unit * un, double deltatime) const
{
	// There should be another function called when received a new position update and creating the spline
	if (this->deltatime==0) {
		return un->curr_physical_state.position;
	} else {
		return QVector ( interpolation_spline.computePoint( deltatime/this->deltatime));
	}
}

/*************************************************************************************/
/**** MixedPrediction class : Linear for orientation, CubicSpline for position    ****/
/*************************************************************************************/

Quaternion MixedPrediction::InterpolateOrientation( Unit * un, double deltatime) const
{
	return LinearPrediction::InterpolateOrientation(un, deltatime);
}

QVector MixedPrediction::InterpolatePosition( Unit * un, double deltatime) const
{
	return CubicSplinePrediction::InterpolatePosition(un, deltatime);
}

Transformation MixedPrediction::Interpolate( Unit * un, double deltatime) const
{
	Transformation linear( LinearPrediction::Interpolate( un, deltatime));
	linear.position = CubicSplinePrediction::InterpolatePosition( un, deltatime);
	return linear;
}

