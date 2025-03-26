/*
 * base_interface.cpp
 *
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#define PY_SSIZE_T_CLEAN
#include <boost/python.hpp>
#include <Python.h>
#include <algorithm>
#include "cmd/vega_py_run.h"
#include "cmd/base.h"
#include "gldrv/winsys.h"
#include "root_generic/vsfilesystem.h"
#include "src/vs_logging.h"
#include "root_generic/lin_time.h"
#include "src/audiolib.h"
#include "gfx/camera.h"
#include "gfx_generic/cockpit_generic.h"
#include "src/python/init.h"
#include "src/python/python_compile.h"
#include "cmd/planet.h"
#include "cmd/base_util.h"
#include "src/config_xml.h"
#include "src/save_util.h"
#include "cmd/unit_util.h"
#include "gfx/cockpit.h"
#include "gfx/ani_texture.h"
#include "cmd/music.h"
#include "root_generic/lin_time.h"
#include "root_generic/load_mission.h"
#include "src/universe_util.h"
#include "gui/guidefs.h"
#ifdef RENDER_FROM_TEXTURE
#include "gfx/stream_texture.h"
#endif
#include "src/main_loop.h"
#include "src/in_mouse.h"
#include "src/in_kb.h"
#include "src/universe.h"

#include "cmd/ai/communication.h"
#include "audio/SceneManager.h"

static unsigned int &getMouseButtonMask() {
    static unsigned int mask = 0;
    return mask;
}

static void biModifyMouseSensitivity(int &x, int &y, bool invert) {
    int xrez = g_game.x_resolution;
    static int
            whentodouble = XMLSupport::parse_int(vs_config->getVariable("joystick", "double_mouse_position", "1280"));
    static float factor = XMLSupport::parse_float(vs_config->getVariable("joystick", "double_mouse_factor", "2"));
    if (xrez >= whentodouble) {
        x -= g_game.x_resolution / 2;
        y -= g_game.y_resolution / 2;
        if (invert) {
            x = int(x / factor);
            y = int(y / factor);
        } else {
            x = int(x * factor);
            y = int(y * factor);
        }
        x += g_game.x_resolution / 2;
        y += g_game.y_resolution / 2;
        if (x > g_game.x_resolution) {
            x = g_game.x_resolution;
        }
        if (y > g_game.y_resolution) {
            y = g_game.y_resolution;
        }
        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
    }
}

static bool createdbase = false;
static int createdmusic = -1;

void ModifyMouseSensitivity(int &x, int &y) {
    biModifyMouseSensitivity(x, y, false);
}

#ifdef BASE_MAKER
#include <stdio.h>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif //tells VCC not to generate min/max macros
#include <windows.h>
#endif
static char makingstate = 0;
#endif
extern const char *mission_key; //defined in main.cpp
bool BaseInterface::Room::BaseTalk::hastalked = false;

#define NEW_GUI

#ifdef NEW_GUI
#include "cmd/basecomputer.h"
#include "../gui/eventmanager.h"
#endif

using namespace VSFileSystem;
std::vector<unsigned int> base_keyboard_queue;

static void CalculateRealXAndY(int xbeforecalc, int ybeforecalc, float *x, float *y) {
    (*x) = (((float) (xbeforecalc * 2)) / g_game.x_resolution) - 1;
    (*y) = -(((float) (ybeforecalc * 2)) / g_game.y_resolution) + 1;
}

#define mymin(a, b) ( ( (a) < (b) ) ? (a) : (b) )

static void SetupViewport() {
    static int base_max_width = XMLSupport::parse_int(vs_config->getVariable("graphics", "base_max_width", "0"));
    static int base_max_height = XMLSupport::parse_int(vs_config->getVariable("graphics", "base_max_height", "0"));
    if (base_max_width && base_max_height) {
        int xrez = mymin(g_game.x_resolution, base_max_width);
        int yrez = mymin(g_game.y_resolution, base_max_height);
        int offsetx = (g_game.x_resolution - xrez) / 2;
        int offsety = (g_game.y_resolution - yrez) / 2;
        glViewport(offsetx, offsety, xrez, yrez);
    }
}

#undef mymin

BaseInterface::Room::~Room() {
    for (size_t i = 0; i < links.size(); ++i) {
        if (links[i] != nullptr) {
            delete links[i];
            links[i] = nullptr;
        }
    }
    for (size_t i = 0; i < objs.size(); ++i) {
        if (objs[i] != nullptr) {
            delete objs[i];
            objs[i] = nullptr;
        }
    }
}

BaseInterface::Room::Room() {
//Do nothing...
}

void BaseInterface::Room::BaseObj::Draw(BaseInterface *base) {
//Do nothing...
}

static FILTER BlurBases() {
    static bool blur_bases = XMLSupport::parse_bool(vs_config->getVariable("graphics", "blur_bases", "true"));
    return blur_bases ? BILINEAR : NEAREST;
}

BaseInterface::Room::BaseVSSprite::BaseVSSprite(const std::string &spritefile, const std::string &ind) :
        BaseObj(ind), spr(spritefile.c_str(), BlurBases(), GFXTRUE) {
}

BaseInterface::Room::BaseVSSprite::~BaseVSSprite() {
    if (soundsource.get() != NULL) {
        BaseUtil::DestroyVideoSoundStream(soundsource, soundscene);
    }
    spr.ClearTimeSource();
}

BaseInterface::Room::BaseVSMovie::BaseVSMovie(const std::string &moviefile, const std::string &ind) :
        BaseVSSprite(ind, VSSprite(AnimatedTexture::CreateVideoTexture(moviefile), 0, 0, 2, 2, 0, 0, true)) {
    playing = false;
    soundscene = "video";
    if (g_game.sound_enabled && spr.LoadSuccess()) {
        soundsource = BaseUtil::CreateVideoSoundStream(moviefile, soundscene);
        spr.SetTimeSource(soundsource);
    } else {
        spr.Reset();
    }
    SetHidePointer(true);
}

void BaseInterface::Room::BaseVSMovie::SetHidePointer(bool hide) {
    hidePointer = hide;
    hidePointerTime = realTime();
}

void BaseInterface::Room::BaseVSSprite::SetSprite(const std::string &spritefile) {
    //Destroy SPR
    spr.~VSSprite();
    //Re-create it (in case you don't know the following syntax,
    //which is a weird but standard syntax,
    //it initializes spr instead of allocating memory for it)
    //PS: I hope it doesn't break many compilers ;)
    //(if it does, spr will have to become a pointer)
    new(&spr)VSSprite(spritefile.c_str(), BlurBases(), GFXTRUE);
}

void BaseInterface::Room::BaseVSMovie::SetMovie(const std::string &moviefile) {
    //Get sprite position and size so that we can preserve them
    float x, y, w, h, rot;
    spr.GetPosition(x, y);
    spr.GetSize(w, h);
    spr.GetRotation(rot);

    //See notes above
    spr.~VSSprite();
    new(&spr)VSSprite(AnimatedTexture::CreateVideoTexture(moviefile), x, y, w, h, 0, 0, true);
    spr.SetRotation(rot);

    if (soundsource.get() != NULL) {
        BaseUtil::DestroyVideoSoundStream(soundsource, soundscene);
    }
    soundscene = "video";
    playing = false;
    if (g_game.sound_enabled) {
        soundsource = BaseUtil::CreateVideoSoundStream(moviefile, soundscene);
        spr.SetTimeSource(soundsource);
    } else {
        spr.Reset();
    }
}

float BaseInterface::Room::BaseVSMovie::GetTime() const {
    return spr.getTexture()->curTime();
}

void BaseInterface::Room::BaseVSMovie::SetTime(float t) {
    spr.getTexture()->setTime(t);
}

void BaseInterface::Room::BaseVSSprite::Draw(BaseInterface *base) {
    static float AlphaTestingCutoff =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "base_alpha_test_cutoff", "0"));
    GFXAlphaTest(GREATER, AlphaTestingCutoff);
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    GFXEnable(TEXTURE0);
    spr.Draw();
    GFXAlphaTest(ALWAYS, 0);

    // Play the associated source if it isn't playing
    if (soundsource.get() != NULL) {
        if (!soundsource->isPlaying()) {
            soundsource->startPlaying();
        }
    }
}

void BaseInterface::Room::BaseVSMovie::Draw(BaseInterface *base) {
    if (soundsource.get() == NULL) {
        // If it's not playing, mark as playing, and reset the sprite's animation
        // (it's not automatic without a time source)
        if (!playing) {
            playing = true;
            spr.Reset();
        }
    }

    // Hide mouse pointer
    if (base && hidePointer && base->mousePointerStyle != MOUSE_POINTER_NONE) {
        double time = realTime();
        if (hidePointerTime < 0.0) {
            hidePointerTime = time + 1.0;
        } else if (time > hidePointerTime) {
            base->mousePointerStyle = MOUSE_POINTER_NONE;
            hidePointerTime = -1.0;
        }
    }

    BaseInterface::Room::BaseVSSprite::Draw(base);

    if (soundsource.get() == NULL) {
        // If there is no sound source, and the sprite is an animated sprite, and
        // it's finished, then we must invoke the callback
        if (!getCallback().empty() && spr.Done()) {
            RunPython(getCallback().c_str());
            playing = false;
        }
    }
}

bool BaseInterface::Room::BaseVSSprite::isPlaying() const {
    return soundsource.get() != NULL
            && soundsource->isPlaying();
}

void BaseInterface::Room::BaseShip::Draw(BaseInterface *base) {
    Unit *un = base->caller.GetUnit();
    if (un) {
        GFXHudMode(GFXFALSE);
        float tmp = g_game.fov;
        static float standard_fov = XMLSupport::parse_float(vs_config->getVariable("graphics", "base_fov", "90"));
        g_game.fov = standard_fov;
        float tmp1 = _Universe->AccessCamera()->GetFov();
        _Universe->AccessCamera()->SetFov(standard_fov);
        Vector p, q, r;
        _Universe->AccessCamera()->GetOrientation(p, q, r);
        float co = _Universe->AccessCamera()->getCockpitOffset();
        _Universe->AccessCamera()->setCockpitOffset(0);
        _Universe->AccessCamera()->UpdateGFX();
        QVector pos = _Universe->AccessCamera()->GetPosition();
        Matrix cam(p.i, p.j, p.k, q.i, q.j, q.k, r.i, r.j, r.k, pos);
        Matrix final;
        Matrix newmat = mat;
        newmat.p.k *= un->rSize();
        newmat.p += QVector(0, 0, g_game.znear);
        newmat.p.i *= newmat.p.k;
        newmat.p.j *= newmat.p.k;
        MultMatrix(final, cam, newmat);
        SetupViewport();
        GFXClear(GFXFALSE);         //clear the zbuf

        GFXEnable(DEPTHTEST);
        GFXEnable(DEPTHWRITE);
        GFXEnable(LIGHTING);
        int light = 0;
        GFXCreateLight(light,
                GFXLight(true,
                        GFXColor(1, 1, 1, 1),
                        GFXColor(1, 1, 1, 1),
                        GFXColor(1, 1, 1, 1),
                        GFXColor(0.1, 0.1, 0.1, 1),
                        GFXColor(1, 0, 0),
                        GFXColor(1, 1, 1, 0),
                        24),
                true);

        (un)->DrawNow(final, FLT_MAX);
        GFXDeleteLight(light);
        GFXDisable(DEPTHTEST);
        GFXDisable(DEPTHWRITE);
        GFXDisable(LIGHTING);
        GFXDisable(TEXTURE1);
        GFXEnable(TEXTURE0);
        _Universe->AccessCamera()->setCockpitOffset(co);
        _Universe->AccessCamera()->UpdateGFX();
        SetupViewport();
        GFXHudMode(GFXTRUE);
        g_game.fov = tmp;
        _Universe->AccessCamera()->SetFov(tmp1);
    }
}

void BaseInterface::Room::Draw(BaseInterface *base) {
    size_t i;
    for (i = 0; i < objs.size(); i++) {
        if (objs[i]) {
            GFXBlendMode(SRCALPHA, INVSRCALPHA);
            objs[i]->Draw(base);
        }
    }
    GFXBlendMode(SRCALPHA, INVSRCALPHA);
    //draw location markers
    //<!-- config options in the "graphics" section -->
    //<var name="base_enable_locationmarkers" value="true"/>
    //<var name="base_locationmarker_sprite" value="base_locationmarker.spr"/>
    //<var name="base_draw_locationtext" value="true"/>
    //<var name="base_locationmarker_textoffset_x" value="0.025"/>
    //<var name="base_locationmarker_textoffset_y" value="0.025"/>
    //<var name="base_locationmarker_drawalways" value="false"/>
    //<var name="base_locationmarker_distance" value="0.5"/>
    //<var name="base_locationmarker_textcolor_r" value="1.0"/>
    //<var name="base_locationmarker_textcolor_g" value="1.0"/>
    //<var name="base_locationmarker_textcolor_b" value="1.0"/>
    //<var name="base_drawlocationborders" value="false"/>
    static bool enable_markers =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "base_enable_locationmarkers", "false"));
    static bool
            draw_text = XMLSupport::parse_bool(vs_config->getVariable("graphics", "base_draw_locationtext", "false"));
    static bool draw_always =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "base_locationmarker_drawalways", "false"));
    static float y_lower =
            -0.9;           //shows the offset on the lower edge of the screen (for the textline there) -> Should be defined globally somewhere
    static float base_text_background_alpha =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "base_text_background_alpha", "0.0625"));
    if (enable_markers) {
        float x, y, text_wid, text_hei;
        //get offset from config;
        static float text_offset_x =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textoffset_x", "0"));
        static float text_offset_y =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textoffset_y", "0"));
        static float text_color_r =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textcolor_r", "1"));
        static float text_color_g =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textcolor_g", "1"));
        static float text_color_b =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textcolor_b", "1"));
        for (size_t i = 0; i < links.size(); i++) {          //loop through all links and draw a marker for each
            if (links[i]) {
                if ((links[i]->alpha < 1) || (draw_always)) {
                    if (draw_always) {
                        links[i]->alpha = 1;
                    }                          //set all alphas to visible
                    x = (links[i]->x + (links[i]->wid / 2));                         //get the center of the location
                    y = (links[i]->y + (links[i]->hei / 2));                         //get the center of the location

                    /* draw marker */
                    static string
                            spritefile_marker = vs_config->getVariable("graphics", "base_locationmarker_sprite", "");
                    if (spritefile_marker.length() && links[i]->text.find("XXX") != 0) {
                        static VSSprite *spr_marker = new VSSprite(spritefile_marker.c_str());
                        float wid, hei;
                        spr_marker->GetSize(wid, hei);
                        //check if the sprite is near a screenedge and correct its position if necessary
                        if ((x + (wid / 2)) >= 1) {
                            x = (1 - (wid / 2));
                        }
                        if ((y + (hei / 2)) >= 1) {
                            y = (1 - (hei / 2));
                        }
                        if ((x - (wid / 2)) <= -1) {
                            x = (-1 + (wid / 2));
                        }
                        if ((y - (hei / 2)) <= y_lower) {
                            y = (y_lower + (hei / 2));
                        }
                        spr_marker->SetPosition(x, y);
                        GFXDisable(TEXTURE1);
                        GFXEnable(TEXTURE0);
                        GFXColor4f(1, 1, 1, links[i]->alpha);
                        spr_marker->Draw();
                    }                     //if spritefile
                    if (draw_text) {
                        GFXDisable(TEXTURE0);
                        TextPlane text_marker;
                        text_marker.SetText(links[i]->text);
                        text_marker.GetCharSize(text_wid,
                                text_hei);                           //get average charactersize
                        float text_pos_x = x + text_offset_x;                                  //align right ...
                        float text_pos_y = y + text_offset_y + text_hei;                         //...and on top
                        text_wid = text_wid * links[i]->text.length()
                                * 0.25;                                 //calc ~width of text (=multiply the average characterwidth with the number of characters)
                        if ((text_pos_x + text_offset_x + text_wid)
                                >= 1) {                                   //check right screenborder
                            text_pos_x =
                                    (x - fabs(text_offset_x) - text_wid);
                        }                                   //align left
                        if ((text_pos_y + text_offset_y)
                                >= 1) {                                            //check upper screenborder
                            text_pos_y = (y
                                    - fabs(text_offset_y));
                        }                                          //align on bottom
                        if ((text_pos_y + text_offset_y - text_hei)
                                <= y_lower) {                             //check lower screenborder
                            text_pos_y = (y + fabs(text_offset_y)
                                    + text_hei);
                        }                                   //align on top
                        text_marker.col = GFXColor(text_color_r, text_color_g, text_color_b, links[i]->alpha);
                        text_marker.SetPos(text_pos_x, text_pos_y);
                        if (links[i]->pythonfile != "#" && text_marker.GetText().find("XXX") != 0) {
                            GFXColor tmpbg = text_marker.bgcol;
                            bool automatte = (0 == tmpbg.a);
                            if (automatte)
                                text_marker.bgcol = GFXColor(0, 0, 0, base_text_background_alpha);
                            text_marker.Draw(text_marker.GetText(), 0, true, false, automatte);
                            text_marker.bgcol = tmpbg;
                        }
                        GFXEnable(TEXTURE0);
                    }                     //if draw_text
                }
            }
        }
        //if link
        //for i
    }     //enable_markers

    static bool draw_borders =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "base_drawlocationborders", "false"));
    static bool debug_markers =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "base_enable_debugmarkers", "false"));
    if (draw_borders || debug_markers) {
        float x, y, text_wid, text_hei;
        //get offset from config;
        static float text_offset_x =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textoffset_x", "0"));
        static float text_offset_y =
                XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_textoffset_y", "0"));
        for (size_t i = 0; i < links.size(); i++) {          //loop through all links and draw a marker for each
            if (links[i]) {
                //Debug marker
                if (debug_markers) {
                    //compute label position
                    x = (links[i]->x + (links[i]->wid / 2));                         //get the center of the location
                    y = (links[i]->y + (links[i]->hei / 2));                         //get the center of the location
                    TextPlane text_marker;
                    text_marker.SetText(links[i]->index);
                    text_marker.GetCharSize(text_wid, text_hei);                       //get average charactersize
                    float text_pos_x = x + text_offset_x;                              //align right ...
                    float text_pos_y = y + text_offset_y + text_hei;                     //...and on top
                    text_wid = text_wid * links[i]->text.length()
                            * 0.25;                             //calc ~width of text (=multiply the average characterwidth with the number of characters)
                    if ((text_pos_x + text_offset_x + text_wid)
                            >= 1) {                               //check right screenborder
                        text_pos_x = (x - fabs(text_offset_x) - text_wid);
                    }                               //align left
                    if ((text_pos_y + text_offset_y)
                            >= 1) {                                        //check upper screenborder
                        text_pos_y = (y - fabs(text_offset_y));
                    }                                      //align on bottom
                    if ((text_pos_y + text_offset_y - text_hei)
                            <= y_lower) {                         //check lower screenborder
                        text_pos_y = (y + fabs(text_offset_y) + text_hei);
                    }                               //align on top
                    if (enable_markers) {
                        text_pos_y += text_hei;
                    }
                    text_marker.col = GFXColor(1, 1, 1, 1);
                    text_marker.SetPos(text_pos_x, text_pos_y);

                    GFXDisable(TEXTURE0);
                    GFXColor tmpbg = text_marker.bgcol;
                    bool automatte = (0 == tmpbg.a);
                    if (automatte) {
                        text_marker.bgcol = GFXColor(0, 0, 0, base_text_background_alpha);
                    }
                    text_marker.Draw(text_marker.GetText(), 0, true, false, automatte);
                    text_marker.bgcol = tmpbg;
                    GFXEnable(TEXTURE0);
                }
                //link border
                GFXColor4f(1, 1, 1, 1);
                Vector c1(links[i]->x, links[i]->y, 0);
                Vector c3(links[i]->wid + c1.i, links[i]->hei + c1.j, 0);
                Vector c2(c1.i, c3.j, 0);
                Vector c4(c3.i, c1.j, 0);
                GFXDisable(TEXTURE0);
                const float verts[5 * 3] = {
                        c1.x, c1.y, c1.z,
                        c2.x, c2.y, c2.z,
                        c3.x, c3.y, c3.z,
                        c4.x, c4.y, c4.z,
                        c1.x, c1.y, c1.z,
                };
                GFXDraw(GFXLINESTRIP, verts, 5);
                GFXEnable(TEXTURE0);
            }
        }
        //if link
        //for i
    }     //if draw_borders
}

