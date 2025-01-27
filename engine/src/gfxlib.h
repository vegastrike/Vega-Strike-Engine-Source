/*
 * gfxlib.h
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_GFXLIB_H
#define VEGA_STRIKE_ENGINE_GFXLIB_H

#if defined (_WIN32) && !defined (__CYGWIN__)
#ifdef GLDRV_EXPORTS
#define GFXDRVAPI __declspec( dllexport )
#else //GLDRV_EXPORTS 
#define GFXDRVAPI __declspec( dllimport )
#endif //GLDRV_EXPORTS
#else //defined (_WIN32) && !defined (__CYGWIN__)
#define GFXDRVAPI
#endif //defined (_WIN32) && !defined (__CYGWIN__)

/*
 * #if 0
 * #if defined(__APPLE__) || defined(MACOSX)
 * #include <GLUT/glut.h>
 * #else
 * #include <GL/glut.h>
 * #endif
 * #endif
 */

#include <cstddef>
#include <vector>
class Matrix;

using std::vector;

#include "gfx/vec.h"
#include "gfxlib_struct.h"

//Init functions

///Loads Glut and graphics drivers
void /*GFXDRVAPI*/ GFXInit(int, char **);

///loops on the function over and over
void /*GFXDRVAPI*/ GFXLoop(void main_loop());

///Shuts down the graphics subsystem
void /*GFXDRVAPI*/ GFXShutdown();

//Misc functions

///resets all local lights and sets identity matrix
void /*GFXDRVAPI*/ GFXBeginScene();
///Flushes and Swaps buffers
void /*GFXDRVAPI*/ GFXEndScene();
///Clears the Z buffer. Also clears color buffer if GFXTRUE passed in
void /*GFXDRVAPI*/ GFXClear(const GFXBOOL colorbuffer,
        const GFXBOOL depthbuffer = GFXTRUE,
        const GFXBOOL stencilbuffer = GFXTRUE);

///creates a light context (relevant to a star system) to add lights to
void /*GFXDRVAPI*/ GFXCreateLightContext(int &con_number);
///Deletes a light context (hence a star system)
void /*GFXDRVAPI*/ GFXDeleteLightContext(const int con_number);
///Sets active light context (restores all gllights in such context)
void /*GFXDRVAPI*/ GFXSetLightContext(const int con_number);
///Sets the ambient light value
GFXBOOL /*GFXDRVAPI*/ GFXLightContextAmbient(const GFXColor &amb);
///Gets the ambient light value
GFXBOOL /*GFXDRVAPI*/ GFXGetLightContextAmbient(GFXColor &amb);
///picks and activates local lights near to "center"
void /*GFXDRVAPI*/ GFXPickLights(const Vector &center, const float radius);
///picks and does not activate local lights near to "center"
void /*GFXDRVAPI*/ GFXPickLights(const Vector &center,
        const float radius,
        vector<int> &lights,
        const int maxlights,
        const bool pickglobals);
///activates local lights picked by GFXPickLight
void /*GFXDRVAPI*/ GFXPickLights(vector<int>::const_iterator begin, vector<int>::const_iterator end);
///loads "lights" with all enabled global lights, computing occlusion to the specified position too
void /*GFXDRVAPI*/ GFXGlobalLights(vector<int> &lights, const Vector &center, const float radius);
///loads "lights" with all enabled global lights
void /*GFXDRVAPI*/ GFXGlobalLights(vector<int> &lights);
///Sets light position offset, use when centering the camera off-origin
void /*GFXDRVAPI*/ GFXSetLightOffset(const QVector &offset);
///Sets light position offset, use when centering the camera off-origin
QVector /*GFXDRVAPI*/ GFXGetLightOffset();
///Sets the light model to have separate specular color (if available)
GFXBOOL /*GFXDRVAPI*/ GFXSetSeparateSpecularColor(const GFXBOOL spec);
///Sets the intensity cutoff before picked lights are ignored
GFXBOOL /*GFXDRVAPI*/ GFXSetCutoff(const float cutoff);
///Sets the optimal intensity given that Optimal number of lights is fulfilled
void /*GFXDRVAPI*/ GFXSetOptimalIntensity(const float newint, const float saturatevalue);
///Sets number of lights graphics can handle optimally
GFXBOOL /*GFXDRVAPI*/ GFXSetOptimalNumLights(const int numlights);

