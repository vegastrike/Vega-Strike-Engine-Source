/**
 * navscreen.h
 *
 * Copyright (C) 2003-2020 hellcatv, ace123, surfdargent, klaussfreire, jacks,
 * pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef _NAVSCREEN_H_
#define _NAVSCREEN_H_

#include "gui/glut_support.h"
#include "navscreenoccupied.h"
#include "drawlist.h"
#include "navitemtypes.h"
#include "gfx/masks.h"
#include "navcomputer.h"
#include "navpath.h"
#include "gfx/hud.h"
#include "gnuhash.h"

#include <map>

#define NAVTOTALMESHCOUNT 8     //same as the button count, 1 mesh for screen and 1 per button(1+7)
#define MAXZOOM 10

void Beautify(string systemfile, string &sector, string &system);
class NavigationSystem {
public:
    class SystemIterator {
        vector<string> vstack;
        unsigned int which;
        unsigned int count;
        unsigned int maxcount;
        vsUMap<string, bool> visited;

    public:
        SystemIterator(string current_system, unsigned int max = 2);
        bool done() const;
        QVector Position();
        string operator*();
        SystemIterator &next();
        SystemIterator &operator++();
    };

    class CachedSystemIterator {
    public:
//typedef std::pair<string, QVector> SystemInfo;
        struct SystemInfo {
            string name;
            QVector position;
            std::vector<unsigned> lowerdestinations;
            GFXColor col;
            bool part_of_path;
            std::set<NavPath *> paths;
            void UpdateColor();
            string &GetName();
            const string &GetName() const;
            bool isDrawable() const;
            QVector &Position();
            const QVector &Position() const;
            unsigned GetDestinationIndex(unsigned index) const;
            unsigned GetDestinationSize() const;
            GFXColor GetColor();
            SystemInfo(const string &name);
            SystemInfo(const string &name,
                    const QVector &position,
                    const std::vector<std::string> &destinations,
                    CachedSystemIterator *csi);
            void loadData(std::map<string, unsigned> *index_table);
        };

    private:
        friend struct SystemInfo;        //inner class needs to be friend in gcc-295
        vector<SystemInfo> systems;
        unsigned currentPosition;
        CachedSystemIterator(const CachedSystemIterator &other);       //May be really slow. Don't try this at home.
        CachedSystemIterator operator++(int);         //Also really slow because it has to use the copy constructor.

    public:
        CachedSystemIterator();
        CachedSystemIterator(string current_system, unsigned max_systems = 2);
        void init(string current_system, unsigned max_systems = 2);
        bool seek(unsigned position = 0);
        unsigned getIndex() const;
        unsigned size() const;
        bool done() const;
        SystemInfo &operator[](unsigned pos);
        const SystemInfo &operator[](unsigned pos) const;
        SystemInfo &operator*();
        const SystemInfo &operator*() const;
        SystemInfo *operator->();
        const SystemInfo *operator->() const;
        CachedSystemIterator &next();
        CachedSystemIterator &operator++();
    };

    class CachedSectorIterator {
    public:
        class SectorInfo {
        public:
            string name;
            std::vector<unsigned> subsystems;
            string &GetName();
            const string &GetName() const;
            unsigned GetSubsystemIndex(unsigned index) const;
            unsigned GetSubsystemSize() const;
            SectorInfo(const string &name);
            void AddSystem(unsigned index);
        };

    private:
        friend class SectorInfo;  //inner class needs to be friend in gcc-295
        vector<SectorInfo> sectors;
        unsigned currentPosition;
        CachedSectorIterator(const CachedSectorIterator &other);       //May be really slow. Don't try this at home.
        CachedSectorIterator operator++(int);         //Also really slow because it has to use the copy constructor.

    public:
        CachedSectorIterator();
        CachedSectorIterator(CachedSystemIterator &systemIter);
        void init(CachedSystemIterator &systemIter);
        bool seek(unsigned position = 0);
        unsigned getIndex() const;
        unsigned size() const;
        bool done() const;
        SectorInfo &operator[](unsigned pos);
        const SectorInfo &operator[](unsigned pos) const;
        SectorInfo &operator*();
        const SectorInfo &operator*() const;
        SectorInfo *operator->();
        const SectorInfo *operator->() const;
        CachedSectorIterator &next();
        CachedSectorIterator &operator++();
    };

    PathManager *pathman;
private:
    friend class NavComputer;
    friend class CurrentPathNode;
    friend class TargetPathNode;
    friend class AbsolutePathNode;
    friend class CriteriaContains;
    friend class CriteriaOwnedBy;
    friend class CriteriaSector;
    friend class NavPath;
    NavComputer *navcomp;
    unsigned currentsystemindex;
    unsigned focusedsystemindex;
    unsigned destinationsystemindex;
    unsigned systemselectionindex;
    unsigned sectorselectionindex;
    CachedSystemIterator systemIter;
    CachedSectorIterator sectorIter;
    std::vector<unsigned> path;
    class navscreenoccupied *screenoccupation;
    class Mesh *mesh[NAVTOTALMESHCOUNT];
    int reverse;
    int rotations; //tried to change to unsigned but gazillions of comparisons to int crop up --chuck_starchaser
    int axis;
    int configmode;

    float rx;   //galaxy
    float ry;
    float rz;
    float zoom;

    float rx_s; //system
    float ry_s;
    float rz_s;
    float zoom_s;

    float camera_z;
    float themaxvalue;
    float zshiftmultiplier;
    float item_zscalefactor;
    float minimumitemscaledown;
    float maximumitemscaleup;

    enum ViewType { VIEW_2D, VIEW_ORTHO, VIEW_3D, VIEW_MAX };
    int system_view;
    int galaxy_view;

    int path_view;
    enum PathType { PATH_OFF, PATH_ON, PATH_ONLY, PATH_MAXIMUM };

    bool system_multi_dimensional;
    bool galaxy_multi_dimensional;

    float center_x;
    float center_y;
    float center_z;

    signed int scrolloffset;

    float mouse_x_previous;
    float mouse_y_previous;
    float mouse_x_current;
    float mouse_y_current;
    signed char draw;
    bool mouse_previous_state[5];
    bool mouse_wentup[5];
    bool mouse_wentdown[5];
    UnitContainer currentselection;
    GFXColor *factioncolours;
    GFXColor currentcol;
    GFXColor destinationcol;
    GFXColor selectcol;
    GFXColor pathcol;

//DrawSectorList's scroll control variables
    unsigned sectorOffset;
    unsigned systemOffset;

    int whattodraw;
//bit 0 = undefined
//bit	1 = draw system screen / mission screen
//bit 2 = draw galaxy/system ship/mission screen
//bit 3 = draw sector list screen in mission mode

//coordinates done 'over left->right' by 'up bottom->top'
//values are 1/100 of the screen width and height
    float screenskipby4[4]; //0 = x-small	1 = x-large	2 = y-small	3 = y-large
    float buttonskipby4_1[4];
    float buttonskipby4_2[4];
    float buttonskipby4_3[4];
    float buttonskipby4_4[4];
    float buttonskipby4_5[4];
    float buttonskipby4_6[4];
    float buttonskipby4_7[4];
    TextPlane screen_objectives;
    float meshcoordinate_x[NAVTOTALMESHCOUNT];
    float meshcoordinate_y[NAVTOTALMESHCOUNT];
    float meshcoordinate_z[NAVTOTALMESHCOUNT];
    float meshcoordinate_z_delta[NAVTOTALMESHCOUNT];

    int buttonstates;   //bit0 = button1, bit1 = button2, etc
    float system_item_scale;
    float unselectedalpha;

//Drawing helper functions
//*************************
    void Adjust3dTransformation(bool three_d, bool is_system_not_galaxy);
    void ReplaceAxes(QVector &pos);
    void RecordMinAndMax(const QVector &pos,
            float &min_x,
            float &max_x,
            float &min_y,
            float &max_y,
            float &min_z,
            float &max_z,
            float &max_all);
    void DrawOriginOrientationTri(float center_nav_x, float center_nav_y, bool system_not_galaxy);

    float CalculatePerspectiveAdjustment(float &zscale,
            float &zdistance,
            QVector &pos,
            QVector &pos_flat,
            float &system_item_scale_temp,
            bool system_not_galaxy);

    void TranslateCoordinates(QVector &pos,
            QVector &pos_flat,
            float center_nav_x,
            float center_nav_y,
            float themaxvalue,
            float &zscale,
            float &zdistance,
            float &the_x,
            float &the_y,
            float &the_x_flat,
            float &the_y_flat,
            float &system_item_scale_temp,
            bool system_not_galaxy);

    void TranslateAndDisplay(QVector &pos,
            QVector &pos_flat,
            float center_nav_x,
            float center_nav_y,
            float themaxvalue,
            float &zscale,
            float &zdistance,
            float &the_x,
            float &the_y,
            float &system_item_scale_temp,
            bool system_not_galaxy);

    void DisplayOrientationLines(float the_x, float the_y, float the_x_flat, float the_y_flat, bool system_not_galaxy);

    bool CheckForSelectionQuery();
    void setCurrentSystemIndex(unsigned newSystemIndex);
    void setFocusedSystemIndex(unsigned newSystemIndex);
    void setDestinationSystemIndex(unsigned newSystemIndex);

    bool BFS(unsigned originIndex, unsigned destIndex);
    bool DoubleRootedBFS(unsigned originIndex, unsigned destIndex);
//*************************

public:
    NavigationSystem();
    ~NavigationSystem();
    static void DrawCircle(float x, float y, float size, const GFXColor &col);
    static void DrawHalfCircleTop(float x, float y, float size, const GFXColor &col);
    static void DrawHalfCircleBottom(float x, float y, float size, const GFXColor &col);
    static void DrawPlanet(float x, float y, float size, const GFXColor &col);
    static void DrawStation(float x, float y, float size, const GFXColor &col);
    static void DrawJump(float x, float y, float size, const GFXColor &col);
    static void DrawMissile(float x, float y, float size, const GFXColor &col);
    static void DrawTargetCorners(float x, float y, float size, const GFXColor &col);
    static void DrawNavCircle(float x, float y, float rot_x, float rot_y, float size, const GFXColor &col);
    void setCurrentSystem(string newSystem);
    std::string getCurrentSystem();
    std::string getSelectedSystem();
    std::string getFocusedSystem();
    std::string getDestinationSystem();

    void DrawButton(float &x1, float &x2, float &y1, float &y2, int button_number, bool outline);
    void DrawButtonOutline(float &x1, float &x2, float &y1, float &y2, const GFXColor &col);
    void DrawCursor(float x, float y, float wid, float hei, const GFXColor &col);
    void DrawGrid(float &screen_x1, float &screen_x2, float &screen_y1, float &screen_y2, const GFXColor &col);

    bool TestIfInRange(float &x1, float &x2, float &y1, float &y2, float tx, float ty);
    bool TestIfInRangeBlk(float &x1, float &x2, float size, float tx, float ty);
    bool TestIfInRangeRad(float &x, float &y, float size, float tx, float ty);
    bool ParseFile(string filename);
    bool CheckDraw();
    void DrawSystem();
    void DrawGalaxy();
    void DrawMission();
    void DrawShip();
    void DrawSectorList();
    void DrawObjectives();
    void SetMouseFlipStatus();
    void ScreenToCoord(float &x);
    void IntersectBorder(float &x, float &y, const float &x1, const float &y1) const;
    void Draw();
    void Setup();
    void SetDraw(bool n);
    void ClearPriorities();
    void updatePath();

    void scroll(signed int scrollamt) {
        scrolloffset += scrollamt;
    }

    static int mousex;
    static int mousey;
    static int mousestat;
    static void mouseDrag(int x, int y);
    static void mouseMotion(int x, int y);
    static void mouseClick(int button, int state, int x, int y);

    static int getMouseButtonStatus() {
        return mousestat;
    }

    static QVector dxyz(QVector, double x_, double y_, double z_);

//float Delta(float a, float b);
};

#endif