static std::vector<BaseInterface::Room::BaseTalk *> active_talks;

BaseInterface::Room::BaseTalk::BaseTalk(const std::string &msg, const std::string &ind, bool only_one)
        : BaseObj(ind), curchar(0), curtime(0), message(msg) {
    if (only_one) {
        active_talks.clear();
    }
    active_talks.push_back(this);
}

void BaseInterface::Room::BaseText::Draw(BaseInterface *base) {
    int tmpx = g_game.x_resolution;
    int tmpy = g_game.y_resolution;
    static int base_max_width = XMLSupport::parse_int(vs_config->getVariable("graphics", "base_max_width", "0"));
    static int base_max_height = XMLSupport::parse_int(vs_config->getVariable("graphics", "base_max_height", "0"));
    if (base_max_width && base_max_height) {
        if (base_max_width < tmpx) {
            g_game.x_resolution = base_max_width;
        }
        if (base_max_height < tmpy) {
            g_game.y_resolution = base_max_height;
        }
    }
    static float base_text_background_alpha =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "base_text_background_alpha", "0.0625"));
    GFXColor tmpbg = text.bgcol;
    bool automatte = (0 == tmpbg.a);
    if (automatte) {
        text.bgcol = GFXColor(0, 0, 0, base_text_background_alpha);
    }
    if (!automatte && text.GetText().empty()) {
        float posx, posy, wid, hei;
        text.GetPos(posy, posx);
        text.GetSize(wid, hei);

        GFXColorf(text.bgcol);
        const float verts[4 * 3] = {
                posx, hei, 0.0f,
                wid, hei, 0.0f,
                wid, posy, 0.0f,
                posx, posy, 0.0f,
        };
        GFXDraw(GFXQUAD, verts, 4);
    } else {
        text.Draw(text.GetText(), 0, true, false, automatte);
    }
    text.bgcol = tmpbg;
    g_game.x_resolution = tmpx;
    g_game.y_resolution = tmpy;
}

