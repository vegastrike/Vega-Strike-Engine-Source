/*
 * star.cpp
 *
 * Copyright (C) 2001-2025 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "star.h"
#include "gfx/ani_texture.h"
#include <assert.h>
#include "src/vegastrike.h"
#include "root_generic/vs_globals.h"
#include "gfx/camera.h"
#include "gfx/cockpit.h"
#include "src/config_xml.h"
#include "root_generic/lin_time.h"
#include "root_generic/galaxy_xml.h"
#include "src/universe.h"
#include "src/vs_logging.h"

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if defined(__APPLE__) && defined (__MACH__)
#include <gl.h>
#else
#include <gl.h>
#endif

#define SINX 1
#define SINY 2
#define SINZ 4

unsigned int NumStarsInGalaxy() {
    unsigned int count = 0;
    vsUMap<std::string, GalaxyXML::SGalaxy>::iterator i = _Universe->getGalaxy()->getHeirarchy().begin();
    vsUMap<std::string, GalaxyXML::SGalaxy>::iterator e = _Universe->getGalaxy()->getHeirarchy().end();
    for (; i != e; ++i) {
        count += (*i).second.getHeirarchy().size();
    }
    return count;
}

class StarIter {
    vsUMap<std::string, GalaxyXML::SGalaxy>::iterator sector;
    vsUMap<std::string, GalaxyXML::SGalaxy>::iterator system;
public:
    bool Done() const {
        return sector == _Universe->getGalaxy()->getHeirarchy().end();
    }

    StarIter() {
        sector = _Universe->getGalaxy()->getHeirarchy().begin();
        if (!Done()) {
            system = (*sector).second.getHeirarchy().begin();
            if (system == (*sector).second.getHeirarchy().end()) {
                ++(*this);
            }
        }
    }

    void operator++() {
        if (!Done()) {
            if (system != (*sector).second.getHeirarchy().end()) {
                ++system;
            }
        }
        while (!Done() && system == (*sector).second.getHeirarchy().end()) {
            ++sector;
            if (!Done()) {
                system = (*sector).second.getHeirarchy().begin();
            }
        }
    }

    GalaxyXML::SGalaxy *Get() const {
        if (!Done()) {
            return &(*system).second;
        } else {
            return NULL;
        }
    }

    std::string GetSystem() const {
        if (!Done()) {
            return (*system).first;
        } else {
            return "Nowhere";
        }
    }

    std::string GetSector() const {
        if (!Done()) {
            return (*sector).first;
        } else {
            return "NoSector";
        }
    }
};

//bool shouldwedraw
static void saturate(float &r, float &g, float &b) {
    const float conemin = vega_config::config->graphics.starmincolorval;
    const float colorpower = vega_config::config->graphics.starcolorpower;
    if (r < conemin) {
        r += conemin;
    }
    if (g < conemin) {
        g += conemin;
    }
    if (b < conemin) {
        b += conemin;
    }
    r = pow(r, colorpower);
    g = pow(g, colorpower);
    b = pow(b, colorpower);
}

bool computeStarColor(float &r, float &g, float &b, Vector luminmax, float distance, float maxdistance) {
    saturate(r, g, b);
    const float luminscale = vega_config::config->graphics.starluminscale;
    const float starcoloraverage = vega_config::config->graphics.starcoloraverage;
    const float starcolorincrement = vega_config::config->graphics.starcolorincrement;
    float dissqr = distance * distance / (maxdistance * maxdistance);
    float lum = 100 * luminmax.i / (luminmax.k * dissqr);
    lum = log((double) luminmax.i * 10. / (double) luminmax.j) * luminscale / dissqr;
    float clamp = starcoloraverage + lum / starcolorincrement;
    if (clamp > 1) {
        clamp = 1;
    }
    if (lum > clamp) {
        lum = clamp;
    }
    r *= lum;
    g *= lum;
    b *= lum;
    const float starcolorcutoff = vega_config::config->graphics.starcolorcutoff;
    return lum > starcolorcutoff;
}

namespace StarSystemGent {
extern GFXColor getStarColorFromRadius(float radius);
}

StarVlist::StarVlist(float spread) {
    lasttime = 0;
    _Universe->AccessCamera()->GetPQR(newcamr, camq, camr);
    newcamr = camr;
    newcamq = camq;
    this->spread = spread;
}

static GFXColorVertex *AllocVerticesForSystem(std::string our_system_name, float spread, int *num, int repetition) {
    const float staroverlap = vega_config::config->graphics.star_overlap;
    float xyzspread = spread * 2 * staroverlap;
    const std::string allowedSectors = vega_config::config->graphics.star_allowable_sectors; /* default: "Vega Sol" */
    if (our_system_name.size() > 0) {
        string lumi = _Universe->getGalaxyProperty(our_system_name, "luminosity");
        if (lumi.length() == 0 || strtod(lumi.c_str(), NULL) == 0) {
            our_system_name = "";
        } else {
            string::size_type slash = our_system_name.find("/");
            if (slash != string::npos) {
                string sec = our_system_name.substr(0, slash);
                if (allowedSectors.find(sec) == string::npos) {
                    our_system_name = "";
                }
            } else {
                our_system_name = "";
            }
        }
    }
    if (!our_system_name.empty()) {
        *num = NumStarsInGalaxy();
    }
    GFXColorVertex *tmpvertex = new GFXColorVertex[(*num) * repetition];
