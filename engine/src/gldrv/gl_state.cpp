/*
 * gl_state.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, Alan Shieh, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
#define GL_STATE_CPP
#include "gl_globals.h"
#undef GL_STATE_CPP

#include "gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include <stack>

#ifndef GL_CONSTANT_ALPHA
#define GL_CONSTANT_ALPHA 0x8003
#endif
#ifndef GL_ONE_MINUS_CONSTANT_ALPHA
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#endif
#ifndef GL_ONE_MINUS_CONSTANT_COLOR
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#endif
#ifndef GL_CONSTANT_COLOR
#define GL_CONSTANT_COLOR 0x8001
#endif

GLenum bTex[32] = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
};

GLenum GetGLTextureTarget(enum TEXTURE_TARGET texture_target) {
    GLenum tt;
    switch (texture_target) {
        case TEXTURE1D:
            tt = GL_TEXTURE_1D;
            break;
        case TEXTURE2D:
            tt = GL_TEXTURE_2D;
            break;
        case TEXTURE3D:
            tt = GL_TEXTURE_3D;
            break;
#ifdef NV_CUBE_MAP
        case CUBEMAP:
            tt = GL_TEXTURE_CUBE_MAP_EXT;
            break;
#endif
        case TEXTURERECT:
            tt = GL_TEXTURE_RECTANGLE_ARB;
            break;
        default:
            assert(false);
    }
    return tt;
}

int activeTextureStage = -1; //FIXME Shouldn't this be a member of a class?, or at least be an official global variable?

static inline bool _GFXActiveTextureValid() {
    return !(activeTextureStage && (activeTextureStage >= static_cast<int>(gl_options.Multitexture)));
}

extern GFXBOOL GFXLIGHTING;

void /*GFXDRVAPI*/ GFXEnable(const STATE state) {
    switch (state) {
        case LIGHTING:
            glEnable(GL_LIGHTING);
            GFXLIGHTING = GFXTRUE;
            break;
        case DEPTHTEST:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            //glDepthFunc (GL_ALWAYS);
            break;
        case DEPTHWRITE:
            glDepthMask(GL_TRUE);
            break;
        case COLORWRITE:
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case TEXTURE0:
            /*if (bTex[0] != GL_TEXTURE_2D) {
             *       GFXActiveTexture(0);
             *   if (bTex[0]) glDisable(bTex[0]);
             *       glEnable (bTex[0]=GL_TEXTURE_2D);
             *  }*/
            //NOTE: The above code should be used, but since not all parts of VS use the GFX,
            //we will use GFXEnable() / GFXDisable() as syncrhonization methods (they will
            //assure correct settings of texture units 1 & 2, used during GUI rendering.
            GFXActiveTexture(0);
#ifdef NV_CUBE_MAP
            if (bTex[0] != GL_TEXTURE_CUBE_MAP_EXT) {
                glDisable(GL_TEXTURE_CUBE_MAP_EXT);
            }
#endif
            if (bTex[0] != 0 && bTex[0] != GL_TEXTURE_2D) {
                glDisable(bTex[0]);
            }
            glEnable(bTex[0] = GL_TEXTURE_2D);
            break;
        case TEXTURE1:
            /*if (gl_options.Multitexture) {
             *     GFXActiveTexture (1);
             * #ifdef NV_CUBE_MAP
             *  if (bTex[1] != GL_TEXTURE_CUBE_MAP_EXT) {
             *  if (bTex[1]) glDisable(bTex[1]);
             *         glEnable (bTex[1]=GL_TEXTURE_CUBE_MAP_EXT);
             *  }
             * #else
             *  if (bTex[1] != GL_TEXTURE_2D) {
             *  if (bTex[1]) glDisable(bTex[1]);
             *         glEnable (bTex[1]=GL_TEXTURE_2D);
             *  }
             * #endif
             *  }*/
            //NOTE: The above code should be used, but since not all parts of VS use the GFX,
            //we will use GFXEnable() / GFXDisable() as syncrhonization methods (they will
            //assure correct settings of texture units 1 & 2, both in the GL and their proxy
            //states in the GFX. Those two units are used during GUI rendering bypassing the GFX.
            if (gl_options.Multitexture) {
                GFXActiveTexture(1);
                if (bTex[1] != 0) {
                    glDisable(bTex[1]);
                }
#ifdef NV_CUBE_MAP
                if (bTex[1] != 0 && bTex[1] != GL_TEXTURE_2D) {
                    glDisable(GL_TEXTURE_2D);
                }
                glEnable(bTex[1] = GL_TEXTURE_CUBE_MAP_EXT);
#else
                glEnable( bTex[1] = GL_TEXTURE_2D );
#endif
            }
            break;
        case CULLFACE:
            glEnable(GL_CULL_FACE);
            break;
        case SMOOTH:
            if (gl_options.smooth_lines) {
                glEnable(GL_LINE_SMOOTH);
            }
            if (gl_options.smooth_points) {
                glEnable(GL_POINT_SMOOTH);
            }
            break;
        case STENCIL:
            glEnable(GL_STENCIL);
            break;
    }
}