void RunPython(const char *filnam) {
#ifdef DEBUG_RUN_PYTHON
    VS_LOG(trace, "Run python:\n");
    VS_LOG(trace, (boost::format("%1%\n") % filnam));
#endif
    if (filnam[0]) {
        if (filnam[0] == '#' && filnam[1] != '\0') {
            ::Python::reseterrors();
            VegaPyRunString(filnam);
//            ::Python::reseterrors();
        } else {
            CompileRunPython(filnam);
        }
    }
}

void BaseInterface::Room::BasePython::Draw(BaseInterface *base) {
    timeleft += GetElapsedTime() / getTimeCompression();
    if (timeleft >= maxtime) {
        timeleft = 0;
        VS_LOG(debug, "Running python script...");
        RunPython(this->pythonfile.c_str());
        return;         //do not do ANYTHING with 'this' after the previous statement...
    }
}

void BaseInterface::Room::BasePython::Relink(const std::string &python) {
    pythonfile = python;
}

void BaseInterface::Room::BaseTalk::Draw(BaseInterface *base) {
    //FIXME: should be called from draw()
    if (hastalked) {
        return;
    }
    curtime += GetElapsedTime() / getTimeCompression();
    static float delay = XMLSupport::parse_float(vs_config->getVariable("graphics", "text_delay", ".05"));
    if ((std::find(active_talks.begin(), active_talks.end(),
            this) == active_talks.end())
            || (curchar >= message.size() && curtime > ((delay * message.size()) + 2))) {
        curtime = 0;
        std::vector<BaseObj *>::iterator ind = std::find(base->rooms[base->curroom]->objs.begin(),
                base->rooms[base->curroom]->objs.end(),
                this);
        if (ind != base->rooms[base->curroom]->objs.end()) {
            *ind = NULL;
        }
        std::vector<BaseTalk *>::iterator ind2 = std::find(active_talks.begin(), active_talks.end(), this);
        if (ind2 != active_talks.end()) {
            *ind2 = NULL;
        }
        base->othtext.SetText("");
        delete this;
        return;         //do not do ANYTHING with 'this' after the previous statement...
    }
    if (curchar < message.size()) {
        static float inbetween = XMLSupport::parse_float(vs_config->getVariable("graphics", "text_speed", ".025"));
        if (curtime > inbetween) {
            base->othtext.SetText(message.substr(0, ++curchar));
            curtime = 0;
        }
    }
    hastalked = true;
}