//    memset( tmpvertex, 0, sizeof (GFXVertex)*(*num)*repetition ); //This is already initialized
    StarIter si;
    int starcount = 0;
    int j = 0;
    float xcent = 0;
    float ycent = 0;
    float zcent = 0;
    Vector starmin(0, 0, 0);
    Vector starmax(0, 0, 0);
    float minlumin = 1;
    float maxlumin = 1;
    float maxdistance = -1;
    float mindistance = -1;
    if (our_system_name.size() > 0) {
        sscanf(_Universe->getGalaxyProperty(our_system_name, "xyz").c_str(),
                "%f %f %f",
                &xcent,
                &ycent,
                &zcent);
        for (StarIter i; !i.Done(); ++i) {
            float xx, yy, zz;
            if (3 == sscanf((*i.Get())["xyz"].c_str(), "%f %f %f", &xx, &yy, &zz)) {
                xx -= xcent;
                yy -= ycent;
                zz -= zcent;
                if (xx < starmin.i) {
                    starmin.i = xx;
                }
                if (yy < starmin.j) {
                    starmin.j = yy;
                }
                if (zz < starmin.k) {
                    starmin.k = zz;
                }
                if (xx > starmax.i) {
                    starmax.i = xx;
                }
                if (yy > starmax.j) {
                    starmax.j = yy;
                }
                if (zz > starmax.k) {
                    starmax.k = zz;
                }
                float magsqr = xx * xx + yy * yy + zz * zz;
                if ((maxdistance < 0) || (maxdistance < magsqr)) {
                    maxdistance = magsqr;
                }
                if ((mindistance < 0) || (mindistance > magsqr)) {
                    mindistance = magsqr;
                }
                float lumin;
                if (1 == sscanf((*i.Get())["luminosity"].c_str(), "%f", &lumin)) {
                    if (lumin > maxlumin) {
                        maxlumin = lumin;
                    }
                    if (lumin < minlumin) {
                        if (lumin > 0) {
                            minlumin = lumin;
                        }
                    }
                }
            }
        }
    }
    if (maxdistance < 0) {
        maxdistance = 0;
    }
    if (mindistance < 0) {
        mindistance = 0;
    }
    maxdistance = sqrt(maxdistance);
    mindistance = sqrt(mindistance);
    VS_LOG(info, (boost::format("Min (%1$f, %2$f, %3$f) Max(%4$f, %5$f, %6$f) MinLumin %7$f, MaxLumin %8$f")
            % starmin.i % starmin.j % starmin.k % starmax.i % starmax.j % starmax.k % minlumin % maxlumin));
    for (int y = 0; y < *num; ++y) {
        tmpvertex[j + repetition - 1].x = -.5 * xyzspread + rand() * ((float) xyzspread / RAND_MAX);
        tmpvertex[j + repetition - 1].y = -.5 * xyzspread + rand() * ((float) xyzspread / RAND_MAX);
        tmpvertex[j + repetition - 1].z = -.5 * xyzspread + rand() * ((float) xyzspread / RAND_MAX);
        float brightness = .1 + .9 * ((float) rand()) / RAND_MAX;
        tmpvertex[j + repetition - 1].r = brightness;
        tmpvertex[j + repetition - 1].g = brightness;
        tmpvertex[j + repetition - 1].b = brightness;
        tmpvertex[j + repetition - 1].a = 1;
        tmpvertex[j + repetition - 1].i = .57735;
        tmpvertex[j + repetition - 1].j = .57735;
        tmpvertex[j + repetition - 1].k = .57735;
        int incj = repetition;
        if (our_system_name.size() > 0 && !si.Done()) {
            starcount++;
            float xorig, yorig, zorig;
            if (3 == sscanf((*si.Get())["xyz"].c_str(),
                    "%f %f %f",
                    &xorig,
                    &yorig,
                    &zorig)) {
                if (xcent != xorig) {
                    tmpvertex[j + repetition - 1].x = xorig - xcent;
                }
                if (ycent != yorig) {
                    tmpvertex[j + repetition - 1].y = yorig - ycent;
                }
                if (zcent != zorig) {
                    tmpvertex[j + repetition - 1].z = zorig - zcent;
                }
            }
            std::string radstr = (*si.Get())["sun_radius"];
            if (radstr.size()) {
                float rad = XMLSupport::parse_float(radstr);
                GFXColor suncolor(StarSystemGent::getStarColorFromRadius(rad));
                tmpvertex[j + repetition - 1].r = suncolor.r;
                tmpvertex[j + repetition - 1].g = suncolor.g;
                tmpvertex[j + repetition - 1].b = suncolor.b;
            }
            float lumin = 1;
            sscanf((*si.Get())["luminosity"].c_str(), "%f", &lumin);

            float distance = Vector(tmpvertex[j + repetition - 1].x,
                    tmpvertex[j + repetition - 1].y,
                    tmpvertex[j + repetition - 1].z).Magnitude();
            if (!computeStarColor(tmpvertex[j + repetition - 1].r,
                    tmpvertex[j + repetition - 1].g,
                    tmpvertex[j + repetition - 1].b,
                    Vector(lumin, minlumin, maxlumin),
                    distance, maxdistance)) {
                incj = 0;
            }
            ++si;
        }
        for (int LC = repetition - 2; LC >= 0; --LC) {
            tmpvertex[j + LC].i = tmpvertex[j + repetition - 1].i;
            tmpvertex[j + LC].j = tmpvertex[j + repetition - 1].j;
            tmpvertex[j + LC].k = tmpvertex[j + repetition - 1].k;
            tmpvertex[j + LC].x = tmpvertex[j + repetition - 1].x;             //+spread*.01;
            tmpvertex[j + LC].y = tmpvertex[j + repetition - 1].y;             //;+spread*.01;
            tmpvertex[j + LC].z = tmpvertex[j + repetition - 1].z;
            tmpvertex[j + LC].r = 0;
            tmpvertex[j + LC].g = 0;
            tmpvertex[j + LC].b = 0;
            tmpvertex[j + LC].a = 0;
        }
        j += incj;
    }
    VS_LOG(info, (boost::format("Read In Star Count %1$d used: %2$d\n") % starcount % (j / 2)));
    *num = j;
    return tmpvertex;
}

