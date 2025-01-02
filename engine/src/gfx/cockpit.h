// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/**
 * cockpit.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_COCKPIT_H
#define VEGA_STRIKE_ENGINE_GFX_COCKPIT_H

#include "gfx/cockpit_generic.h"
#include "gfxlib.h"
#include "gfxlib_struct.h"
#include <vector>
#include <list>

class TextPlane;
class VSSprite;
class Gauge;
class Unit;
class NavigationSystem;
#include "radar/radar.h"
#include "radar/sensor.h"
#include "vdu.h"
#include "camera.h"
#include "physics.h"
#include "nav/navscreen.h"
using namespace XMLSupport;   // DONT PUT THIS ABOVE HEADERS
#define NUM_CAM CP_NUMVIEWS
/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */

struct soundContainer   //used to contain static sounds that will only be
{
    //created once and will get deleted automatically
    int sound;

    soundContainer() {
        sound = -2;
    }

    void loadsound(string sooundfile, bool looping = false);
    void playsound();
    ~soundContainer();
};

struct soundArray {
    soundContainer *ptr;

    soundArray() {
        ptr = NULL;
    }

    void deallocate() {
        if (ptr != NULL) {
            delete[] ptr;
            ptr = NULL;
        }
    }

    void allocate(int siz) {
        deallocate();
        ptr = new soundContainer[siz];
    }

    ~soundArray() {
        deallocate();
    }
};

class GameCockpit : public Cockpit {
    Camera cam[NUM_CAM];

    float insidePanYaw, insidePanPitch;
    float insidePanYawSpeed, insidePanPitchSpeed;

    float vdu_time[MAXVDUS];
    ///saved values to compare with current values (might need more for damage)
    std::list<Matrix> headtrans;
    float shake_time;
    int shake_type;
    Vector oaccel;
    float smooth_fov;
    float last_locktime;
    float last_mlocktime;
    std::vector<class Mesh *> mesh;
    int soundfile;
    VSSprite *Pit[4];
    VSSprite *radarSprites[2];
    std::unique_ptr<Radar::Display> radarDisplay;
    ///Video Display Units (may need more than 2 in future)
    std::vector<VDU *> vdu;
    /// An information string displayed in the VDU.
    std::string targetLabel;
    /// Comparison pointer to clear the label when the target changes.
    void *labeledTargetUnit;
    ///Color of cockpit default text
    GFXColor textcol;
    ///The font that the entire cockpit will use. Currently without color
    TextPlane *text;
    Gauge *gauges[UnitImages<void>::NUMGAUGES];
    ///holds misc panels.  Panel[0] is always crosshairs (and adjusted to be in center of view screen, not cockpit)
    std::vector<VSSprite *> Panel;
    ///flag to decide whether to draw all target boxes
    bool draw_all_boxes;
    bool draw_line_to_target, draw_line_to_targets_target;
    bool draw_line_to_itts;
    ///flag to tell wheter to draw the itts, even if the ship has none
    bool always_itts;
    ///flag controlling whether to use old school, less jumpy (and less accurate) itts // ** jay
    bool steady_itts;
    //colors of blips/targetting boxes
    GFXColor friendly, enemy, neutral, targeted, targetting, planet;

    // Last observed values for edge-triggered events
    struct LastState {
        double processing_time;
        int warplooplevel;
        int warpskiplevel;

        bool jumpok: 1, jumpnotok: 1;
        bool specon: 1, specoff: 1;
        bool asapon: 1, asapoff: 1;
        bool asap_dockon: 1, asap_dockoff: 1;
        bool asap_dock_avail: 1;
        bool dock: 1;
        bool dock_avail: 1;
        bool lock: 1, missilelock: 1;
        bool eject: 1;
        bool flightcompon: 1, flightcompoff: 1;
        bool warpready: 1, warpunready: 1;

        LastState();
    } last;