int BaseInterface::Room::MouseOver(BaseInterface *base, float x, float y) {
    for (size_t i = 0; i < links.size(); i++) {
        if (links[i]) {
            if (x >= links[i]->x
                    && x <= (links[i]->x + links[i]->wid)
                    && y >= links[i]->y
                    && y <= (links[i]->y + links[i]->hei)) {
                return i;
            }
        }
    }
    return -1;
}

BaseInterface *BaseInterface::CurrentBase = NULL;

bool RefreshGUI(void) {
    bool retval = false;
    if (_Universe->AccessCockpit()) {
        if (BaseInterface::CurrentBase) {
            if (_Universe->AccessCockpit()->GetParent() == BaseInterface::CurrentBase->caller.GetUnit()) {
                if (BaseInterface::CurrentBase->CallComp) {
#ifdef NEW_GUI
                    globalWindowManager().draw();
                    return true;

#else
                    return RefreshInterface();
#endif
                } else {
                    BaseInterface::CurrentBase->Draw();
                }
                retval = true;
            }
        }
    }
    return retval;
}

void base_main_loop() {
    UpdateTime();
    Music::MuzakCycle();

    GFXBeginScene();
    if (createdbase) {
        createdbase = false;
        AUDStopAllSounds(createdmusic);
    }
    if (!RefreshGUI()) {
        restore_main_loop();
    } else {
        GFXEndScene();
        micro_sleep(1000);
    }
    BaseComputer::dirty = false;
}

void BaseInterface::Room::Click(BaseInterface *base, float x, float y, int button, int state) {
    if (button == WS_LEFT_BUTTON) {
        int linknum = MouseOver(base, x, y);
        if (linknum >= 0) {
            Link *link = links[linknum];
            if (link) {
                link->Click(base, x, y, button, state);
            }
        }
    } else {
#ifdef BASE_MAKER
        if (state == WS_MOUSE_UP) {
            char  input[201];
            char *str;
            if (button == WS_RIGHT_BUTTON) {
                str = "Please create a file named stdin.txt and type\nin the sprite file that you wish to use.";
            } else if (button == WS_MIDDLE_BUTTON) {
                str =
                    "Please create a file named stdin.txt and type\nin the type of room followed by arguments for the room followed by text in quotations:\n1 ROOM# \"TEXT\"\n2 \"TEXT\"\n3 vector<MODES>.size vector<MODES> \"TEXT\"";
            } else {
                return;
            }
#ifdef _WIN32
            int ret = MessageBox( NULL, str, "Input", MB_OKCANCEL );
#else
            // 2020-10-29 stephengtuggy: Leaving this here, since it is obviously intended for real-time user interaction when in BASE_MAKER mode
            printf( "\n%s\n", str );
            int ret = 1;
#endif
            int index;
            int rmtyp;
            if (ret == 1) {
                if (button == WS_RIGHT_BUTTON) {
#ifdef _WIN32
                    FILE *fp = VSFileSystem::vs_open( "stdin.txt", "rt" );
#else
                    FILE *fp = stdin;
#endif
                    VSFileSystem::vs_fscanf( fp, "%200s", input );
#ifdef _WIN32
                    VSFileSystem::vs_close( fp );
#endif
                } else if (button == WS_MIDDLE_BUTTON && makingstate == 0) {
                    int   i;
#ifdef _WIN32
                    FILE *fp = VSFileSystem::vs_open( "stdin.txt", "rt" );
#else
                    FILE *fp = stdin;
#endif
                    VSFileSystem::vs_fscanf( fp, "%d", &rmtyp );
                    switch (rmtyp)
                    {
                    case 1:
                        links.push_back( new Goto( "linkind", "link" ) );
                        VSFileSystem::vs_fscanf( fp, "%d", &( (Goto*) links.back() )->index );
                        break;
                    case 2:
                        links.push_back( new Launch( "launchind", "launch" ) );
                        break;
                    case 3:
                        links.push_back( new Comp( "compind", "comp" ) );
                        VSFileSystem::vs_fscanf( fp, "%d", &index );
                        for (i = 0; i < index && ( !VSFileSystem::vs_feof( fp ) ); i++) {
                            VSFileSystem::vs_fscanf( fp, "%d", &ret );
                            ( (Comp*) links.back() )->modes.push_back( (BaseComputer::DisplayMode) ret );
                        }
                        break;
                    default:
#ifdef _WIN32
                        VSFileSystem::vs_close( fp );
                        MessageBox( NULL, "warning: invalid basemaker option", "Error", MB_OK );
#endif
                        // 2020-10-29 stephengtuggy: Leaving this here, since it is obviously intended for real-time user interaction when in BASE_MAKER mode
                        printf( "warning: invalid basemaker option: %d", rmtyp );
                        return;
                    }
                    VSFileSystem::vs_fscanf( fp, "%200s", input );
                    input[200] = input[199] = '\0';
                    links.back()->text = string( input );
#ifdef _WIN32
                    VSFileSystem::vs_close( fp );
#endif
                }
                if (button == WS_RIGHT_BUTTON) {
                    input[200] = input[199] = '\0';
                    objs.push_back( new BaseVSSprite( input, "tex" ) );
                    ( (BaseVSSprite*) objs.back() )->texfile = string( input );
                    ( (BaseVSSprite*) objs.back() )->spr.SetPosition( x, y );
                } else if (button == WS_MIDDLE_BUTTON && makingstate == 0) {
                    links.back()->x   = x;
                    links.back()->y   = y;
                    links.back()->wid = 0;
                    links.back()->hei = 0;
                    makingstate = 1;
                } else if (button == WS_MIDDLE_BUTTON && makingstate == 1) {
                    links.back()->wid = x-links.back()->x;
                    if (links.back()->wid < 0)
                        links.back()->wid = -links.back()->wid;
                    links.back()->hei = y-links.back()->y;
                    if (links.back()->hei < 0)
                        links.back()->hei = -links.back()->hei;
                    makingstate = 0;
                }
            }
        }
#else
        if (state == WS_MOUSE_UP && links.size()) {
            size_t count = 0;
            while (count++ < links.size()) {
                Link *curlink = links[base->curlinkindex++ % links.size()];
                if (curlink) {
                    int x = int((((curlink->x + (curlink->wid / 2)) + 1) / 2) * g_game.x_resolution);
                    int y = -int((((curlink->y + (curlink->hei / 2)) - 1) / 2) * g_game.y_resolution);
                    biModifyMouseSensitivity(x, y, true);
                    winsys_warp_pointer(x, y);
                    PassiveMouseOverWin(x, y);
                    break;
                }
            }
        }
#endif
    }
}