PointStarVlist::PointStarVlist(int num, float spread, const std::string &sysnam) : StarVlist(spread) {
    smoothstreak = 0;
    //const bool StarStreaks = vega_config::config->graphics.star_streaks;
    GFXColorVertex *tmpvertex = AllocVerticesForSystem(sysnam, this->spread, &num, 2);
    //if(StarStreaks) {
    vlist = new GFXVertexList(GFXLINE, num, tmpvertex, num, true, 0);
    //}else {
    for (int i = 0, j = 1; i < num / 2; ++i, j += 2) {
        tmpvertex[i] = tmpvertex[j];
    }
    nonstretchvlist = new GFXVertexList(GFXPOINT, num / 2, tmpvertex, num / 2, false, 0);
    //}
    delete[] tmpvertex;
}

void StarVlist::UpdateGraphics() {
    double time = getNewTime();
    if (time != lasttime) {
        camr = newcamr;
        camq = newcamq;
        Vector newcamp;
        _Universe->AccessCamera()->GetPQR(newcamp, newcamq, newcamr);
        lasttime = time;
    }
}

bool PointStarVlist::BeginDrawState(const QVector &center,
        const Vector &velocity,
        const Vector &torque,
        bool roll,
        bool yawpitch,
        int whichTexture) {
    UpdateGraphics();
    const bool StarStreaks = vega_config::config->graphics.star_streaks;
    GFXColorMaterial(AMBIENT | DIFFUSE);
    bool ret = false;
    if (StarStreaks) {
        Matrix rollMatrix;
        static float velstreakscale =
                vega_config::config->graphics.velocity_star_streak_scale;
        static float minstreak =
                vega_config::config->graphics.velocity_star_streak_min;
        const float fov_smoothing = vega_config::config->warp.fov_link.smoothing; /* default: .4 */
        float fov_smoot = pow(double(fov_smoothing), GetElapsedTime());
        Vector vel(-velocity * velstreakscale);
        float speed = vel.Magnitude();
        if ((smoothstreak >= minstreak || vel.MagnitudeSquared() >= minstreak * minstreak) && (speed > 1.0e-7)) {
            ret = true;
            vel *= 1. / speed;
            speed = fov_smoot * speed + (1 - fov_smoot) * smoothstreak;
            if (speed < minstreak) {
                speed = minstreak;
            }
            static float streakcap =
                    vega_config::config->graphics.velocity_star_streak_max;
            if (speed > streakcap) {
                speed = streakcap;
            }
            vel = vel * speed;
            smoothstreak = speed;
            GFXColorVertex *v = vlist->BeginMutate(0)->colors;
            int numvertices = vlist->GetNumVertices();

            static float torquestreakscale =
                    vega_config::config->graphics.torque_star_streak_scale;
            for (int j = 0; j < numvertices - 1; j += 2) {
                int i = j;
//if (SlowStarStreaks)
//i=((rand()%numvertices)/2)*2;
                Vector vpoint(v[i + 1].x, v[i + 1].y, v[i + 1].z);
                Vector recenter = (vpoint - center.Cast());
                if (roll) {
                    RotateAxisAngle(rollMatrix, torque, torque.Magnitude() * torquestreakscale * .003);
                    vpoint = Transform(rollMatrix, recenter) + center.Cast();
                }
                v[i].x = vpoint.i - vel.i;
                v[i].y = vpoint.j - vel.j;
                v[i].z = vpoint.k - vel.k;
//const float NumSlowStarStreaks = vega_config::config->graphics.num_star_streaks;
//if (SlowStarStreaks&&j<NumSlowStarStreaks*numvertices)
//break;
            }
            vlist->EndMutate();
        }
    }
    if (ret) {
        vlist->LoadDrawState();
        vlist->BeginDrawState();
    } else {
        nonstretchvlist->LoadDrawState();
        nonstretchvlist->BeginDrawState();
    }
    return ret;
}

