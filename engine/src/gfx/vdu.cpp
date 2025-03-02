/*
 * vdu.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


/// Draws VDU parts
/// Draws shield, armor, comm strings and animation, messages, manifest,
/// target info, and objectives

#include "vdu.h"
#include "cmd/unit_util.h"
#include "hud.h"
#include "vs_globals.h"
#include "cockpit.h"
#include "cmd/script/mission.h"
#include "cmd/script/flightgroup.h"
#include "cmd/script/msgcenter.h"
#include "cmd/images.h"
#include "cmd/planet.h"
#include "cmd/beam.h"
#include "xml_support.h"
#include "gfx/animation.h"
#include "galaxy_gen.h"
#include "universe_util.h"
#include "vsfilesystem.h"
#include "cmd/ai/communication.h"
#include "universe.h"
#include "mount_size.h"
#include "weapon_info.h"
#include "configuration/configuration.h"
#include "dock_utils.h"

template<typename T>
inline T mymin(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
inline T mymax(T a, T b) {
    return (a > b) ? a : b;
}

bool VDU::staticable() const {
    unsigned int thismode = getMode();
    static bool only_scanner_modes_static =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "only_scanner_modes_static", "true"));
    if (thismode != COMM && thismode != TARGETMANIFEST && thismode != TARGET && thismode != NAV && thismode != VIEW
            && thismode != WEBCAM && only_scanner_modes_static) {
        return false;
    }
    return (posmodes & (posmodes - 1)) != 0;       //check not power of two
}

///ALERT to change must change enum in class
const std::string vdu_modes[] =
        {"Target", "Nav", "Objectives", "Comm", "Weapon", "Damage", "Shield", "Manifest", "TargetManifest", "View",
                "Message"};

string reformatName(string nam) {
    nam = nam.substr(0, nam.find("."));
    if (nam.length()) {
        nam[0] = toupper(nam[0]);
    }
    return nam;
}

string getUnitNameAndFgNoBase(Unit *target) {
    Flightgroup *fg = target->getFlightgroup();
    if (target->isUnit() == Vega_UnitType::planet) {
        string hr = ((Planet *) target)->getHumanReadablePlanetType();
        if (!hr.empty()) {
            return hr + string(":") + reformatName(target->name);
        }
    } else if (target->isUnit() == Vega_UnitType::unit) {
        if (fg) {
            int fgsnumber = target->getFgSubnumber();
            string fgnstring = XMLSupport::tostring(fgsnumber);
            const bool print_fg_sub_id = configuration()->graphics_config.hud.print_fg_sub_id;
            string fgname;
            if (fg->name != "Base" && fg->name != "Asteroid" && fg->name != "Nebula") {
                const bool print_ship_type = configuration()->graphics_config.hud.print_ship_type;
                if (configuration()->graphics_config.hud.print_fg_name) {
                    fgname += fg->name
                            + (print_ship_type ? ((print_fg_sub_id && (!fgnstring.empty())) ? " =" : " : ") : "");
                }
                if (print_fg_sub_id && ("" != fgnstring)) {
                    fgname += fgnstring + "= ";
                }
                if (print_ship_type) {
                    return fgname + reformatName(target->getFullname());
                }
                return fgname;
            } else if (fg->name == "Base") {
                if (configuration()->graphics_config.hud.basename_colon_basename == false || reformatName(target->name) == (reformatName(target->getFullname()))) {
                    std::string retval(reformatName(target->getFullname()));
                    if (print_fg_sub_id && ("" != fgnstring)) {
                        retval += " : " + fgnstring;
                    }
                    return retval;
                } else {
                    if (reformatName(target->name) == (reformatName(target->getFullname()))) {
                        std::string retval(reformatName(
                                target->name) + " "
                                + ((print_fg_sub_id && ("" != fgnstring)) ? (": " + fgnstring) : ""));
                        return retval;
                    } else {
                        std::string retval(reformatName(target->name) + " : " + target->getFullname());
                        return retval;
                    }
                }
            }
        }
    }
    return reformatName(target->name);
}

int vdu_lookup(char *&s) {
#ifdef _MSC_VER
#define strcasecmp stricmp
#endif
    int retval = 0;
    char *t = strdup(s);
    int i;
    for (i = 0; t[i] != '\0'; i++) {
        if (isspace(t[i])) {
            s += i + 1;
            break;
        }
    }
    if (t[i] == '\0') {
        s[0] = '\0';
    }
    t[i] = '\0';
    for (unsigned int j = 0; j < ((sizeof(vdu_modes) / sizeof(std::string))); j++) {
        if (0 == strcasecmp(t, vdu_modes[j].c_str())) {
            retval |= (1 << j);
        }
    }
    free(t);
    return retval;
}

int parse_vdu_type(const char *x) {
    char *mystr = strdup(x);
    char *s = mystr;
    int retval = 0;
    while (s[0] != '\0') {
        retval |= vdu_lookup(s);
    }
    free(mystr);
    return retval;
}

VDU::VDU(const char *file, TextPlane *textp, unsigned short modes, short rwws, short clls) : VSSprite(file), tp(textp), posmodes(modes), rows(rwws), cols(clls), scrolloffset(0) {
    thismode.push_back(MSG);
    if (_Universe->numPlayers() > 1) {
        posmodes &= (~VIEW);
    }
    comm_ani = NULL;
    viewStyle = CP_TARGET;
    got_target_info = true;
    SwitchMode(NULL);
}

GFXColor getDamageColor(float armor, bool gradient = false) {
    static GFXColor damaged = vs_config->getColor("default", "hud_target_damaged",
            GFXColor(1, 0, 0, 1));
    static GFXColor half_damaged = vs_config->getColor("default", "hud_target_half_damaged",
            GFXColor(1, 1, 0, 1));
    static GFXColor full = vs_config->getColor("default", "hud_target_full",
            GFXColor(1, 1, 1, 1));
    if (armor >= .9) {
        return full;
    }
    float avghalf = armor >= .3 ? 1 : 0;
    if (gradient && armor >= .3) {
        avghalf = (armor - .3) / .6;
    }
    return colLerp(damaged, half_damaged, avghalf);
}

static void DrawHUDSprite(VDU *thus,
        VSSprite *s,
        float per,
        float &sx,
        float &sy,
        float &w,
        float &h,
        float aup,
        float aright,
        float aleft,
        float adown,
        float hull,
        bool drawsprite,
        bool invertsprite) {
    static bool HighQTargetVSSprites =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "high_quality_sprites", "false"));
    float nw, nh;
    thus->GetPosition(sx, sy);
    thus->GetSize(w, h);

    //Use margins specified from config file
    static float width_factor = XMLSupport::parse_float(vs_config->getVariable("graphics", "reduced_vdus_width", "0"));
    static float
            height_factor = XMLSupport::parse_float(vs_config->getVariable("graphics", "reduced_vdus_height", "0"));
    w = w - width_factor;
    h = h + height_factor;

    h = -fabs(h * per);
    if (!s) {
        h = -h;
        w = fabs(w * per);
    } else {
        if (HighQTargetVSSprites) {
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
        } else {
            GFXBlendMode(ONE, ZERO);
            GFXAlphaTest(GREATER, .4);
        }
        s->SetPosition(sx, sy);
        s->GetSize(nw, nh);
        w = fabs(nw * h / nh);
        s->SetSize(w, invertsprite ? -h : h);
        Texture *spritetex = s->getTexture();
        if (drawsprite && spritetex) {
            const float middle_point = configuration()->graphics_config.hud.armor_hull_size;
            const bool top_view = configuration()->graphics_config.hud.top_view;
            const float middle_point_small = 1.0F - middle_point;
            Vector ll, lr, ur, ul, mll, mlr, mur, mul;
            spritetex->MakeActive();
            GFXDisable(CULLFACE);
            s->DrawHere(ll, lr, ur, ul);
            mll = middle_point * ll + middle_point_small * ur;
            mlr = middle_point * lr + middle_point_small * ul;
            mur = middle_point * ur + middle_point_small * ll;
            mul = middle_point * ul + middle_point_small * lr;
            const GFXColor c[9] = {
                    getDamageColor(top_view ? adown : aleft),
                    getDamageColor(top_view ? adown : aup),
                    getDamageColor(hull, true),
                    getDamageColor(top_view ? aright : aup),
                    getDamageColor(aright),
                    getDamageColor(top_view ? aup : aright),
                    getDamageColor(top_view ? aup : adown),
                    getDamageColor(top_view ? aleft : adown),
                    getDamageColor(aleft),
            };
            const float verts[20 * (3 + 4 + 2)] = {
                    ul.x, ul.y, ul.z, c[0].r, c[0].g, c[0].b, c[0].a, 0.0f, 0.0f,
                    ur.x, ur.y, ur.z, c[1].r, c[1].g, c[1].b, c[1].a, 1.0f, 0.0f,
                    mur.x, mur.y, mur.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point, middle_point_small,
                    mul.x, mul.y, mul.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point_small, middle_point_small,

                    ur.x, ur.y, ur.z, c[3].r, c[3].g, c[3].b, c[3].a, 1.0f, 0.0f,
                    lr.x, lr.y, lr.z, c[4].r, c[4].g, c[4].b, c[4].a, 1.0f, 1.0f,
                    mlr.x, mlr.y, mlr.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point, middle_point,
                    mur.x, mur.y, mur.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point, middle_point_small,

                    lr.x, lr.y, lr.z, c[5].r, c[5].g, c[5].b, c[5].a, 1.0f, 1.0f,
                    ll.x, ll.y, ll.z, c[6].r, c[6].g, c[6].b, c[6].a, 0.0f, 1.0f,
                    mll.x, mll.y, mll.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point_small, middle_point,
                    mlr.x, mlr.y, mlr.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point, middle_point,

                    ll.x, ll.y, ll.z, c[7].r, c[7].g, c[7].b, c[7].a, 0.0f, 1.0f,
                    ul.x, ul.y, ul.z, c[8].r, c[8].g, c[8].b, c[8].a, 0.0f, 0.0f,
                    mul.x, mul.y, mul.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point_small, middle_point_small,
                    mll.x, mll.y, mll.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point_small, middle_point,

                    mul.x, mul.y, mul.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point_small, middle_point_small,
                    mur.x, mur.y, mur.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point, middle_point_small,
                    mlr.x, mlr.y, mlr.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point, middle_point,
                    mll.x, mll.y, mll.z, c[2].r, c[2].g, c[2].b, c[2].a, middle_point_small, middle_point,
            };
            GFXDraw(GFXQUAD, verts, 20, 3, 4, 2);
            GFXEnable(CULLFACE);
        }
        s->SetSize(nw, nh);
        h = fabs(h);
        if (HighQTargetVSSprites) {
            GFXBlendMode(ONE, ZERO);
        } else {
            GFXAlphaTest(ALWAYS, 0);
        }
    }
}

void VDU::DrawTargetSpr(VSSprite *s, float per, float &sx, float &sy, float &w, float &h) {
    DrawHUDSprite(this, s, per, sx, sy, w, h, 1, 1, 1, 1, 1, true, false);
}

void VDU::Scroll(int howmuch) {
    scrolloffset += howmuch;
}

#define MangleString(a, b) (a)

/*
static std::string MangleStrung( std::string in, float probability )
{
    //fails with ppc
    vector< char >str;
    for (int i = 0; i < (int) in.length(); i++) {
        if (in[i] != '\n') {
            str.push_back( in[i] );
            if (rand() < probability*RAND_MAX)
                str.back() += rand()%12-6;
            if (rand() < .1*probability*RAND_MAX)
                str.push_back( 'a'+rand()%26 );
        } else {
            if (rand() < .1*probability*RAND_MAX)
                while (rand()%5)
                    str.push_back( 'a'+rand()%26 );
            str.push_back( in[i] );
        }
    }
    return std::string( str.begin(), str.end() );
}
*/