void BaseInterface::MouseOver(int xbeforecalc, int ybeforecalc) {
    float x, y;
    CalculateRealXAndY(xbeforecalc, ybeforecalc, &x, &y);
    int i = rooms[curroom]->MouseOver(this,
            x,
            y); //FIXME Whatever this is, it shouldn't be named just "i"; & possibly should be size_t
    Room::Link *link = 0;
    Room::Link *hotlink = 0;
    if (i >= 0) {
        link = rooms[curroom]->links[i];
    }
    if (lastmouseindex >= 0 && lastmouseindex < static_cast<int>(rooms[curroom]->links.size())) {
        hotlink = rooms[curroom]->links[lastmouseindex];
    }
    if (hotlink && (lastmouseindex != i)) {
        hotlink->MouseLeave(this, x, y, getMouseButtonMask());
    }
    if (link && (lastmouseindex != i)) {
        link->MouseEnter(this, x, y, getMouseButtonMask());
    }
    if (link) {
        link->MouseMove(this, x, y, getMouseButtonMask());
    }
    lastmouseindex = i;
    static float overcolor[4] = {1, .666666667, 0, 1};
    static float inactivecolor[4] = {0, 1, 0, 1};
    if (link) {
        curtext.SetText(link->text);
    } else {
        curtext.SetText(rooms[curroom]->deftext);
    }
    if (link && link->pythonfile != "#") {
        curtext.col = GFXColor(overcolor[0], overcolor[1], overcolor[2], overcolor[3]);
        mousePointerStyle = MOUSE_POINTER_HOVER;
    } else {
        curtext.col = GFXColor(inactivecolor[0], inactivecolor[1], inactivecolor[2], inactivecolor[3]);
        mousePointerStyle = MOUSE_POINTER_NORMAL;
    }
    static bool draw_always =
            XMLSupport::parse_bool(vs_config->getVariable("graphics", "base_locationmarker_drawalways", "false"));
    static float defined_distance =
            fabs(XMLSupport::parse_float(vs_config->getVariable("graphics", "base_locationmarker_distance", "0.5")));
    if (!draw_always) {
        float cx, cy;
        float dist_cur2link;
        for (i = 0; i < static_cast<int>(rooms[curroom]->links.size()); i++) {
            cx = (rooms[curroom]->links[i]->x + (rooms[curroom]->links[i]->wid / 2)); //get the x center of the location
            cy = (rooms[curroom]->links[i]->y + (rooms[curroom]->links[i]->hei / 2)); //get the y center of the location
            dist_cur2link = sqrt(pow((cx - x), 2) + pow((cy - y), 2));
            if (dist_cur2link < defined_distance) {
                rooms[curroom]->links[i]->alpha = (1 - (dist_cur2link / defined_distance));
            } else {
                rooms[curroom]->links[i]->alpha = 1;
            }
        }
    }
}

void BaseInterface::Click(int xint, int yint, int button, int state) {
    float x, y;
    CalculateRealXAndY(xint, yint, &x, &y);
    rooms[curroom]->Click(this, x, y, button, state);
}

void BaseInterface::ClickWin(int button, int state, int x, int y) {
    ModifyMouseSensitivity(x, y);
    if (state == WS_MOUSE_DOWN) {
        getMouseButtonMask() |= (1 << (button - 1));
    } else if (state == WS_MOUSE_UP) {
        getMouseButtonMask() &= ~(1 << (button - 1));
    }
    if (CurrentBase) {
        if (CurrentBase->CallComp) {
#ifdef NEW_GUI
            EventManager::
#else
            UpgradingInfo::
#endif
            ProcessMouseClick(button, state, x, y);
        } else {
            CurrentBase->Click(x, y, button, state);
        }
    } else {
        NavigationSystem::mouseClick(button, state, x, y);
    }
}

void BaseInterface::PassiveMouseOverWin(int x, int y) {
    ModifyMouseSensitivity(x, y);
    SetSoftwareMousePosition(x, y);
    if (CurrentBase) {
        if (CurrentBase->CallComp) {
#ifdef NEW_GUI
            EventManager::
#else
            UpgradingInfo::
#endif
            ProcessMousePassive(x, y);
        } else {
            CurrentBase->MouseOver(x, y);
        }
    } else {
        NavigationSystem::mouseMotion(x, y);
    }
}

void BaseInterface::ActiveMouseOverWin(int x, int y) {
    ModifyMouseSensitivity(x, y);
    SetSoftwareMousePosition(x, y);
    if (CurrentBase) {
        if (CurrentBase->CallComp) {
#ifdef NEW_GUI
            EventManager::
#else
            UpgradingInfo::
#endif
            ProcessMouseActive(x, y);
        } else {
            CurrentBase->MouseOver(x, y);
        }
    } else {
        NavigationSystem::mouseDrag(x, y);
    }
}

void BaseInterface::Key(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    if (!python_kbhandler.empty()) {
        const std::string *evtype;
        if (release) {
            static const std::string release_evtype("keyup");
            evtype = &release_evtype;
        } else {
            static const std::string press_evtype("keydown");
            evtype = &press_evtype;
        }
        BaseUtil::SetKeyEventData(*evtype, ch);
        RunPython(python_kbhandler.c_str());
    }
}

