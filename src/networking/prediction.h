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

	public:
		Prediction();
		virtual ~Prediction();
		virtual void			InitInterpolation( Unit * un, double deltatime);

	// Interpolate between current received position and predicted position at time 0+deltatime
	// On the first call, deltatime is the lag value between client and server
	// for the following calls deltatime = lag+elapsed time since we received last packet
		virtual QVector			InterpolatePosition( Unit * un, double deltatime) = 0;
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime) = 0;
		virtual Transformation	Interpolate( Unit * un, double deltatime);
};

class NullPrediction : public Prediction
{
	public:
		virtual void			InitInterpolation( Unit * un, double deltatime);
		virtual QVector			InterpolatePosition( Unit * un, double deltatime);
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime);
};

class LinearPrediction : public Prediction
{
	public:
		virtual QVector			InterpolatePosition( Unit * un, double deltatime);
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime);
		virtual Transformation	Interpolate( Unit * un, double deltatime);
};

class CubicSplinePrediction : public LinearPrediction, public CubicSpline
{
	public:
		virtual void			InitInterpolation( Unit * un, double deltatime);
		virtual QVector			InterpolatePosition( Unit * un, double deltatime);
		virtual Quaternion		InterpolateOrientation( Unit * un, double deltatime);
};

class MixedPrediction : public CubicSplinePrediction
{
	public:
		virtual Transformation	Interpolate( Unit * un, double deltatime);
};

#endif