/**
 * GFXCreateLight gets a light as a parameter.  It sets light to have the
 * index of the created light for modification in the future.
 * If the light is global, it will always affect objects in space.
 * if the light is not global, it is local and goes into a 3d hashtable
 * to be picked at a later point on a per object basis.
 */
GFXBOOL /*GFXDRVAPI*/ GFXCreateLight(int &light, const GFXLight &, const bool global);

///Deletes a light, removing it from the light table if necessary
void /*GFXDRVAPI*/ GFXDeleteLight(const int light);

///activates a light if not already activated
GFXBOOL /*GFXDRVAPI*/ GFXEnableLight(const int light);

///Deactivates an active light.
GFXBOOL /*GFXDRVAPI*/ GFXDisableLight(const int light);

///Modifies the parameters of the given light
GFXBOOL /*GFXDRVAPI*/ GFXSetLight(const int light, const enum LIGHT_TARGET, const GFXColor &color);

///Retrieves the parameters of the given light
const GFXLight & /*GFXDRVAPI*/ GFXGetLight(const int light);

/**
 * In the case of shields and other T&L based effects, the global lights
 * must be disabled yet saved for later use while the selected lights are
 * used shortly then disabled. This saves values of global lights
 */
void /*GFXDRVAPI*/ GFXPushGlobalEffects();

///This function undoes the above function to restore the global lights after specialFX phase
GFXBOOL /*GFXDRVAPI*/ GFXPopGlobalEffects();

///Creates a new material given a material struct.  Will search through already made materials for matching materials to minimize switching
void /*GFXDRVAPI*/ GFXSetMaterial(unsigned int &number, const GFXMaterial &material);
///Modifies the given material to be like the passed in structure
void /*GFXDRVAPI*/ GFXModifyMaterial(const unsigned int number, const GFXMaterial &material);
///Gets the statistics of the material stored in number
GFXBOOL /*GFXDRVAPI*/ GFXGetMaterial(const unsigned int number, GFXMaterial &material);
const GFXMaterial &GFXGetMaterial(const unsigned int number);
///Tells Graphics library which material to use on next polygon if lighting enabled
void /*GFXDRVAPI*/ GFXSelectMaterialHighlights(const unsigned int number,
        const GFXColor &ambient,
        const GFXColor &diffuse,
        const GFXColor &specular,
        const GFXColor &emmissive);
void /*GFXDRVAPI*/ GFXSelectMaterial(const unsigned int number);

//Matrix
/**
 * Hud Mode saves the current matrices and sets projection and view matrices to identity.
 * Caution: use of other matrix functions in HudMode could alter state.
 */
void /*GFXDRVAPI*/ GFXHudMode(const bool Enter);
//this resets the matrices to what they were when we entered it, without chanigng pushed orpoped matrices
void /*GFXDRVAPI*/ GFXRestoreHudMode();
void /*GFXDRVAPI*/ GFXCenterCamera(const bool Enter);
void /*GFXDRVAPI*/ GFXTranslateView(const QVector &r);
void /*GFXDRVAPI*/ GFXLoadMatrixView(const Matrix &matrix);
void /*GFXDRVAPI*/ GFXGetMatrixView(Matrix &m);
///Translates the current "mode" matrix by a given vector

void /*GFXDRVAPI*/ GFXTranslateProjection(const Vector &r);
void /*GFXDRVAPI*/ GFXTranslateModel(const QVector &r);
///Multipliex the current "mode" matrix by a given matrix
void /*GFXDRVAPI*/ GFXMultMatrixModel(const Matrix &matrix);

///loads a given matrix to the current "mode"
void /*GFXDRVAPI*/ GFXLoadMatrixModel(const Matrix &matrix);
void /*GFXDRVAPI*/ GFXLoadMatrixProjection(const float matrix[16]);

void /*GFXDRVAPI*/ GFXLoadMatrixView(const Matrix &matrix);

///Loads the identity matrix for the given mode
void /*GFXDRVAPI*/ GFXLoadIdentity(const MATRIXMODE mode);

///retrieves the matrix for a given mode.
void /*GFXDRVAPI*/ GFXGetMatrixModel(Matrix &matrix);