static void DrawShield(float fs,
        float rs,
        float ls,
        float bs,
        float x,
        float y,
        float w,
        float h,
        bool invert,
        GFXColor outershield,
        GFXColor middleshield,
        GFXColor innershield) {
    //FIXME why is this static?

    if (invert) {
        float tmp = fs;
        fs = bs;
        bs = tmp;
    }
    GFXColor shcolor[4][3] = {
            {innershield, middleshield, outershield},
            {innershield, middleshield, outershield},
            {innershield, middleshield, outershield},
            {innershield, middleshield, outershield}
    };
    float shthresh[3] = { configuration()->graphics_config.hud.shield_vdu_thresh[0], configuration()->graphics_config.hud.shield_vdu_thresh[1], configuration()->graphics_config.hud.shield_vdu_thresh[2] };

    float shtrans[3] = {1.0f, 1.0f, 1.0f};
    if (configuration()->graphics_config.hud.shield_vdu_fade) {
        shcolor[0][0].a *= mymax(0.0f, mymin(1.0f, (fs - shthresh[0]) / (shthresh[1] - shthresh[0]) * shtrans[0]));
        shcolor[0][1].a *= mymax(0.0f, mymin(1.0f, (fs - shthresh[1]) / (shthresh[2] - shthresh[1]) * shtrans[1]));
        shcolor[0][2].a *= mymax(0.0f, mymin(1.0f, (fs - shthresh[2]) / (1.0f - shthresh[2]) * shtrans[2]));
        shcolor[1][0].a *= mymax(0.0f, mymin(1.0f, (rs - shthresh[0]) / (shthresh[1] - shthresh[0]) * shtrans[0]));
        shcolor[1][1].a *= mymax(0.0f, mymin(1.0f, (rs - shthresh[1]) / (shthresh[2] - shthresh[1]) * shtrans[1]));
        shcolor[1][2].a *= mymax(0.0f, mymin(1.0f, (rs - shthresh[2]) / (1.0f - shthresh[2]) * shtrans[2]));
        shcolor[2][0].a *= mymax(0.0f, mymin(1.0f, (ls - shthresh[0]) / (shthresh[1] - shthresh[0]) * shtrans[0]));
        shcolor[2][1].a *= mymax(0.0f, mymin(1.0f, (ls - shthresh[1]) / (shthresh[2] - shthresh[1]) * shtrans[1]));
        shcolor[2][2].a *= mymax(0.0f, mymin(1.0f, (ls - shthresh[2]) / (1.0f - shthresh[2]) * shtrans[2]));
        shcolor[3][0].a *= mymax(0.0f, mymin(1.0f, (bs - shthresh[0]) / (shthresh[1] - shthresh[0]) * shtrans[0]));
        shcolor[3][1].a *= mymax(0.0f, mymin(1.0f, (bs - shthresh[1]) / (shthresh[2] - shthresh[1]) * shtrans[1]));
        shcolor[3][2].a *= mymax(0.0f, mymin(1.0f, (bs - shthresh[2]) / (1.0f - shthresh[2]) * shtrans[2]));
    }

    GFXEnable(SMOOTH);
    GFXPushBlendMode();
    GFXBlendMode(SRCALPHA, INVSRCALPHA);

    static VertexBuilder<float, 3, 0, 4> verts;
    verts.clear();
    verts.reserve(72);
    if (fs > shthresh[0]) {
        verts.insert(GFXColorVertex(Vector(x - w / 8, y + h / 2, 0), shcolor[0][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 3, y + .9 * h / 2, 0), shcolor[0][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y + h / 2, 0), shcolor[0][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 3, y + .9 * h / 2, 0), shcolor[0][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y + h / 2, 0), shcolor[0][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 8, y + h / 2, 0), shcolor[0][0]));
    }
    if (fs > shthresh[1]) {
        verts.insert(GFXColorVertex(Vector(x - w / 8, y + 1.1 * h / 2, 0), shcolor[0][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y + 1.1 * h / 2, 0), shcolor[0][1]));
        verts.insert(GFXColorVertex(Vector(x - w / 8, y + 1.1 * h / 2, 0), shcolor[0][1]));
        verts.insert(GFXColorVertex(Vector(x - w / 3, y + h / 2, 0), shcolor[0][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y + 1.1 * h / 2, 0), shcolor[0][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 3, y + h / 2, 0), shcolor[0][1]));
    }
    if (fs > shthresh[2]) {
        verts.insert(GFXColorVertex(Vector(x - w / 8, y + 1.2 * h / 2, 0), shcolor[0][2]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y + 1.2 * h / 2, 0), shcolor[0][2]));
        verts.insert(GFXColorVertex(Vector(x - w / 8, y + 1.2 * h / 2, 0), shcolor[0][2]));
        verts.insert(GFXColorVertex(Vector(x - w / 3, y + 1.1 * h / 2, 0), shcolor[0][2]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y + 1.2 * h / 2, 0), shcolor[0][2]));
        verts.insert(GFXColorVertex(Vector(x + w / 3, y + 1.1 * h / 2, 0), shcolor[0][2]));
    }
    if (rs > shthresh[0]) {
        verts.insert(GFXColorVertex(Vector(x + w / 2, y - h / 8, 0), shcolor[1][0]));
        verts.insert(GFXColorVertex(Vector(x + .9 * w / 2, y - h / 3, 0), shcolor[1][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 2, y + h / 8, 0), shcolor[1][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 2, y - h / 8, 0), shcolor[1][0]));
        verts.insert(GFXColorVertex(Vector(x + .9 * w / 2, y + h / 3, 0), shcolor[1][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 2, y + h / 8, 0), shcolor[1][0]));
    }
    if (rs > shthresh[1]) {
        verts.insert(GFXColorVertex(Vector(x + 1.1 * w / 2, y - h / 8, 0), shcolor[1][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 2, y - h / 3, 0), shcolor[1][1]));
        verts.insert(GFXColorVertex(Vector(x + 1.1 * w / 2, y + h / 8, 0), shcolor[1][1]));
        verts.insert(GFXColorVertex(Vector(x + 1.1 * w / 2, y - h / 8, 0), shcolor[1][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 2, y + h / 3, 0), shcolor[1][1]));
        verts.insert(GFXColorVertex(Vector(x + 1.1 * w / 2, y + h / 8, 0), shcolor[1][1]));
    }
    if (rs > shthresh[2]) {
        verts.insert(GFXColorVertex(Vector(x + 1.2 * w / 2, y - h / 8, 0), shcolor[1][2]));
        verts.insert(GFXColorVertex(Vector(x + 1.1 * w / 2, y - h / 3, 0), shcolor[1][2]));
        verts.insert(GFXColorVertex(Vector(x + 1.2 * w / 2, y + h / 8, 0), shcolor[1][2]));
        verts.insert(GFXColorVertex(Vector(x + 1.2 * w / 2, y - h / 8, 0), shcolor[1][2]));
        verts.insert(GFXColorVertex(Vector(x + 1.1 * w / 2, y + h / 3, 0), shcolor[1][2]));
        verts.insert(GFXColorVertex(Vector(x + 1.2 * w / 2, y + h / 8, 0), shcolor[1][2]));
    }
    if (ls > shthresh[0]) {
        verts.insert(GFXColorVertex(Vector(x - w / 2, y - h / 8, 0), shcolor[2][0]));
        verts.insert(GFXColorVertex(Vector(x - .9 * w / 2, y - h / 3, 0), shcolor[2][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 2, y + h / 8, 0), shcolor[2][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 2, y - h / 8, 0), shcolor[2][0]));
        verts.insert(GFXColorVertex(Vector(x - .9 * w / 2, y + h / 3, 0), shcolor[2][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 2, y + h / 8, 0), shcolor[2][0]));
    }
    if (ls > shthresh[1]) {
        verts.insert(GFXColorVertex(Vector(x - 1.1 * w / 2, y - h / 8, 0), shcolor[2][1]));
        verts.insert(GFXColorVertex(Vector(x - w / 2, y - h / 3, 0), shcolor[2][1]));
        verts.insert(GFXColorVertex(Vector(x - 1.1 * w / 2, y + h / 8, 0), shcolor[2][1]));
        verts.insert(GFXColorVertex(Vector(x - 1.1 * w / 2, y - h / 8, 0), shcolor[2][1]));
        verts.insert(GFXColorVertex(Vector(x - w / 2, y + h / 3, 0), shcolor[2][1]));
        verts.insert(GFXColorVertex(Vector(x - 1.1 * w / 2, y + h / 8, 0), shcolor[2][1]));
    }
    if (ls > shthresh[2]) {
        verts.insert(GFXColorVertex(Vector(x - 1.2 * w / 2, y - h / 8, 0), shcolor[2][2]));
        verts.insert(GFXColorVertex(Vector(x - 1.1 * w / 2, y - h / 3, 0), shcolor[2][2]));
        verts.insert(GFXColorVertex(Vector(x - 1.2 * w / 2, y + h / 8, 0), shcolor[2][2]));
        verts.insert(GFXColorVertex(Vector(x - 1.2 * w / 2, y - h / 8, 0), shcolor[2][2]));
        verts.insert(GFXColorVertex(Vector(x - 1.1 * w / 2, y + h / 3, 0), shcolor[2][2]));
        verts.insert(GFXColorVertex(Vector(x - 1.2 * w / 2, y + h / 8, 0), shcolor[2][2]));
    }
    if (bs > shthresh[0]) {
        verts.insert(GFXColorVertex(Vector(x - w / 8, y - h / 2, 0), shcolor[3][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 3, y - .9 * h / 2, 0), shcolor[3][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y - h / 2, 0), shcolor[3][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 3, y - .9 * h / 2, 0), shcolor[3][0]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y - h / 2, 0), shcolor[3][0]));
        verts.insert(GFXColorVertex(Vector(x - w / 8, y - h / 2, 0), shcolor[3][0]));
    }
    if (bs > shthresh[1]) {
        verts.insert(GFXColorVertex(Vector(x - w / 8, y - 1.1 * h / 2, 0), shcolor[3][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y - 1.1 * h / 2, 0), shcolor[3][1]));
        verts.insert(GFXColorVertex(Vector(x - w / 8, y - 1.1 * h / 2, 0), shcolor[3][1]));
        verts.insert(GFXColorVertex(Vector(x - w / 3, y - h / 2, 0), shcolor[3][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y - 1.1 * h / 2, 0), shcolor[3][1]));
        verts.insert(GFXColorVertex(Vector(x + w / 3, y - h / 2, 0), shcolor[3][1]));
    }
    if (bs > shthresh[2]) {
        verts.insert(GFXColorVertex(Vector(x - w / 8, y - 1.2 * h / 2, 0), shcolor[3][2]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y - 1.2 * h / 2, 0), shcolor[3][2]));
        verts.insert(GFXColorVertex(Vector(x - w / 8, y - 1.2 * h / 2, 0), shcolor[3][2]));
        verts.insert(GFXColorVertex(Vector(x - w / 3, y - 1.1 * h / 2, 0), shcolor[3][2]));
        verts.insert(GFXColorVertex(Vector(x + w / 8, y - 1.2 * h / 2, 0), shcolor[3][2]));
        verts.insert(GFXColorVertex(Vector(x + w / 3, y - 1.1 * h / 2, 0), shcolor[3][2]));
    }
    GFXDraw(GFXLINE, verts);

    GFXDisable(SMOOTH);
    GFXPopBlendMode();
}

