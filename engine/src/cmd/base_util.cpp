/*
 * base_util.cpp
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


#define PY_SSIZE_T_CLEAN
//#include <Python.h>

#include <boost/version.hpp>
#include <boost/python.hpp>
#if BOOST_VERSION != 102800
typedef boost::python::dict BoostPythonDictionary;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary;
#endif

#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include "python/python_class.h"
#include <string>
#include <cstdlib>
#include <cassert>
#include "vega_cast_utils.h"
#include "audiolib.h"
#include "base.h"
#include "base_util.h"
#include "universe_util.h"
#include "basecomputer.h"
#include "main_loop.h"
#include "music.h"
#include "in_kb.h"

#include "audio/SceneManager.h"
#include "audio/Sound.h"
#include "audio/Source.h"
#include "audio/SourceListener.h"
#include "audio/Renderer.h"
#include "audio/Scene.h"

#include "configxml.h"
#include "vs_logging.h"

extern float getFontHeight();

using Audio::Source;
using Audio::Sound;
using Audio::SceneManager;
using Audio::SourceListener;
using Audio::LVector3;
using Audio::Vector3;

using namespace XMLSupport;
namespace BaseUtil {
inline BaseInterface::Room *CheckRoom(int room) {
    if (!BaseInterface::CurrentBase) {
        return 0;
    }
    if (room < 0 || room >= static_cast<int>(BaseInterface::CurrentBase->rooms.size())) {
        return 0;
    }
    return BaseInterface::CurrentBase->rooms[room];
}

/*
 * Ad-hoc listener used to trigger end-of-stream behavior
 */

class VideoAudioStreamListener : public SourceListener {
    int sourceRoom;
    std::string index;

public:
    VideoAudioStreamListener(int sourceRoom, const std::string &index) {
        // Just play events
        events.attach =
                events.update = 0;
        events.play = 1;

        this->sourceRoom = sourceRoom;
        this->index = index;
    }

    virtual void onPreAttach(Source &source, bool detach) {
    };

    virtual void onPostAttach(Source &source, bool detach) {
    };

    virtual void onPrePlay(Source &source, bool stop) {
    };

    virtual void onPostPlay(Source &source, bool stop) {
    };

    virtual void onUpdate(Source &source, int updateFlags) {
    };

    virtual void onEndOfStream(Source &source) {
        // Verify context before switching rooms
        if (BaseInterface::CurrentBase != NULL) {
            if (BaseUtil::GetCurRoom() == sourceRoom) {
                // We're in the right context, switch to target room
                BaseInterface::Room *room = CheckRoom(sourceRoom);

                if (!room) {
                    return;
                }

                for (size_t i = 0; i < room->objs.size(); i++) {
                    if (room->objs[i]) {
                        if (room->objs[i]->index == index) {
                            BaseInterface::Room::BaseVSMovie *movie =
                                    vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSMovie>(room->objs[i]);

                            if (!movie->getCallback().empty()) {
                                RunPython(movie->getCallback().c_str());
                                break;
                            }
                        }
                    }
                }
            }
        }
    };
};

int Room(std::string text) {
    if (!BaseInterface::CurrentBase) {
        return -1;
    }
    BaseInterface::CurrentBase->rooms.push_back(new BaseInterface::Room());
    BaseInterface::CurrentBase->rooms.back()->deftext = text;
    return BaseInterface::CurrentBase->rooms.size() - 1;
}

    void Texture(int room, std::string index, std::string file, float x, float y) {
        VS_LOG_AND_FLUSH(debug, (boost::format("BaseUtil::Texture called. room: %1%; index: '%2%'; file: '%3%'; x: %4%; y: %5%") % room % index % file % x % y));
        BaseInterface::Room *newroom = CheckRoom(room);
        if (!newroom) {
            return;
        }
        newroom->objs.push_back(new BaseInterface::Room::BaseVSSprite(file, index));
        BaseInterface::Room::BaseVSSprite
                *p_base_vs_sprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>( newroom->objs.back());
#ifdef BASE_MAKER
        p_base_vs_sprite->texfile = file;
#endif
        float tx = 0, ty = 0;
        static bool
                addspritepos = XMLSupport::parse_bool(vs_config->getVariable("graphics", "offset_sprites_by_pos", "true"));
        if (addspritepos) {
            p_base_vs_sprite->spr.GetPosition(tx, ty);
        }
        p_base_vs_sprite->spr.SetPosition(x + tx, y + ty);
    }