void PointStarVlist::Draw(bool stretch, int whichTexture) {
    if (stretch) {
        vlist->Draw();
        vlist->Draw(GFXPOINT, vlist->GetNumVertices());
    } else {
        nonstretchvlist->Draw();
    }
}

void PointStarVlist::EndDrawState(bool stretch, int whichTexture) {
    if (stretch) {
        vlist->EndDrawState();
    } else {
        nonstretchvlist->EndDrawState();
    }
    GFXColorMaterial(0);
}

PointStarVlist::~PointStarVlist() {
    delete vlist;
    delete nonstretchvlist;
}

Stars::Stars(int num, float spread) : vlist(NULL), spread(spread) {
    const std::string starspritetextures = vega_config::config->graphics.near_stars_sprite_texture; /* default: "" */
    const float starspritesize = vega_config::config->graphics.near_stars_sprite_size;
    if (starspritetextures.length() == 0) {
        vlist = new PointStarVlist((num / STARnumvlist) + 1, spread, "");
    } else {
        vlist = new SpriteStarVlist((num / STARnumvlist) + 1, spread, "", starspritetextures, starspritesize);
    }
    fade = blend = true;
    ResetPosition(QVector(0, 0, 0));
}

void Stars::SetBlend(bool blendit, bool fadeit) {
    blend = true;     //blendit;
    fade = true;     //fadeit;
}

