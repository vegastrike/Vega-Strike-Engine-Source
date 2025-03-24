/**
 * base_util_server.cpp
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


#include <string>
#include <stdlib.h>
#include "cmd/unit_generic.h"
#include "base_util.h"
namespace BaseUtil {
int Room(std::string text) {
    return 0;
}

void Texture(int room, std::string index, std::string file, float x, float y) {
}

bool Video(int room, std::string index, std::string file, float x, float y) {
    return false;
}

bool VideoStream(int room, std::string index, std::string file, float x, float y, float w, float h) {
    return false;
}

void PlayVideo(int room, std::string index) {
}

void StopVideo(int room, std::string index) {
}

void SetDJEnabled(bool enabled) {
}

void Ship(int room, std::string index, QVector pos, Vector Q, Vector R) {
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
}

void EjectPython(int room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
        std::string text) {
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
}

void Python(int room,
        std::string index,
        float x,
        float y,
        float wid,
        float hei,
        std::string text,
        std::string pythonfile) {
}

void Message(std::string text) {
}

void EnqueueMessage(std::string text) {
}

void EraseLink(int room, std::string index) {
}

void EraseObj(int room, std::string index) {
}

int GetCurRoom() {
    return 0;
}

int GetNumRoom() {
    return 1;
}

bool HasObject(int room, std::string index) {
    return false;
}

void refreshBaseComputerUI(const class Cargo *carg) {
}
}

