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
		virtual void			InitInterpolation( ClientPtr clt);
		virtual QVector			InterpolatePosition( ClientPtr) = 0;
		virtual Quaternion		InterpolateOrientation( ClientPtr clt) = 0;
		virtual Transformation	Interpolate( ClientPtr clt);
};

class NullPrediction : public Prediction
{
	public:
		virtual void			InitInterpolation( ClientPtr clt);
		virtual QVector			InterpolatePosition( ClientPtr clt);
		virtual Quaternion		InterpolateOrientation( ClientPtr clt);
};

class LinearPrediction : public Prediction
{
	public:
		virtual QVector			InterpolatePosition( ClientPtr);
		virtual Quaternion		InterpolateOrientation( ClientPtr clt);
		virtual Transformation	Interpolate( ClientPtr clt);
};

class CubicSplinePrediction : public Prediction, public CubicSpline
{
	public:
		virtual void			InitInterpolation( ClientPtr clt);
		virtual QVector			InterpolatePosition( ClientPtr);
		virtual Quaternion		InterpolateOrientation( ClientPtr clt);
};

class MixedPrediction : public LinearPrediction, public CubicSplinePrediction
{
	public:
		virtual Transformation	Interpolate( ClientPtr clt);
};

#endif