///Given the current projection matrix, how much will the model be divided by
float /*GFXDRVAPI*/ GFXGetZPerspective(const float z);
///Screen to eye
float /*GFXDRVAPI*/ GFXGetXInvPerspective();
float /*GFXDRVAPI*/ GFXGetYInvPerspective();

///Sets the Projection matrix to have fov and aspect as follows (fov is field of view in radians, aspect is width/height znear and zfar are clip planes
void /*GFXDRVAPI*/ GFXPerspective(float fov, float aspect, float znear, float zfar, float cockpit_offset);

///Sets the Projection matrix to a parallel view with given paramters
void /*GFXDRVAPI*/ GFXParallel(float left, float right, float bottom, float top, float znear, float zfar);

///Sets the final translation to screen coordinates, Also adjusts range of clearing
void /*GFXDRVAPI*/ GFXViewPort(int minx, int miny, int maxx, int maxy);

///Sets the VIEW matrix to look from center in direction of eye with up vector up
void /*GFXDRVAPI*/ GFXLookAt(Vector eye, QVector center, Vector up);

///Gets the 6 clip planes of the current Projection matrix
void /*GFXDRVAPI*/ GFXGetFrustum(double f[6][4]);

///Calculates frustum matrix (internal use)
void /*GFXDRVAPI*/ GFXCalculateFrustum();

///Calculates the planes for a given frustum in 3space given a matrix and a projection.
void /*GFXDRVAPI*/ GFXCalculateFrustum(double frustum[6][4], const Matrix &modlmatrix, const float *projection);

///Saves and restores last used left,right,bot,top,near,far vals (internal use)
void /*GFXDRVAPI*/ GFXGetFrustumVars(bool, float *l, float *r, float *b, float *t, float *n, float *f);

///checks if a sphere is in the currently loaded Frustum given its center and radius
float /*GFXDRVAPI*/ GFXSphereInFrustum(const QVector &Center, float Radius);

///Checks if a sphere is in the given frustum calculated by GFXCalculateFrustum. Used in Unit clipping
float /*GFXDRVAPI*/ GFXSphereInFrustum(double f[6][4], const QVector &Center, float Radius);

void /*GFXDRVAPI*/ GFXBoxInFrustumModel(const Matrix &model);

CLIPSTATE /*GFXDRVAPI*/ GFXBoxInFrustum(const Vector &min, const Vector &max);

CLIPSTATE /*GFXDRVAPI*/ GFXBoxInFrustum(double f[6][4], const Vector &min, const Vector &max);
CLIPSTATE /*GFXDRVAPI*/ GFXTransformedBoxInFrustum(const Vector &min, const Vector &max);
CLIPSTATE /*GFXDRVAPI*/ GFXSpherePartiallyInFrustum(const Vector &cent, const float r);
CLIPSTATE /*GFXDRVAPI*/ GFXTransformedSpherePartiallyInFrustum(const Vector &cent, const float r);
CLIPSTATE /*GFXDRVAPI*/ GFXSpherePartiallyInFrustum(double f[6][4], const Vector &cent, const float r);

///Given matrices, calculates the matrix and inverse matrix of a projection matrix to go from screen to 3-space coordinates
void /*GFXDRVAPI*/ GFXFrustum(float *mat,
        float *inv,
        float left,
        float right,
        float bottom,
        float top,
        float nearval,
        float farval);

//Textures
/**
 * Creates a texture with given w,h & tex format. Returns handle in handle var
 * The palette may be specified but defaults to NULL.  Texture Stage indicates
 * Which texture unit the texture must reside on.  The filter indicates
 * If linear, bilinear, mipmapping or trilinear filterring should be used.
 * Filtering is clamped to g_game.mipmap so hardware isn't abused (3dfx had
 * trilinear troubles, and it might be a memory constraint)
 * Texture target defines the type of texture it is for eventual cube mapping
 */
GFXBOOL /*GFXDRVAPI*/ GFXCreateTexture(int width,
        int height,
        TEXTUREFORMAT externaltextureformat,
        int *handle,
        char *palette = 0,
        int texturestage = 0,
        enum FILTER mipmap = MIPMAP,
        enum TEXTURE_TARGET texture_target = TEXTURE2D,
        enum ADDRESSMODE address_mode = DEFAULT_ADDRESS_MODE);

