"""Build the ``VS``, ``Director``, ``Briefing`` and ``Base`` modules.

These are installed into ``sys.modules`` exactly like the engine does
(``PyDict_SetItemString(PyImport_GetModuleDict(), ...)``) so the data pack
imports them with a plain ``import VS``.
"""

import math
import os
import sys
import time
import types

from . import boostargs as B
from .boostargs import exported
from . import base as basemod
from . import engine as engmod
from .units import UnitWrapper, UnIter, Cargo, wrap


def E():
    return engmod.CURRENT


def _u(w):
    return w._get() if w is not None else None


# --------------------------------------------------------------------------
# VS
# --------------------------------------------------------------------------

@exported('VS.getUnitList')
def getUnitList():
    s = E().active_system()
    return UnIter(s.units if s else [])


@exported('VS.getScratchUnit')
def getScratchUnit():
    return wrap(E().scratch_unit)


@exported('VS.setScratchUnit', B.UNITPTR)
def setScratchUnit(un):
    E().scratch_unit = _u(un)


@exported('VS.getNumPlayers')
def getNumPlayers():
    return 1


@exported('VS.getVariable', B.STR, B.STR, B.STR)
def getVariable(section, name, default):
    return E().data.config.get(section, name, default)


@exported('VS.getSubVariable', B.STR, B.STR, B.STR, B.STR)
def getSubVariable(section, sub, name, default):
    return E().data.config.get_sub(section, sub, name, default)


@exported('VS.getPlayer')
def getPlayer():
    return wrap(E().player)


newGetPlayer = getPlayer


@exported('VS.getPlayerX', B.INT)
def getPlayerX(which):
    if which != 0:
        return wrap(None)
    return wrap(E().player)


@exported('VS.GetContrabandList', B.STR)
def GetContrabandList(faction):
    eng = E()
    if faction not in eng.contraband_lists:
        cname = eng.factions.contraband.get(faction, '')
        eng.contraband_lists[faction] = eng.create_unit(cname, faction) if cname else None
    return wrap(eng.contraband_lists[faction])


@exported('VS.getUnit', B.INT)
def getUnit(index):
    s = E().active_system()
    i = -1
    for u in (s.units if s else []):
        if u.killed:
            continue
        if u.hull > 0:
            i += 1
        if i == index:
            return wrap(u)
    return wrap(None)


@exported('VS.getUnitByName', B.STR)
def getUnitByName(name):
    s = E().active_system()
    for u in (s.units if s else []):
        if not u.killed and (u.name == name or u.fullname == name):
            return wrap(u)
    return wrap(None)


@exported('VS.launchJumppoint', B.STR, B.STR, B.STR, B.STR, B.STR, B.INT, B.INT, B.VEC, B.STR, B.STR)
def launchJumppoint(name, faction, typ, unittype, ai, nships, nwaves, pos, logo, destinations):
    return wrap(E().launch(name, typ, faction, unittype, ai, nships, nwaves, pos, logo, destinations))


@exported('VS.launch', B.STR, B.STR, B.STR, B.STR, B.STR, B.INT, B.INT, B.VEC, B.STR)
def launch(name, typ, faction, unittype, ai, nships, nwaves, pos, logo):
    return wrap(E().launch(name, typ, faction, unittype, ai, nships, nwaves, pos, logo))


@exported('VS.GetMasterPartList')
def GetMasterPartList():
    return wrap(E().get_master_part_list())


def _objectives():
    m = E().current_mission
    return m.objectives if m is not None else []


@exported('VS.setOwner', B.INT, B.UNIT)
def setOwner(obj, un):
    objs = _objectives()
    if 0 <= obj < len(objs):
        objs[obj].owner = _u(un)


@exported('VS.getOwner', B.INT)
def getOwner(which):
    objs = _objectives()
    if 0 <= which < len(objs):
        return wrap(objs[which].owner)
    return wrap(None)


@exported('VS.StopAllSounds')
def StopAllSounds():
    pass


@exported('VS.getNumUnits')
def getNumUnits():
    s = E().active_system()
    return len([u for u in s.units if not u.killed]) if s else 0