SharedPtr<Source> CreateVideoSoundStream(const std::string &afile, const std::string &scene) {
    SharedPtr<Sound> sound = SceneManager::getSingleton()->getRenderer()->getSound(
            afile,
            VSFileSystem::VideoFile,
            true);

    SharedPtr<Source> source = SceneManager::getSingleton()->createSource(
            sound,
            false);

    source->setAttenuated(false);
    source->setRelative(true);
    source->setPosition(LVector3(0, 0, 1));
    source->setDirection(Vector3(0, 0, -1));
    source->setVelocity(Vector3(0, 0, 0));
    source->setRadius(1.0);
    source->setGain(1.0);

    SceneManager::getSingleton()->getScene(scene)->add(source);

    return source;
}

void DestroyVideoSoundStream(SharedPtr<Source> source, const std::string &scene) {
    if (source->isPlaying()) {
        source->stopPlaying();
    }
    SceneManager::getSingleton()->getScene(scene)->remove(source);
}

bool Video(int room, std::string index, std::string vfile, std::string afile, float x, float y) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return false;
    }
    BaseUtil::Texture(room, index, vfile, x, y);

    BaseInterface::Room::BaseVSSprite
            *baseSprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>( newroom->objs.back());

    if (!afile.empty()) {
        if (g_game.sound_enabled) {
            try {
                baseSprite->soundscene = "video";
                baseSprite->soundsource = CreateVideoSoundStream(afile, baseSprite->soundscene);
                baseSprite->spr.SetTimeSource(baseSprite->soundsource);
            } catch (const Audio::FileOpenException &e) {
                baseSprite->spr.Reset();
            } catch (const VidFile::FileOpenException &e) {
                baseSprite->spr.Reset();
            }
        } else {
            baseSprite->spr.Reset();
        }
    }

    return true;
}

bool VideoStream(int room, std::string index, std::string streamfile, float x, float y, float w, float h) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        VS_LOG(error, "ERROR: Room not found!!\n");
        return false;
    }

    BaseInterface::Room::BaseVSMovie *newobj = new BaseInterface::Room::BaseVSMovie(streamfile, index);
    newobj->SetPos(x, y);
    newobj->SetSize(w, h);

#ifdef BASE_MAKER
    newobj->texfile = file;
#endif

    if (newobj->spr.LoadSuccess()) {
        VS_LOG(info, (boost::format("INFO: Added video stream %1$s\n") % streamfile.c_str()));
        newroom->objs.push_back(newobj);
    } else {
        VS_LOG(info, (boost::format("INFO: Missing video stream %1$s\n") % streamfile.c_str()));
        delete newobj;
        return false;
    }

    return true;
}

void SetVideoCallback(int room, std::string index, std::string callback) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseVSMovie *movie =
                        vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSMovie>(newroom->objs[i]);
                movie->setCallback(callback);

                if (movie->soundsource.get() != NULL) {
                    SharedPtr<SourceListener> transitionListener(
                            new VideoAudioStreamListener(room, index));

                    movie->soundsource->setSourceListener(transitionListener);
                }
            }
        }
    }
}

void SetTexture(int room, std::string index, std::string file) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseVSSprite
                        *p_base_vs_sprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>(newroom->objs[i]);
                p_base_vs_sprite->SetSprite(file);
            }
        }
    }
}