void GFXToggleTexture(bool enable, int whichstage, enum TEXTURE_TARGET target) {
    if ((whichstage < static_cast<int>(gl_options.Multitexture)) || (whichstage == 0)) {
        GLenum tt = GetGLTextureTarget(target);
        GLenum btt = (enable ? tt : 0);
        if (bTex[whichstage] != btt) {
            GFXActiveTexture(whichstage);
            if (bTex[whichstage]) {
                glDisable(bTex[whichstage]);
            }
            if (enable) {
                glEnable(bTex[whichstage] = tt);
            } else {
                bTex[whichstage] = 0;
            }
        }
    }
}

void /*GFXDRVAPI*/ GFXDisable(const STATE state) {
    switch (state) {
        case LIGHTING:
            glDisable(GL_LIGHTING);
            GFXLIGHTING = GFXFALSE;
            break;
        case DEPTHTEST:
            glDisable(GL_DEPTH_TEST);
            break;
        case DEPTHWRITE:
            glDepthMask(GL_FALSE);
            break;
        case COLORWRITE:
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case TEXTURE0:
            /*if (bTex[0]) {
             *       GFXActiveTexture(0);
             *       glDisable (bTex[0]);
             *       bTex[0] = 0;
             *  }*/
            //NOTE: The above code should be used, but since not all parts of VS use the GFX,
            //we will use GFXEnable() / GFXDisable() as syncrhonization methods (they will
            //assure correct settings of texture units 1 & 2, both in the GL and their proxy
            //states in the GFX. Those two units are used during GUI rendering bypassing the GFX.
            GFXActiveTexture(0);
#ifdef NV_CUBE_MAP
            glDisable(GL_TEXTURE_CUBE_MAP_EXT);
            if (bTex[0] == GL_TEXTURE_CUBE_MAP_EXT) {
                bTex[0] = 0;
            }
#endif
            glDisable(GL_TEXTURE_2D);
            if (bTex[0] != 0 && bTex[0] != GL_TEXTURE_2D) {
                glDisable(bTex[0]);
            }
            bTex[0] = 0;
            break;
        case TEXTURE1:
            /*if (bTex[1]&&gl_options.Multitexture) {
             *   GFXActiveTexture(1);
             *   glDisable(bTex[1]);
             *   bTex[1] = 0;
             *  }*/
            //NOTE: The above code should be used, but since not all parts of VS use the GFX,
            //we will use GFXEnable() / GFXDisable() as syncrhonization methods (they will
            //assure correct settings of texture units 1 & 2, both in the GL and their proxy
            //states in the GFX. Those two units are used during GUI rendering bypassing the GFX.
            if (gl_options.Multitexture) {
                GFXActiveTexture(1);
#ifdef NV_CUBE_MAP
                glDisable(GL_TEXTURE_CUBE_MAP_EXT);
                if (bTex[1] == GL_TEXTURE_CUBE_MAP_EXT) {
                    bTex[1] = 0;
                }
#endif
                glDisable(GL_TEXTURE_2D);
                if (bTex[1] != 0 && bTex[1] != GL_TEXTURE_2D) {
                    glDisable(bTex[1]);
                }
                bTex[1] = 0;
            }
            break;
        case CULLFACE:
            glDisable(GL_CULL_FACE);
            break;
        case SMOOTH:
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_POINT_SMOOTH);
            break;
        case STENCIL:
            glDisable(GL_STENCIL);
            break;
    }
}