@exported('VS.GetRelation', B.STR, B.STR)
def GetRelation(a, b):
    return E().get_relation(a, b)


@exported('VS.AdjustRelation', B.STR, B.STR, B.FLOAT, B.FLOAT)
def AdjustRelation(a, b, factor, rank):
    E().adjust_relation(a, b, factor, rank)


@exported('VS.GetFactionName', B.INT)
def GetFactionName(index):
    names = E().factions.names
    if 0 <= index < len(names):
        return names[index]
    return ''


@exported('VS.GetFactionIndex', B.STR)
def GetFactionIndex(name):
    return E().factions.get_index(name)


@exported('VS.isCitizen', B.STR)
def isCitizen(name):
    return name in ('privateer',)


@exported('VS.GetNumFactions')
def GetNumFactions():
    return len(E().factions.names)


@exported('VS.GetAdjacentSystem', B.STR, B.INT)
def GetAdjacentSystem(sysname, which):
    j = E().adjacent_systems(sysname)
    if 0 <= which < len(j):
        return j[which]
    return ''


@exported('VS.pushSystem', B.STR)
def pushSystem(name):
    eng = E()
    eng.active_stack.append(eng.get_system(name))


@exported('VS.popSystem')
def popSystem():
    eng = E()
    if eng.active_stack:
        eng.active_stack.pop()


@exported('VS.precacheUnit', B.STR, B.STR)
def precacheUnit(typ, faction):
    eng = E()
    if eng.data.units.lookup(typ, faction) is None:
        eng.warn('precacheUnit: unit type "%s" (faction %s) not found in units.csv' % (typ, faction))


@exported('VS.getSystemFile')
def getSystemFile():
    return E().active_system_name()


@exported('VS.getSystemName')
def getSystemName():
    return E().active_system_name().split('/')[-1]


@exported('VS.systemInMemory', B.STR)
def systemInMemory(name):
    return name in E().systems


@exported('VS.GetGalaxyProperty', B.STR, B.STR)
def GetGalaxyProperty(sysname, prop):
    v = E().galaxy_property(sysname, prop)
    return v if v is not None else ''


@exported('VS.GetGalaxyPropertyDefault', B.STR, B.STR, B.STR)
def GetGalaxyPropertyDefault(sysname, prop, default):
    v = E().galaxy_property(sysname, prop)
    return v if v is not None else default


@exported('VS.GetGalaxyFaction', B.STR)
def GetGalaxyFaction(sysname):
    return E().get_galaxy_faction(sysname)


@exported('VS.SetGalaxyFaction', B.STR, B.STR)
def SetGalaxyFaction(sysname, fac):
    eng = E()
    eng.galaxy_faction[sysname] = fac
    eng.event('galaxy_faction', system=sysname, faction=fac)


@exported('VS.GetNumAdjacentSystems', B.STR)
def GetNumAdjacentSystems(sysname):
    return len(E().adjacent_systems(sysname))


@exported('VS.terminateMission', B.BOOL)
def terminateMission(win):
    eng = E()
    if eng.current_mission is not None:
        eng.terminate_mission(eng.current_mission, win)


@exported('VS.getTargetLabel')
def getTargetLabel():
    return E().target_label


@exported('VS.setTargetLabel', B.STR)
def setTargetLabel(label):
    E().target_label = label


@exported('VS.getRelationModifierInt', B.INT, B.INT)
def getRelationModifierInt(cp, fac):
    eng = E()
    if cp != 0:
        return 0.0
    return eng.relation_modifier(GetFactionName(fac))


@exported('VS.getRelationModifier', B.INT, B.STR)
def getRelationModifier(cp, fac):
    return E().relation_modifier(fac) if cp == 0 else 0.0


@exported('VS.getFGRelationModifier', B.INT, B.STR)
def getFGRelationModifier(cp, fg):
    return E().fg_relation_modifier(fg) if cp == 0 else 0.0


@exported('VS.adjustRelationModifierInt', B.INT, B.INT, B.FLOAT)
def adjustRelationModifierInt(cp, fac, delta):
    if cp == 0:
        E().adjust_relation_modifier(GetFactionName(fac), delta)


