#ifndef __PREDICTION_H
#define __PREDICTION_H

#include "clientptr.h"
#include "gfx/quaternion.h"
#include "cubicsplines.h"

enum	interpolationMethod { Linear, Spline };

class Prediction
{
	protected:
		// A2 is the predicted position after InitInterpolation()
		QVector					A0, B, A1, A2, A3;
		Vector					VA, VB, AA, AB;
		Quaternion				OA, OB;
		
		double					deltatime;
		
	public:
		Prediction();
		virtual ~Prediction();
		virtual void			InitInterpolation( Unit * un, const ClientState &last_packet_state, double elapsed_since_last_packet, double deltatime);

	// Interpolate between current received position and predicted position at time 0+deltatime
	// On the first call, deltatime is the lag value between client and server
	// for the following calls deltatime = lag+elapsed time since we received last packet
		virtual QVector			InterpolatePosition( Unit * un, double deltatime) const = 0;
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime) const = 0;
		virtual Transformation	Interpolate( Unit * un, double deltatime) const;
};

class NullPrediction : public Prediction
{
	public:
		virtual void			InitInterpolation( Unit * un, const ClientState &last_packet_state, double elapsed_since_last_packet, double deltatime);
		virtual QVector			InterpolatePosition( Unit * un, double deltatime) const;
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime) const;
};

class LinearPrediction : public Prediction
{
	public:
		virtual QVector			InterpolatePosition( Unit * un, double deltatime) const;
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime) const;
		virtual Transformation	Interpolate( Unit * un, double deltatime) const;
};

class CubicSplinePrediction : public Prediction
{
	private:
		CubicSpline interpolation_spline;
	public:
		virtual void			InitInterpolation( Unit * un, const ClientState &last_packet_state, double elapsed_since_last_packet, double deltatime);
		virtual QVector			InterpolatePosition( Unit * un, double deltatime) const;
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime) const;
};

class MixedPrediction : public CubicSplinePrediction, public LinearPrediction
{
	public:
		virtual QVector			InterpolatePosition( Unit * un, double deltatime) const;
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime) const;
		virtual Transformation	Interpolate( Unit * un, double deltatime) const;
};

#endif

