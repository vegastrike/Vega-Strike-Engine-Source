#include <vector>
#include "gfxlib.h"
#include "gfx/matrix.h"
#include "gfx/mesh.h"

class Mesh;
class Atmosphere {
public:
	struct Parameters {
	/* High level parameters */
	/* coefficients for capturing scattering effects of the 
	   atmospheric composition/density */
	float radius;
	GFXColor low_color[2]; /* low angle */
	GFXColor low_ambient_color[2]; /* low angle */
	GFXColor high_color[2]; /* high angle */
	GFXColor high_ambient_color[2]; /* high angle */
	float scattering; /* "scattering" coefficient, basically 
	                 how much the low_color spreads around */
	Parameters() { };
	};
private:
	Parameters user_params;

	struct SunBox {
		Mesh *sunbox;
		SunBox(Mesh *m) : sunbox(m) { }
		~SunBox() { delete sunbox; }
	};

	/* Internal state information */
	Mesh *dome; /* entire sky; for texture mapping */
	/* 3 rendering passes, to preserve framerate */
	Mesh *cap; /* mid to zenith */
	Mesh *stack; /* horizon to zenith */
	std::vector <SunBox *> sunboxes; /* box around the sun (single star for now) */
	Matrix tmatrix;
	QVector position;
	/* lighting contexts for the above 3 */
public:

	Atmosphere(const Parameters &params);
	~Atmosphere();
	const Parameters &parameters();
	void SetParameters(const Parameters &params);

	/* Update the precomputed mesh to the new parameters. This is 
	   expensive, so let the upper layers decide when to do this
	*/

	/* ugh, make update just change the lighting state */
	void Update(const QVector &position, const Matrix &tmatrix);
	void Draw();
	static void ProcessDrawQueue();
	void SetMatricesAndDraw (const QVector &position, const Matrix tmatrix); /* inherit the orientation of 
	                                    the transformation matrix  */
	static void DrawAtmospheres();
};