#ifndef GL_CLAMP_TO_EDGE_EXT
#define GL_CLAMP_TO_EDGE_EXT 0x812F
#endif
#ifndef GL_CLAMP_TO_BORDER_ARB
#define GL_CLAMP_TO_BORDER_ARB 0x812D
#endif

void GFXTextureAddressMode(const ADDRESSMODE mode, enum TEXTURE_TARGET target) {
    if (!_GFXActiveTextureValid()) {
        return;
    }
    GLenum tt = GetGLTextureTarget(target);
    float BColor[4] = {0, 0, 0, 0};      //set border color to clear... dunno if we wanna change?
    GLenum wm1, wm2;
    switch (mode) {
        case DEFAULT_ADDRESS_MODE:
        case WRAP:
            wm1 = GL_REPEAT;
            wm2 = 0;
            break;
        case BORDER:
            wm1 = GL_CLAMP;
            wm2 = GL_CLAMP_TO_BORDER_ARB;
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BColor);
            //nobreak
        case CLAMP:
            wm1 = GL_CLAMP;
            wm2 = GL_CLAMP_TO_EDGE_EXT;
            break;
        case MIRROR:     //nope not goin here I hope nVidia extension?
        default:
            return;              //won't work
    }
    glTexParameteri(tt, GL_TEXTURE_WRAP_S, wm1);
    if (target != TEXTURE1D) {
        glTexParameteri(tt, GL_TEXTURE_WRAP_T, wm1);
    }
    if (target == TEXTURE3D || target == CUBEMAP) {
        glTexParameteri(tt, GL_TEXTURE_WRAP_R, wm1);
    }
    if (wm2) {
        glTexParameteri(tt, GL_TEXTURE_WRAP_S, wm2);
        if (target != TEXTURE1D) {
            glTexParameteri(tt, GL_TEXTURE_WRAP_T, wm2);
        }
        if (target == TEXTURE3D || target == CUBEMAP) {
            glTexParameteri(tt, GL_TEXTURE_WRAP_R, wm2);
        }
    }
}

struct BlendMode {
    BLENDFUNC sfactor, dfactor;

    BlendMode() {
        sfactor = dfactor = ONE;
    }
}
        currBlendMode;

vega_types::Stack<BlendMode> blendstack;

void /*GFXDRVAPI*/ GFXGetBlendMode(enum BLENDFUNC &src, enum BLENDFUNC &dst) {
    src = currBlendMode.sfactor;
    dst = currBlendMode.dfactor;
}

static GLenum blendToGL(const enum BLENDFUNC func) {
    switch (func) {
        default:
        case ZERO:
            return GL_ZERO;
        case ONE:
            return GL_ONE;
        case SRCALPHA:
            return GL_SRC_ALPHA;
        case INVSRCALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        case DESTALPHA:
            return GL_DST_ALPHA;
        case INVDESTALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
        case DESTCOLOR:
            return GL_DST_COLOR;
        case INVDESTCOLOR:
            return GL_ONE_MINUS_DST_COLOR;
        case SRCALPHASAT:
            return GL_SRC_ALPHA_SATURATE;
#ifndef _WIN32
        case CONSTALPHA:
            return GL_CONSTANT_ALPHA;
        case INVCONSTALPHA:
            return GL_ONE_MINUS_CONSTANT_ALPHA;
        case CONSTCOLOR:
            return GL_CONSTANT_COLOR;
        case INVCONSTCOLOR:
            return GL_ONE_MINUS_CONSTANT_COLOR;
#endif
        case SRCCOLOR:
            return GL_SRC_COLOR;
        case INVSRCCOLOR:
            return GL_ONE_MINUS_SRC_COLOR;
    }
}