///Sets the priority of the texture for memory management.
void /*GFXDRVAPI*/ GFXPrioritizeTexture(unsigned int handle, float priority);

///Attaches a given palette to the current texture
void /*GFXDRVAPI*/ GFXAttachPalette(unsigned char *palette, int handle);

/**
 * Transfers texture to the graphics card with given buffer
 * in previously specified format. Scales texture approrpiately to
 * Max texture sizes gotten from vid card
 */
GFXBOOL /*GFXDRVAPI*/ GFXTransferTexture(unsigned char *buffer,
        int handle,
        int inWidth,
        int inHeight,
        enum TEXTUREFORMAT internalformat,
        enum TEXTURE_IMAGE_TARGET image2D = TEXTURE_2D,
        int max_texture_dimension = 65536 /*make sure it's not bigger than this--usually maybe 65536*/,
        GFXBOOL detailtexture = GFXFALSE,
        unsigned int pageIndex = 0);

GFXBOOL /*GFXDRVAPI*/ GFXTransferSubTexture(unsigned char *buffer,
        int handle,
        int x,
        int y,
        unsigned int width,
        unsigned int height,
        enum TEXTURE_IMAGE_TARGET image2D = TEXTURE_2D);

///Deletes the texture from the graphics card
void /*GFXDRVAPI*/ GFXDeleteTexture(size_t handle);

///Cleans up all textures
void /*GFXDRVAPI*/ GFXDestroyAllTextures();

///Selects the current texture as being "active" so further drawn objects will have it
void /*GFXDRVAPI*/ GFXSelectTexture(int handle, int stage = 0);

///Screen capture (FIXME)
GFXBOOL /*GFXDRVAPI*/ GFXCapture(char *filename);

//State
///Enables and disables given state
void /*GFXDRVAPI*/ GFXEnable(const enum STATE);
void /*GFXDRVAPI*/ GFXDisable(const enum STATE);
void /*GFXDRVAPI*/ GFXToggleTexture(bool enable, int whichstage, enum TEXTURE_TARGET target = TEXTURE2D);
///Sets texture to clamp or wrap texture coordinates
void /*GFXDRVAPI*/ GFXTextureAddressMode(const ADDRESSMODE mode, enum TEXTURE_TARGET target = TEXTURE2D);

///Sets the current blend mode to src,dst
void /*GFXDRVAPI*/ GFXBlendMode(const enum BLENDFUNC src, const enum BLENDFUNC dst);

///Gets the current blend mode to src,dst
void /*GFXDRVAPI*/ GFXGetBlendMode(enum BLENDFUNC &src, enum BLENDFUNC &dst);

void /*GFXDRVAPI*/ GFXColorMaterial(int LIGHTTARG);

///Sets the size in pixels of a GFXPOINT
void /*GFXDRVAPI*/ GFXPointSize(const float size);

///Sets the line width in pixels of a GFXLINE
void /*GFXDRVAPI*/ GFXLineWidth(const float size);

///Pushes and saves current blend mode
void /*GFXDRVAPI*/ GFXPushBlendMode();

///Restores previous blend mode
void /*GFXDRVAPI*/ GFXPopBlendMode();

///Sets the active texture stage in multitexture
void /*GFXDRVAPI*/ GFXActiveTexture(const int stage);

///Returns the current depth function
enum DEPTHFUNC /*GFXDRVAPI*/ GFXDepthFunc();

///Sets up depth compare function
void /*GFXDRVAPI*/ GFXDepthFunc(const enum DEPTHFUNC);

///Returns the current stencil test function
enum DEPTHFUNC /*GFXDRVAPI*/ GFXStencilFunc();

///Gets details about the current stancil compare function - Specify NULL for unwanted fields.
void /*GFXDRVAPI*/ GFXStencilFunc(enum DEPTHFUNC *pFunc, int *pRef, int *pMask);

///Sets up a stencil compare function
void /*GFXDRVAPI*/ GFXStencilFunc(enum DEPTHFUNC sfunc, int ref, unsigned int mask);

///Gets the current stencil write operations - Specify NULL for unwanted fields
void /*GFXDRVAPI*/ GFXStencilOp(enum STENCILOP *pFail, enum STENCILOP *pZfail, enum STENCILOP *pZpass);

