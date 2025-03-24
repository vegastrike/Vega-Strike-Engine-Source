/*
 * background.cpp
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors.
 * Copyright (C) 2021-2022 Stephen G. Tuggy
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "src/vegastrike.h"
#include "gfx/camera.h"
#include "gfx/aux_texture.h"
#include "star.h"
#include "gfx/background.h"
#include "src/gfxlib.h"
#include "gfx/aux_texture.h"
#include "gfx_generic/sphere.h"
#include "root_generic/vs_globals.h"
#include "../gldrv/gl_globals.h"
#include "src/config_xml.h"
#include "src/universe_util.h"
#include "src/universe.h"

#include <float.h>
const float size = 100;

Background::Background(const char *file,
        int numstars,
        float spread,
        const std::string &filename,
        const GFXColor &color_,
        bool degamma_)
        : Enabled(true), degamma(degamma_), color(color_), stars(NULL) {
    string temp;
    static string starspritetextures = vs_config->getVariable("graphics", "far_stars_sprite_texture", "");
    static float starspritesize =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "far_stars_sprite_size", "2"));
    if (starspritetextures.length() == 0) {
        stars =
                new PointStarVlist(numstars, 200 /*spread*/,
                        XMLSupport::parse_bool(vs_config->getVariable("graphics",
                                "use_star_coords",
                                "true")) ? filename : "");
    } else {
        stars =
                new SpriteStarVlist(numstars,
                        200 /*spread*/,
                        XMLSupport::parse_bool(vs_config->getVariable("graphics",
                                "use_star_coords",
                                "true")) ? filename : "",
                        starspritetextures,
                        starspritesize);
    }
    up = left = down = front = right = back = NULL;

    SphereBackground = NULL;

#ifndef NV_CUBE_MAP
    static int max_cube_size = XMLSupport::parse_int( vs_config->getVariable( "graphics", "max_cubemap_size", "1024" ) );
    string     suffix = ".image";
    temp = string( file )+"_up.image";
    up   = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
    if ( !up->LoadSuccess() ) {
        temp = string( file )+"_up.bmp";
        delete up;
        up   = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
        if ( up->LoadSuccess() )
            suffix = ".bmp";              //backwards compatibility
    }
    if ( !up->LoadSuccess() ) {
        temp = string( file )+"_sphere.image";
        if (VSFileSystem::LookForFile( temp, VSFileSystem::TextureFile ) > VSFileSystem::Ok) {
            //> Ok means failed to load.
            temp   = string( file )+"_sphere.bmp";
            suffix = ".bmp";             //backwards compatibility
        }
        SphereBackground = new SphereMesh( 20, 8, 8, temp.c_str(), "", NULL, true );
        //SphereBackground->Pitch(PI*.5);//that's the way prophecy's textures are set up
        //SphereBackground->SetOrientation(Vector(1,0,0),
        //Vector(0,0,-1),
        //Vector(0,1,0));//that's the way prophecy's textures are set up
        delete up;
        up = NULL;
    } else {
        //up->Clamp();
        //up->Filter();

        temp = string( file )+"_left"+suffix;
        left = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
        //left->Clamp();
        //left->Filter();

        temp  = string( file )+"_front"+suffix;
        front = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
        //front->Clamp();
        //front->Filter();

        temp  = string( file )+"_right"+suffix;
        right = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
        //right->Clamp();
        //right->Filter();

        temp = string( file )+"_back"+suffix;
        back = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
        //back->Clamp();
        //back->Filter();

        temp = string( file )+"_down"+suffix;
        down = new Texture( temp.c_str(), 0, MIPMAP, TEXTURE2D, TEXTURE_2D, GFXTRUE, max_cube_size );
        //down->Clamp();
        //down->Filter();
    }
#endif
}

void Background::EnableBG(bool tf) {
    Enabled = tf;
}

Background::~Background() {
#ifndef NV_CUBE_MAP
    if (up != nullptr) {
        delete up;
        up = nullptr;
    }
    if (left != nullptr) {
        delete left;
        left = nullptr;
    }
    if (front != nullptr) {
        delete front;
        front = nullptr;
    }
    if (right != nullptr) {
        delete right;
        right = nullptr;
    }
    if (back != nullptr) {
        delete back;
        back = nullptr;
    }
    if (down != nullptr) {
        delete down;
        down = nullptr;
    }
#endif
    if (SphereBackground != nullptr) {
        delete SphereBackground;
        SphereBackground = nullptr;
    }
    if (stars != nullptr) {
        delete stars;
        stars = nullptr;
    }
}