    /// Used to display the arrow pointing to the currently selected target.
    float projection_limit_x, projection_limit_y;
    float inv_screen_aspect_ratio; //Precomputed division 1 / g_game.aspect.
    virtual void SetParent(Unit *unit, const char *filename, const char *unitmodname, const QVector &startloc);
    void LoadXML(const char *file);
    void LoadXML(VSFileSystem::VSFile &f);
    void beginElement(const string &name, const AttributeList &attributes);
    void endElement(const string &name);
    ///Destructs cockpit info for new loading
    void Delete();
    ///Trigger scripted events
    void TriggerEvents(Unit *un);
    NavigationSystem ThisNav;
    void updateRadar(Unit *un);
public:
    std::string textMessage;
    bool editingTextMessage;
    std::string autoMessage;
    float autoMessageTime;
    virtual void setTargetLabel(const string &msg);
    virtual string getTargetLabel();
    void ReceivedTargetInfo();
    static void NavScreen(const KBData &, KBSTATE k); //scheherazade
    static string getsoundending(int which = 0);
    static string getsoundfile(string filename);
    void InitStatic();
    void Shake(float amt, int level /*0= shield 1=armor 2=hull*/ );
    int Autopilot(Unit *target);
    ///Restores the view from the IDentity Matrix needed to draw sprites
    void RestoreViewPort();
    GameCockpit(const char *file, Unit *parent, const std::string &pilotname);
    ~GameCockpit();
    ///Looks up a particular Gauge stat on target unit
    float LookupTargetStat(int stat, Unit *target);
    ///Looks up a particular Gauge stat on unit
    float LookupUnitStat(int stat, Unit *target);
    ///Loads cockpit info...just as constructor
    void Init(const char *file);
    ///Draws Cockpit then restores viewport
    void Draw();
    //void Update();//respawns and the like.
    void UpdAutoPilot();
    ///Sets up the world for rendering...call before draw
    void SetupViewPort(bool clip = true);
    int getVDUMode(int vdunum);
    void VDUSwitch(int vdunum);
    void ScrollVDU(int vdunum, int howmuch);
    void ScrollAllVDU(int howmuch);
    int getScrollOffset(unsigned int whichtype);
    void SelectProperCamera();
    void Eject();
    void EjectDock();
    static void Respawn(const KBData &, KBSTATE);
    static void SwitchControl(const KBData &, KBSTATE);
    static void ForceSwitchControl(const KBData &, KBSTATE);
    static void TurretControl(const KBData &, KBSTATE);
    void SetSoundFile(std::string sound);

    int GetSoundFile() {
        return soundfile;
    }

    void SetCommAnimation(Animation *ani, Unit *un);
    void SetStaticAnimation();

    ///Accesses the current navigationsystem
    NavigationSystem *AccessNavSystem() {
        return &ThisNav;
    }

    virtual std::string GetNavSelectedSystem();

    ///Accesses the current camera
    Camera *AccessCamera() {
        return &cam[currentcamera];
    }

    ///Returns the passed in cam
    Camera *AccessCamera(int);
    ///Changes current camera to selected camera
    void SelectCamera(int);

    ///GFXLoadMatrix proper camera
    void SetViewport() {
        cam[currentcamera].UpdateGFX();
    }

    virtual bool SetDrawNavSystem(bool);
    virtual bool CanDrawNavSystem();
    virtual bool CheckCommAnimation(Unit *un);
    virtual void visitSystem(std::string systemName);
    void AutoLanding();
    void DoAutoLanding(Unit *, Unit *);

    virtual void SetInsidePanYawSpeed(float speed);
    virtual void SetInsidePanPitchSpeed(float speed);

    bool IsPaused() const;
    // Game is paused
    void OnPauseBegin();
    void OnPauseEnd();
    // Ship has undocked from station
    void OnDockEnd(Unit *station, Unit *unit);
    // Ship is jumping
    void OnJumpBegin(Unit *unit);
    void OnJumpEnd(Unit *unit);

protected:
    /// Override to use a specific kind of sound implementation
    SoundContainer *soundImpl(const SoundContainer &specs);
};

#endif //VEGA_STRIKE_ENGINE_GFX_COCKPIT_H