void VDU::DrawVDUShield(Unit *parent) {
    float x, y, w, h;
    GetPosition(x, y);
    GetSize(w, h);
    //Use margins specified from config file
    w = w - configuration()->graphics_config.reduced_vdus_width;
    h = h + configuration()->graphics_config.reduced_vdus_height;

    h = fabs(h * .6);
    w = fabs(w * .6);

    double hull_percent = parent->hull.Percent();
    //static bool invert_friendly_shields =
    //    XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "invert_friendly_shields", "false" ) );
    //DrawShieldArmor(parent,StartArmor,x,y,w,h,invert_friendly_shields);
    GFXColor4f(1, hull_percent, hull_percent, 1);
    GFXEnable(TEXTURE0);
    GFXColor4f(1, hull_percent, hull_percent, 1);
    const bool invert_friendly_sprite = configuration()->graphics_config.hud.invert_friendly_sprite;
    DrawHUDSprite(this, parent->getHudImage(), .25, x, y, w, h, hull_percent,
            hull_percent, hull_percent, hull_percent,
            hull_percent, true, invert_friendly_sprite);
}

// TODO: make into function
#define RETURN_STATIC_SPRITE(name)            \
    do {                                        \
        static VSSprite s( name ".sprite" );    \
        static VSSprite sCompat( name ".spr" ); \
        if ( s.LoadSuccess() )                  \
            return &s;                          \
        else                                    \
            return &sCompat;                    \
    }                                           \
    while (0)

VSSprite *getTargetQuadShield() {
    RETURN_STATIC_SPRITE("shield_quad");
}

VSSprite *getTargetDualShield() {
    RETURN_STATIC_SPRITE("shield_dual");
}

VSSprite *getJumpImage() {
    RETURN_STATIC_SPRITE("jump-hud");
}

VSSprite *getSunImage() {
    RETURN_STATIC_SPRITE("sun-hud");
}

VSSprite *getPlanetImage() {
    RETURN_STATIC_SPRITE("planet-hud");
}

VSSprite *getNavImage() {
    RETURN_STATIC_SPRITE("nav-hud");
}


static float OneOfFour(float a, float b, float c, float d) {
    int aa = a != 0 ? 1 : 0;
    int bb = b != 0 ? 1 : 0;
    int cc = c != 0 ? 1 : 0;
    int dd = d != 0 ? 1 : 0;
    if (aa + bb + cc + dd == 4) {
        return 1;
    }
    if (aa + bb + cc + dd == 3) {
        return .85;
    }
    if (aa + bb + cc + dd == 2) {
        return .4;
    }
    return 0;
}

static float TwoOfFour(float a, float b, float c, float d) {
    int aa = a != 0 ? 1 : 0;
    int bb = b != 0 ? 1 : 0;
    int cc = c != 0 ? 1 : 0;
    int dd = d != 0 ? 1 : 0;
    if (aa + bb + cc + dd == 4) {
        return 1;
    }
    if (aa + bb + cc + dd == 3) {
        return 1;
    }
    if (aa + bb + cc + dd == 2) {
        return .8;
    }
    if (aa + bb + cc + dd == 1) {
        return .4;
    }
    return 0;
}