void SetTextureSize(int room, std::string index, float w, float h) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseVSSprite
                        *p_base_vs_sprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>(newroom->objs[i]);
                p_base_vs_sprite->SetSize(w, h);
            }
        }
    }
}

void SetTexturePos(int room, std::string index, float x, float y) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseVSSprite
                        *p_base_vs_sprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>(newroom->objs[i]);
                p_base_vs_sprite->SetPos(x, y);
            }
        }
    }
}

void PlayVideo(int room, std::string index) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseVSSprite
                        *p_base_vs_sprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>(newroom->objs[i]);
                SharedPtr<Source> source = p_base_vs_sprite->soundsource;
                assert(source && source.get() != nullptr);
                if (source && !source->isPlaying()) {
                    source->startPlaying();
                }
            }
        }
    }
}

void StopVideo(int room, std::string index) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseVSSprite
                        *p_base_vs_sprite = vega_dynamic_cast_ptr<BaseInterface::Room::BaseVSSprite>(newroom->objs[i]);
                SharedPtr<Source> source = p_base_vs_sprite->soundsource;
                if (source && source->isPlaying()) {
                    source->stopPlaying();
                }
            }
        }
    }
}

void SetDJEnabled(bool enabled) {
    BaseInterface::CurrentBase->setDJEnabled(enabled);
    if (!enabled) {
        Music::Stop();
    }
}

void Ship(int room, std::string index, QVector pos, Vector Q, Vector R) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    Vector P = R.Cross(Q);
    P.Normalize();
    newroom->objs.push_back(new BaseInterface::Room::BaseShip(P.i, P.j, P.k, Q.i, Q.j, Q.k, R.i, R.j, R.k, pos, index));
}

void RunScript(int room, std::string ind, std::string pythonfile, float time) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    newroom->objs.push_back(new BaseInterface::Room::BasePython(ind, pythonfile, time));
}

void TextBox(int room,
        std::string ind,
        std::string text,
        float x,
        float y,
        Vector widheimult,
        Vector backcol,
        float backalp,
        Vector forecol) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    newroom->objs.push_back(new BaseInterface::Room::BaseText(text, x, y, widheimult.i, widheimult.j, widheimult.k,
            GFXColor(backcol, backalp), GFXColor(forecol), ind));
}

void SetTextBoxText(int room, std::string index, std::string text) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                BaseInterface::Room::BaseText
                        *p_base_text = vega_dynamic_cast_ptr<BaseInterface::Room::BaseText>(newroom->objs[i]);
                p_base_text->SetText(text);
            }
        }
    }
}

void SetLinkArea(int room, std::string index, float x, float y, float wid, float hei) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->links.size(); i++) {
        if (newroom->links[i]) {
            if (newroom->links[i]->index == index) {
                newroom->links[i]->x = x;
                newroom->links[i]->y = y;
                newroom->links[i]->wid = wid;
                newroom->links[i]->hei = hei;
            }
        }
    }
}

void SetLinkText(int room, std::string index, std::string text) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->links.size(); i++) {
        if (newroom->links[i]) {
            if (newroom->links[i]->index == index) {
                newroom->links[i]->text = text;
            }
        }
    }
}

void SetLinkPython(int room, std::string index, std::string python) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->links.size(); i++) {
        if (newroom->links[i]) {
            if (newroom->links[i]->index == index) {
                newroom->links[i]->Relink(python);
            }
        }
    }
}

void SetLinkRoom(int room, std::string index, int to) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (size_t i = 0; i < newroom->links.size(); i++) {
        if (newroom->links[i]) {
            if (newroom->links[i]->index == index) {
                BaseInterface::Room::Goto *p_goto = vega_dynamic_cast_ptr<BaseInterface::Room::Goto>(newroom->links[i]);
                p_goto->index = to;
            }
        }
    }
}