void Stars::Draw() {
    const bool stars_dont_move = vega_config::config->graphics.stars_dont_move;
    if (stars_dont_move) {
        return;
    }
    const QVector cp(_Universe->AccessCamera()->GetPosition());
    UpdatePosition(cp);
    //GFXLightContextAmbient(GFXColor(0,0,0,1));
    GFXColor(1, 1, 1, 1);
    GFXLoadIdentity(MODEL);
    GFXDisable(DEPTHWRITE);
    GFXDisable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXEnable(DEPTHTEST);
    static bool near_stars_alpha =
            vega_config::config->graphics.near_stars_alpha;
    static bool near_stars_alpha_blend =
            vega_config::config->graphics.near_stars_alpha_blend;
    static float AlphaTestingCutoff =
            vega_config::config->graphics.stars_alpha_test_cutoff;
    if (near_stars_alpha) {
        GFXAlphaTest(GREATER, AlphaTestingCutoff);
        if (!near_stars_alpha_blend) {
            GFXBlendMode(ONE, ZERO);
        } else {
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
        }
        GFXEnable(DEPTHWRITE);
    } else {
        if (blend) {
            GFXBlendMode(ONE, ONE);
        } else {
            GFXBlendMode(ONE, ZERO);
        }
    }
    int ligh;
    GFXSelectMaterial(0);
    if (fade) {
        static float star_spread_attenuation =
                vega_config::config->graphics.star_spread_attenuation;
        GFXPushGlobalEffects();
        GFXLight fadeLight(true, GFXColor(cp.i, cp.j, cp.k),
                GFXColor(0, 0, 0, 1),
                GFXColor(0, 0, 0, 1),
                GFXColor(1, 1, 1, 1),
                GFXColor(.01, 0, 1 / (star_spread_attenuation * star_spread_attenuation * spread * spread)));
        GFXCreateLight(ligh, fadeLight, true);
        GFXEnable(LIGHTING);
    } else {
        GFXDisable(LIGHTING);
    }
    _Universe->AccessCamera()->UpdateGFX(GFXFALSE, GFXFALSE, GFXFALSE);
    int LC = 0, LN = vlist->NumTextures();
    for (LC = 0; LC < LN; ++LC) {
        bool stretch = vlist->BeginDrawState(_Universe->AccessCamera()->GetR().Scale(
                        -spread).Cast(), _Universe->AccessCamera()->GetVelocity(),
                _Universe->AccessCamera()->GetAngularVelocity(), false, false, LC);
        int i;
        for (i = 0; i < STARnumvlist; i++) {
            if (i >= 1) {
                GFXTranslateModel(pos[i] - pos[i - 1]);
            } else {
                GFXTranslateModel(pos[i]);
            }
            vlist->Draw(stretch, LC);
        }
        GFXTranslateModel(-pos[i - 1]);
        vlist->EndDrawState(stretch, LC);
    }
    if (near_stars_alpha) {
        GFXAlphaTest(ALWAYS, 0);
    } else {
        GFXEnable(DEPTHWRITE);
    }
    GFXBlendMode(ONE, ZERO);
    _Universe->AccessCamera()->UpdateGFX(GFXTRUE, GFXFALSE, GFXFALSE);

    GFXEnable(TEXTURE0);
    GFXEnable(TEXTURE1);
    if (fade) {
        GFXDeleteLight(ligh);
        GFXPopGlobalEffects();
    }
    GFXLoadIdentity(MODEL);
}

