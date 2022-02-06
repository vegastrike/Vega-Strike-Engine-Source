/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_factory.h"    //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "networking/netserver.h"
#include "star_system_generic.h"
#include <string>
#include "lin_time.h"
#include "load_mission.h"
#include "configxml.h"
#include "vs_globals.h"

void SetStarSystemLoading(bool value)
{
}

using std::string;

//less to write
#define activeSys _Universe->activeStarSystem()

void ClientServerSetLightContext(int lc)
{
}

namespace UniverseUtil {
void playVictoryTune()
{
}

int musicAddList(string str)
{
    return 0;
}

void musicLayerPlaySong(string str, int layer)
{
}

void addParticle(QVector loc, Vector velocity, Vector color, float size)
{
}

void musicLayerPlayList(int which, int layer)
{
}

void musicLayerLoopList(int numloops, int layer)
{
}

void musicLayerSetSoftVolume(float vol, float latency_override, int layer)
{
}

void musicLayerSetHardVolume(float vol, int layer)
{
}

void musicSetSoftVolume(float vol, float latency_override)
{
}

void musicSetHardVolume(float vol)
{
}

void musicMute(bool stopSound)
{
}

void playSound(string soundName, QVector loc, Vector speed)
{
}

void playSoundCockpit(string soundName)
{
}

void cacheAnimation(string aniName)
{
}

void playAnimation(string aniName, QVector loc, float size)
{
}

void playAnimationGrow(string aniName, QVector loc, float size, float growpercent)
{
}

unsigned int getCurrentPlayer()
{
    return _Universe->CurrentCockpit();
}

void musicLayerSkip(int layer)
{
}

void musicLayerStop(int layer)
{
}

void StopAllSounds(void)
{
}

void loadGame(const string &savename)
{
    unsigned int num = 0;
    sscanf(savename.c_str(), "%u", &num);
    if (num != UINT_MAX && num < _Universe->numPlayers()) {
        Unit *un = _Universe->AccessCockpit(num)->GetParent();
        if (un) {
            un->hull = 0;
            un->Destroy();
        }
    }
}

void saveGame(const string &savename)
{
    unsigned int num = 0;
    sscanf(savename.c_str(), "%u", &num);
    if (num != UINT_MAX && num < _Universe->numPlayers()) {
        if (SERVER) {
            VSServer->saveAccount(num);
        }
    } else if (num == 0 && SERVER) {
        cout << ">>> Manually Saving server status..." << endl;
        VSServer->save();
        cout << "<<< Finished saving." << endl;
    }
}

void showSplashScreen(const string &filename)
{
}

void showSplashMessage(const string &text)
{
}

void showSplashProgress(float progress)
{
}

void hideSplashScreen()
{
}

bool isSplashScreenShowing()
{
    return false;
}

void startMenuInterface(bool firstTime, string error)
{
    //Critical game error... enough to bring you back to the game menu!
    printf("GAME ERROR: %s\n", error.c_str());
}

void sendCustom(int cp, string cmd, string args, string id)
{
    if (cp < 0 || (unsigned int) cp >= _Universe->numPlayers()) {
        fprintf(stderr, "sendCustom %s with invalid player %d\n", cmd.c_str(), cp);
        return;
    }
    VSServer->sendCustom(cp, cmd, args, id);
}
}

#undef activeSys