///Sets up the stencil write operations
void /*GFXDRVAPI*/ GFXStencilOp(enum STENCILOP fail, enum STENCILOP zfail, enum STENCILOP zpass);

///Returns the current stencil write mask
unsigned int /*GFXDRVAPI*/ GFXStencilMask();

///Sets the stencil write mask
void /*GFXDRVAPI*/ GFXStencilMask(unsigned int mask);

///Turns on alpha testing mode (or turns if off if DEPTHFUNC is set to ALWAYS
void /*GFXDRVALP*/ GFXAlphaTest(const enum DEPTHFUNC, const float ref);
enum GFXTEXTUREWRAPMODES {
    GFXREPEATTEXTURE,
    GFXCLAMPTEXTURE,     //clamp to edge, not to border
    GFXBORDERTEXTURE     //clamp to border, not to edge
};
enum GFXTEXTUREENVMODES {
    GFXREPLACETEXTURE,
    GFXADDTEXTURE,
    GFXMODULATETEXTURE,
    GFXADDSIGNEDTEXTURE,
    GFXCOMPOSITETEXTURE,
    GFXINTERPOLATETEXTURE,
    GFXDETAILTEXTURE
};
void GFXTextureWrap(int stage, GFXTEXTUREWRAPMODES mode, enum TEXTURE_TARGET target = TEXTURE2D);
void GFXTextureEnv(int stage, GFXTEXTUREENVMODES mode, float arg2 = 0);
bool GFXMultiTexAvailable();
///Sets Depth Offset for polgyons
void /*GFXDRVAPI*/ GFXPolygonOffset(float factor, float units);
void GFXGetPolygonOffset(float *factor, float *units);
///Sets the polygon rasterization mode
void /*GFXDRVAPI*/ GFXPolygonMode(const enum POLYMODE);
///Sets the facecull mode
void /*GFXDRVAPI*/ GFXCullFace(const enum POLYFACE);

///Specifies a color for henceforth drawn vertices to share
void /*GFXDRVAPI*/ GFXColorf(const GFXColor &col);
///specifies the current blend color
void /*GFXDRVAPI*/ GFXBlendColor(const GFXColor &col);
///Specifies a color for henceforth drawn vertices to share
void /*GFXDRVAPI*/ GFXColor4f(const float r, const float g, const float b, const float a = 1.0);
///Gets the current color
GFXColor /*GFXDRVAPI*/ GFXColorf();

void /*GFXDRVAPI*/ GFXCircle(float x, float y, float r1, float r2);
unsigned int /*GFXDRVAPI*/ PolyLookup(POLYTYPE poly);
void /*GFXDRVAPI*/ GFXDraw(POLYTYPE type, const float data[], int vnum,
        int vsize = 3, int csize = 0, int tsize0 = 0, int tsize1 = 0);

void /*GFXDRVAPI*/ GFXDrawElements(POLYTYPE type,
        const float data[],
        int vnum,
        const unsigned char indices[],
        int nelem,
        int vsize = 3,
        int csize = 0,
        int tsize0 = 0,
        int tsize1 = 0);

void /*GFXDRVAPI*/ GFXDrawElements(POLYTYPE type,
        const float data[],
        int vnum,
        const unsigned short indices[],
        int nelem,
        int vsize = 3,
        int csize = 0,
        int tsize0 = 0,
        int tsize1 = 0);

void /*GFXDRVAPI*/ GFXDrawElements(POLYTYPE type, const float data[], int vnum, const unsigned int indices[], int nelem,
        int vsize = 3, int csize = 0, int tsize0 = 0, int tsize1 = 0);

template<int VSIZE, int CSIZE, int TSIZE0, int TSIZE1>
void GFXDraw(POLYTYPE type, const VertexBuilder<float, VSIZE, 0, CSIZE, TSIZE0, TSIZE1> &buffer) {
    if (buffer.size() > 0) {
        GFXDraw(type, buffer.buffer_pointer(), buffer.size(), VSIZE, CSIZE, TSIZE0, TSIZE1);
    }
}