static void upd(double &a,
        double &b,
        double &c,
        double &d,
        double &e,
        double &f,
        double &g,
        double &h,
        double &i,
        const double cp,
        const float spread) {
    //assert (a==b&&b==c&&c==d&&d==e&&e==f);
    if (a != b || a != c || a != d || a != e || a != f || !FINITE(a)) {
        a = b = c = d = e = f = 0;
    }
    while (a - cp > 1.5 * spread) {
        a -= 3 * spread;
        b -= 3 * spread;
        c -= 3 * spread;
        d -= 3 * spread;
        e -= 3 * spread;
        f -= 3 * spread;
        g -= 3 * spread;
        h -= 3 * spread;
        i -= 3 * spread;
    }
    while (a - cp < -1.5 * spread) {
        a += 3 * spread;
        b += 3 * spread;
        c += 3 * spread;
        d += 3 * spread;
        e += 3 * spread;
        f += 3 * spread;
        g += 3 * spread;
        h += 3 * spread;
        i += 3 * spread;
    }
}

void Stars::ResetPosition(const QVector &cent) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                const double spread_temp = spread;
                pos[i * 9 + j * 3 + k].Set((i - 1) * spread_temp, (j - 1) * spread_temp, (k - 1) * spread_temp);
                pos[i * 9 + j * 3 + k] += cent;
            }
        }
    }
}

void Stars::UpdatePosition(const QVector &cp) {
    if (fabs(pos[0].i - cp.i) > 3 * spread || fabs(pos[0].j - cp.j) > 3 * spread
            || fabs(pos[0].k - cp.k) > 3 * spread) {
        ResetPosition(cp);
        return;
    }
    upd(pos[0].i, pos[1].i, pos[2].i, pos[3].i, pos[4].i, pos[5].i, pos[6].i, pos[7].i, pos[8].i, cp.i, spread);
    upd(pos[9].i, pos[10].i, pos[11].i, pos[12].i, pos[13].i, pos[14].i, pos[15].i, pos[16].i, pos[17].i, cp.i, spread);
    upd(pos[18].i,
            pos[19].i,
            pos[20].i,
            pos[21].i,
            pos[22].i,
            pos[23].i,
            pos[24].i,
            pos[25].i,
            pos[26].i,
            cp.i,
            spread);

    upd(pos[0].j, pos[1].j, pos[2].j, pos[9].j, pos[10].j, pos[11].j, pos[18].j, pos[19].j, pos[20].j, cp.j, spread);
    upd(pos[3].j, pos[4].j, pos[5].j, pos[12].j, pos[13].j, pos[14].j, pos[21].j, pos[22].j, pos[23].j, cp.j, spread);
    upd(pos[6].j, pos[7].j, pos[8].j, pos[15].j, pos[16].j, pos[17].j, pos[24].j, pos[25].j, pos[26].j, cp.j, spread);

    upd(pos[0].k, pos[3].k, pos[6].k, pos[9].k, pos[12].k, pos[15].k, pos[18].k, pos[21].k, pos[24].k, cp.k, spread);
    upd(pos[1].k, pos[4].k, pos[7].k, pos[10].k, pos[13].k, pos[16].k, pos[19].k, pos[22].k, pos[25].k, cp.k, spread);
    upd(pos[2].k, pos[5].k, pos[8].k, pos[11].k, pos[14].k, pos[17].k, pos[20].k, pos[23].k, pos[26].k, cp.k, spread);
}

Stars::~Stars() {
    delete vlist;
}

static Vector GetConstVertex(const GFXColorVertex &c) {
    return Vector(c.x, c.y, c.z);
}

