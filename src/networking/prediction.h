#ifndef __PREDICTION_H
#define __PREDICTION_H

#include "clientptr.h"
#include "gfx/quaternion.h"
#include "cubicsplines.h"

enum	interpolationMethod { Linear, Spline };

class Prediction
{
	protected:
		virtual QVector			InterpolatePosition( ClientPtr, double blend) = 0;
		virtual Quaternion		InterpolateOrientation( ClientPtr clt, double blend) = 0;

	public:
		Prediction();
		virtual ~Prediction();
		virtual void			InitInterpolation( ClientPtr clt) = 0;
		virtual Transformation	Interpolate( ClientPtr clt, double blend);
};

class LinearPrediction : public Prediction
{
	protected:
		virtual QVector			InterpolatePosition( ClientPtr, double blend);
		virtual Quaternion		InterpolateOrientation( ClientPtr clt, double blend);

	public:
		virtual void InitInterpolation( ClientPtr clt);
		virtual Transformation	Interpolate( ClientPtr clt, double blend);
};

class CubicSplinePrediction : public Prediction, public CubicSpline
{
	protected:
		virtual QVector			InterpolatePosition( ClientPtr, double blend);
		virtual Quaternion		InterpolateOrientation( ClientPtr clt, double blend);

	public:
		virtual void InitInterpolation( ClientPtr clt);
};

class MixedPrediction : public LinearPrediction, public CubicSplinePrediction
{
	protected:

	public:
		virtual void InitInterpolation( ClientPtr clt);
		virtual Transformation	Interpolate( ClientPtr clt, double blend);
};

#endif

