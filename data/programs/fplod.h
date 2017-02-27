#ifndef VGL_NV_fragment_program2

#ifndef GL_ARB_shader_texture_lod
#define VGL_ARB_shader_texture_lod 0
#else
#extension GL_ARB_shader_texture_lod : enable
#define VGL_ARB_shader_texture_lod GL_ARB_shader_texture_lod
#endif

#if (VGL_ARB_shader_texture_lod == 0)

#ifndef GL_ATI_shader_texture_lod
#define VGL_ATI_shader_texture_lod 0
#else
#extension GL_ATI_shader_texture_lod : enable
#define VGL_ATI_shader_texture_lod GL_ATI_shader_texture_lod
#endif

#if (VGL_ATI_shader_texture_lod == 0)
#define NO_TEXTURE_LOD 1
#endif

#endif

#endif

// keep it below extensions
#include "config.h"


#ifdef NO_TEXTURE_LOD

vec4 texture1DLod(sampler1D sampler, float P, float lod)
{
    // Turn into bias
    if (lod <= 1.0)
        lod = -10.0;
    return texture1D(sampler, P, lod-1.0);
}

vec4 texture2DLod(sampler2D sampler, vec2 P, float lod)
{
    // Turn into bias
    if (lod <= 1.0)
        lod = -10.0;
    return texture2D(sampler, P, lod-1.0);
}

vec4 texture3DLod(sampler3D sampler, vec3 P, float lod)
{
    // Turn into bias
    if (lod <= 1.0)
        lod = -10.0;
    return texture3D(sampler, P, lod-1.0);
}

vec4 textureCubeLod(samplerCube sampler, vec3 P, float lod)
{
    // Turn into bias
    if (lod <= 1.0)
        lod = -10.0;
    return textureCube(sampler, P, lod-1.0);
}

vec4 texture1DGradARB(sampler1D sampler, float P, float dPdx, float dPdy)
{
    return texture1D(sampler, P);
}

vec4 texture2DGradARB(sampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy)
{
    return texture2D(sampler, P);
}

vec4 texture3DGradARB(sampler3D sampler, vec3 P, vec3 dPdx, vec3 dPdy)
{
    return texture3D(sampler, P);
}

vec4 textureCubeGradARB(samplerCube sampler, vec3 P, vec3 dPdx, vec3 dPdy)
{
    return textureCube(sampler, P);
}

#endif
