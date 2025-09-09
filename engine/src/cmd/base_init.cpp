/*
 * base_init.cpp
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

#include <boost/version.hpp>
#include <boost/python.hpp>
#if BOOST_VERSION != 102800
typedef boost::python::dict BoostPythonDictionary;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary;
#endif
#include <math.h>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#else
#include <boost/python/objects.hpp>
#endif

#include <Python.h>
#include "src/python/python_class.h"
#include "cmd/base.h"
#include "cmd/base_util.h"
#include "vegadisk/vsfilesystem.h"

static BoostPythonDictionary GetEventDataPython() {
    return BaseUtil::GetEventData();
}

static boost::python::tuple GetRandomBarMessage() {
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

BOOST_PYTHON_MODULE(Base) {
    boost::python::def("Room", &BaseUtil::Room);
    boost::python::def("SetCurRoom", &BaseUtil::SetCurRoom);
    boost::python::def("GetCurRoom", &BaseUtil::GetCurRoom);
    boost::python::def("GetNumRoom", &BaseUtil::GetNumRoom);
    boost::python::def("HasObject", &BaseUtil::GetNumRoom);
    boost::python::def("Comp", &BaseUtil::Comp);
    boost::python::def("CompPython", &BaseUtil::CompPython);
    boost::python::def("Launch", &BaseUtil::Launch);
    boost::python::def("LaunchPython", &BaseUtil::LaunchPython);
    boost::python::def("Link", &BaseUtil::Link);
    boost::python::def("LinkPython", &BaseUtil::LinkPython);
    boost::python::def("Python", &BaseUtil::Python);
    boost::python::def("EraseLink", &BaseUtil::EraseLink);
    boost::python::def("Ship", &BaseUtil::Ship);
    boost::python::def("Texture", &BaseUtil::Texture);
    boost::python::def("Video", &BaseUtil::Video);
    boost::python::def("VideoStream", &BaseUtil::VideoStream);
    boost::python::def("PlayVideo", &BaseUtil::PlayVideo);
    boost::python::def("StopVideo", &BaseUtil::StopVideo);
    boost::python::def("SetVideoCallback", &BaseUtil::SetVideoCallback);
    boost::python::def("SetDJEnabled", &BaseUtil::SetDJEnabled);
    boost::python::def("SetTexture", &BaseUtil::SetTexture);
    boost::python::def("SetTextureSize", &BaseUtil::SetTextureSize);
    boost::python::def("SetTexturePos", &BaseUtil::SetTexturePos);
    boost::python::def("EnqueueMessageToRoom", &BaseUtil::EnqueueMessageToRoom);
    boost::python::def("MessageToRoom", &BaseUtil::MessageToRoom);
    boost::python::def("EnqueueMessage", &BaseUtil::EnqueueMessage);
    boost::python::def("Message", &BaseUtil::Message);
    boost::python::def("EraseObj", &BaseUtil::EraseObj);
    boost::python::def("GetRandomBarMessage", &::GetRandomBarMessage);
    boost::python::def("RunScript", &BaseUtil::RunScript);
    boost::python::def("TextBox", static_cast<void (*)(int, std::string, std::string, float, float, Vector, GFXColor, float, GFXColor)>(&BaseUtil::TextBox));
    boost::python::def("TextBox", static_cast<void (*)(int, std::string, std::string, float, float, Vector, Vector, float, Vector)>(&BaseUtil::TextBox));
    boost::python::def("SetTextBoxText", &BaseUtil::SetTextBoxText);
    boost::python::def("GlobalKeyPython", &BaseUtil::GlobalKeyPython);
    boost::python::def("SetLinkArea", &BaseUtil::SetLinkArea);
    boost::python::def("SetLinkText", &BaseUtil::SetLinkText);
    boost::python::def("SetLinkPython", &BaseUtil::SetLinkPython);
    boost::python::def("SetLinkRoom", &BaseUtil::SetLinkRoom);
    boost::python::def("SetLinkEventMask", &BaseUtil::SetLinkEventMask);
    boost::python::def("BuyShip", &BaseUtil::BuyShip);
    boost::python::def("SellShip", &BaseUtil::SellShip);

    boost::python::def("SetEventData", &BaseUtil::SetEventData);
    boost::python::def("SetMouseEventData", &BaseUtil::SetMouseEventData);
    boost::python::def("GetEventData", &::GetEventDataPython);

    boost::python::def("GetTextWidth", &BaseUtil::GetTextWidth);
    boost::python::def("GetTextHeight", &BaseUtil::GetTextHeight);

    boost::python::def("LoadBaseInterface", &BaseUtil::LoadBaseInterface);
    boost::python::def("ExitGame", &BaseUtil::ExitGame);

}

void InitBase() {
    PyImport_AppendInittab("Base", PYTHON_MODULE_INIT_FUNCTION(Base));
}

void InitBase2() {
    Python::reseterrors();
    PYTHON_INIT_MODULE(Base);
    Python::reseterrors();
}