void BaseInterface::GotoLink(int linknum) {
    othtext.SetText("");
    if (static_cast<int>(rooms.size()) > linknum && linknum >= 0) {
        curlinkindex = 0;
        curroom = linknum;
        curtext.SetText(rooms[curroom]->deftext);
        mousePointerStyle = MOUSE_POINTER_NORMAL;
    } else {
#ifndef BASE_MAKER
        VS_LOG_AND_FLUSH(fatal,
                (boost::format("\nWARNING: base room #%d tried to go to an invalid index: #%d") % curroom
                        % linknum));
        assert(0);
#else
        while (rooms.size() <= linknum) {
            rooms.push_back( new Room() );
            char roomnum[50];
            sprintf( roomnum, "Room #%d", linknum );
            rooms.back()->deftext = roomnum;
        }
        GotoLink( linknum );
#endif
    }
}

BaseInterface::~BaseInterface() {
#ifdef BASE_MAKER
    FILE *fp = VSFileSystem::vs_open( "bases/NEW_BASE" BASE_EXTENSION, "wt" );
    if (fp) {
        EndXML( fp );
        VSFileSystem::vs_close( fp );
    }
#endif
    CurrentBase = 0;
    restore_main_loop();
    for (size_t i = 0; i < rooms.size(); i++) {
        delete rooms[i];
    }
}

void base_main_loop();
int shiftup(int);

static void base_keyboard_cb(unsigned int ch, unsigned int mod, bool release, int x, int y) {
    //Set modifiers
    unsigned int amods = 0;
    amods |= (mod & (WSK_MOD_LSHIFT | WSK_MOD_RSHIFT)) ? KB_MOD_SHIFT : 0;
    amods |= (mod & (WSK_MOD_LCTRL | WSK_MOD_RCTRL)) ? KB_MOD_CTRL : 0;
    amods |= (mod & (WSK_MOD_LALT | WSK_MOD_RALT)) ? KB_MOD_ALT : 0;
    setActiveModifiers(amods);
    unsigned int shiftedch =
            ((WSK_MOD_LSHIFT == (mod & WSK_MOD_LSHIFT)) || (WSK_MOD_RSHIFT == (mod & WSK_MOD_RSHIFT))) ? shiftup(ch)
                    : ch;
    if (BaseInterface::CurrentBase && !BaseInterface::CurrentBase->CallComp) {
        //Flush buffer
        if (base_keyboard_queue.size()) {
            BaseInterface::ProcessKeyboardBuffer();
        }
        //Send directly to base interface handlers
        BaseInterface::CurrentBase->Key(shiftedch, amods, release, x, y);
    } else
        //Queue keystroke
    if (!release) {
        base_keyboard_queue.push_back(shiftedch);
    }
}

void BaseInterface::InitCallbacks() {
    winsys_set_keyboard_func(base_keyboard_cb);
    winsys_set_mouse_func(ClickWin);
    winsys_set_motion_func(ActiveMouseOverWin);
    winsys_set_passive_motion_func(PassiveMouseOverWin);
    CurrentBase = this;
    CallComp = false;
    static bool simulate_while_at_base =
            XMLSupport::parse_bool(vs_config->getVariable("physics", "simulate_while_docked", "false"));
    if (!(simulate_while_at_base || _Universe->numPlayers() > 1)) {
        GFXLoop(base_main_loop);
    }
}

BaseInterface::Room::Talk::Talk(const std::string &ind, const std::string &pythonfile) :
        BaseInterface::Room::Link(ind, pythonfile), index(-1) {
#ifndef BASE_MAKER
    gameMessage last;
    int i = 0;
    vector<std::string> who;
    string newmsg;
    string newsound;
    who.push_back("bar");
    while ((mission->msgcenter->last(i++, last, who))) {
        newmsg = last.message;
        newsound = "";
        string::size_type first = newmsg.find_first_of("[");
        {
            string::size_type last = newmsg.find_first_of("]");
            if (first != string::npos && (first + 1) < newmsg.size()) {
                newsound = newmsg.substr(first + 1, last - first - 1);
                newmsg = newmsg.substr(0, first);
            }
        }
        this->say.push_back(newmsg);
        this->soundfiles.push_back(newsound);
    }
#endif
}

double compute_light_dot(Unit *base, Unit *un) {
    StarSystem *ss = base->getStarSystem();
    double ret = -1;
    Unit *st;
    Unit *base_owner = NULL;
    if (ss) {
        _Universe->pushActiveStarSystem(ss);
        un_iter ui = ss->getUnitList().createIterator();
        for (; (st = *ui); ++ui) {
            if (st->isPlanet()) {
                if (((Planet *) st)->hasLights()) {
#ifdef VS_DEBUG
                    QVector v1  = ( un->Position()-base->Position() ).Normalize();
                    QVector v2  = ( st->Position()-base->Position() ).Normalize();

                    double  dot = v1.Dot( v2 );
                    if (dot > ret) {
                        VS_LOG(debug, (boost::format("dot %1%") % dot));
                        ret = dot;
                    }
#endif
                } else {
                    un_iter ui = ((Planet *) st)->satellites.createIterator();
                    Unit *ownz = NULL;
                    for (; (ownz = *ui); ++ui) {
                        if (ownz == base) {
                            base_owner = st;
                        }
                    }
                }
            }
        }
        _Universe->popActiveStarSystem();
    } else {
        return 1;
    }
    if (base_owner == NULL || base->isUnit() == Vega_UnitType::planet) {
        return ret;
    } else {
        return compute_light_dot(base_owner, un);
    }
}

const char *compute_time_of_day(Unit *base, Unit *un) {
    if (!base || !un) {
        return "day";
    }
    float rez = compute_light_dot(base, un);
    if (rez > .2) {
        return "day";
    }
    if (rez < -.1) {
        return "night";
    }
    return "sunset";
}

extern void ExecuteDirector();

BaseInterface::BaseInterface(const char *basefile, Unit *base, Unit *un) :
        curtext(vs_config->getColor("Base_Text_Color_Foreground", GFXColor(0, 1, 0, 1)),
                vs_config->getColor("Base_Text_Color_Background", GFXColor(0, 0, 0, 1))),
        othtext(vs_config->getColor("Fixer_Text_Color_Foreground", GFXColor(1, 1, .5, 1)),
                vs_config->getColor("FixerTextColor_Background", GFXColor(0, 0, 0, 1))) {
    CurrentBase = this;
    CallComp = false;
    lastmouseindex = 0;
    enabledj = true;
    createdbase = true;
    midloop = false;
    terminate_scheduled = false;
    createdmusic = -1;
    caller = un;
    curroom = 0;
    curlinkindex = 0;
    this->baseun = base;
    float x, y;
    curtext.GetCharSize(x, y);
    curtext.SetCharSize(x * 2, y * 2);
    curtext.SetSize(1 - .01, -2);
    othtext.GetCharSize(x, y);
    othtext.SetCharSize(x * 2, y * 2);
    othtext.SetSize(1 - .01, -.75);

    std::string fac = base ? FactionUtil::GetFaction(base->faction) : "neutral";
    if (base && fac == "neutral") {
        fac = UniverseUtil::GetGalaxyFaction(UnitUtil::getUnitSystemFile(base));
    }
    Load(basefile, compute_time_of_day(base, un), fac.c_str());
    createdmusic = AUDHighestSoundPlaying();
    if (base && un) {
        vector<string> vec;
        vec.push_back(base->name);
        int cpt = UnitUtil::isPlayerStarship(un);
        if (cpt >= 0) {
            saveStringList(cpt, mission_key, vec);
        }
    }
    if (!rooms.size()) {
        VS_LOG(error, (boost::format("ERROR: there are no rooms in basefile \"%1%%2%%3%\" ...\n")
                % basefile
                % compute_time_of_day(base, un)
                % BASE_EXTENSION));
        rooms.push_back(new Room());
        rooms.back()->deftext = "ERROR: No rooms specified...";
#ifndef BASE_MAKER
        rooms.back()->objs.push_back(new Room::BaseShip(-1, 0, 0, 0, 0, -1, 0, 1, 0, QVector(0, 0, 2), "default room"));
        BaseUtil::Launch(0, "default room", -1, -1, 1, 2, "ERROR: No rooms specified... - Launch");
        BaseUtil::Comp(0, "default room", 0, -1, 1, 2, "ERROR: No rooms specified... - Computer",
                "Cargo Upgrade Info ShipDealer News Missions");
#endif
    }
    GotoLink(0);
    {
        for (unsigned int i = 0; i < 16; ++i) {
            ExecuteDirector();
        }
    }
}