void VDU::DrawTarget(GameCockpit *cp, Unit *parent, Unit *target) {
    float x, y, w, h;
    
    GFXEnable(TEXTURE0);
    const bool invert_target_sprite = configuration()->graphics_config.hud.invert_target_sprite;
    
    float armor_up = target->armor.Percent(Armor::front);
    float armor_down = target->armor.Percent(Armor::back);
    float armor_left = target->armor.Percent(Armor::right);
    float armor_right = target->armor.Percent(Armor::left);
    if (target->isUnit() == Vega_UnitType::planet) {
        armor_up = armor_down = armor_left = armor_right = target->hull.Percent();
    }

    DrawHUDSprite(this,
            ((target->isUnit() != Vega_UnitType::planet || target->getHudImage() != NULL) ? target->getHudImage()
                    : (
                            target->GetDestinations().size() != 0 ? getJumpImage()
                                    : (((Planet *) target)->hasLights() ? getSunImage()
                                    : (
                                            target->getFullname().find(
                                                    "invisible") != string::npos
                                                    ? getNavImage() : getPlanetImage())))),
            .6,
            x,
            y,
            w,
            h,
            armor_up,
            armor_right,
            armor_left,
            armor_down,
            target->hull.Percent(),
            true,
            invert_target_sprite);

    GFXDisable(TEXTURE0);
    //sprintf (t,"\n%4.1f %4.1f",target->FShieldData()*100,target->RShieldData()*100);
    double mm = 0;
    string unitandfg = getUnitNameAndFgNoBase(target).c_str();
    const bool out_of_cone_information = configuration()->graphics_config.hud.out_of_cone_distance;
    bool inrange = parent->InRange(target, mm, out_of_cone_information == false && !UnitUtil::isSignificant(
            target), false, false);
    if (inrange) {
        static int neut = FactionUtil::GetFactionIndex("neutral");
        static int upgr = FactionUtil::GetFactionIndex("upgrades");
        if (target->faction != neut && target->faction != upgr) {
            if (configuration()->graphics_config.hud.print_faction) {
                unitandfg += std::string("\n") + FactionUtil::GetFaction(target->faction);
            }
        }
    }
    unitandfg += std::string("\n");
    unitandfg += cp->getTargetLabel();
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(MangleString(unitandfg, _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0),
            0,
            true,
            false,
            automatte);
    tp->bgcol = tpbg;
    static float auto_message_lim =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "auto_message_time_lim", "5"));
    float delautotime = UniverseUtil::GetGameTime() - cp->autoMessageTime;
    bool draw_auto_message = (delautotime < auto_message_lim && cp->autoMessage.length() != 0);
    if (inrange) {
        char st[1024];
        memset(st, '\n', 1023);
        int tmplim = rows - 3;
        if (draw_auto_message == true) {
            tmplim--;
        }
        st[tmplim] = '\0';
        std::string newst(st);
        if (draw_auto_message) {
            newst += cp->autoMessage + "\n";
        }
        newst += '\n';
        double actual_range = DistanceTwoTargets(parent, target);
        newst += GetDockingText(parent, target, actual_range);
        newst += string("\nRange: ") + PrettyDistanceString(actual_range);
        const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
        GFXColor tpbg = tp->bgcol;
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            tp->bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        tp->Draw(MangleString(newst, _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0),
                0,
                true,
                false,
                automatte);
        tp->bgcol = tpbg;
        static float ishieldcolor[4] = {.4, .4, 1, 1};
        static float mshieldcolor[4] = {.4, .4, 1, 1};
        static float oshieldcolor[4] = {.4, .4, 1, 1};
        //code replaced by target shields defined in cockpit.cpt files, preserve for mods
        static bool builtin_shields =
                XMLSupport::parse_bool(vs_config->getVariable("graphics", "vdu_builtin_shields", "false"));
        if (builtin_shields) {
            DrawShield(target->shield.Percent(Shield::front), 
            target->shield.Percent(Shield::right), 
            target->shield.Percent(Shield::left), 
            target->shield.Percent(Shield::back), x, y, w, h, false,
                    GFXColor(ishieldcolor[0], ishieldcolor[1], ishieldcolor[2], ishieldcolor[3]),
                    GFXColor(mshieldcolor[0], mshieldcolor[1], mshieldcolor[2], mshieldcolor[3]),
                    GFXColor(oshieldcolor[0], oshieldcolor[1], oshieldcolor[2], oshieldcolor[3]));
        }
        //this is a possibility to draw target shields but without gauging
        //the gauging method is implemented in cockpit.cpp
/*  if (target->isUnit()!=Vega_UnitType::planet) {
 *   GFXEnable (TEXTURE0);
 *   //Dev:GFXColor4f (1,target->GetHullPercent(),target->GetHullPercent(),1);
 *   DrawHUDSprite(this,getTargetQuadShield(),0.9,x,y,w,h,fs,rs,ls,bs,target->GetHullPercent(),true,invert_target_sprite);
 *   GFXDisable (TEXTURE0);
 *  }*/
        GFXColor4f(1, 1, 1, 1);
    } else {
        const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
        GFXColor tpbg = tp->bgcol;
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            tp->bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        if (draw_auto_message) {
            tp->Draw(MangleString(std::string("\n") + cp->autoMessage, _Universe->AccessCamera()->GetNebula()
                            != NULL ? .4 : 0),
                    0,
                    true,
                    false,
                    automatte);
        } else {
            tp->Draw(MangleString("\n[OutOfRange]",
                    _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0), 0, true, false, automatte);
        }
        tp->bgcol = tpbg;
    }
}

void VDU::DrawMessages(GameCockpit *parentcp, Unit *target) {
    static bool draw_messages = XMLSupport::parse_bool(vs_config->getVariable("graphics", "chat_text", "true"));

    if (draw_messages == false) {
        return;
    }

    string fullstr;
    double nowtime = mission->getGametime(); //for message display duration

    string targetstr;
    //int    msglen      = targetstr.size();
    int rows_needed = 0;  //msglen/(cols>0?cols:1);
    MessageCenter *mc = mission->msgcenter;
    int rows_used = rows_needed;
    vector<std::string> whoNOT;
    whoNOT.push_back("briefing");
    whoNOT.push_back("news");
    whoNOT.push_back("bar");

    static float oldtime = XMLSupport::parse_float(vs_config->getVariable("graphics", "last_message_time", "5"));
    static int num_messages = XMLSupport::parse_int(vs_config->getVariable("graphics", "num_messages", "2"));
    static bool showStardate = XMLSupport::parse_bool(vs_config->getVariable("graphics", "show_stardate", "true"));

    vector<std::string> message_people;     //should be "all", parent's name
    gameMessage lastmsg;
    int row_lim = ((scrolloffset < 0 || num_messages > rows) ? rows : num_messages);

    for (int i = scrolloffset < 0 ? -scrolloffset - 1 : 0;
            rows_used < row_lim && mc->last(i, lastmsg, message_people, whoNOT);
            i++) {
        char timebuf[100];
        double sendtime = lastmsg.time;
        if (scrolloffset >= 0 && sendtime < nowtime - oldtime * 4) {
            break;
        }
        if (sendtime <= nowtime && (sendtime > nowtime - oldtime || scrolloffset < 0)) {
            if (showStardate) {
                string stardate = _Universe->current_stardate
                        .ConvertFullTrekDate(
                                sendtime + _Universe->current_stardate.GetElapsedStarTime());
                sprintf(timebuf, "%s", stardate.c_str());
            } else {
                int sendtime_mins = (int) (sendtime / 60.0);
                int sendtime_secs = (int) (sendtime - sendtime_mins * 60);
                sprintf(timebuf, "%d.%02d", sendtime_mins, sendtime_secs);
            }

            string mymsg;
            if (lastmsg.from != "game") {
                mymsg = lastmsg.from + " (" + timebuf + "): " + lastmsg.message;
            } else {
                mymsg = string(timebuf) + ": " + lastmsg.message;
            }
            int msglen = mymsg.size();
            int rows_needed = (int) (msglen / (1.6 * cols));
            fullstr = mymsg + "\n" + fullstr;
            //fullstr=fullstr+mymsg+"\n";

            rows_used += rows_needed + 1;
        }
    }
    static std::string newline("\n");
    std::string textMessage = parentcp->textMessage;
    if (parentcp->editingTextMessage) {
        textMessage = "Chat> " + textMessage;
        if (floor(nowtime / 2) != floor(nowtime) / 2.0) {
            textMessage += "]";
        }
    }
    /*
     *  if (rows_used>=row_lim&&parentcp->editingTextMessage) {
     *  size_t where=fullstr.find(newline);
     *  if (where!=string::npos) {
     *   if (where>1.6*cols) {
     *     where=(size_t)(1.6*cols+1);
     *   }
     *   fullstr=fullstr.substr(where+1);
     *  }
     *  }
     */
    if (parentcp->editingTextMessage) {
        fullstr += textMessage;
        fullstr += newline;
    }
    const std::string message_prefix = configuration()->graphics_config.hud.message_prefix;
    fullstr = targetstr + fullstr;
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(message_prefix + MangleString(fullstr,
                    _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0),
            0,
            true,
            false,
            automatte);
    tp->bgcol = tpbg;
}

void VDU::DrawScanningMessage() {
    //tp->Draw(MangleString ("Scanning target...",_Universe->AccessCamera()->GetNebula()!=NULL?.4:0),0,true);
}

bool VDU::SetCommAnimation(Animation *ani, Unit *un, bool force) {
    if (comm_ani == NULL || force) {
        if (posmodes & COMM) {
            if (ani != NULL && comm_ani == NULL) {
                thismode.push_back(COMM);
            } else if (comm_ani != NULL && thismode.size() > 1 && ani != NULL) {
                thismode.back() = COMM;
            }
            if (ani) {
                comm_ani = ani;
                ani->Reset();
            }
            communicating.SetUnit(un);
            return true;
        }
    }
    return false;
}

Unit *VDU::GetCommunicating() {
    if (comm_ani) {
        if (!comm_ani->Done()) {
            return communicating.GetUnit();
        }
    }
    return NULL;
}

