/*
 * background.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#include "gldrv/gl_globals.h"
#include "src/config_xml.h"
#include "src/universe_util.h"
#include "src/universe.h"
#include "gfx/ani_texture.h"
#include "src/vs_logging.h"

#include <boost/format.hpp>

#include <float.h>
#include <algorithm>
#include <cmath>
const float size = 100;

Background::Background(const char *file,
        float spread,
        const std::string &filename,
        const BackgroundSettings &settings)
        : Enabled(true), degamma(settings.degamma), color(settings.color), draw_skybox(settings.draw_skybox),
        fill_color_set(settings.fill_color_set), stars(NULL) {
    string temp;
    const string starspritetextures = configuration().graphics.far_stars_sprite_texture;
    const float starspritesize = configuration().graphics.far_stars_sprite_size_flt;
    if (starspritetextures.length() == 0) {
        stars =
                new PointStarVlist(settings.num_stars, 200 /*spread*/,
                        configuration().graphics.use_star_coords ? filename : "",
                        settings.star_brightness_exponent);
    } else {
        stars =
                new SpriteStarVlist(settings.num_stars,
                        200 /*spread*/,
                        configuration().graphics.use_star_coords ? filename : "",
                        starspritetextures,
                        starspritesize,
                        settings.star_brightness_exponent,
                        configuration().graphics.far_stars_size_by_brightness);
    }
    up = left = down = front = right = back = NULL;

    SphereBackground = NULL;

    for (const BackdropSpec &spec : settings.backdrops) {
        Texture *texture;
        // Bilinear rather than mipmapped: mipmapping replaces images whose sides
        // aren't powers of two with a placeholder, and backdrops often aren't.
        if (spec.file.find(".ani") != string::npos) {
            texture = new AnimatedTexture(spec.file.c_str(), 0, BILINEAR);
        } else {
            texture = new Texture(spec.file.c_str(), 0, BILINEAR, TEXTURE2D, TEXTURE_2D, GFXTRUE);
        }
        if (texture->LoadSuccess()) {
            backdrops.push_back(Backdrop{spec, texture});
        } else {
            VS_LOG(error, (boost::format("Backdrop image %1% could not be loaded") % spec.file));
            delete texture;
        }
    }
    if (!draw_skybox) {
        return;  // nothing else to load
    }

