/*
 * cockpit_generic.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
#ifndef VEGA_STRIKE_ENGINE_GFX_COCKPIT_GENERIC_H
#define VEGA_STRIKE_ENGINE_GFX_COCKPIT_GENERIC_H

#include "root_generic/xml_support.h"
#include "cmd/container.h"
#include <vector>
#include "gfx_generic/vec.h"
#include "root_generic/vsfilesystem.h"
//using namespace XMLSupport;  DONT DO THIS IN HEADERS
enum VIEWSTYLE {
    CP_FRONT,
    CP_BACK,
    CP_LEFT,
    CP_RIGHT,
    CP_VIEWTARGET,
    CP_PANINSIDE,
    CP_CHASE,
    CP_PAN,
    CP_PANTARGET,
    CP_TARGET,
    CP_FIXED,
    CP_FIXEDPOS,
    CP_FIXEDPOSTARGET,
    CP_NUMVIEWS
};
#ifdef NETCOMM_WEBCAM
#define MAXVDUS (13)
#else
#define MAXVDUS (12)
#endif
class Unit;
class Camera;
class Animation;
class NavigationSystem;
namespace Radar {
class Sensor;
}
#include "src/in.h"
#include "cmd/images.h"
#include "gfx_generic/soundcontainer_generic.h"
/**
 * The Cockpit Contains all displayable information about a particular Unit *
 * Gauges are used to indicate analog controls, and some diagital ones
 * The ones starting from KPS are digital with text readout
 */
class Cockpit {
public:
    ///cockpit events.
    enum EVENTID {
        EVENTID_FIRST,

        /// Warp is possible at this time
        WARP_READY = EVENTID_FIRST,

        /// Warp ceased to be possible at this time
        WARP_UNREADY,

        /// Warp started
        WARP_ENGAGED,

        /// Warp stopped
        WARP_DISENGAGED,

        /// Asap autopilot engaged
        ASAP_ENGAGED,

        /// Asap autopilot disengaged
        ASAP_DISENGAGED,

        /// Asap docking available
        ASAP_DOCKING_AVAILABLE,

        /// Asap docking engaged
        ASAP_DOCKING_ENGAGED,

        /// Asap docking disengaged
        ASAP_DOCKING_DISENGAGED,

        /// Docking ready
        DOCK_AVAILABLE,

        /// Docking no longer ready
        DOCK_UNAVAILABLE,

        /// Docking triggered yet not ready
        DOCK_FAILED,

        /// Jump ready
        JUMP_AVAILABLE,

        /// Jump no longer ready
        JUMP_UNAVAILABLE,

        /// Jump triggered yet not ready
        JUMP_FAILED,

        /// Weapon lock warning active
        LOCK_WARNING,

        /// Missile lock warning active
        MISSILELOCK_WARNING,

        /// Eject light
        EJECT_WARNING,

        /// Governor enabled
        FLIGHT_COMPUTER_ENABLED,

        /// Governor disabled
        FLIGHT_COMPUTER_DISABLED,

        /// Warp loop, warp speed 0 (+1 = warp 1, +2 = warp 2, etc...)
        WARP_LOOP0,
        WARP_LOOP9 = WARP_LOOP0 + 9,
        /// Last warp level
        WRAP_LOOPLAST = WARP_LOOP9,

        /// Warp threshold, warp speed 0 (+1 = warp 1, +2 = warp 2, etc...)
        WARP_SKIP0,
        WARP_SKIP9 = WARP_SKIP0 + 9,
        /// Last warp level
        WRAP_SKIPLAST = WARP_SKIP9,

        /// Just after all valid values
        NUM_EVENTS
    };

protected:
    ///style of current view (chase cam, inside)
    enum VIEWSTYLE view;

    int currentcamera;
    float radar_time;
    float gauge_time[UnitImages<void>::NUMGAUGES];

    ///this is the parent that Cockpit will read data from
    UnitContainer parent;
    UnitContainer parentturret;
    int unitfaction;