void VDU::DrawNav(GameCockpit *cp, Unit *you, Unit *targ, const Vector &nav) {
    //Unit * you = _Universe->AccessCockpit()->GetParent();
    //Unit * targ = you!=NULL?you->Target():NULL;
    //static float game_speed = XMLSupport::parse_float( vs_config->getVariable( "physics", "game_speed", "1" ) );
    //static bool  lie = XMLSupport::parse_bool( vs_config->getVariable( "physics", "game_speed_lying", "true" ) );
    string nam = "none";
    if (targ) {
        nam = reformatName(targ->name);
    }
    int faction =
            FactionUtil::GetFactionIndex(UniverseUtil::GetGalaxyFaction(_Universe->activeStarSystem()->getFileName()));
    std::string navdata =
            std::string("#ff0000Sector:\n     #ffff00" + getStarSystemSector(
                    _Universe->activeStarSystem()->getFileName())
                    + "\n\n#ff0000System:\n     #ffff00") + _Universe->activeStarSystem()->getName() + " ("
                    + FactionUtil::GetFactionName(faction)
                    + ")\n\n#ff0000Target:\n  #ffff00" + (targ ? getUnitNameAndFgNoBase(targ) : std::string("Nothing"))
                    + "\n\n#ff0000Range: #ffff00"
                    + PrettyDistanceString(((you && targ) ? DistanceTwoTargets(you, targ) : 0.0));
    static float auto_message_lim =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "auto_message_time_lim", "5"));
    float delautotime = UniverseUtil::GetGameTime() - cp->autoMessageTime;
    bool draw_auto_message = (delautotime < auto_message_lim && cp->autoMessage.length() != 0);
    std::string msg = cp->autoMessage;
    std::string::size_type where = msg.find("#");
    while (where != std::string::npos) {
        msg = msg.substr(0, where) + msg.substr(where + 7);
        where = msg.find("#");
    }
    msg = std::string("\n\n#ffff00     ") + msg;
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(MangleString(navdata + (draw_auto_message ? msg : std::string()), _Universe->AccessCamera()->GetNebula()
                    != NULL ? .4 : 0),
            scrolloffset,
            true,
            true,
            automatte);
    tp->bgcol = tpbg;
}