void SetLinkEventMask(int room, std::string index, std::string maskdef) {
    size_t i;
    //c=click, u=up, d=down, e=enter, l=leave, m=move
    unsigned int mask = 0;
    for (i = 0; i < maskdef.length(); ++i) {
        switch (maskdef[i]) {
            case 'c':
            case 'C':
                mask |= BaseInterface::Room::Link::ClickEvent;
                break;
            case 'u':
            case 'U':
                mask |= BaseInterface::Room::Link::UpEvent;
                break;
            case 'd':
            case 'D':
                mask |= BaseInterface::Room::Link::DownEvent;
                break;
            case 'e':
            case 'E':
                mask |= BaseInterface::Room::Link::EnterEvent;
                break;
            case 'l':
            case 'L':
                mask |= BaseInterface::Room::Link::LeaveEvent;
                break;
            case 'm':
            case 'M':
                VS_LOG(warning,
                        (boost::format("%1$s: WARNING: Ignoring request for movement event mask.\n") % __FILE__));
                break;
        }
    }
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (i = 0; i < newroom->links.size(); i++) {
        BaseInterface::Room::Link *&p_link = newroom->links[i];
        if (p_link) {
            if (p_link->index == index) {
                p_link->setEventMask(mask);
            }
        }
    }
}

static void BaseLink(BaseInterface::Room *room,
        float x,
        float y,
        float wid,
        float hei,
        std::string text,
        bool reverse = false) {
    BaseInterface::Room::Link *lnk;
    if (reverse) {
        lnk = room->links.front();
    } else {
        lnk = room->links.back();
    }
    lnk->x = x;
    lnk->y = y;
    lnk->wid = wid;
    lnk->hei = hei;
    lnk->text = text;
}

void Link(int room, std::string index, float x, float y, float wid, float hei, std::string text, int to) {
    LinkPython(room, index, "", x, y, wid, hei, text, to);
}

void LinkPython(int room,
        std::string index,
        std::string pythonfile,
        float x,
        float y,
        float wid,
        float hei,
        std::string text,
        int to) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    newroom->links.push_back(new BaseInterface::Room::Goto(index, pythonfile));
    BaseLink(newroom, x, y, wid, hei, text);
    ((BaseInterface::Room::Goto *) newroom->links.back())->index = to;
}

void Launch(int room, std::string index, float x, float y, float wid, float hei, std::string text) {
    LaunchPython(room, index, "", x, y, wid, hei, text);
}

void LaunchPython(int room,
        std::string index,
        std::string pythonfile,
        float x,
        float y,
        float wid,
        float hei,
        std::string text) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    newroom->links.push_back(new BaseInterface::Room::Launch(index, pythonfile));
    BaseLink(newroom, x, y, wid, hei, text);
}

void EjectPython(int room,
        std::string index,
        std::string pythonfile,
        float x,
        float y,
        float wid,
        float hei,
        std::string text) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    newroom->links.push_back(new BaseInterface::Room::Eject(index, pythonfile));
    BaseLink(newroom, x, y, wid, hei, text);
}

void Comp(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes) {
    CompPython(room, index, "", x, y, wid, hei, text, modes);
}

