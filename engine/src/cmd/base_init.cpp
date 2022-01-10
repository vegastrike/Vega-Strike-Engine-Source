/**
 * base_init.cpp
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


#include <math.h>

#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python.hpp>
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

#include <Python.h>
#include "python/python_class.h"
#include "base.h"
#include "base_util.h"
#include "vsfilesystem.h"

static BoostPythonDictionary GetEventDataPython()
{
    return BaseUtil::GetEventData();
}

static boost::python::tuple GetRandomBarMessage()
{
    gameMessage last;
    int i = 0;
    vector<std::string> who;
    vector<std::string> say;
    vector<std::string> sounds;
    string newmsg;
    string newsound;
    who.push_back("bar");
    while ((mission->msgcenter->last(i++, last, who))) {
        newmsg = last.message;
        newsound = "";
        std::string::size_type first = newmsg.find_first_of("[");
        {
            string::size_type last = newmsg.find_first_of("]");
            if (first != string::npos && (first + 1) < newmsg.size()) {
                newsound = newmsg.substr(first + 1, last - first - 1);
                newmsg = newmsg.substr(0, first);
            }
        }
        sounds.push_back(newsound);
        say.push_back(newmsg);
    }
    if (say.size()) {
        int index = rand() % say.size();
        return VS_BOOST_MAKE_TUPLE_2(say[index], sounds[index]);
    } else {
        return VS_BOOST_MAKE_TUPLE_2("", "");
    }
}

PYTHON_BEGIN_MODULE(Base)
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Room, "Room");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetCurRoom, "SetCurRoom");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::GetCurRoom, "GetCurRoom");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::GetNumRoom, "GetNumRoom");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::GetNumRoom, "HasObject");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Comp, "Comp");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::CompPython, "CompPython");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Launch, "Launch");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::LaunchPython, "LaunchPython");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Link, "Link");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::LinkPython, "LinkPython");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Python, "Python");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::EraseLink, "EraseLink");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Ship, "Ship");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Texture, "Texture");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Video, "Video");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::VideoStream, "VideoStream");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::PlayVideo, "PlayVideo");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::StopVideo, "StopVideo");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetVideoCallback, "SetVideoCallback");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetDJEnabled, "SetDJEnabled");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetTexture, "SetTexture");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetTextureSize, "SetTextureSize");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetTexturePos, "SetTexturePos");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::EnqueueMessageToRoom, "EnqueueMessageToRoom");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::MessageToRoom, "MessageToRoom");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::EnqueueMessage, "EnqueueMessage");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::Message, "Message");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::EraseObj, "EraseObj");
    PYTHON_DEFINE_GLOBAL(Base, &::GetRandomBarMessage, "GetRandomBarMessage");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::RunScript, "RunScript");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::TextBox, "TextBox");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetTextBoxText, "SetTextBoxText");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::GlobalKeyPython, "GlobalKeyPython");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetLinkArea, "SetLinkArea");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetLinkText, "SetLinkText");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetLinkPython, "SetLinkPython");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetLinkRoom, "SetLinkRoom");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetLinkEventMask, "SetLinkEventMask");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::BuyShip, "BuyShip");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SellShip, "SellShip");

    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetEventData, "SetEventData");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::SetMouseEventData, "SetMouseEventData");
    PYTHON_DEFINE_GLOBAL(Base, &::GetEventDataPython, "GetEventData");

    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::GetTextWidth, "GetTextWidth");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::GetTextHeight, "GetTextHeight");

    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::LoadBaseInterface, "LoadBaseInterface");
    PYTHON_DEFINE_GLOBAL(Base, &BaseUtil::ExitGame, "ExitGame");

PYTHON_END_MODULE(Base)

void InitBase()
{
    PyImport_AppendInittab("Base", PYTHON_MODULE_INIT_FUNCTION(Base));
}

void InitBase2()
{
    Python::reseterrors();
    PYTHON_INIT_MODULE(Base);
    Python::reseterrors();
}