@exported('VS.adjustRelationModifier', B.INT, B.STR, B.FLOAT)
def adjustRelationModifier(cp, fac, delta):
    if cp == 0:
        E().adjust_relation_modifier(fac, delta)


@exported('VS.adjustFGRelationModifier', B.INT, B.STR, B.FLOAT)
def adjustFGRelationModifier(cp, fg, delta):
    if cp == 0:
        E().adjust_fg_relation_modifier(fg, delta)


@exported('VS.addObjective', B.STR)
def addObjective(text):
    objs = _objectives()
    objs.append(engmod.Objective(text.replace('.blank', '')))
    return len(objs) - 1


@exported('VS.setObjective', B.INT, B.STR)
def setObjective(which, text):
    objs = _objectives()
    if 0 <= which < len(objs):
        objs[which].text = text.replace('.blank', '')


@exported('VS.eraseObjective', B.INT)
def eraseObjective(which):
    objs = _objectives()
    if 0 <= which < len(objs):
        del objs[which]


@exported('VS.clearObjectives')
def clearObjectives():
    del _objectives()[:]


@exported('VS.setCompleteness', B.INT, B.FLOAT)
def setCompleteness(which, c):
    objs = _objectives()
    if 0 <= which < len(objs):
        objs[which].completeness = c


@exported('VS.getCompleteness', B.INT)
def getCompleteness(which):
    objs = _objectives()
    if 0 <= which < len(objs):
        return objs[which].completeness
    return 0.0


@exported('VS.vsConfig', B.STR, B.STR, B.STR)
def vsConfig(cat, opt, default):
    return E().data.config.get(cat, opt, default)


def _mathfn(name, fn):
    @exported('VS.' + name, B.FLOAT)
    def f(x):
        return fn(x)
    f.__name__ = name
    return f


sqrt = _mathfn('sqrt', math.sqrt)
log = _mathfn('log', math.log)
exp = _mathfn('exp', math.exp)
acos = _mathfn('acos', math.acos)
asin = _mathfn('asin', math.asin)
atan = _mathfn('atan', math.atan)
tan = _mathfn('tan', math.tan)
sin = _mathfn('sin', math.sin)
cos = _mathfn('cos', math.cos)


@exported('VS.timeofday')
def timeofday():
    return 1000.0 + E().game_time_total()


@exported('VS.IOmessage', B.INT, B.STR, B.STR, B.STR)
def IOmessage(delay, frm, to, msg):
    E().io_message(delay, frm, to, msg)


@exported('VS.SafeEntrancePoint', B.VEC, B.FLOAT)
def SafeEntrancePoint(pos, radial):
    return E().safe_entrance_point(pos, radial)


@exported('VS.setScratchVector', B.VEC)
def setScratchVector(v):
    E().scratch_vector = v


@exported('VS.getScratchVector')
def getScratchVector():
    return E().scratch_vector


@exported('VS.numActiveMissions')
def numActiveMissions():
    eng = E()
    return len(eng.player_missions()) + len(eng.delayed_missions)


@exported('VS.SetAutoStatus', B.INT, B.INT)
def SetAutoStatus(g, p):
    pass


@exported('VS.LoadMission', B.STR)
def LoadMission(name):
    E().load_mission(name, '')


@exported('VS.LoadMissionScript', B.STR)
def LoadMissionScript(script):
    E().load_mission('nothing.mission', script, title='nothing.mission')


@exported('VS.LoadNamedMissionScript', B.STR, B.STR)
def LoadNamedMissionScript(title, script):
    E().load_mission('', script, title=title)


@exported('VS.setMissionOwner', B.INT)
def setMissionOwner(n):
    m = E().current_mission
    if m is not None:
        m.player_num = n


@exported('VS.getMissionOwner')
def getMissionOwner():
    m = E().current_mission
    return m.player_num if m is not None else 0


@exported('VS.GetDifficulty')
def GetDifficulty():
    return E().difficulty


@exported('VS.SetDifficulty', B.FLOAT)
def SetDifficulty(d):
    E().difficulty = d