Background::BackgroundClone Background::Cache() {
    BackgroundClone ret{};
#ifndef NV_CUBE_MAP
    ret.backups[0] = up ? up->Clone() : NULL;
    ret.backups[1] = down ? down->Clone() : NULL;
    ret.backups[2] = left ? left->Clone() : NULL;
    ret.backups[3] = right ? right->Clone() : NULL;
    ret.backups[4] = front ? front->Clone() : NULL;
    ret.backups[5] = back ? back->Clone() : NULL;
    ret.backups[6] = NULL;
    if (SphereBackground)
        for (int i = 0; i < 7 && i < SphereBackground->numTextures(); ++i)
            ret.backups[(i+6)%7] = SphereBackground->texture( i )->Clone();
#endif
    return ret;
}

void Background::BackgroundClone::FreeClone() {
#ifndef NV_CUBE_MAP
    for (int i = 0; i < 7; ++i)
        if (backups[i]) {
            delete backups[i];
            backups[i] = NULL;
        }

#endif

}

void Background::Draw() {
    GFXClear(Enabled ? GFXFALSE : GFXTRUE);
    if (Enabled) {
        if (degamma) {
            GFXBlendMode(SRCCOLOR, ZERO);
        } else {
            GFXBlendMode(ONE, ZERO);
        }
        GFXDisable(LIGHTING);
        GFXDisable(DEPTHWRITE);
        GFXDisable(DEPTHTEST);
        GFXEnable(TEXTURE0);
        GFXDisable(TEXTURE1);
        GFXColorf(color);
        if (SphereBackground) {
            SphereBackground->DrawNow(FLT_MAX, true);
            //Mesh::ProcessUndrawnMeshes();//background must be processed...dumb but necessary--otherwise might collide with other mehses
        } else {
            GFXCenterCamera(true);
            GFXLoadMatrixModel(identity_matrix);
            //GFXLoadIdentity(MODEL);
            //GFXTranslate (MODEL,_Universe->AccessCamera()->GetPosition());

            //GFXTextureWrap(0,GFXCLAMPTEXTURE);
            //glMatrixMode(GL_MODELVIEW);

            /***********************?????????
            *                        //Matrix oldproject;
            *                        //GFXGetMatrix(VIEW, oldproject);
            *                        //glPushMatrix();
            *                        //gluPerspective (90,1.33,0.01,1500); //set perspective to 78 degree FOV
            ********************************/
            //_Universe->AccessCamera()->UpdateGLCenter();

            static struct skybox_rendering_record {
                Texture *tex;
                float vertices[4][3];              //will be *= size
                signed char tcoord[4][4];             //S-T-S-T: 0 >= min, 1 => max
            }
                    skybox_rendering_sequence[6] = {
#ifdef NV_CUBE_MAP
                    //For rendering with a single cube map as texture

                    {                 //up
                            NULL,
                            {
                                    {-1, +1, +1}, {-1, +1, -1}, {+1, +1, -1}, {+1, +1, +1}
                            },
                            {
                                    {-1, +2, +1, 0}, {-1, +2, -1, 0}, {+1, +2, -1, 0}, {+1, +2, +1, 0}
                            },
                    },
                    {                 //left
                            NULL,
                            {
                                    {-1, +1, -1}, {-1, +1, +1}, {-1, -1, +1}, {-1, -1, -1}
                            },
                            {
                                    {-2, +1, -1, 0}, {-2, +1, +1, 0}, {-2, -1, +1, 0}, {-2, -1, -1, 0}
                            },
                    },
                    {                 //front
                            NULL,
                            {
                                    {-1, +1, +1}, {+1, +1, +1}, {+1, -1, +1}, {-1, -1, +1}
                            },
                            {
                                    {-1, +1, +2, 0}, {+1, +1, +2, 0}, {+1, -1, +2, 0}, {-1, -1, +2, 0}
                            },
                    },
                    {                 //right
                            NULL,
                            {
                                    {+1, +1, +1}, {+1, +1, -1}, {+1, -1, -1}, {+1, -1, +1}
                            },
                            {
                                    {+2, +1, +1, 0}, {+2, +1, -1, 0}, {+2, -1, -1, 0}, {+2, -1, +1, 0}
                            },
                    },
                    {                 //back
                            NULL,
                            {
                                    {+1, +1, -1}, {-1, +1, -1}, {-1, -1, -1}, {+1, -1, -1}
                            },
                            {
                                    {+1, +1, -2, 0}, {-1, +1, -2, 0}, {-1, -1, -2, 0}, {+1, -1, -2, 0}
                            },
                    },
                    {                 //down
                            NULL,
                            {
                                    {-1, -1, +1}, {+1, -1, +1}, {+1, -1, -1}, {-1, -1, -1}
                            },
                            {
                                    {-1, -2, +1, 0}, {+1, -2, +1, 0}, {+1, -2, -1, 0}, {-1, -2, -1, 0}
                            },
                    }

#else
                    //For rendering with multiple 2D texture faces

                    {                 //up
                        NULL,
                        {
                            {-1, +1, +1}, {-1, +1, -1}, {+1, +1, -1}, {+1, +1, +1}
                        },
                        {
                            {1, 0, 1, 0}, {0, 0, 0, 0}, {0,  1,  0, 1}, {1,  1,  1, 1}
                        }
                    },
                    {                 //left
                        NULL,
                        {
                            {-1, +1, -1}, {-1, +1, +1}, {-1, -1, +1}, {-1, -1, -1}
                        },
                        {
                            {1, 0, 1, 0}, {0, 0, 0, 0}, {0,  1,  0, 1}, {1,  1,  1, 1}
                        }
                    },
                    {                 //front
                        NULL,
                        {
                            {-1, +1, +1}, {+1, +1, +1}, {+1, -1, +1}, {-1, -1, +1}
                        },
                        {
                            {1, 0, 1, 0}, {0, 0, 0, 0}, {0,  1,  0, 1}, {1,  1,  1, 1}
                        }
                    },
                    {                 //right
                        NULL,
                        {
                            {+1, +1, +1}, {+1, +1, -1}, {+1, -1, -1}, {+1, -1, +1}
                        },
                        {
                            {1, 0, 1, 0}, {0, 0, 0, 0}, {0,  1,  0, 1}, {1,  1,  1, 1}
                        }
                    },
                    {                 //back
                        NULL,
                        {
                            {+1, +1, -1}, {-1, +1, -1}, {-1, -1, -1}, {+1, -1, -1}
                        },
                        {
                            {1, 0, 1, 0}, {0, 0, 0, 0}, {0,  1,  0, 1}, {1,  1,  1, 1}
                        }
                    },
                    {                 //down
                        NULL,
                        {
                            {-1, -1, +1}, {+1, -1, +1}, {+1, -1, -1}, {-1, -1, -1}
                        },
                        {
                            {1, 0, 1, 0}, {0, 0, 0, 0}, {0,  1,  0, 1}, {1,  1,  1, 1}
                        }
                    }
#endif
            };
            skybox_rendering_sequence[0].tex = up;
            skybox_rendering_sequence[1].tex = left;
            skybox_rendering_sequence[2].tex = front;
            skybox_rendering_sequence[3].tex = right;
            skybox_rendering_sequence[4].tex = back;
            skybox_rendering_sequence[5].tex = down;
            for (size_t skr = 0; skr < sizeof(skybox_rendering_sequence) / sizeof(skybox_rendering_sequence[0]);
                    skr++) {
                Texture *tex = skybox_rendering_sequence[skr].tex;

#ifdef NV_CUBE_MAP
                if (tex == NULL) {
                    tex = _Universe->getLightMap();
                }
                const int numpasses = 1;
                static float edge_fixup =
                        XMLSupport::parse_float(vs_config->getVariable("graphics", "background_edge_fixup", "0"));
                const float ms = 0.f, Ms = 1.f - edge_fixup / tex->boundSizeX;
                const float mt = 0.f, Mt = 1.f - edge_fixup / tex->boundSizeY;
                const float _stca[] = {-1.f, -Ms, ms, Ms, +1.f}, _ttca[] = {-1.f, -Mt, mt, Mt, +1.f};
                const float *stca = _stca + 2, *ttca = _ttca + 2;

                GFXColorf(color);

                _Universe->activateLightMap(0);
                GFXToggleTexture(true, 0, CUBEMAP);
#else
                int   lyr;
                int   numlayers = tex->numLayers();
                bool  multitex  = (numlayers > 1);
                int   numpasses = tex->numPasses();
                float ms = tex->mintcoord.i, Ms = tex->maxtcoord.i;
                float mt = tex->mintcoord.j, Mt = tex->maxtcoord.j;
                if (!gl_options.ext_clamp_to_edge) {
                    ms += 1.0/tex->boundSizeX;
                    Ms -= 1.0/tex->boundSizeX;
                    mt += 1.0/tex->boundSizeY;
                    Mt -= 1.0/tex->boundSizeY;
                }
                float stca[] = {ms, Ms}, ttca[] = {mt, Mt};

                GFXColorf(color);
                for (lyr = 0; (lyr < gl_options.Multitexture) || (lyr < numlayers); lyr++) {
                    GFXToggleTexture( (lyr < numlayers), lyr );
                    if (lyr < numlayers)
                        GFXTextureCoordGenMode( lyr, NO_GEN, NULL, NULL );
                }
#endif
                for (int pass = 0; pass < numpasses; pass++) {
                    if (!tex || tex->SetupPass(pass, 0, ONE, ZERO)) {
                        if (tex) {
                            tex->MakeActive(0, pass);
                        }
                        GFXTextureAddressMode(CLAMP);
                        GFXTextureEnv(0, GFXMODULATETEXTURE);
                        GFXTextureCoordGenMode(0, NO_GEN, NULL, NULL);

#define X(i) skybox_rendering_sequence[skr].vertices[i][0]*size
#define Y(i) skybox_rendering_sequence[skr].vertices[i][1]*size
#define Z(i) skybox_rendering_sequence[skr].vertices[i][2]*size
#define S(i) stca[size_t(skybox_rendering_sequence[skr].tcoord[i][0])]
#define T(i) ttca[size_t(skybox_rendering_sequence[skr].tcoord[i][1])]
#define U(i) stca[size_t(skybox_rendering_sequence[skr].tcoord[i][2])]
#define V(i) ttca[size_t(skybox_rendering_sequence[skr].tcoord[i][3])]

#ifdef NV_CUBE_MAP
                        const float verts[4 * (3 + 3)] = {
                                X(0), Y(0), Z(0), S(0), T(0), U(0),
                                X(1), Y(1), Z(1), S(1), T(1), U(1),
                                X(2), Y(2), Z(2), S(2), T(2), U(2),
                                X(3), Y(3), Z(3), S(3), T(3), U(3),
                        };
                        GFXDraw(GFXQUAD, verts, 4, 3, 0, 3);
#else
                        if (!multitex) {
                            const float verts[4 * (3 + 2)] = {
                                X(0), Y(0), Z(0), S(0), T(0),
                                X(1), Y(1), Z(1), S(1), T(1),
                                X(2), Y(2), Z(2), S(2), T(2),
                                X(3), Y(3), Z(3), S(3), T(3),
                            };
                            GFXDraw( GFXQUAD, verts, 4, 3, 0, 2 );
                        } else {
                            const float verts[4 * (3 + 2 + 2)] = {
                                X(0), Y(0), Z(0), S(0), T(0), U(0), V(0),
                                X(1), Y(1), Z(1), S(1), T(1), U(1), V(1),
                                X(2), Y(2), Z(2), S(2), T(2), U(2), V(2),
                                X(3), Y(3), Z(3), S(3), T(3), U(3), V(3),
                            };
                            GFXDraw( GFXQUAD, verts, 4, 3, 0, 2, 2 );
                        }
#endif

#undef X
#undef Y
#undef Z
#undef S
#undef T
#undef U
#undef V
                    }
                }

#ifdef NV_CUBE_MAP
                GFXToggleTexture(false, 0, CUBEMAP);
#else
                for (lyr = 0; lyr < numlayers; lyr++) {
                    GFXToggleTexture( false, lyr );
                    if (lyr < numlayers) GFXTextureCoordGenMode( lyr, NO_GEN, NULL, NULL );
                }
                if (tex)
                    tex->SetupPass( -1, 0, ONE, ZERO );
#endif
            }

            GFXActiveTexture(0);
            GFXTextureAddressMode(WRAP);
            GFXCenterCamera(false);
        }
    }
    //GFXLoadIdentity(MODEL);
    //GFXTranslate (MODEL,_Universe->AccessCamera()->GetPosition());
    GFXCenterCamera(true);
//GFXEnable(DEPTHWRITE);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    GFXColor(1, 1, 1, 1);
    GFXDisable(TEXTURE1);
    GFXDisable(DEPTHWRITE);
    GFXBlendMode(ONE, ONE);
    static float background_velocity_scale =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "background_star_streak_velocity_scale", "0"));
    stars->DrawAll(QVector(0, 0, 0), _Universe->AccessCamera()->GetVelocity().Scale(
            background_velocity_scale), _Universe->AccessCamera()->GetAngularVelocity(), true, true);
    GFXBlendMode(ONE, ZERO);
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXCenterCamera(false);
    //_Universe->AccessCamera()->UpdateGFX(false);
}