void CompPython(int room,
        std::string index,
        std::string pythonfile,
        float x,
        float y,
        float wid,
        float hei,
        std::string text,
        std::string modes) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    BaseInterface::Room::Comp *newcomp = new BaseInterface::Room::Comp(index, pythonfile);
    newroom->links.push_back(newcomp);
    BaseLink(newroom, x, y, wid, hei, text);
    static const EnumMap::Pair modelist[] = {
            EnumMap::Pair("Cargo", BaseComputer::CARGO),
            EnumMap::Pair("Upgrade", BaseComputer::UPGRADE),
            EnumMap::Pair("ShipDealer", BaseComputer::SHIP_DEALER),
            EnumMap::Pair("Missions", BaseComputer::MISSIONS),
            EnumMap::Pair("News", BaseComputer::NEWS),
            EnumMap::Pair("Info", BaseComputer::INFO),
            EnumMap::Pair("LoadSave", BaseComputer::LOADSAVE),
            EnumMap::Pair("Network", BaseComputer::NETWORK),
            EnumMap::Pair("UNKNOWN", BaseComputer::LOADSAVE),
    };
    static const EnumMap modemap(modelist, sizeof(modelist) / sizeof(*modelist));
    const char *newmode = modes.c_str();
    int newlen = modes.size();
    char *curmode = new char[newlen + 1];
    for (int i = 0; i < newlen;) {
        int j;
        for (j = 0; newmode[i] != ' ' && newmode[i] != '\0'; i++, j++) {
            curmode[j] = newmode[i];
        }
        while (newmode[i] == ' ') {
            i++;
        }
        if (j == 0) {
            continue;
        }
        //in otherwords, if j is 0 then the 0th index will become null
        //EnumMap crashes if the string is empty.
        curmode[j] = '\0';
        int modearg = modemap.lookup(curmode);
        if (modearg < BaseComputer::DISPLAY_MODE_COUNT) {
            newcomp->modes.push_back((BaseComputer::DisplayMode) (modearg));
        } else {
            VS_LOG(warning,
                    (boost::format("WARNING: Unknown computer mode %1$s found in python script...\n") % curmode));
        }
    }
    delete[] curmode;
}

void Python(int room,
        std::string index,
        float x,
        float y,
        float wid,
        float hei,
        std::string text,
        std::string pythonfile,
        bool front) {
    //instead of "Talk"/"Say" tags
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    BaseInterface::Room::Python *tmp = new BaseInterface::Room::Python(index, pythonfile);
    if (front) {
        newroom->links.insert(newroom->links.begin(), tmp);
    } else {
        newroom->links.push_back(tmp);
    }
    BaseLink(newroom, x, y, wid, hei, text, front);
}

void GlobalKeyPython(std::string pythonfile) {
    if (BaseInterface::CurrentBase) {
        BaseInterface::CurrentBase->python_kbhandler = pythonfile;
    }
}

void MessageToRoom(int room, std::string text) {
    if (!BaseInterface::CurrentBase) {
        return;
    }
    BaseInterface::CurrentBase->rooms[room]->objs
            .push_back(new BaseInterface::Room::BaseTalk(text, "currentmsg", true));
}

void EnqueueMessageToRoom(int room, std::string text) {
    if (!BaseInterface::CurrentBase) {
        return;
    }
    BaseInterface::CurrentBase->rooms[room]->objs
            .push_back(new BaseInterface::Room::BaseTalk(text, "currentmsg", false));
}

void Message(std::string text) {
    if (!BaseInterface::CurrentBase) {
        return;
    }
    MessageToRoom(BaseInterface::CurrentBase->curroom, text);
}

void EnqueueMessage(std::string text) {
    if (!BaseInterface::CurrentBase) {
        return;
    }
    EnqueueMessageToRoom(BaseInterface::CurrentBase->curroom, text);
}

void EraseLink(int room, std::string index) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (int i = 0; i < (int) newroom->links.size(); i++) {
        if (newroom->links[i]) {
            if (newroom->links[i]->index == index) {
                newroom->links.erase(newroom->links.begin() + i);
                i--;
//break;
            }
        }
    }
}

void EraseObj(int room, std::string index) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    for (int i = 0; i < (int) newroom->objs.size(); i++) {
        if (newroom->objs[i]) {
            if (newroom->objs[i]->index == index) {
                newroom->objs.erase(newroom->objs.begin() + i);
                i--;
//break;
            }
        }
    }
}

int GetCurRoom() {
    if (!BaseInterface::CurrentBase) {
        return -1;
    }
    return BaseInterface::CurrentBase->curroom;
}

