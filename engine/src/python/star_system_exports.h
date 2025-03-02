/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
// NO HEADER GUARD

/// Exports functions for python modules
///

voidEXPORT_UTIL(StopAllSounds)
EXPORT_UTIL(getNumUnits, 0)
EXPORT_UTIL(GetRelation, 0.)
voidEXPORT_UTIL(AdjustRelation)
EXPORT_FACTION(GetFactionName, "")
EXPORT_FACTION(GetFactionIndex, -1)
EXPORT_FACTION(isCitizen, 0)
EXPORT_FACTION(GetNumFactions, 0)
EXPORT_UTIL(GetAdjacentSystem, "")
voidEXPORT_UTIL(pushSystem)
voidEXPORT_UTIL(popSystem)
voidEXPORT_UTIL(precacheUnit)
EXPORT_UTIL(getSystemFile, "")
EXPORT_UTIL(getSystemName, "")
EXPORT_UTIL(systemInMemory, 0)
EXPORT_UTIL(GetGalaxyProperty, "")
EXPORT_UTIL(GetGalaxyFaction, "")
voidEXPORT_UTIL(SetGalaxyFaction)
EXPORT_UTIL(GetGalaxyPropertyDefault, "")
EXPORT_UTIL(GetNumAdjacentSystems, 0)
EXPORT_UTIL(GetJumpPath, 0)
voidEXPORT_UTIL(terminateMission)
EXPORT_UTIL(getTargetLabel, "")
voidEXPORT_UTIL(setTargetLabel)
EXPORT_UTIL(getRelationModifierInt, 0.)
EXPORT_UTIL(getRelationModifier, 0.)
EXPORT_UTIL(getFGRelationModifier, 0.)
voidEXPORT_UTIL(adjustRelationModifierInt)
voidEXPORT_UTIL(adjustRelationModifier)
voidEXPORT_UTIL(adjustFGRelationModifier)
EXPORT_UTIL(addObjective, 0)
voidEXPORT_UTIL(setObjective)
voidEXPORT_UTIL(eraseObjective)
voidEXPORT_UTIL(clearObjectives)
voidEXPORT_UTIL(setCompleteness)
EXPORT_UTIL(getCompleteness, 0)
EXPORT_UTIL(vsConfig, "")
EXPORT_UTIL(sqrt, 1)
EXPORT_UTIL(log, 0)
EXPORT_UTIL(timeofday, 10003)
EXPORT_UTIL(exp, 1)
EXPORT_UTIL(acos, 1)
EXPORT_UTIL(asin, 1)
EXPORT_UTIL(atan, 1)
EXPORT_UTIL(tan, 1)
EXPORT_UTIL(sin, .86)
EXPORT_UTIL(cos, .86)
voidEXPORT_UTIL(IOmessage)
EXPORT_UTIL(SafeEntrancePoint, (10000000, 0, 0))
voidEXPORT_UTIL(setScratchVector)
EXPORT_UTIL(getScratchVector, (0, 0, 0))
EXPORT_UTIL(numActiveMissions, 1)
voidEXPORT_UTIL(SetAutoStatus)
voidEXPORT_UTIL(LoadMission)
voidEXPORT_UTIL(LoadMissionScript)
voidEXPORT_UTIL(LoadNamedMissionScript)
voidEXPORT_UTIL(setMissionOwner)
EXPORT_UTIL(getMissionOwner, 1)
EXPORT_UTIL(GetDifficulty, 1)
voidEXPORT_UTIL(SetDifficulty)
EXPORT_UTIL(GetGameTime, 0)
EXPORT_UTIL(getStarTime, 0)
EXPORT_UTIL(getStarDate, "0.0000:000")
voidEXPORT_UTIL(SetTimeCompression)
EXPORT_UTIL(getRandCargo, Cargo())
EXPORT_UTIL(getPlanetRadiusPercent, .75)

EXPORT_UTIL(musicAddList, 0)
voidEXPORT_UTIL(musicPlaySong)
voidEXPORT_UTIL(musicSkip)
voidEXPORT_UTIL(musicStop)
voidEXPORT_UTIL(musicPlayList)
voidEXPORT_UTIL(musicLoopList)

voidEXPORT_UTIL(musicLayerPlaySong)
voidEXPORT_UTIL(musicLayerSkip)
voidEXPORT_UTIL(musicLayerStop)
voidEXPORT_UTIL(musicLayerPlayList)
voidEXPORT_UTIL(musicLayerLoopList)
voidEXPORT_UTIL(musicLayerSetSoftVolume)
voidEXPORT_UTIL(musicLayerSetHardVolume)
voidEXPORT_UTIL(musicSetSoftVolume)
voidEXPORT_UTIL(musicSetHardVolume)

voidEXPORT_UTIL(playSound)
voidEXPORT_UTIL(playSoundCockpit)
voidEXPORT_UTIL(cacheAnimation)
voidEXPORT_UTIL(playAnimation)
voidEXPORT_UTIL(playAnimationGrow)
voidEXPORT_UTIL(micro_sleep)
EXPORT_UTIL(getCurrentPlayer, 0)
voidEXPORT_UTIL(addParticle)
voidEXPORT_UTIL(TargetEachOther)
voidEXPORT_UTIL(StopTargettingEachOther)
EXPORT_UTIL(LookupUnitStat, "")

EXPORT_UTIL(networked, false)
EXPORT_UTIL(isserver, false)
EXPORT_UTIL(getSaveDir, "")
EXPORT_UTIL(getSaveInfo, "")
EXPORT_UTIL(getCurrentSaveGame, "")
EXPORT_UTIL(getNewGameSaveName, "")
voidEXPORT_UTIL(loadGame)
voidEXPORT_UTIL(saveGame)

voidEXPORT_UTIL(sendCustom)
voidEXPORT_UTIL(receivedCustom)

voidEXPORT_UTIL(showSplashScreen)
voidEXPORT_UTIL(showSplashMessage)
voidEXPORT_UTIL(hideSplashScreen)
EXPORT_UTIL(isSplashScreenShowing, true)