void VDU::DrawComm() {
    if (comm_ani != NULL) {
        GFXDisable(TEXTURE1);
        GFXEnable(TEXTURE0);
        GFXDisable(LIGHTING);

        comm_ani->DrawAsVSSprite(this);
        if (comm_ani->Done()) {
            if (thismode.size() > 1) {
                if (configuration()->graphics_config.hud.switch_back_from_comms) {
                    thismode.pop_back();
                } else {
                    unsigned int blah = thismode.back();
                    thismode.pop_back();
                    thismode.back() = blah;
                }
            }
            communicating.SetUnit(NULL);
            comm_ani = NULL;
        }
        GFXDisable(TEXTURE0);
    } else {
        const string message_prefix = configuration()->graphics_config.hud.message_prefix;
        const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
        GFXColor tpbg = tp->bgcol;
        bool automatte = (0 == tpbg.a);
        if (automatte) {
            tp->bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        tp->Draw(message_prefix
                + MangleString(_Universe->AccessCockpit()->communication_choices.c_str(),
                        _Universe->AccessCamera()->GetNebula()
                                != NULL ? .4 : 0), scrolloffset, true, false, automatte);
        tp->bgcol = tpbg;
    }
}

void VDU::DrawManifest(Unit *parent, Unit *target) {
    //zadeVDUmanifest
    const std::string manifest_heading = configuration()->graphics_config.hud.manifest_heading;
    const bool simple_manifest = configuration()->graphics_config.hud.simple_manifest;
    std::string retval(manifest_heading);
    if (target != parent && simple_manifest == false) {
        retval += string("Tgt: ") + reformatName(target->name) + string("\n");
    } else {
        retval += string("--------\nCredits: ") + tostring((int) _Universe->AccessCockpit()->credits) + string("\n");
    }
    unsigned int load = 0;
    unsigned int cred = 0;
    unsigned int vol = 0;
    unsigned int numCargo = target->numCargo();
    unsigned int maxCargo = 16;
    string lastCat;
    for (unsigned int i = 0; i < numCargo; i++) {
        if ((target->GetCargo(i).GetCategory().find("upgrades/") != 0)
                && (target->GetCargo(i).GetQuantity() > 0)) {
            Cargo ca = target->GetCargo(i);
            int cq = ca.GetQuantity();
            float cm = ca.GetMass();
            float cv = ca.GetVolume();
            float cp = ca.GetPrice();
            string cc = ca.GetCategory();
            cred += cq * (int) cp;
            vol += (int) ((float) cq * cv);
            load += (int) ((float) cq * cm);
            if (((target == parent) || (maxCargo + i >= numCargo) || lastCat.compare(cc)) && (maxCargo > 0)) {
                maxCargo--;
                lastCat = cc;
                if (target == parent && !simple_manifest) {
                    //retval += string("(") + tostring(cq)+string(") "); // show quantity
                    if (cm >= cv) {
                        retval += tostring((int) ((float) cq * cm)) + string("t ");
                    } else {
                        retval += tostring((int) ((float) cq * cv)) + string("m3 ");
                    }
                } else {
                    retval += tostring((int) cq) + " ";
                }
                retval += target->GetManifest(i, parent, parent->GetVelocity());
                if (!simple_manifest) {
                    retval += string(" ") + tostring(((target == parent) ? cq : 1) * (int) cp)
                            + string("Cr.");
                }
                retval += "\n";
            }
        }
    }
    if (target == parent && !simple_manifest) {
        retval += string("--------\nLoad: ") + tostring(load) + string("t ")
                + tostring(vol) + string("m3 ") + tostring(cred) + string("Cr.\n");
    }
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(MangleString(retval,
                    _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0),
            scrolloffset,
            true,
            false,
            automatte);
    tp->bgcol = tpbg;
}

static void DrawGun(Vector pos, float w, float h, MOUNT_SIZE sz) {
    w = fabs(w);
    h = fabs(h);
    float oox = 1. / g_game.x_resolution;
    float ooy = 1. / g_game.y_resolution;
    pos.j -= h / 3.8;
    if (sz == MOUNT_SIZE::NOWEAP) {
        GFXPointSize(4);
        const float verts[3] = {
                pos.x, pos.y, pos.z
        };
        GFXDraw(GFXPOINT, verts, 1);
        GFXPointSize(1);
    } else if (sz < MOUNT_SIZE::SPECIAL) {
        if (sz == MOUNT_SIZE::LIGHT) {
            const float verts[10 * 3] = {
                    pos.i + oox, pos.j, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i, pos.j, 0,
                    pos.i, pos.j + h / 4, 0,
                    pos.i, pos.j + h / 4 + ooy * 2, 0,
                    pos.i, pos.j + h / 4 + ooy * 5, 0,
            };
            GFXDraw(GFXLINE, verts, 10);
        } else if (sz == MOUNT_SIZE::MEDIUM) {
            const float verts[12 * 3] = {
                    pos.i + oox, pos.j, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i, pos.j, 0,
                    pos.i, pos.j + h / 5, 0,
                    pos.i, pos.j + h / 5 + ooy * 4, 0,
                    pos.i, pos.j + h / 5 + ooy * 5, 0,
                    pos.i + oox, pos.j + h / 5 + ooy * 2, 0,
                    pos.i - oox, pos.j + h / 5 + ooy * 2, 0,
            };
            GFXDraw(GFXLINE, verts, 12);
        } else if (sz == MOUNT_SIZE::HEAVY) {
            const float verts[14 * 3] = {
                    pos.i + oox, pos.j, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i, pos.j, 0,
                    pos.i, pos.j + h / 5, 0,
                    pos.i, pos.j + h / 5 + ooy * 4, 0,
                    pos.i, pos.j + h / 5 + ooy * 5, 0,
                    pos.i + 2 * oox, pos.j + h / 5 + ooy * 3, 0,
                    pos.i, pos.j + h / 5 + ooy * 2, 0,
                    pos.i - 2 * oox, pos.j + h / 5 + ooy * 3, 0,
                    pos.i, pos.j + h / 5 + ooy * 2, 0,
            };
            GFXDraw(GFXLINE, verts, 14);
        } else { //capship gun
            const float verts[14 * 3] = {
                    pos.i + oox, pos.j, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i + oox, pos.j - h / 15, 0,
                    pos.i - oox, pos.j - h / 15, 0,
                    pos.i, pos.j, 0,
                    pos.i, pos.j + h / 6, 0,
                    pos.i, pos.j + h / 6 + ooy * 6, 0,
                    pos.i, pos.j + h / 6 + ooy * 7, 0,
                    pos.i - oox, pos.j + h / 6 + ooy * 2, 0,
                    pos.i + oox, pos.j + h / 6 + ooy * 2, 0,
                    pos.i - 2 * oox, pos.j + h / 6 + ooy * 4, 0,
                    pos.i + 2 * oox, pos.j + h / 6 + ooy * 4, 0,
            };
            GFXDraw(GFXLINE, verts, 14);
        }
    } else if (sz == MOUNT_SIZE::SPECIAL || sz == MOUNT_SIZE::SPECIALMISSILE) {
        GFXPointSize(4);
        const float verts[3] = {
                pos.x, pos.y, pos.z
        };
        GFXDraw(GFXPOINT, verts, 1);
        GFXPointSize(1);         //classified...  FIXME
    } else if (sz < MOUNT_SIZE::HEAVYMISSILE) {
        const float verts[4 * 3] = {
                pos.i, pos.j - h / 8, 0,
                pos.i, pos.j + h / 8, 0,
                pos.i + 2 * oox, pos.j - h / 8 + 2 * ooy, 0,
                pos.i - 2 * oox, pos.j - h / 8 + 2 * ooy, 0,
        };
        GFXDraw(GFXLINE, verts, 4);
    } else if (sz <= MOUNT_SIZE::CAPSHIPHEAVYMISSILE) {
        const float verts[8 * 3] = {
                pos.i, pos.j - h / 6, 0,
                pos.i, pos.j + h / 6, 0,
                pos.i + 3 * oox, pos.j - h / 6 + 2 * ooy, 0,
                pos.i - 3 * oox, pos.j - h / 6 + 2 * ooy, 0,
                pos.i + oox, pos.j - h / 6, 0,
                pos.i + oox, pos.j + h / 9, 0,
                pos.i - oox, pos.j - h / 6, 0,
                pos.i - oox, pos.j + h / 9, 0,
        };
        GFXDraw(GFXLINE, verts, 8);
    }
}

extern const char *DamagedCategory;

void VDU::DrawDamage(Unit *parent) {
    //VDUdamage
    float x, y, w, h;
    //float th;
    //char st[1024];
    double hull_percent = parent->hull.Percent();
    GFXColor4f(1, hull_percent, hull_percent, 1);
    GFXEnable(TEXTURE0);
    const bool draw_damage_sprite = configuration()->graphics_config.hud.draw_damage_sprite;
    DrawHUDSprite(this, draw_damage_sprite ? parent->getHudImage() : nullptr, .6, x, y, w, h,
            parent->armor.Percent(Armor::front),
            parent->armor.Percent(Armor::right),
            parent->armor.Percent(Armor::left),
            parent->armor.Percent(Armor::back),
            hull_percent, true, false);
    GFXDisable(TEXTURE0);
    //Unit *thr = parent->Threat();
    parent->Threat();
    std::string fullname(getUnitNameAndFgNoBase(parent));
    //sprintf (st,"%s\nHull: %.3f",blah.c_str(),parent->GetHull());
    //tp->Draw (MangleString (st,_Universe->AccessCamera()->GetNebula()!=NULL?.5:0),0,true);
    char ecmstatus[256];
    ecmstatus[0] = '\0';
    static bool print_ecm = XMLSupport::parse_bool(vs_config->getVariable("graphics", "print_ecm_status", "true"));
    if (print_ecm) {
        if (UnitUtil::getECM(parent) > 0) {
            GFXColor4f(0, 1, 0, .5);
            strcpy(ecmstatus, "ECM Active");
            static float s = 0;
            s += .125 * SIMULATION_ATOM;
            if (s > 1) {
                s = 0;
            }
            DrawShield(0, s, s, 0, x, y, w, h, false, GFXColor(0, 1, 0), GFXColor(0, .75, 0), GFXColor(0, .5, 0));
        }
    }
    GFXColor4f(1, 1, 1, 1);

/*
 *
 *  Cargo & GetCargo (unsigned int i);
 *  void GetCargoCat (const std::string &category, vector <Cargo> &cat);
 *  ///below function returns NULL if not found
 *  Cargo * GetCargo (const std::string &s, unsigned int &i);
 *
 */

    //*******************************************************zade

    //char hullval[128];
    //sprintf (hullval,"%.3f",parent->GetHull());
    //string retval (fullname+"\nHull: "+hullval+"\n");
    static GFXColor cfullpower = vs_config->getColor("default", "hud_repair_repaired",
            GFXColor(1, 1, 1, 1));
    static GFXColor chdamaged = vs_config->getColor("default", "hud_repair_half_damaged",
            GFXColor(1, 1, 0, 1));
    static GFXColor cdamaged = vs_config->getColor("default", "hud_repair_damaged",
            GFXColor(1, 0, 0, 1));
    static GFXColor cdestroyed = vs_config->getColor("default", "hud_repair_destroyed",
            GFXColor(.2, .2, .2, 1));

    RGBstring fpstring = colToString(cfullpower);
    const std::string damage_report_heading = configuration()->graphics_config.hud.damage_report_heading;
    std::string retval(damage_report_heading);
    retval += fpstring.str;
    unsigned int numCargo = parent->numCargo();
    double percent_working = 0.88;
    const std::string non_repair_screen_cargo = configuration()->graphics_config.hud.not_included_in_damage_report;
    const bool print_percent_working = configuration()->graphics_config.hud.print_damage_percent;

// TODO: make into function
#define REPORTITEM(percent_working, max_functionality, print_percent_working, component_string) \
    do { \
        GFXColor final_color = colLerp( cdamaged, chdamaged, percent_working ); \
        if ((percent_working) == 0.0) \
            final_color = cdestroyed; /*dead = grey*/ \
        std::string trailer; \
        if ((percent_working) < (max_functionality)) \
            retval += colToString( final_color ).str; \
        else \
            retval += fpstring.str; \
        trailer = fpstring.str; \
        retval += (component_string); \
        if (print_percent_working) \
            retval += string( " (" )+tostring( int((percent_working)*100) )+string( "%)" ); \
        retval += trailer+std::string( "\n" ); \
    } while(0)

// TODO: make into function
#define REPORTINTEGRATED(which, which_key, which_name_default) \
    do { \
        const string name = vs_config->getVariable( "graphics", "hud", which_key, which_name_default ); \
        if (!name.empty()) { \
            REPORTITEM( parent->which##Functionality, parent->which##FunctionalityMax, \
                print_percent_working, \
                name \
            ); \
        } \
    } while(0)

// TODO: make into function
#define REPORTINTEGRATEDFLAG(which, which_key, which_name_default) \
    do { \
        const string name = vs_config->getVariable( "graphics", "hud", which_key, which_name_default ); \
        if (!name.empty()) { \
            REPORTITEM( ((parent->damages & (which)) ? 0.1 : 1.0), 1.0, \
                false, \
                name \
            ); \
        } \
    } while(0)

    for (unsigned int i = 0; i < numCargo; i++) {
        percent_working = 0.88;         //cargo.damage
        Cargo &the_cargo = parent->GetCargo(i);
        bool damaged = the_cargo.GetCategory().find(DamagedCategory) == 0;
        if (damaged
                || (the_cargo.GetCategory().find("upgrades/") == 0
                        && the_cargo.GetInstalled()
                        && the_cargo.GetName().find("mult_") != 0
                        && the_cargo.GetName().find("add_") != 0
                        && non_repair_screen_cargo.find(the_cargo.GetName())
                                == std::string::npos)) {
            percent_working = UnitUtil::PercentOperational(parent, the_cargo.GetName(), the_cargo.GetCategory(), false);
            //retval+=parent->GetManifest (i,parent,parent->GetVelocity())+string (" (")+tostring (int(percent_working*100))+string ("%)" +the_cargo.category+"\n");
            REPORTITEM(percent_working,
                    1.0,
                    print_percent_working,
                    parent->GetManifest(i, parent, parent->GetVelocity()));
        }
    }
    if (parent->pImage != nullptr) {
        // Integrated systems with percent working values
        REPORTINTEGRATED(LifeSupport, "damage.names.life_support", "Life Support");
        REPORTINTEGRATED(fireControl, "damage.names.fire_control", "Fire Control");
        REPORTINTEGRATED(SPECDrive, "damage.names.spec_drive", "SPEC Drive");
        REPORTINTEGRATED(Comm, "damage.names.comm", "Comm");

        // Integrated system with boolean damage flags
        REPORTINTEGRATEDFLAG(Unit::LIMITS_DAMAGED, "damage.names.limits_name", "Thrusters");
        REPORTINTEGRATEDFLAG(Unit::SHIELD_DAMAGED, "damage.names.shield_name", ""); // default invisible, is an upgrade
        REPORTINTEGRATEDFLAG(Unit::COMPUTER_DAMAGED, "damage.names.computer_name", "Targetting Computer");
        REPORTINTEGRATEDFLAG(Unit::JUMP_DAMAGED, "damage.names.jump_name", ""); // default invisible, is an upgrade
        REPORTINTEGRATEDFLAG(Unit::CLOAK_DAMAGED, "damage.names.cloak_name", ""); // default invisible, is an upgrade
    }

    retval += ecmstatus;
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(MangleString(retval,
                    _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0),
            scrolloffset,
            true,
            false,
            automatte);
    tp->bgcol = tpbg;
    //*******************************************************
}

void VDU::SetViewingStyle(VIEWSTYLE vs) {
    viewStyle = vs;
}

void VDU::DrawStarSystemAgain(float x, float y, float w, float h, VIEWSTYLE viewStyle, Unit *parent, Unit *target) {
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    VIEWSTYLE which = viewStyle;
    float tmpaspect = g_game.aspect;
    g_game.aspect = w / h;
    _Universe->AccessCamera(which)->SetSubwindow(x, y, w, h);
    _Universe->SelectCamera(which);
    VIEWSTYLE tmp = _Universe->AccessCockpit()->GetView();
    _Universe->AccessCockpit()->SetView(viewStyle);
    _Universe->AccessCockpit()->SelectProperCamera();
    _Universe->AccessCockpit()->SetupViewPort(true);     ///this is the final, smoothly calculated cam
    GFXClear(GFXFALSE);
    GFXColor4f(1, 1, 1, 1);
    _Universe->activeStarSystem()->Draw(false);
    g_game.aspect = tmpaspect;
    _Universe->AccessCamera(which)->SetSubwindow(0, 0, 1, 1);
    _Universe->AccessCockpit()->SetView(tmp);
    _Universe->AccessCockpit()->SelectProperCamera();
    _Universe->AccessCockpit()->SetupViewPort(true);     ///this is the final, smoothly calculated cam
    GFXRestoreHudMode();
    GFXBlendMode(ONE, ZERO);
    GFXDisable(TEXTURE1);
    GFXDisable(TEXTURE0);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    char buf[1024];
    bool inrange = false;
    if (target) {
        double mm = 0;
        std::string blah(getUnitNameAndFgNoBase(target));
        sprintf(buf, "%s\n", blah.c_str());
        const bool out_of_cone_information = configuration()->graphics_config.hud.out_of_cone_distance;
        inrange =
                parent->InRange(target, mm, out_of_cone_information || !UnitUtil::isSignificant(target), false, false);
    }
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(MangleString(buf, _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0), 0, true, false, automatte);
    tp->bgcol = tpbg;
    if (inrange) {
        int i = 0;
        char st[1024];
        for (i = 0; i < rows - 1 && i < 128; i++) {
            st[i] = '\n';
        }
        st[i] = '\0';
        std::string qr = PrettyDistanceString(DistanceTwoTargets(parent, target));
        strcat(st, "Range: ");
        strcat(st, qr.c_str());
//        GFXColor tpbg = tp->bgcol;
//        bool automatte = (0 == tpbg.a);
        if (automatte) {
            tp->bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        tp->Draw(MangleString(st, _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0), 0, true, false, automatte);
        tp->bgcol = tpbg;
        GFXColor4f(.4, .4, 1, 1);
        GetPosition(x, y);
        GetSize(w, h);
        if (target && configuration()->graphics_config.hud.draw_vdu_view_shields) {
            if (viewStyle == CP_PANTARGET) {
                DrawHUDSprite(this, getSunImage(), 1, x, y, w, h, 1, 1, 1, 1, 1, false, false);
                h = fabs(h * .6);
                w = fabs(w * .6);
            }
        }
        GFXColor4f(1, 1, 1, 1);
    } else if (target) {
//        GFXColor tpbg = tp->bgcol;
//        bool automatte = (0 == tpbg.a);
        if (automatte) {
            tp->bgcol = GFXColor(0, 0, 0, background_alpha);
        }
        tp->Draw(MangleString("\n[OutOfRange]",
                _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0), 0, true, false, automatte);
        tp->bgcol = tpbg;
    }
    //_Universe->AccessCockpit()->RestoreViewPort();
}

GFXColor MountColor(Mount *mnt) {
    static GFXColor col_mount_default = vs_config->getColor("mount_default", GFXColor(0.0, 1.0, 0.2, 1.0));
    static GFXColor col_mount_not_ready = vs_config->getColor("mount_not_ready", GFXColor(0.0, 1.0, 0.4, 1.0));
    static GFXColor col_mount_out_of_ammo = vs_config->getColor("mount_out_of_ammo", GFXColor(0.2, 0.2, 0.4, 1.0));
    static GFXColor col_mount_inactive = vs_config->getColor("mount_inactive", GFXColor(0.7, 0.7, 0.7, 1.0));
    static GFXColor col_mount_destroyed = vs_config->getColor("mount_destroyed", GFXColor(1.0, 0.0, 0.0, 1.0));
    static GFXColor col_mount_unchosen = vs_config->getColor("mount_unchosen", GFXColor(0.3, 0.3, 0.3, 1.0));

    GFXColor mountcolor = col_mount_default;
    switch (mnt->ammo != 0 ? mnt->status : 127) {
        case Mount::ACTIVE: {
            if (mnt->functionality == 1) {
                float tref = mnt->type->Refire();
                float cref = 0;
                if ((mnt->type->type == WEAPON_TYPE::BEAM) && mnt->ref.gun) {
                    cref = mnt->ref.gun->refireTime();
                } else {
                    cref = mnt->ref.refire;
                }
                if (cref < tref) {
                    mountcolor = colLerp(col_mount_out_of_ammo, col_mount_not_ready, cref / tref);
                }
            } else      // damaged
            {
                mountcolor = colLerp(col_mount_destroyed, col_mount_default, mnt->functionality);
            }
            break;
        }
        case Mount::DESTROYED:
            mountcolor = col_mount_destroyed;
            break;
        case Mount::INACTIVE:
            mountcolor = col_mount_inactive;
            break;
        case Mount::UNCHOSEN:
            mountcolor = col_mount_unchosen;
            break;
        case 127:
            mountcolor = col_mount_out_of_ammo;
            break;
        default:
// already set default color before switch; other than that, "default" should not happen with the current code, ever, so maybe we have to do something when it does
            break;
    }
    return mountcolor;
}

void VDU::DrawWeapon(Unit *parent) {
    const bool draw_weapon_sprite = configuration()->graphics_config.hud.draw_weapon_sprite;
    const std::string list_empty_mounts_as = configuration()->graphics_config.hud.mounts_list_empty;
    const bool do_list_empty_mounts = (list_empty_mounts_as.length() != 0);

//  without fixed font we would need some sneaky tweaking to make it a table, probably with multiple TPs
//    static int weaponcolumns =
//        XMLSupport::parse_int( vs_config->getVariable( "graphics", "hud", "gun_list_columns", "1" ) );
//    int    count  = 0;
//    int    mcount = 0;
    float x, y, w, h;
    const float percent = .6;
    string buf("#00ff00WEAPONS\n\n#ffffffGuns:#000000");
    string mbuf("\n#ffffffMissiles:#000000");
    string::size_type mlen = mbuf.length();
    GFXEnable(TEXTURE0);
    DrawTargetSpr(draw_weapon_sprite ? parent->getHudImage() : NULL, percent, x, y, w, h);
    GFXDisable(TEXTURE0);
    GFXDisable(LIGHTING);
    int nummounts = parent->getNumMounts();
    int numave = 1;
    GFXColor average(0, 0, 0, 0);
    for (int i = 0; i < nummounts; i++) {
        GFXColor mntcolor = MountColor(&parent->mounts[i]);
        if (draw_weapon_sprite) {
            Vector pos(parent->mounts[i].GetMountLocation());
            pos.i = -pos.i * fabs(w) / parent->rSize() * percent + x;
            pos.j = pos.k * fabs(h) / parent->rSize() * percent + y;
            pos.k = 0;
            GFXColorf(mntcolor);
            DrawGun(pos, w, h, parent->mounts[i].type->size);
        }
        average.r += mntcolor.r;
        average.g += mntcolor.g;
        average.b += mntcolor.b;
        average.a += mntcolor.a;
        if (i + 1 < nummounts && parent->mounts[i].bank) {
            // add up numave and average, waiting for the next non-bank mount
            if (parent->mounts[i].status != Mount::UNCHOSEN) { //skip empty mounts for banks that aren't full
                numave++;
            }
        } else {
            if ((parent->mounts[i].status != Mount::UNCHOSEN) || do_list_empty_mounts) {
                GFXColor mountcolor(average.r / numave, average.g / numave, average.b / numave, average.a / numave);
                string baseweaponreport = colToString(mountcolor).str;
                if (parent->mounts[i].status == Mount::UNCHOSEN) {
                    baseweaponreport += list_empty_mounts_as;
                } else {
                    baseweaponreport += parent->mounts[i].type->name;
                }
                if (numave != 1) {    //  show banks, if any, here; "#" seems to be a reserved char, see colToString
                    baseweaponreport += string(" x") + tostring(numave);
                }
                if (parent->mounts[i].ammo >= 0) {
                    baseweaponreport += string(" (") + tostring(parent->mounts[i].ammo) + string(")");
                }
                if (parent->mounts[i].type->isMissile()) {
                    mbuf += "\n" + baseweaponreport;
                    // here we should also add to different columns - when we know how to make these work, that is
                } else {
                    buf += "\n" + baseweaponreport;
                }
            }
            numave = 1;
            average = GFXColor(0, 0, 0, 0);
        }
    }
    if (mbuf.length() != mlen) {
        buf += mbuf;
    }
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(buf, 0, true, false, automatte);
    tp->bgcol = tpbg;
}

using std::vector;

/*
 *  static GFXColor GetColorFromSuccess (float suc){
 *  suc +=1.;
 *  suc/=2.;
 *  return GFXColor(1-suc,suc,0);
 *  }
 */

char printHex(unsigned int hex) {
    if (hex < 10) {
        return hex + '0';
    }
    return hex - 10 + 'A';
}

static char suc_col_str[8] = "#000000";
static const char suc_gt_plusone[8] = "#00FF00";
static const char suc_gt_minusone[8] = "#FF0000";

inline const char *GetColorFromSuccess(float suc) {
    if (suc >= 1) {
        return suc_gt_plusone;
    }
    if (suc <= -1) {
        return suc_gt_minusone;
    }
    suc += 1.;
    suc *= 128;
    unsigned int tmp2 = (unsigned int) suc;
    unsigned int tmp1 = (unsigned int) (255 - suc);
    suc_col_str[0] = '#';
    suc_col_str[1] = printHex(tmp1 / 16);
    suc_col_str[2] = printHex(tmp1 % 16);
    suc_col_str[3] = printHex(tmp2 / 16);
    suc_col_str[4] = printHex(tmp2 % 16);
    suc_col_str[5] = '0';
    suc_col_str[6] = '0';

    return suc_col_str;
}

#if 0
                                                                                                                        int VDU::DrawVDUObjective( void *obj, int offset )
{
    static bool VDU_DrawVDUObjective_is_now_outdated = false;
    assert( VDU_DrawVDUObjective_is_now_outdated == true );
    return 0;
}
#endif

void DrawObjectivesTextPlane(TextPlane *tp, int scrolloffset, Unit *parent) {
    std::string rez("\n");
    std::string rezcompleted("");
    for (unsigned int i = 0; i < active_missions.size(); ++i) {
        if (!active_missions[i]->objectives.empty()) {
            rez += "#FFFFFF";
            const bool force_anonymous_missions = configuration()->general_config.force_anonymous_mission_names;
            const bool completed_objectives_last = configuration()->graphics_config.hud.completed_objectives_last;
            if (active_missions[i]->mission_name.empty() || force_anonymous_missions) {
                rez += "Mission " + XMLSupport::tostring((int) i) + "\n";
            } else {
                rez += active_missions[i]->mission_name + "\n";
            }
            vector<Mission::Objective>::iterator j = active_missions[i]->objectives.begin();
            for (; j != active_missions[i]->objectives.end(); ++j) {
                if (j->getOwner() == NULL || j->getOwner() == parent) {
                    if (j->objective.length()) {
                        std::string tmp("");
                        tmp += GetColorFromSuccess(j->completeness);
                        tmp += j->objective;
                        tmp += '\n';
                        if (j->completeness && completed_objectives_last) {
                            rezcompleted += tmp;
                        } else {
                            rez += tmp;
                        }
                    }
                }
            }
            // Put completed mission objectives at the end as they are less interesting
            rez += rezcompleted;
            rez += '\n';
        }
    }
    const float background_alpha = configuration()->graphics_config.hud.text_background_alpha;
    GFXColor tpbg = tp->bgcol;
    bool automatte = (0 == tpbg.a);
    if (automatte) {
        tp->bgcol = GFXColor(0, 0, 0, background_alpha);
    }
    tp->Draw(rez, scrolloffset, false, false, automatte);
    tp->bgcol = tpbg;
}

void VDU::DrawVDUObjectives(Unit *parent) {
    DrawObjectivesTextPlane(tp, scrolloffset, parent);
}

bool VDU::SetWebcamAnimation() {
    if (comm_ani == NULL) {
        if (posmodes & WEBCAM) {
            comm_ani = new Animation();
            communicating.SetUnit(NULL);
            thismode.push_back(WEBCAM);
            comm_ani->Reset();
            return true;
        }
    }
    return false;
}

void VDU::DrawWebcam(Unit *parent) {
    using VSFileSystem::JPEGBuffer;
    tp->Draw(MangleString("No webcam to view",
            _Universe->AccessCamera()->GetNebula() != NULL ? .4 : 0), scrolloffset, true);

}

void VDU::Draw(GameCockpit *parentcp, Unit *parent, const GFXColor &color) {
    tp->col = color;
    GFXDisable(LIGHTING);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    VSSprite::Draw();
    //glDisable( GL_ALPHA_TEST);
    if (!parent) {
        return;
    }
    //configure text plane;
    float x, y;
    float h, w;
    GetSize(w, h);

    static float width_factor = XMLSupport::parse_float(vs_config->getVariable("graphics", "reduced_vdus_width", "0"));
    static float
            height_factor = XMLSupport::parse_float(vs_config->getVariable("graphics", "reduced_vdus_height", "0"));
    w = w - width_factor;
    h = h + height_factor;

    GetPosition(x, y);
    //tp->SetCharSize (fabs(w/cols),fabs(h/rows));
    float csx, csy;
    tp->GetCharSize(csx, csy);
    //This was as below:
    //cols = abs( (int) ceil( w/csx ) );
    //rows = abs( (int) ceil( h/csy ) );
    //I'm changing it to as below, which avoids abs blues with visual studio, and is
    //also faster, as computing the abs of a float amounts to setting the sign bit;
    //though I'm less than 100% entirely sure of the correctness of the change... --chuck_starchaser
    cols = int(fabs(ceil(w / csx)));
    rows = int(fabs(ceil(h / csy)));

    Unit *targ;
    h = fabs(h / 2);
    w = fabs(w / 2);
    tp->SetPos(x - w, y + h);
    tp->SetSize(x + w, y - h - .5 * fabs(w / cols));
    targ = parent->GetComputerData().target.GetUnit();
    if (thismode.back() != COMM && comm_ani != NULL) {
        if (comm_ani->Done()) {
            comm_ani = NULL;
            communicating.SetUnit(NULL);
        }
    }
    float delautotime = UniverseUtil::GetGameTime() - parentcp->autoMessageTime;
    static float auto_switch_lim =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "auto_message_nav_switch_time_lim", ".15"));
    if ((delautotime < auto_switch_lim) && (parentcp->autoMessage.length() != 0)) {
        if ((thismode.back() != COMM) && ((posmodes & NAV) != 0)) {
            thismode.back() = NAV;
            parentcp->autoMessageTime -= auto_switch_lim * 1.125;
        }
    }
    switch (thismode.back()) {
        case NETWORK:
            break;
        case WEBCAM:
            break;
        case SCANNING:
            if (!got_target_info) {
                DrawScanningMessage();
            }
            /*
         *  else
         *       DrawScanner();
         */
            break;
        case TARGET:
            if (targ) {
                DrawTarget(parentcp, parent, targ);
            }
            break;
        case MANIFEST:
            DrawManifest(parent, parent);
            break;
        case TARGETMANIFEST:
            if (targ) {
                DrawManifest(parent, targ);
            }
            break;
        case VIEW:
            GetPosition(x, y);
            GetSize(w, h);
            DrawStarSystemAgain(.5 * (x - fabs(w / 2) + 1), .5 * ((y - fabs(h / 2)) + 1), fabs(w / 2), fabs(
                    h / 2), viewStyle, parent, targ);
            break;
        case NAV:
            DrawNav(parentcp,
                    parent,
                    targ,
                    parent->ToLocalCoordinates(parent->GetComputerData().NavPoint - parent->Position().Cast()));
            break;
        case MSG:
            DrawMessages(parentcp, targ);
            break;
        case COMM:
            DrawComm();
            break;
        case DAMAGE:
            DrawDamage(parent);
            break;
        case WEAPON:
            DrawWeapon(parent);
            break;
        case SHIELD:
            DrawVDUShield(parent);
            break;
        case OBJECTIVES:
            DrawVDUObjectives(parent);
            break;
        default:
            break; //FIXME --chuck_starchaser; please verify correctness and/or add a errlog or throw
    }
}