SpriteStarVlist::SpriteStarVlist(int num, float spread, std::string sysnam, std::string texturenames,
        float size) : StarVlist(spread) {
    int curtexture = 0;
    vector<AnimatedTexture *> animations;
    static bool
            near_stars_alpha = vega_config::config->graphics.near_stars_alpha;
    for (curtexture = 0; curtexture < NUM_ACTIVE_ANIMATIONS; ++curtexture) {
        std::string::size_type where = texturenames.find(" ");
        string texturename = texturenames.substr(0, where);
        if (where != string::npos) {
            texturenames = texturenames.substr(where + 1);
        } else {
            texturenames = "";
        }
        if (texturename.find(".ani") != string::npos) {
            animations.push_back(new AnimatedTexture(texturename.c_str(), 0, near_stars_alpha ? NEAREST : BILINEAR));
            decal[curtexture] = animations.back();
        } else if (texturename.length() == 0) {
            if (curtexture == 0) {
                decal[curtexture] = new Texture("white.bmp", 0, near_stars_alpha ? NEAREST : BILINEAR);
            } else {
                if (animations.size()) {
                    AnimatedTexture *tmp =
                            static_cast< AnimatedTexture * > ( animations[curtexture % animations.size()]->Clone());
                    int num = tmp->numFrames();
                    if (num) {
                        num = rand() % num;
                        tmp->setTime(num / tmp->framesPerSecond());
                    }
                    decal[curtexture] = tmp;
                } else {
                    decal[curtexture] = decal[rand() % curtexture]->Clone();
                }
            }
        } else {
            decal[curtexture] = new Texture(texturename.c_str());
        }
    }
    int numVerticesPer = near_stars_alpha ? 4 : 12;
    GFXColorVertex *tmpvertex = AllocVerticesForSystem(sysnam, this->spread, &num, numVerticesPer);
    for (int LC = 0; LC < num; LC += numVerticesPer) {
        int LAST = LC + numVerticesPer - 1;
        for (int i = LC; i <= LAST; ++i) {
            tmpvertex[i].r = tmpvertex[LAST].r;
            tmpvertex[i].g = tmpvertex[LAST].g;
            tmpvertex[i].b = tmpvertex[LAST].b;
            tmpvertex[i].a = tmpvertex[LAST].a;
        }
        Vector I(rand() * 2.0 / RAND_MAX - 1,
                rand() * 2.0 / RAND_MAX - 1,
                rand() * 2.0 / RAND_MAX - 1);
        Vector J(rand() * 2.0 / RAND_MAX - 1,
                rand() * 2.0 / RAND_MAX - 1,
                rand() * 2.0 / RAND_MAX - 1);
        Vector K(rand() * 2.0 / RAND_MAX - 1,
                rand() * 2.0 / RAND_MAX - 1,
                rand() * 2.0 / RAND_MAX - 1);
        if (I.MagnitudeSquared() < .00001) {
            I.i += .5;
        }
        if (J.MagnitudeSquared() < .00001) {
            J.j += .5;
        }
        if (K.MagnitudeSquared() < .00001) {
            K.k += .5;
        }
        Orthogonize(I, J, K);
        I = I * size;
        J = J * size;
        K = K * size;
        tmpvertex[LC + 0].SetVertex(GetConstVertex(tmpvertex[LC + 0]) - I + J);
        tmpvertex[LC + 0].s = 0.15625;
        tmpvertex[LC + 0].t = .984375;
        tmpvertex[LC + 1].SetVertex(GetConstVertex(tmpvertex[LC + 1]) + I + J);
        tmpvertex[LC + 1].s = .984375;
        tmpvertex[LC + 1].t = .984375;
        tmpvertex[LC + 2].SetVertex(GetConstVertex(tmpvertex[LC + 2]) + I - J);
        tmpvertex[LC + 2].s = .984375;
        tmpvertex[LC + 2].t = .015625;
        tmpvertex[LC + 3].SetVertex(GetConstVertex(tmpvertex[LC + 3]) - I - J);
        tmpvertex[LC + 3].s = .015625;
        tmpvertex[LC + 3].t = .015625;
        if (numVerticesPer > 4) {
            tmpvertex[LC + 4].SetVertex(GetConstVertex(tmpvertex[LC + 4]) - I + K);
            tmpvertex[LC + 4].s = .015625;
            tmpvertex[LC + 4].t = .984375;
            tmpvertex[LC + 5].SetVertex(GetConstVertex(tmpvertex[LC + 5]) + I + K);
            tmpvertex[LC + 5].s = .984375;
            tmpvertex[LC + 5].t = .984375;
            tmpvertex[LC + 6].SetVertex(GetConstVertex(tmpvertex[LC + 6]) + I - K);
            tmpvertex[LC + 6].s = .984375;
            tmpvertex[LC + 6].t = .015625;
            tmpvertex[LC + 7].SetVertex(GetConstVertex(tmpvertex[LC + 7]) - I - K);
            tmpvertex[LC + 7].s = .015625;
            tmpvertex[LC + 7].t = .015625;
        }
        if (numVerticesPer > 8) {
            tmpvertex[LC + 8].SetVertex(GetConstVertex(tmpvertex[LC + 8]) - J + K);
            tmpvertex[LC + 8].s = .015625;
            tmpvertex[LC + 8].t = .984375;
            tmpvertex[LC + 9].SetVertex(GetConstVertex(tmpvertex[LC + 9]) + J + K);
            tmpvertex[LC + 9].s = .984375;
            tmpvertex[LC + 9].t = .984375;
            tmpvertex[LC + 10].SetVertex(GetConstVertex(tmpvertex[LC + 10]) + J - K);
            tmpvertex[LC + 10].s = .984375;
            tmpvertex[LC + 10].t = .015625;
            tmpvertex[LC + 11].SetVertex(GetConstVertex(tmpvertex[LC + 11]) - J - K);
            tmpvertex[LC + 11].s = .015625;
            tmpvertex[LC + 11].t = .015625;
        }
    }
    {
        int start = 0;
        int inc = num / NUM_ACTIVE_ANIMATIONS;
        inc -= inc % numVerticesPer;
        for (int i = 0; i < NUM_ACTIVE_ANIMATIONS; ++i, start += inc) {
            int later = start + inc;
            if (i == NUM_ACTIVE_ANIMATIONS - 1) {
                later = num;
            }
            vlist[i] = new GFXVertexList(GFXQUAD, later - start, tmpvertex + start, later - start, true, 0);
        }
    }
    delete[] tmpvertex;
}