void SetCurRoom(int room) {
    BaseInterface::Room *newroom = CheckRoom(room);
    if (!newroom) {
        return;
    }
    if (!BaseInterface::CurrentBase) {
        return;
    }
    BaseInterface::CurrentBase->GotoLink(room);
}

int GetNumRoom() {
    if (!BaseInterface::CurrentBase) {
        return -1;
    }
    return BaseInterface::CurrentBase->rooms.size();
}

bool BuyShip(std::string name, bool my_fleet, bool force_base_inventory) {
    Unit *base = BaseInterface::CurrentBase->baseun.GetUnit();
    Unit *un = BaseInterface::CurrentBase->caller.GetUnit();
    return ::buyShip(base, un, name, my_fleet, force_base_inventory, NULL);
}

bool SellShip(std::string name) {
    Unit *base = BaseInterface::CurrentBase->baseun.GetUnit();
    Unit *un = BaseInterface::CurrentBase->caller.GetUnit();
    return ::sellShip(base, un, name, NULL);
}

Dictionary &_GetEventData() {
    static BoostPythonDictionary data;
    return data;
}

void SetEventData(BoostPythonDictionary data) {
    _GetEventData() = data;
}

void SetMouseEventData(std::string type, float x, float y, int buttonMask) {
    BoostPythonDictionary &data = _GetEventData();

    //Event type
    data["type"] = type;

    //Mouse data
    data["mousex"] = x;
    data["mousey"] = y;
    data["mousebuttons"] = buttonMask;

    SetKeyStatusEventData();
}

void SetKeyStatusEventData(unsigned int modmask) {
    BoostPythonDictionary &data = _GetEventData();
    //Keyboard modifiers (for kb+mouse)
    if (modmask == UINT_MAX) {
        modmask = pullActiveModifiers();
    }
    data["modifiers"] = modmask;
    data["alt"] = ((modmask & KB_MOD_ALT) != 0);
    data["shift"] = ((modmask & KB_MOD_SHIFT) != 0);
    data["ctrl"] = ((modmask & KB_MOD_CTRL) != 0);
}

void SetKeyEventData(std::string type, unsigned int keycode, unsigned int modmask) {
    BoostPythonDictionary &data = _GetEventData();

    //Event type
    data["type"] = type;

    //Keycode
    data["key"] = keycode;
    if ((keycode > 0x20) && (keycode < 0xff)) {
        data["char"] = string(1, keycode);
    } else {
        data["char"] = string();
    }
    SetKeyStatusEventData(modmask);
}

const Dictionary &GetEventData() {
    return _GetEventData();
}

float GetTextHeight(std::string text, Vector widheimult) {
    static bool force_highquality = true;
    static bool use_bit = force_highquality
            || XMLSupport::parse_bool(vs_config->getVariable("graphics", "high_quality_font", "false"));
    static float font_point = XMLSupport::parse_float(vs_config->getVariable("graphics", "font_point", "16"));
    return use_bit ? getFontHeight() : (font_point * 2 / g_game.y_resolution);
}

float GetTextWidth(std::string text, Vector widheimult) {
    //Unsupported for now
    return 0;
}

void LoadBaseInterface(string name) {
    LoadBaseInterfaceAtDock(name, UniverseUtil::getPlayer(), UniverseUtil::getPlayer());
}

void LoadBaseInterfaceAtDock(string name, Unit *dockat, Unit *dockee) {
    if (BaseInterface::CurrentBase) {
        BaseInterface::CurrentBase->Terminate();
    }
    BaseInterface *base = new BaseInterface(name.c_str(), dockat, dockee);
    base->InitCallbacks();
}

void refreshBaseComputerUI(const Cargo *carg) {
    if (carg) {
        //BaseComputer::draw() used dirty to determine what to recalculate.
        BaseComputer::dirty = 1;               //everything.
    } else {
        BaseComputer::dirty = 2;
    }               //only title.
}

void ExitGame() {
    CockpitKeys::QuitNow();
}
}