template<typename ITYPE, int VSIZE, int CSIZE, int TSIZE0, int TSIZE1>
void GFXDrawElements(POLYTYPE type,
        const VertexBuilder<float, VSIZE, 0, CSIZE, TSIZE0, TSIZE1> &buffer,
        const ITYPE *indices,
        int nelements) {
    if (buffer.size() > 0 && nelements > 0) {
        GFXDrawElements(type, buffer.buffer_pointer(), buffer.size(), indices, nelements, VSIZE, CSIZE, TSIZE0, TSIZE1);
    }
}

template<typename ITYPE, int VSIZE, int CSIZE, int TSIZE0, int TSIZE1>
void GFXDrawElements(POLYTYPE type,
        const VertexBuilder<float, VSIZE, 0, CSIZE, TSIZE0, TSIZE1> &buffer,
        const std::vector<ITYPE> &indices) {
    if (buffer.size() > 0 && indices.size() > 0) {
        GFXDrawElements(type,
                buffer.buffer_pointer(),
                buffer.size(),
                &indices[0],
                indices.size(),
                VSIZE,
                CSIZE,
                TSIZE0,
                TSIZE1);
    }
}

///Bind VBO data, noop if VBO not supported
void GFXBindBuffer(unsigned int vbo_data);
void GFXBindElementBuffer(unsigned int element_data);
///Optimizes a list to reuse repeated vertices!
void GFXOptimizeList(GFXVertex *old, int numV, GFXVertex **newlist, int *numnewVertices, unsigned int **indices);

void GFXFogMode(const FOGMODE fog);
void GFXFogDensity(const float fogdensity);
void GFXFogLimits(const float fognear, const float fogfar);
void GFXFogColor(GFXColor c);
void GFXFogIndex(const int index);
//display list
///Draws a given display list
void /*GFXDRVAPI*/ GFXCallList(int list);

//pick mode
void /*GFXDRVAPI*/ GFXBeginPick(int x, int y, int xsize, int ysize);
void /*GFXDRVAPI*/ GFXSetPickName(int name);
//bool GFXCheckPicked();
vector<PickData> * /*GFXDRVAPI*/ GFXEndPick();

//Subwindow management
///Creates a subwindow with the current parameters for center and size
void /*GFXDRVAPI*/ GFXSubwindow(int x, int y, int xsize, int ysize);
void /*GFXDRVAPI*/ GFXSubwindow(float x, float y, float xsize, float ysize);

//Coordinate system conversion
///convertes eyespace to 3space at the near plane
Vector /*GFXDRVAPI*/ GFXDeviceToEye(int x, int y);
enum GFXTEXTURECOORDMODE {
    NO_GEN,
    EYE_LINEAR_GEN,
    OBJECT_LINEAR_GEN,
    SPHERE_MAP_GEN,
    CUBE_MAP_GEN
};
void GFXTextureCoordGenMode(int stage, GFXTEXTURECOORDMODE tex, const float params[4], const float paramt[4]);

int GFXCreateProgram(const char *vertex, const char *fragment, const char *extra_defines);
void GFXDestroyProgram(int program);
//program created if necessary and active
int GFXActivateShader(const char *program = NULL /*null for default prog*/ );
int GFXActivateShader(int program);
void GFXDeactivateShader();
//return location of named value
int GFXNamedShaderConstant(char *progID, const char *name);
int GFXNamedShaderConstant(int progID, const char *name);
int GFXShaderConstant(int name, Vector value);
int GFXShaderConstant(int name, GFXColor value);
int GFXShaderConstant(int name, const float *value);
int GFXShaderConstanti(int name, int value);
int GFXShaderConstant(int name, float v1, float v2, float v3, float v4);
int GFXShaderConstant(int name, float v1);
int GFXShaderConstant4v(int name, unsigned int numvals, const float *value);
int GFXShaderConstantv(int name, unsigned int numvals, const float *value);
int GFXShaderConstantv(int name, unsigned int numvals, const int *value);
bool GFXDefaultShaderSupported();
void GFXReloadDefaultShader();
void GFXUploadLightState(int max_light_location,
        int active_light_array,
        int apparent_light_size_array,
        bool shader,
        vector<int>::const_iterator begin,
        vector<int>::const_iterator end);
bool GFXShaderReloaded();
int GFXGetProgramVersion();

#endif //VEGA_STRIKE_ENGINE_GFXLIB_H