#ifndef NV_CUBE_MAP
    static int max_cube_size = configuration().graphics.max_cubemap_size;
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
    for (Backdrop &backdrop : backdrops) {
        delete backdrop.texture;
    }
    backdrops.clear();
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
    // Enabled is the player's graphics.background setting; draw_skybox is the
    // system's own. A system without a skybox is cleared to its fill color.
    const bool skybox = Enabled && draw_skybox;
    if (skybox) {
        GFXClear(GFXFALSE);
    } else if (Enabled && fill_color_set) {
        GFXClearToColor(color);
    } else {
        GFXClear(GFXTRUE);
    }
    if (skybox) {
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
                const float edge_fixup = configuration().graphics.background_edge_fixup_flt;
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
                if (!configuration().graphics.ext_clamp_to_edge) {
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
    if (Enabled) {
        DrawBackdrops();
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
    const float background_velocity_scale = configuration().graphics.background_star_streak_velocity_scale_flt;
    stars->DrawAll(QVector(0, 0, 0), _Universe->AccessCamera()->GetVelocity().Scale(
            background_velocity_scale), _Universe->AccessCamera()->GetAngularVelocity(), true, true);
    GFXBlendMode(ONE, ZERO);
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXCenterCamera(false);
    //_Universe->AccessCamera()->UpdateGFX(false);
}


// Draws each backdrop as a quad facing the camera, centred on the camera like
// the skybox so it stays at the same place in the sky however far the ship
// flies. They are drawn in file order, after the skybox and before the far
// stars, with depth testing off, so later backdrops cover earlier ones.
void Background::DrawBackdrops() {
    if (backdrops.empty()) {
        return;
    }
    const float distance = size * 0.9F;  // inside the skybox
    GFXCenterCamera(true);
    GFXLoadMatrixModel(identity_matrix);
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHWRITE);
    GFXDisable(DEPTHTEST);
    GFXDisable(TEXTURE1);
    GFXEnable(TEXTURE0);
    for (Backdrop &backdrop : backdrops) {
        const BackdropSpec &spec = backdrop.spec;
        Texture *texture = backdrop.texture;

        // A frame across the line of sight: right and up on screen as seen
        // from the centre. Up follows the system's +y (or +z when looking
        // straight up or down), matching how the skybox images are laid out.
        Vector forward = spec.direction;
        if (forward.MagnitudeSquared() < 1e-12) {
            continue;
        }
        forward.Normalize();
        Vector reference = std::fabs(forward.j) > 0.99F ? Vector(0, 0, 1) : Vector(0, 1, 0);
        Vector up = reference - forward * forward.Dot(reference);
        up.Normalize();
        Vector right = forward.Cross(up);  // +x is screen left in Vega Strike
        const float turn = spec.rotation_degrees * static_cast<float>(M_PI) / 180.0F;
        const Vector turned_up = up * std::cos(turn) - right * std::sin(turn);
        const Vector turned_right = right * std::cos(turn) + up * std::sin(turn);

        // Angular size to half-widths on a quad at that distance; under 180
        // degrees, beyond which the quad would have to wrap behind the camera.
        float height_degrees = spec.height_degrees;
        if (height_degrees <= 0) {
            height_degrees = spec.size_degrees;
            if (texture->boundSizeX > 0 && texture->boundSizeY > 0) {
                height_degrees *= static_cast<float>(texture->boundSizeY) / static_cast<float>(texture->boundSizeX);
            }
        }
        auto half_extent = [distance](float degrees) {
            const float clamped = std::min(std::max(degrees, 0.0F), 170.0F);
            return distance * std::tan(clamped * static_cast<float>(M_PI) / 360.0F);
        };
        const Vector centre = forward * distance;
        const Vector across = turned_right * half_extent(spec.size_degrees);
        const Vector down = turned_up * half_extent(height_degrees);

        GFXBlendMode(spec.blend_source, spec.blend_destination);
        GFXColorf(spec.color);
        GFXToggleTexture(true, 0);
        GFXTextureCoordGenMode(0, NO_GEN, NULL, NULL);
        texture->MakeActive(0, 0);  // also picks an animation's current frame
        GFXTextureAddressMode(CLAMP);
        GFXTextureEnv(0, GFXMODULATETEXTURE);

        // Texture coordinates: s grows to the image's right, t downwards, as
        // the skybox faces use them. Read after MakeActive, which sets them
        // for an animation's frame.
        const float ms = texture->mintcoord.i, Ms = texture->maxtcoord.i;
        const float mt = texture->mintcoord.j, Mt = texture->maxtcoord.j;
        const Vector top_left = centre - across + down;
        const Vector top_right = centre + across + down;
        const Vector bottom_right = centre + across - down;
        const Vector bottom_left = centre - across - down;
        // Counterclockwise as seen from the centre, like the skybox faces.
        const float verts[4 * (3 + 2)] = {
                top_right.i, top_right.j, top_right.k, Ms, mt,
                top_left.i, top_left.j, top_left.k, ms, mt,
                bottom_left.i, bottom_left.j, bottom_left.k, ms, Mt,
                bottom_right.i, bottom_right.j, bottom_right.k, Ms, Mt,
        };
        GFXDraw(GFXQUAD, verts, 4, 3, 0, 2);
    }
    GFXTextureAddressMode(WRAP);
    GFXColor4f(1, 1, 1, 1);
    GFXBlendMode(ONE, ZERO);
    GFXCenterCamera(false);
}