    ///4 views f/r/l/b
    float shakin;

    ///Video Display Units (may need more than 2 in future)
    std::string unitmodname;

    ///Color of cockpit default text
    ///The font that the entire cockpit will use. Currently without color
    //Gauge *gauges[UnitImages::NUMGAUGES];
    //0 means no autopilot...positive autopilto in progress
    float autopilot_time;
    UnitContainer autopilot_target; //usually null

    /**
     * two values that represent the adjustment to perspective needed to center teh crosshairs in the perceived view.
     */
    float cockpit_offset, viewport_offset;

    virtual void LoadXML(const char *file) {
    }

    virtual void LoadXML(VSFileSystem::VSFile &f) {
    }

    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);

    virtual void beginElement(const std::string &name, const XMLSupport::AttributeList &attributes) {
    }

    virtual void endElement(const std::string &name) {
    }

    ///Destructs cockpit info for new loading
    virtual void Delete();

    ///draws the navigation symbol around targetted location
    virtual void DrawNavigationSymbol(const Vector &loc, const Vector &p, const Vector &q, float size) {
    }

    ///draws the target box around targetted unit
    virtual float computeLockingSymbol(Unit *par) {
        return 1;
    }

    float cockpit_time;
    bool ejecting;
    bool going_to_dock_screen;
    int partial_number_of_attackers;

private:
    std::vector<std::string> unitfilename;
    std::vector<std::string> unitsystemname;
    std::vector<std::string> unitbasename;
    std::vector<SoundContainer *> sounds;