//Need this for NEW_GUI.  Can't ifdef it out because it needs to link.
void InitCallbacks(void) {
    if (BaseInterface::CurrentBase) {
        BaseInterface::CurrentBase->InitCallbacks();
    }
}

void TerminateCurrentBase(void) {
    if (BaseInterface::CurrentBase) {
        BaseInterface::CurrentBase->Terminate();
        BaseInterface::CurrentBase = NULL;
    }
}

void CurrentBaseUnitSet(Unit *un) {
    if (BaseInterface::CurrentBase) {
        BaseInterface::CurrentBase->caller.SetUnit(un);
    }
}
//end NEW_GUI.

void BaseInterface::Room::Comp::Click(BaseInterface *base, float x, float y, int button, int state) {
    if (state == WS_MOUSE_UP) {
        Link::Click(base, x, y, button, state);
        Unit *un = base->caller.GetUnit();
        Unit *baseun = base->baseun.GetUnit();
        if (un && baseun) {
            base->CallComp = true;
#ifdef NEW_GUI
            BaseComputer *bc = new BaseComputer(un, baseun, modes);
            bc->init();
            bc->run();
#else
            UpgradeCompInterface( un, baseun, modes );
#endif //NEW_GUI
        }
    }
}

void BaseInterface::Terminate() {
    if (midloop) {
        terminate_scheduled = true;
    } else {
        Unit *un = caller.GetUnit();
        int cpt = UnitUtil::isPlayerStarship(un);
        if (un && cpt >= 0) {
            vector<string> vec;
            vec.push_back(string());
            saveStringList(cpt, mission_key, vec);
        }
        BaseInterface::CurrentBase = NULL;
        restore_main_loop();
        delete this;
    }
}

extern void abletodock(int dock);

void BaseInterface::Room::Launch::Click(BaseInterface *base, float x, float y, int button, int state) {
    if (state == WS_MOUSE_UP) {
        Link::Click(base, x, y, button, state);
        static bool
                auto_undock_var = XMLSupport::parse_bool(vs_config->getVariable("physics", "AutomaticUnDock", "true"));
        bool auto_undock = auto_undock_var;
        Unit *bas = base->baseun.GetUnit();
        Unit *playa = base->caller.GetUnit();

        if (playa && bas) {
            if (((playa->name == "eject") || (playa->name == "ejecting") || (playa->name == "pilot")
                    || (playa->name == "Pilot") || (playa->name == "Eject")) && (bas->faction == playa->faction)) {
                playa->name = "return_to_cockpit";
            }
        }
        if ((playa && bas) && (auto_undock || (playa->name == "return_to_cockpit"))) {
            playa->UnDock(bas);
            CommunicationMessage c(bas, playa, NULL, 0);
            c.SetCurrentState(c.fsm->GetUnDockNode(), NULL, 0);
            if (playa->getAIState()) {
                playa->getAIState()->Communicate(c);
            }
            abletodock(5);
            if (playa->name == "return_to_cockpit") {
                if (playa->faction == bas->faction) {
                    playa->owner = bas;
                }
            }
        }
        base->Terminate();
    }
}

inline float aynrand(float min, float max) {
    return ((float) (rand()) / RAND_MAX) * (max - min) + min;
}

inline QVector randyVector(float min, float max) {
    return QVector(aynrand(min, max),
            aynrand(min, max),
            aynrand(min, max));
}

void BaseInterface::Room::Eject::Click(BaseInterface *base, float x, float y, int button, int state) {
    if (state == WS_MOUSE_UP) {
        Link::Click(base, x, y, button, state);
        XMLSupport::parse_bool(vs_config->getVariable("physics", "AutomaticUnDock", "true"));
        Unit *bas = base->baseun.GetUnit();
        Unit *playa = base->caller.GetUnit();
        if (playa && bas) {
            if (playa->name == "return_to_cockpit") {
                playa->name = "ejecting";
                Vector tmpvel = bas->Velocity * -1;
                if (tmpvel.MagnitudeSquared() < .00001) {
                    tmpvel = randyVector(-(bas->rSize()), bas->rSize()).Cast();
                    if (tmpvel.MagnitudeSquared() < .00001) {
                        tmpvel = Vector(1, 1, 1);
                    }
                }
                tmpvel.Normalize();
                playa->SetPosAndCumPos(bas->Position() + tmpvel * 1.5 * bas->rSize()
                        + randyVector(-.5 * bas->rSize(), .5 * bas->rSize()));
                playa->SetAngularVelocity(bas->AngularVelocity);
                playa->SetOwner(bas);
                static float
                        velmul = XMLSupport::parse_float(vs_config->getVariable("physics", "eject_cargo_speed", "1"));
                playa->SetVelocity(bas->Velocity * velmul + randyVector(-.25, .25).Cast());
            }
            playa->UnDock(bas);
            CommunicationMessage c(bas, playa, NULL, 0);
            c.SetCurrentState(c.fsm->GetUnDockNode(), NULL, 0);
            if (playa->getAIState()) {
                playa->getAIState()->Communicate(c);
            }
            abletodock(5);
            playa->EjectCargo((unsigned int) -1);
            if ((playa->name == "return_to_cockpit") || (playa->name == "ejecting") || (playa->name == "eject")
                    || (playa->name == "Eject") || (playa->name == "Pilot") || (playa->name == "pilot")) {
                playa->Kill();
            }
        }
        base->Terminate();
    }
}

void BaseInterface::Room::Goto::Click(BaseInterface *base, float x, float y, int button, int state) {
    if (state == WS_MOUSE_UP) {
        Link::Click(base, x, y, button, state);
        base->GotoLink(index);
    }
}