@exported('VS.GetGameTime')
def GetGameTime():
    m = E().current_mission
    return m.gametime if m is not None else 0.0


@exported('VS.SetTimeCompression')
def SetTimeCompression():
    pass


@exported('VS.getRandCargo', B.INT, B.STR)
def getRandCargo(quantity, category):
    eng = E()
    mpl = eng.get_master_part_list()
    choices = []
    if category:
        choices = [c for c in mpl.cargo if c._category == category or c._category.startswith(category + '/')]
    else:
        choices = [c for c in mpl.cargo if 'mission' not in c._content]
    if not choices:
        c = Cargo()
        c._quantity = 0
        return c
    c = choices[eng.rng.randrange(len(choices))].copy()
    c._quantity = quantity
    return c


@exported('VS.getPlanetRadiusPercent')
def getPlanetRadiusPercent():
    return E().planet_radius_percent()


@exported('VS.musicAddList', B.STR)
def musicAddList(name):
    eng = E()
    eng.music_lists.append(name)
    return len(eng.music_lists) - 1


@exported('VS.musicPlaySong', B.STR)
def musicPlaySong(name):
    pass


@exported('VS.musicSkip')
def musicSkip():
    pass


@exported('VS.musicStop')
def musicStop():
    pass


@exported('VS.musicPlayList', B.INT)
def musicPlayList(which):
    pass


@exported('VS.musicLoopList', B.INT)
def musicLoopList(n):
    pass


@exported('VS.musicLayerPlaySong', B.STR, B.INT)
def musicLayerPlaySong(name, layer):
    pass


@exported('VS.musicLayerSkip', B.INT)
def musicLayerSkip(layer):
    pass


@exported('VS.musicLayerStop', B.INT)
def musicLayerStop(layer):
    pass


@exported('VS.musicLayerPlayList', B.INT, B.INT)
def musicLayerPlayList(which, layer):
    pass


@exported('VS.musicLayerLoopList', B.INT, B.INT)
def musicLayerLoopList(n, layer):
    pass


@exported('VS.musicLayerSetSoftVolume', B.FLOAT, B.FLOAT, B.INT)
def musicLayerSetSoftVolume(v, lat, layer):
    pass


@exported('VS.musicLayerSetHardVolume', B.FLOAT, B.INT)
def musicLayerSetHardVolume(v, layer):
    pass


@exported('VS.musicSetSoftVolume', B.FLOAT, B.FLOAT)
def musicSetSoftVolume(v, lat):
    pass


@exported('VS.musicSetHardVolume', B.FLOAT)
def musicSetHardVolume(v):
    pass


def _check_sound(name):
    eng = E()
    if not name:
        return
    eng.sounds.append(name)
    for d in ('', 'sounds', 'music', 'communications'):
        if os.path.isfile(eng.data.path(d, name)):
            return
    eng.warn('sound file not found: %s' % name)


@exported('VS.playSound', B.STR, B.VEC, B.VEC)
def playSound(name, loc, speed):
    _check_sound(name)


@exported('VS.playSoundCockpit', B.STR)
def playSoundCockpit(name):
    _check_sound(name)


@exported('VS.cacheAnimation', B.STR)
def cacheAnimation(name):
    pass


@exported('VS.playAnimation', B.STR, B.VEC, B.FLOAT)
def playAnimation(name, loc, size):
    pass


@exported('VS.playAnimationGrow', B.STR, B.VEC, B.FLOAT, B.FLOAT)
def playAnimationGrow(name, loc, size, grow):
    pass


@exported('VS.micro_sleep', B.INT)
def micro_sleep(n):
    pass


@exported('VS.getCurrentPlayer')
def getCurrentPlayer():
    return 0


@exported('VS.addParticle', B.VEC, B.VEC, B.VEC, B.FLOAT)
def addParticle(loc, vel, col, size):
    pass


def _fg_units(fgname, faction):
    s = E().active_system()
    return [u for u in (s.units if s else []) if not u.killed and u.fg_name() == fgname and u.faction == faction]


