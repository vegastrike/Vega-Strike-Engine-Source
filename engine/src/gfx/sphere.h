/**
* sphere.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
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

#ifndef _GFX_SPHERE_H_
#define _GFX_SPHERE_H_

#include "mesh.h"
#include "quaternion.h"
#include <assert.h>
#include <string>

#ifndef M_PI
#define M_PI (3.1415926536F)
#endif

class SphereMesh : public Mesh
{
//no local vars allowed
protected:
    virtual float GetT( float rho, float rho_min, float rho_max ) const;
    virtual float GetS( float theta, float theta_min, float theta_max ) const;
    virtual Mesh * AllocNewMeshesEachInSizeofMeshSpace( int num )
    {
        assert( sizeof (Mesh) == sizeof (*this) );
        return new SphereMesh[num];
    }
    void InitSphere( float radius,
                     int stacks,
                     int slices,
                     const char *texture,
                     const std::string &technique, 
                     const char *alpha = NULL,
                     bool insideout = false,
                     const BLENDFUNC a = ONE,
                     const BLENDFUNC b = ZERO,
                     bool envMap = false,
                     float rho_min = 0.0,
                     float rho_max = M_PI,
                     float theta_min = 0.0,
                     float theta_max = 2 *M_PI,
                     FILTER mipmap = MIPMAP,
                     bool reverse_normals = false,
                     bool subclass = false );
public: SphereMesh() : Mesh()
    {
        setConvex( true );
    }
    virtual int MeshType() const
    {
        return 1;
    }
    virtual void SelectCullFace( int whichdrawqueue );
    virtual void RestoreCullFace( int whichdrawqueue );
    SphereMesh( float radius,
                int stacks,
                int slices,
                const char *texture,
                const std::string &technique, 
                const char *alpha = NULL,
                bool insideout = false,
                const BLENDFUNC a = ONE,
                const BLENDFUNC b = ZERO,
                bool envMap = false,
                float rho_min = 0.0,
                float rho_max = M_PI,
                float theta_min = 0.0,
                float theta_max = 2 *M_PI,
                FILTER mipmap = MIPMAP,
                bool reverse_normals = false )
    {
        InitSphere( radius,
                    stacks,
                    slices,
                    texture,
                    technique,
                    alpha,
                    insideout,
                    a,
                    b,
                    envMap,
                    rho_min,
                    rho_max,
                    theta_min,
                    theta_max,
                    mipmap,
                    reverse_normals );
    }
    void Draw( float lod, bool centered = false, const Matrix &m = identity_matrix );
    virtual void ProcessDrawQueue( int whichpass, int which, bool zsort, const QVector &sortctr );
};

class CityLights : public SphereMesh
{
//no local vars allowed
//these VARS BELOW ARE STATIC...change it and DIE
    static float wrapx;
    static float wrapy;
protected:
    virtual float GetT( float rho, float rho_min, float rho_max ) const;
    virtual float GetS( float theta, float theta_min, float theta_max ) const;
    virtual Mesh * AllocNewMeshesEachInSizeofMeshSpace( int num )
    {
        assert( sizeof (Mesh) == sizeof (*this) );
        return new CityLights[num];
    }
public: CityLights() : SphereMesh() {}
    CityLights( float radius,
                int stacks,
                int slices,
                const char *texture,
                int texturewrapx,
                int texturewrapy,
                bool insideout = false,
                const BLENDFUNC a = ONE,
                const BLENDFUNC b = ZERO,
                bool envMap = false,
                float rho_min = 0.0,
                float rho_max = M_PI,
                float theta_min = 0.0,
                float theta_max = 2 *M_PI,
                bool inside_out = true );
    virtual void ProcessDrawQueue( int whichpass, int which, bool zsort, const QVector &sortctr );
};
#endif