void BaseInterface::Room::Talk::Click(BaseInterface *base, float x, float y, int button, int state) {
    if (state == WS_MOUSE_UP) {
        Link::Click(base, x, y, button, state);
        if (index >= 0) {
            delete base->rooms[curroom]->objs[index];
            base->rooms[curroom]->objs[index] = NULL;
            index = -1;
            base->othtext.SetText("");
        } else if (say.size()) {
            curroom = base->curroom;
            int sayindex = rand() % say.size();
            base->rooms[curroom]->objs.push_back(new Room::BaseTalk(say[sayindex], "currentmsg", true));
            if (soundfiles[sayindex].size() > 0) {
                int sound = AUDCreateSoundWAV(soundfiles[sayindex], false);
                if (sound == -1) {
                    VS_LOG(error, (boost::format("\nCan't find the sound file %1%\n") % soundfiles[sayindex].c_str()));
                } else {
                    AUDStartPlaying(sound);
                    AUDDeleteSound(sound);                     //won't actually toast it until it stops
                }
            }
        } else {
            VS_LOG_AND_FLUSH(fatal, "\nThere are no things to say...\n");
            assert(0);
        }
    }
}

void BaseInterface::Room::Link::Click(BaseInterface *base, float x, float y, int button, int state) {
    unsigned int buttonmask = getMouseButtonMask();
    if (state == WS_MOUSE_UP) {
        if (eventMask & UpEvent) {
            static std::string evtype("up");
            BaseUtil::SetMouseEventData(evtype, x, y, buttonmask);
            RunPython(this->pythonfile.c_str());
        }
    }
    if (state == WS_MOUSE_UP) {
        //For now, the same. Eventually, we'll want click & double-click
        if (eventMask & ClickEvent) {
            static std::string evtype("click");
            BaseUtil::SetMouseEventData(evtype, x, y, buttonmask);
            RunPython(this->pythonfile.c_str());
        }
    }
    if (state == WS_MOUSE_DOWN) {
        if (eventMask & DownEvent) {
            static std::string evtype("down");
            BaseUtil::SetMouseEventData(evtype, x, y, buttonmask);
            RunPython(this->pythonfile.c_str());
        }
    }
}

void BaseInterface::Room::Link::MouseMove(::BaseInterface *base, float x, float y, int buttonmask) {
    //Compiling Python code each mouse movement == Bad idea!!!
    //If this support is needed we will need to use Python-C++ inheritance.
    //Like the Execute() method of AI and Mission classes.
    //Even better idea: Rewrite the entire BaseInterface python interface.
    if (eventMask & MoveEvent) {
        static std::string evtype("move");
        BaseUtil::SetMouseEventData(evtype, x, y, buttonmask);
        RunPython(this->pythonfile.c_str());
    }
}

void BaseInterface::Room::Link::MouseEnter(::BaseInterface *base, float x, float y, int buttonmask) {
    if (eventMask & EnterEvent) {
        static std::string evtype("enter");
        BaseUtil::SetMouseEventData(evtype, x, y, buttonmask);
        RunPython(this->pythonfile.c_str());
    }
}

void BaseInterface::Room::Link::MouseLeave(::BaseInterface *base, float x, float y, int buttonmask) {
    if (eventMask & LeaveEvent) {
        static std::string evtype("leave");
        BaseUtil::SetMouseEventData(evtype, x, y, buttonmask);
        RunPython(this->pythonfile.c_str());
    }
    clickbtn = -1;
}

void BaseInterface::Room::Link::Relink(const std::string &pfile) {
    pythonfile = pfile;
}

struct BaseColor {
    unsigned char r, g, b, a;
};

static void AnimationDraw() {
#ifdef RENDER_FROM_TEXTURE
    static StreamTexture T( 512, 256, NEAREST, NULL );
    BaseColor( *data )[512] = reinterpret_cast< BaseColor(*)[512] > ( T.Map() );
    bool counter = false;
    srand( time( NULL ) );
    for (int i = 0; i < 256; ++i)
        for (int j = 0; j < 512; ++j) {
            data[i][j].r = rand()&0xff;
            data[i][j].g = rand()&0xff;
            data[i][j].b = rand()&0xff;
            data[i][j].a = rand()&0xff;
        }
    T.UnMap();
    T.MakeActive();
    GFXTextureEnv( 0, GFXREPLACETEXTURE );
    GFXEnable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXDisable( CULLFACE );
    const float verts[4 * (3 + 2)] = {
        -1, -1, 0, 0, 0,
         1, -1, 0, 1, 0,
         1,  1, 0, 1, 1,
        -1,  1, 0, 0, 1,
    };
    GFXDraw( GFXQUAD, verts, 4, 3, 0, 2 );
#endif
}

void BaseInterface::Draw() {
    // Some operations cannot be performed in the middle of a Draw() loop
    midloop = true;

    GFXColor(0, 0, 0, 0);
    SetupViewport();
    StartGUIFrame(GFXTRUE);
    if (GetElapsedTime() < 1) {
        AnimatedTexture::UpdateAllFrame();
    }
    Room::BaseTalk::hastalked = false;
    rooms[curroom]->Draw(this);
    AnimationDraw();

    float x, y;
    glViewport(0, 0, g_game.x_resolution, g_game.y_resolution);
    static float base_text_background_alpha =
            XMLSupport::parse_float(vs_config->getVariable("graphics", "base_text_background_alpha", "0.0625"));

    curtext.GetCharSize(x, y);
    curtext.SetPos(-.99, -1 + (y * 1.5));

    if (curtext.GetText().find("XXX") != 0) {
        GFXColor tmpbg = curtext.bgcol;
        bool automatte = (0 == tmpbg.a);
        if (automatte) {
            curtext.bgcol = GFXColor(0, 0, 0, base_text_background_alpha);
        }
        curtext.Draw(curtext.GetText(), 0, true, false, automatte);
        curtext.bgcol = tmpbg;
    }
    othtext.SetPos(-.99, 1);

    if (othtext.GetText().length() != 0) {
        GFXColor tmpbg = othtext.bgcol;
        bool automatte = (0 == tmpbg.a);
        if (automatte) {
            othtext.bgcol = GFXColor(0, 0, 0, base_text_background_alpha);
        }
        othtext.Draw(othtext.GetText(), 0, true, false, automatte);
        othtext.bgcol = tmpbg;
    }
    SetupViewport();
    EndGUIFrame(mousePointerStyle);
    glViewport(0, 0, g_game.x_resolution, g_game.y_resolution);
    Unit *un = caller.GetUnit();
    Unit *base = baseun.GetUnit();
    if (un && (!base)) {
        VS_LOG(error, "Error: Base NULL");
        mission->msgcenter->add("game", "all", "[Computer] Docking unit destroyed. Emergency launch initiated.");
        for (size_t i = 0; i < un->pImage->dockedunits.size(); i++) {
            if (un->pImage->dockedunits[i]->uc.GetUnit() == base) {
                un->FreeDockingPort(i);
            }
        }
        Terminate();
    }

    //Commit audio scene status to renderer
    if (g_game.sound_enabled) {
        Audio::SceneManager::getSingleton()->commit();
    }

    // Some operations cannot be performed in the middle of a Draw() loop
    // If any of them are scheduled for deferred execution, do so now
    midloop = false;
    if (terminate_scheduled) {
        Terminate();
    }
}

void BaseInterface::ProcessKeyboardBuffer() {
    if (CurrentBase) {
        if (!CurrentBase->CallComp) {
            for (std::vector<unsigned int>::iterator it = base_keyboard_queue.begin(); it != base_keyboard_queue.end();
                    ++it) {
                CurrentBase->Key(*it, 0, false, 0, 0);
                CurrentBase->Key(*it, 0, true, 0, 0);
            }
            base_keyboard_queue.clear();
        }
    }
}

void BaseInterface::setDJEnabled(bool enabled) {
    enabledj = enabled;
}