@exported('VS.TargetEachOther', B.STR, B.STR, B.STR, B.STR)
def TargetEachOther(fgname, faction, enfgname, enfaction):
    al = _fg_units(fgname, faction)
    en = _fg_units(enfgname, enfaction)
    if al and en:
        for a in al:
            a.target = en[0]
            if a.flightgroup:
                a.flightgroup.directive = 'A.'
        for e in en:
            e.target = al[0]
            if e.flightgroup:
                e.flightgroup.directive = 'A.'


@exported('VS.StopTargettingEachOther', B.STR, B.STR, B.STR, B.STR)
def StopTargettingEachOther(fgname, faction, enfgname, enfaction):
    for u in _fg_units(fgname, faction) + _fg_units(enfgname, enfaction):
        if u.flightgroup:
            u.flightgroup.directive = 'b'


@exported('VS.LookupUnitStat', B.STR, B.STR, B.STR)
def LookupUnitStat(unitname, faction, stat):
    return E().data.units.stat(unitname, faction, stat)


@exported('VS.networked')
def networked():
    return False


@exported('VS.isserver')
def isserver():
    return False


@exported('VS.getSaveDir')
def getSaveDir():
    d = E().opts.save_dir
    return (d + '/') if d else ''


@exported('VS.getSaveInfo', B.STR, B.BOOL)
def getSaveInfo(filename, fmt):
    return 'Saved game ' + filename


@exported('VS.getCurrentSaveGame')
def getCurrentSaveGame():
    return E().save_name


@exported('VS.getNewGameSaveName')
def getNewGameSaveName():
    return E().opts.new_game


@exported('VS.loadGame', B.STR)
def loadGame(name):
    E().request_load_game(name)


@exported('VS.saveGame', B.STR)
def saveGame(name):
    E().save_game(name)


@exported('VS.sendCustom', B.INT, B.STR, B.STR, B.STR)
def sendCustom(cp, cmd, args, id_):
    E().warn('sendCustom called in single player (%s)' % cmd)


@exported('VS.receivedCustom', B.INT, B.BOOL, B.STR, B.STR, B.STR)
def receivedCustom(cp, trusted, cmd, args, id_):
    pass


@exported('VS.showSplashScreen', B.STR)
def showSplashScreen(name):
    pass


@exported('VS.showSplashMessage', B.STR)
def showSplashMessage(text):
    pass


@exported('VS.showSplashProgress', B.FLOAT)
def showSplashProgress(p):
    pass


@exported('VS.hideSplashScreen')
def hideSplashScreen():
    pass


@exported('VS.isSplashScreenShowing')
def isSplashScreenShowing():
    return False


class PythonAI:
    """VS.PythonAI (FireAt) base class for Python AI scripts."""

    def __init__(self):
        eng = E()
        eng.last_python_ai = self
        self._parent = None

    def Execute(self):
        pass

    def ChooseTarget(self):
        pass

    def init(self, un):
        pass

    def GetParent(self):
        return wrap(self._parent)

    def AddReplaceLastOrder(self, replace):
        pass

    def ExecuteLastScriptFor(self, time):
        pass

    def FaceTarget(self, end):
        pass

    def FaceTargetITTS(self, end):
        pass

    def MatchLinearVelocity(self, *a):
        pass

    def MatchAngularVelocity(self, *a):
        pass

    def ChangeHeading(self, *a):
        pass

    def ChangeLocalDirection(self, *a):
        pass

    def MoveTo(self, *a):
        pass

    def MatchVelocity(self, *a):
        pass

    def Cloak(self, *a):
        pass

    def FormUp(self, *a):
        pass

    def FormUpToOwner(self, *a):
        pass

    def FaceDirection(self, *a):
        pass

    def XMLScript(self, name):
        pass

    def LastPythonScript(self):
        pass


