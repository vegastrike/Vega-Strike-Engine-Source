/**
 * base.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_CMD_BASE_H
#define VEGA_STRIKE_ENGINE_CMD_BASE_H

#include <vector>
#include <string>
#include "basecomputer.h"
#include "gfx/hud.h"
#include "gfx/sprite.h"
#include <stdio.h>
#include "gui/glut_support.h"

#include "audio/Types.h"
#include "audio/Source.h"

//#define BASE_MAKER
//#define BASE_XML //in case you want to write out XML instead...

#define BASE_EXTENSION ".py"

void RunPython(const char *filnam);

class BaseInterface {
    int curlinkindex;
    int lastmouseindex;    //Last link index to be under the mouse
    MousePointerStyle mousePointerStyle;
    bool enabledj;
    bool terminate_scheduled;
    bool midloop;
    TextPlane curtext;
public:
    class Room {
    public:
        class Link {
        public:
            enum {
                ClickEvent = (1 << 0),
                DownEvent = (1 << 1),
                UpEvent = (1 << 2),
                MoveEvent = (1 << 3),
                EnterEvent = (1 << 4),
                LeaveEvent = (1 << 5)
            };

            std::string pythonfile;
            float x, y, wid, hei, alpha;
            std::string text;
            const std::string index;
            unsigned int eventMask;
            int clickbtn;

            virtual void Click(::BaseInterface *base, float x, float y, int button, int state);
            virtual void MouseMove(::BaseInterface *base, float x, float y, int buttonmask);
            virtual void MouseEnter(::BaseInterface *base, float x, float y, int buttonmask);
            virtual void MouseLeave(::BaseInterface *base, float x, float y, int buttonmask);

            void setEventMask(unsigned int mask) {
                eventMask = mask;
            }

            explicit Link(const std::string &ind, const std::string &pfile)
                    : pythonfile(pfile), alpha(1.0f), index(ind), eventMask(ClickEvent), clickbtn(-1) {
            }

            virtual ~Link() {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
            virtual void Relink(const std::string &pfile);
        };
        class Goto : public Link {
        public:
            int index;
            virtual void Click(::BaseInterface *base, float x, float y, int button, int state);

            virtual ~Goto() {
            }

            explicit Goto(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
        };
        class Comp : public Link {
        public:
            vector<BaseComputer::DisplayMode> modes;
            virtual void Click(::BaseInterface *base, float x, float y, int button, int state);

            virtual ~Comp() {
            }

            explicit Comp(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
        };
        class Launch : public Link {
        public:
            virtual void Click(::BaseInterface *base, float x, float y, int button, int state);

            virtual ~Launch() {
            }

            explicit Launch(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
        };
        class Eject : public Link {
        public:
            virtual void Click(::BaseInterface *base, float x, float y, int button, int state);

            virtual ~Eject() {
            }

            explicit Eject(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
        };
        class Talk : public Link {
        public:
//At the moment, the BaseInterface::Room::Talk class is unused... but I may find a use for it later...
            std::vector<std::string> say;
            std::vector<std::string> soundfiles;
            int index;
            int curroom;
            virtual void Click(::BaseInterface *base, float x, float y, int button, int state);
            explicit Talk(const std::string &ind, const std::string &pythonfile);

            virtual ~Talk() {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
        };
        class Python : public Link {
        public:
            Python(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {
            }

            virtual ~Python() {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif
        };
        class BaseObj {
        public:
            const std::string index;
            virtual void Draw(::BaseInterface *base);
#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif

            virtual ~BaseObj() {
            }

            explicit BaseObj(const std::string &ind) : index(ind) {
            }
        };
        class BasePython : public BaseObj {
        public:
            std::string pythonfile;
            float timeleft;
            float maxtime;
            virtual void Draw(::BaseInterface *base);
#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif

            virtual ~BasePython() {
            }

            BasePython(const std::string &ind, const std::string &python, float time)
                    : BaseObj(ind), pythonfile(python), timeleft(0), maxtime(time) {
            }

            virtual void Relink(const std::string &python);
        };
        class BaseText : public BaseObj {
        public:
            TextPlane text;
            virtual void Draw(::BaseInterface *base);
#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif

            virtual ~BaseText() {
            }

            BaseText(const std::string &texts,
                    float posx,
                    float posy,
                    float wid,
                    float hei,
                    float charsizemult,
                    GFXColor backcol,
                    GFXColor forecol,
                    const std::string &ind) : BaseObj(ind), text(forecol, backcol) {
                text.SetPos(posx, posy);
                text.SetSize(wid, hei);
                float cx = 0, cy = 0;
                text.GetCharSize(cx, cy);
                cx *= charsizemult;
                cy *= charsizemult;
                text.SetCharSize(cx, cy);
                text.SetText(texts);
            }

            void SetText(const std::string &newtext) {
                text.SetText(newtext);
            }

            void SetPos(float posx, float posy) {
                text.SetPos(posx, posy);
            }

            void SetSize(float wid, float hei) {
                text.SetSize(wid, hei);
            }
        };
        class BaseShip : public BaseObj {
        public:
            virtual void Draw(::BaseInterface *base);
            Matrix mat;

            virtual ~BaseShip() {
            }

#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp );
#endif

            explicit BaseShip(const std::string &ind) : BaseObj(ind) {
            }

            BaseShip(float r0,
                    float r1,
                    float r2,
                    float r3,
                    float r4,
                    float r5,
                    float r6,
                    float r7,
                    float r8,
                    QVector pos,
                    const std::string &ind) :
                    BaseObj(ind), mat(r0, r1, r2, r3, r4, r5, r6, r7, r8, QVector(pos.i / 2, pos.j / 2, pos.k)) {
            }
        };
        class BaseVSSprite : public BaseObj {
        public:
            virtual void Draw(::BaseInterface *base);
            VSSprite spr;
            SharedPtr<Audio::Source> soundsource;
            std::string soundscene;

#ifdef BASE_MAKER
            std::string  texfile;
            virtual void EndXML( FILE *fp );
#endif
            virtual ~BaseVSSprite();
            BaseVSSprite(const std::string &spritefile, const std::string &ind);
            void SetSprite(const std::string &spritefile);

            void SetPos(float posx, float posy) {
                spr.SetPosition(posx, posy);
            }

            void SetSize(float wid, float hei) {
                spr.SetSize(wid, hei);
            }

            void SetTime(float t) {
                spr.SetTime(t);
            }

            bool isPlaying() const;

        protected:
            BaseVSSprite(const std::string &ind, const VSSprite &sprite) : BaseObj(ind), spr(sprite) {
            }
        };

        class BaseVSMovie : public BaseVSSprite {
            std::string callback;
            bool playing;
            bool hidePointer;
            double hidePointerTime;

        public:
            virtual ~BaseVSMovie() {
            }

            BaseVSMovie(const std::string &moviefile, const std::string &ind);
            void SetMovie(const std::string &moviefile);

            float GetTime() const;
            void SetTime(float t);

            bool GetHidePointer() const {
                return hidePointer;
            };
            void SetHidePointer(bool hide);

            const std::string &getCallback() const {
                return callback;
            }

            void setCallback(const std::string &callback) {
                this->callback = callback;
            }

            virtual void Draw(::BaseInterface *base);
        };

        class BaseTalk : public BaseObj {
        public:
            static bool hastalked;
            virtual void Draw(::BaseInterface *base);
//Talk * caller;
            unsigned int curchar;
            float curtime;

            virtual ~BaseTalk() {
            }

            std::string message;
            BaseTalk(const std::string &msg, const std::string &ind, bool only_one_talk);
#ifdef BASE_MAKER
            virtual void EndXML( FILE *fp ) {}
#endif
        };
        std::string soundfile;
        std::string deftext;
        std::vector<Link *> links;
        std::vector<BaseObj *> objs;
#ifdef BASE_MAKER
        void EndXML( FILE *fp );
#endif
        void Draw(::BaseInterface *base);
        void Click(::BaseInterface *base, float x, float y, int button, int state);
        int MouseOver(::BaseInterface *base, float x, float y);
        Room();
        ~Room();
    };
    friend class Room;
    friend class Room::BaseTalk;
    int curroom;
    std::vector<Room *> rooms;
    TextPlane othtext;
    static BaseInterface *CurrentBase;
    bool CallComp;
    UnitContainer caller;
    UnitContainer baseun;

    std::string python_kbhandler;

#ifdef BASE_MAKER
    void EndXML( FILE *fp );
#endif
    void Terminate();
    void GotoLink(int linknum);
    void InitCallbacks();
    void CallCommonLinks(const std::string &name, const std::string &value);
    void Load(const char *filename, const char *time_of_day, const char *faction);
    static void ClickWin(int x, int y, int button, int state);
    void Click(int x, int y, int button, int state);
    void Key(unsigned int ch, unsigned int mod, bool release, int x, int y);
    static void PassiveMouseOverWin(int x, int y);
    static void ActiveMouseOverWin(int x, int y);
    static void ProcessKeyboardBuffer();
    void MouseOver(int x, int y);
    BaseInterface(const char *basefile, Unit *base, Unit *un);
    ~BaseInterface();
    void Draw();

    void setDJEnabled(bool enabled);

    bool isDJEnabled() const {
        return enabledj;
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_BASE_H