void GFXBlendMode(const enum BLENDFUNC src, const enum BLENDFUNC dst) {
    GLenum sfactor = blendToGL(src);
    GLenum dfactor = blendToGL(dst);
    glBlendFunc(sfactor, dfactor);
    currBlendMode.sfactor = src;
    currBlendMode.dfactor = dst;
}

void GFXPushBlendMode() {
    blendstack.push(currBlendMode);
}

void GFXPopBlendMode() {
    if (!blendstack.empty()) {
        currBlendMode = blendstack.top();
        GFXBlendMode(currBlendMode.sfactor, currBlendMode.dfactor);
        blendstack.pop();
    }
}

void GFXColorMaterial(int LIGHTTARG) {
    if (LIGHTTARG) {
        glEnable(GL_COLOR_MATERIAL);
        switch (LIGHTTARG) {
            case EMISSION:
                glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
                break;
            case AMBIENT:
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
                break;
            case DIFFUSE:
                glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
                break;
            case (AMBIENT | DIFFUSE):
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                break;
            case SPECULAR:
                glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
                break;
        }
    } else {
        glDisable(GL_COLOR_MATERIAL);
    }
}

static DEPTHFUNC cur_depth_func = LESS;

enum DEPTHFUNC GFXDepthFunc() {
    return cur_depth_func;
}

void GFXDepthFunc(enum DEPTHFUNC dfunc) {
    GLenum func;
    switch (dfunc) {
        case NEVER:
            func = GL_NEVER;
            break;
        case LESS:
            func = GL_LESS;
            break;
        case EQUAL:
            func = GL_EQUAL;
            break;
        case LEQUAL:
            func = GL_LEQUAL;
            break;
        case GREATER:
            func = GL_GREATER;
            break;
        case NEQUAL:
            func = GL_NOTEQUAL;
            break;
        case GEQUAL:
            func = GL_GEQUAL;
            break;
        case ALWAYS:
            func = GL_ALWAYS;
            break;
        default:
            func = GL_NEVER;
            break;
    }
    glDepthFunc(func);
    cur_depth_func = dfunc;
}

static DEPTHFUNC cur_stencil_func = ALWAYS;
static int cur_stencil_func_ref = 0;
static unsigned int cur_stencil_func_mask = ~0;
static unsigned int cur_stencil_mask = ~0;
static STENCILOP cur_stencil_op_f = KEEP;
static STENCILOP cur_stencil_op_zf = KEEP;
static STENCILOP cur_stencil_op_zp = KEEP;

enum DEPTHFUNC GFXStencilFunc() {
    return cur_stencil_func;
}

void GFXStencilFunc(enum DEPTHFUNC *pFunc, int *pRef, int *pMask) {
    if (pFunc) {
        *pFunc = cur_stencil_func;
    }
    if (pRef) {
        *pRef = cur_stencil_func_ref;
    }
    if (pMask) {
        *pMask = cur_stencil_func_mask;
    }
}

void GFXStencilFunc(enum DEPTHFUNC sfunc, int ref, unsigned int mask) {
    GLenum func;
    switch (sfunc) {
        case NEVER:
            func = GL_NEVER;
            break;
        case LESS:
            func = GL_LESS;
            break;
        case EQUAL:
            func = GL_EQUAL;
            break;
        case LEQUAL:
            func = GL_LEQUAL;
            break;
        case GREATER:
            func = GL_GREATER;
            break;
        case NEQUAL:
            func = GL_NOTEQUAL;
            break;
        case GEQUAL:
            func = GL_GEQUAL;
            break;
        case ALWAYS:
            func = GL_ALWAYS;
            break;
        default:
            func = GL_NEVER;
            break;
    }
    glStencilFunc(func, (GLint) ref, (GLuint) mask);
    cur_stencil_func = sfunc;
    cur_stencil_func_ref = ref;
    cur_stencil_func_mask = mask;
}