def build_vs_module():
    m = types.ModuleType('VS')
    g = globals()
    names = ['getUnitList', 'getScratchUnit', 'setScratchUnit', 'getNumPlayers', 'getVariable',
             'getSubVariable', 'newGetPlayer', 'GetContrabandList', 'getUnit', 'getUnitByName',
             'launchJumppoint', 'launch', 'getPlayer', 'getPlayerX', 'GetMasterPartList',
             'setOwner', 'getOwner', 'StopAllSounds', 'getNumUnits', 'GetRelation',
             'AdjustRelation', 'GetFactionName', 'GetFactionIndex', 'isCitizen', 'GetNumFactions',
             'GetAdjacentSystem', 'pushSystem', 'popSystem', 'precacheUnit', 'getSystemFile',
             'getSystemName', 'systemInMemory', 'GetGalaxyProperty', 'GetGalaxyFaction',
             'SetGalaxyFaction', 'GetGalaxyPropertyDefault', 'GetNumAdjacentSystems',
             'terminateMission', 'getTargetLabel', 'setTargetLabel', 'getRelationModifierInt',
             'getRelationModifier', 'getFGRelationModifier', 'adjustRelationModifierInt',
             'adjustRelationModifier', 'adjustFGRelationModifier', 'addObjective', 'setObjective',
             'eraseObjective', 'clearObjectives', 'setCompleteness', 'getCompleteness', 'vsConfig',
             'sqrt', 'log', 'timeofday', 'exp', 'acos', 'asin', 'atan', 'tan', 'sin', 'cos',
             'IOmessage', 'SafeEntrancePoint', 'setScratchVector', 'getScratchVector',
             'numActiveMissions', 'SetAutoStatus', 'LoadMission', 'LoadMissionScript',
             'LoadNamedMissionScript', 'setMissionOwner', 'getMissionOwner', 'GetDifficulty',
             'SetDifficulty', 'GetGameTime', 'SetTimeCompression', 'getRandCargo',
             'getPlanetRadiusPercent', 'musicAddList', 'musicPlaySong', 'musicSkip', 'musicStop',
             'musicPlayList', 'musicLoopList', 'musicLayerPlaySong', 'musicLayerSkip',
             'musicLayerStop', 'musicLayerPlayList', 'musicLayerLoopList',
             'musicLayerSetSoftVolume', 'musicLayerSetHardVolume', 'musicSetSoftVolume',
             'musicSetHardVolume', 'playSound', 'playSoundCockpit', 'cacheAnimation',
             'playAnimation', 'playAnimationGrow', 'micro_sleep', 'getCurrentPlayer',
             'addParticle', 'TargetEachOther', 'StopTargettingEachOther', 'LookupUnitStat',
             'networked', 'isserver', 'getSaveDir', 'getSaveInfo', 'getCurrentSaveGame',
             'getNewGameSaveName', 'loadGame', 'saveGame', 'sendCustom', 'receivedCustom',
             'showSplashScreen', 'showSplashMessage', 'showSplashProgress', 'hideSplashScreen',
             'isSplashScreenShowing']
    for n in names:
        setattr(m, n, g[n])
    m.Unit = UnitWrapper
    m.un_iter = UnIter
    m.Cargo = Cargo
    m.PythonAI = PythonAI
    return m


# --------------------------------------------------------------------------
# Director
# --------------------------------------------------------------------------

class DirectorMission:
    """Director.Mission: the last one constructed becomes the mission's
    Python object (PythonClass::last_instance)."""

    def __init__(self):
        E().register_python_mission(self)

    def Execute(self):
        pass

    def Pickle(self):
        return ''

    def UnPickle(self, s):
        pass


@exported('Director.putSaveData', B.INT, B.STR, B.UINT, B.FLOAT)
def putSaveData(cp, key, num, val):
    E().put_save_data(cp, key, num, val)


@exported('Director.pushSaveData', B.INT, B.STR, B.FLOAT)
def pushSaveData(cp, key, val):
    return E().push_save_data(cp, key, val)


@exported('Director.eraseSaveData', B.INT, B.STR, B.UINT)
def eraseSaveData(cp, key, index):
    return E().erase_save_data(cp, key, index)


@exported('Director.clearSaveData', B.INT, B.STR)
def clearSaveData(cp, key):
    return E().clear_save_data(cp, key)


@exported('Director.getSaveData', B.INT, B.STR, B.UINT)
def getSaveData(cp, key, num):
    return E().get_save_data(cp, key, num)