int SpriteStarVlist::NumTextures() {
    return NUM_ACTIVE_ANIMATIONS;
}

bool SpriteStarVlist::BeginDrawState(const QVector &center,
        const Vector &velocity,
        const Vector &torque,
        bool roll,
        bool yawpitch,
        int whichTex) {
    UpdateGraphics();
    GFXEnable(TEXTURE0);
    decal[whichTex]->MakeActive();
    GFXDisable(CULLFACE);
    GFXColorMaterial(AMBIENT | DIFFUSE);
    vlist[whichTex]->LoadDrawState();
    vlist[whichTex]->BeginDrawState();
    return false;
}

void SpriteStarVlist::EndDrawState(bool stretch, int whichTex) {
    vlist[whichTex]->EndDrawState();
    GFXDisable(TEXTURE0);
    GFXEnable(CULLFACE);
    GFXColorMaterial(0);
}

extern bool isVista;

void SpriteStarVlist::Draw(bool strertch, int whichTexture) {
    static bool force_draw = vega_config::config->graphics.vista_draw_stars;
    if (force_draw || !isVista) {
        vlist[whichTexture]->Draw();
    }
}

SpriteStarVlist::~SpriteStarVlist() {
    for (int i = 0; i < NUM_ACTIVE_ANIMATIONS; ++i) {
        if (decal[i] != nullptr) {
            delete decal[i];
            decal[i] = nullptr;
        }
    }
    for (int j = 0; j < NUM_ACTIVE_ANIMATIONS; ++j) {
        if (vlist[j] != nullptr) {
            delete vlist[j];
            vlist[j] = nullptr;
        }
    }
}