public:
    int number_of_attackers;
    unsigned int retry_dock;
    double TimeOfLastCollision;
    char jumpok;

    virtual void setTargetLabel(const std::string &msg) {
    }

    virtual std::string getTargetLabel() {
        return std::string();
    }

    void updateAttackers();
    static bool tooManyAttackers(); //checks config file and declares if too many folks are attacking any of the players (to avoid expensive tests where unnecessary).
    virtual void ReceivedTargetInfo() {
    }

    bool autoInProgress() {
        return autopilot_time > 0;
    }

    bool unitInAutoRegion(Unit *un);

    ///Sets the current viewstyle
    void SetView(const enum VIEWSTYLE tmp) {
        view = tmp;
    }

    enum VIEWSTYLE GetView() {
        return view;
    }

    virtual void InitStatic();

    virtual void Shake(float amt, int level /*0 = shield, 1 = armor 2 = hull*/ ) {
    }

    float godliness;
    virtual int Autopilot(Unit *target);
    void RestoreGodliness();

    ///Restores the view from the IDentity Matrix needed to draw sprites
    virtual void RestoreViewPort() {
    }

    std::string &GetUnitFileName(unsigned int which = 0) {
        while (which >= unitfilename.size()) {
            unitfilename.push_back("");
        }
        return unitfilename[which];
    }

    std::string &GetUnitSystemName(unsigned int which = 0) {
        while (which >= unitsystemname.size()) {
            unitsystemname.push_back("");
        }
        return unitsystemname[which];
    }

    std::string &GetUnitBaseName(unsigned int which = 0) {
        while (which >= unitbasename.size()) {
            unitbasename.push_back("");
        }
        return unitbasename[which];
    }

    const std::string &GetUnitFileName(unsigned int which = 0) const;
    const std::string &GetUnitSystemName(unsigned int which = 0) const;
    const std::string &GetUnitBaseName(unsigned int which = 0) const;

    void RemoveUnit(unsigned int which = 0);

    static std::string MakeBaseName(const Unit *base);

    std::string GetUnitModifications() {
        return unitmodname;
    }

    size_t GetNumUnits() const {
        return unitfilename.size();
    }

    void PackUnitInfo(vector<std::string> &info) const;

    void UnpackUnitInfo(vector<std::string> &info);

    std::string communication_choices;
    float credits; //how much money player has

    ///How far away chasecam and pan cam is
    float zoomfactor;
    Cockpit(const char *file, Unit *parent, const std::string &pilotname);
    virtual ~Cockpit();

    ///Loads cockpit info...just as constructor
    virtual void Init(const char *file, bool defaultCockpit = false);
    void recreate(const std::string &pilotname);

    ///Sets owner of this cockpit
    //unsigned int whichcockpit;//0 is the first player, 1 is the second and so forth
    class Flightgroup *fg;
    class StarSystem *activeStarSystem; //used for context switch in Universe
    virtual void SetParent(Unit *unit, const char *filename, const char *unitmodname, const QVector &startloc);

    Unit *GetParent() {
        return parent.GetUnit();
    }

    Unit *GetSaveParent();

    ///Draws Cockpit then restores viewport
    virtual void Draw() {
    }

    bool Update(); //respawns and the like. Returns true if starsystem was reloaded
    virtual void UpdAutoPilot();

    ///Sets up the world for rendering...call before draw
    virtual void SetupViewPort(bool clip = true) {
    }

    virtual int getVDUMode(int vdunum) {
        return 0;
    }

    virtual void VDUSwitch(int vdunum) {
    }

    virtual void ScrollVDU(int vdunum, int howmuch) {
    }

    virtual void ScrollAllVDU(int howmuch) {
    }

    virtual int getScrollOffset(unsigned int whichtype) {
        return 1;
    }

    virtual void SelectProperCamera() {
    }

    virtual void Eject();
    virtual void EjectDock();
    virtual void SetInsidePanYawSpeed(float speed);
    virtual void SetInsidePanPitchSpeed(float speed);

    static void Respawn(const KBData &, KBSTATE) {
    }

    static void SwitchControl(const KBData &, KBSTATE) {
    }

    static void ForceSwitchControl(const KBData &, KBSTATE) {
    }

    static void TurretControl(const KBData &, KBSTATE) {
    }

    virtual void SetCommAnimation(Animation *ani, Unit *un) {
    }

    virtual void SetStaticAnimation() {
    }

    /**
     * Retrieves the sound associated to the given event.
     * Returns NULL if no sound has been associated
     */
    SoundContainer *GetSoundForEvent(EVENTID eventId) const;

    /**
     * Sets the sound associated to the given event to match the given specs
     * (the actual container will be a different, implementation-specific one)
     */
    void SetSoundForEvent(EVENTID eventId, const SoundContainer &soundSpecs);

    class SaveGame *savegame;

    ///Accesses the current navigationsystem
    virtual NavigationSystem *AccessNavSystem() {
        return NULL;
    }

    virtual std::string GetNavSelectedSystem() {
        return "";
    }

    ///Accesses the current camera
    virtual Camera *AccessCamera() {
        return NULL;
    }

    ///Returns the passed in cam
    virtual Camera *AccessCamera(int) {
        return NULL;
    }

    ///Changes current camera to selected camera
    virtual void SelectCamera(int) {
    }

    ///GFXLoadMatrix proper camera
    virtual void SetViewport() {
    }

    virtual void visitSystem(std::string systemName);

    virtual bool SetDrawNavSystem(bool) {
        return false;
    }

    virtual bool CanDrawNavSystem() {
        return false;
    }

    virtual bool CheckCommAnimation(Unit *un) {
        return false;
    }

    // Cockpit events
    virtual void OnPauseBegin() {
    }

    virtual void OnPauseEnd() {
    }

    virtual void OnDockEnd(Unit *, Unit *) {
    }

    virtual void OnJumpBegin(Unit *) {
    }

    virtual void OnJumpEnd(Unit *) {
    }

protected:
    /// Override to use a specific kind of sound implementation
    virtual SoundContainer *soundImpl(const SoundContainer &specs);
};

#endif //VEGA_STRIKE_ENGINE_GFX_COCKPIT_GENERIC_H