void UpdateViewstyle(VIEWSTYLE &vs) {
    switch (vs) {
        case CP_FRONT:
            vs = CP_TARGET;
            break;
        case CP_BACK:
            vs = CP_FRONT;
            break;
        case CP_LEFT:
            vs = CP_BACK;
            break;
        case CP_RIGHT:
            vs = CP_LEFT;
            break;
        case CP_CHASE:
            vs = CP_TARGET;
            break;
        case CP_PAN:
            vs = CP_CHASE;
            break;
        case CP_PANTARGET:
            vs = CP_CHASE;
            break;
        case CP_TARGET:
            vs = CP_PANTARGET;
            break;
        case CP_VIEWTARGET: //FIXME cases not previously handled in switch --added by chuck_starchaser; please verify correctness
        case CP_PANINSIDE:
        case CP_FIXED:
        case CP_FIXEDPOS:
        case CP_FIXEDPOSTARGET:
        case CP_NUMVIEWS:
        default:
            break;
    }
}

void VDU::SwitchMode(Unit *parent) {
    if (!posmodes) {
        return;
    }
    scrolloffset = 0;

    if (thismode.back() == VIEW && viewStyle != CP_CHASE && (thismode.back() & posmodes)) {
        UpdateViewstyle(viewStyle);
    } else {
        viewStyle = CP_TARGET;
        thismode.back() <<= 1;
        while (!(thismode.back() & posmodes)) {
            if (thismode.back() > posmodes) {
                thismode.back() = 0x1;
            } else {
                thismode.back() <<= 1;
            }
        }
    }
}

bool VDU::CheckCommAnimation(Unit *un) const {
    if (comm_ani && comm_ani->Done() == false) {
        if (communicating == un || communicating == NULL) {
            return true;
        }
    }
    return false;
}