void GFXStencilOp(enum STENCILOP *pFail, enum STENCILOP *pZfail, enum STENCILOP *pZpass) {
    if (pFail) {
        *pFail = cur_stencil_op_f;
    }
    if (pZfail) {
        *pZfail = cur_stencil_op_zf;
    }
    if (pZpass) {
        *pZpass = cur_stencil_op_zp;
    }
}

void GFXStencilOp(enum STENCILOP fail, enum STENCILOP zfail, enum STENCILOP zpass) {
    GLenum ffunc, zffunc, zpfunc;
    switch (fail) {
        case KEEP:
            ffunc = GL_KEEP;
            break;
        case CLEAR:
            ffunc = GL_ZERO;
            break;
        case REPLACE:
            ffunc = GL_REPLACE;
            break;
        case INCR:
            ffunc = GL_INCR;
            break;
        case DECR:
            ffunc = GL_DECR;
            break;
        case INVERT:
            ffunc = GL_INVERT;
            break;
        default:
            ffunc = GL_KEEP;
            break;
    }
    switch (zfail) {
        case KEEP:
            zffunc = GL_KEEP;
            break;
        case CLEAR:
            zffunc = GL_ZERO;
            break;
        case REPLACE:
            zffunc = GL_REPLACE;
            break;
        case INCR:
            zffunc = GL_INCR;
            break;
        case DECR:
            zffunc = GL_DECR;
            break;
        case INVERT:
            zffunc = GL_INVERT;
            break;
        default:
            zffunc = GL_KEEP;
            break;
    }
    switch (zpass) {
        case KEEP:
            zpfunc = GL_KEEP;
            break;
        case CLEAR:
            zpfunc = GL_ZERO;
            break;
        case REPLACE:
            zpfunc = GL_REPLACE;
            break;
        case INCR:
            zpfunc = GL_INCR;
            break;
        case DECR:
            zpfunc = GL_DECR;
            break;
        case INVERT:
            zpfunc = GL_INVERT;
            break;
        default:
            zpfunc = GL_KEEP;
            break;
    }
    glStencilOp(ffunc, zffunc, zpfunc);
    cur_stencil_op_f = fail;
    cur_stencil_op_zf = zfail;
    cur_stencil_op_zp = zpass;
}

unsigned int GFXStencilMask() {
    return cur_stencil_mask;
}

void GFXStencilMask(unsigned int mask) {
    glStencilMask((GLuint) mask);
    cur_stencil_mask = mask;
}

void GFXActiveTexture(const int stage) {
#if !defined (IRIX)
    if (gl_options.Multitexture && stage != activeTextureStage && glActiveTextureARB_p) {
        glActiveTextureARB_p(GL_TEXTURE0_ARB + stage);
        activeTextureStage = stage;
    } else {
        activeTextureStage = stage;         //This ensures consistent behavior - they shouldn't even call us
    }
#endif
}

void GFXAlphaTest(const enum DEPTHFUNC df, const float ref) {
    if (df == ALWAYS) {
        glDisable(GL_ALPHA_TEST);
        return;
    } else {
        glEnable(GL_ALPHA_TEST);
    }
    GLenum tmp;
    switch (df) {
        case NEVER:
            tmp = GL_NEVER;
            break;
        case LESS:
            tmp = GL_LESS;
            break;
        case EQUAL:
            tmp = GL_EQUAL;
            break;
        case LEQUAL:
            tmp = GL_LEQUAL;
            break;
        case GREATER:
            tmp = GL_GREATER;
            break;
        case NEQUAL:
            tmp = GL_NOTEQUAL;
            break;
        case GEQUAL:
            tmp = GL_GEQUAL;
            break;
        default:
        case ALWAYS:
            tmp = GL_ALWAYS;
            break;
    }
    glAlphaFunc(tmp, ref);
}