@exported('Director.getSaveDataLength', B.INT, B.STR)
def getSaveDataLength(cp, key):
    return E().get_save_data_length(cp, key)


@exported('Director.putSaveString', B.INT, B.STR, B.UINT, B.STR)
def putSaveString(cp, key, num, val):
    E().put_save_string(cp, key, num, val)


@exported('Director.pushSaveString', B.INT, B.STR, B.STR)
def pushSaveString(cp, key, val):
    return E().push_save_string(cp, key, val)


@exported('Director.getSaveString', B.INT, B.STR, B.UINT)
def getSaveString(cp, key, num):
    return E().get_save_string(cp, key, num)


@exported('Director.getSaveStringLength', B.INT, B.STR)
def getSaveStringLength(cp, key):
    return E().get_save_string_length(cp, key)


@exported('Director.eraseSaveString', B.INT, B.STR, B.UINT)
def eraseSaveString(cp, key, index):
    return E().erase_save_string(cp, key, index)


@exported('Director.clearSaveString', B.INT, B.STR)
def clearSaveString(cp, key):
    return E().clear_save_string(cp, key)


def build_director_module():
    m = types.ModuleType('Director')
    m.Mission = DirectorMission
    for n in ('putSaveData', 'pushSaveData', 'eraseSaveData', 'clearSaveData', 'getSaveData',
              'getSaveDataLength', 'putSaveString', 'pushSaveString', 'getSaveString',
              'getSaveStringLength', 'eraseSaveString', 'clearSaveString'):
        setattr(m, n, globals()[n])
    return m


# --------------------------------------------------------------------------
# Briefing
# --------------------------------------------------------------------------

def build_briefing_module():
    m = types.ModuleType('Briefing')
    counter = [0]

    @exported('Briefing.addShip', B.STR, B.STR, B.VEC)
    def addShip(name, faction, pos):
        counter[0] += 1
        return counter[0]

    @exported('Briefing.removeShip', B.INT)
    def removeShip(which):
        pass

    @exported('Briefing.enqueueOrder', B.INT, B.VEC, B.FLOAT)
    def enqueueOrder(which, pos, time):
        pass

    @exported('Briefing.replaceOrder', B.INT, B.VEC, B.FLOAT)
    def replaceOrder(which, pos, time):
        pass

    @exported('Briefing.getShipPosition', B.INT)
    def getShipPosition(which):
        return (0.0, 0.0, 0.0)

    @exported('Briefing.setShipPosition', B.INT, B.VEC)
    def setShipPosition(which, pos):
        pass

    @exported('Briefing.terminate')
    def terminate():
        pass

    @exported('Briefing.setCamPosition', B.VEC)
    def setCamPosition(pos):
        pass

    @exported('Briefing.setCamOrientation', B.VEC, B.VEC, B.VEC)
    def setCamOrientation(p, q, r):
        pass

    @exported('Briefing.setCloak', B.INT, B.FLOAT)
    def setCloak(which, amt):
        pass

    for f in (addShip, removeShip, enqueueOrder, replaceOrder, getShipPosition, setShipPosition,
              terminate, setCamPosition, setCamOrientation, setCloak):
        setattr(m, f.__name__, f)
    return m


def build_base_module():
    m = types.ModuleType('Base')
    for name, fn in basemod.BASE_FUNCTIONS.items():
        setattr(m, name, fn)
    return m


def install(data_root, main_globals=None):
    """Install the engine modules and the engine's sys.path entries.

    Like Python::initpaths() this also runs ``import sys`` in __main__,
    which scripts executed there (base links, Python AIs) rely on."""
    sys.modules['VS'] = build_vs_module()
    sys.modules['Director'] = build_director_module()
    sys.modules['Briefing'] = build_briefing_module()
    sys.modules['Base'] = build_base_module()
    for sub in ('modules/quests', 'modules/missions', 'modules/ai', 'modules', 'bases'):
        p = os.path.join(data_root, sub)
        if p not in sys.path:
            sys.path.append(p)
    if main_globals is not None:
        exec('import sys\n', main_globals)
