"""Units, cargo and flightgroups for the mock engine.

``MockUnit`` is the engine-side object (the C++ ``Unit``); scripts never
see it directly.  Python code receives ``UnitWrapper`` objects (exported as
``VS.Unit``), which - like the engine's ``UnitContainer`` - stop resolving
to their unit once it has been killed.
"""

import math

from . import boostargs as B
from .boostargs import method

_serial = [100]


def next_serial():
    _serial[0] += 1
    return _serial[0]


# --------------------------------------------------------------------------
# Cargo (exported as VS.Cargo)
# --------------------------------------------------------------------------

class Cargo:
    """Value type; the engine hands out copies, never references."""

    def __init__(self, *args):
        if args:
            content, category, price, quantity, mass, volume = B.convert_args(
                'Cargo.__init__', (B.STR, B.STR, B.FLOAT, B.INT, B.FLOAT, B.FLOAT), args)
        else:
            content, category, price, quantity, mass, volume = '', '', 0.0, 1, 0.0, 0.0
        self._content = content
        self._category = category
        self._price = price
        self._quantity = quantity
        self._mass = mass
        self._volume = volume
        self._mission = False
        self._description = ''
        self._functionality = 1.0
        self._maxfunctionality = 1.0

    def copy(self):
        c = Cargo()
        c.__dict__.update(self.__dict__)
        return c

    def sort_key(self):
        return (self._category, self._content)

    @method('Cargo.SetPrice', B.FLOAT)
    def SetPrice(self, v):
        self._price = v

    @method('Cargo.GetPrice')
    def GetPrice(self):
        return self._price

    @method('Cargo.SetMass', B.FLOAT)
    def SetMass(self, v):
        self._mass = v

    @method('Cargo.GetMass')
    def GetMass(self):
        return self._mass

    @method('Cargo.SetVolume', B.FLOAT)
    def SetVolume(self, v):
        self._volume = v

    @method('Cargo.GetVolume')
    def GetVolume(self):
        return self._volume

    @method('Cargo.SetQuantity', B.INT)
    def SetQuantity(self, v):
        self._quantity = v

    @method('Cargo.GetQuantity')
    def GetQuantity(self):
        return self._quantity

    @method('Cargo.SetContent', B.STR)
    def SetContent(self, v):
        self._content = v

    @method('Cargo.GetContent')
    def GetContent(self):
        return self._content

    @method('Cargo.SetCategory', B.STR)
    def SetCategory(self, v):
        self._category = v

    @method('Cargo.GetCategory')
    def GetCategory(self):
        return self._category

    @method('Cargo.SetMissionFlag', B.BOOL)
    def SetMissionFlag(self, v):
        self._mission = v

    @method('Cargo.GetMissionFlag')
    def GetMissionFlag(self):
        return self._mission

    @method('Cargo.GetDescription')
    def GetDescription(self):
        return self._description

    @method('Cargo.SetFunctionality', B.FLOAT)
    def SetFunctionality(self, v):
        self._functionality = v

    @method('Cargo.GetFunctionality')
    def GetFunctionality(self):
        return self._functionality

    @method('Cargo.SetMaxFunctionality', B.FLOAT)
    def SetMaxFunctionality(self, v):
        self._maxfunctionality = v

    @method('Cargo.GetMaxFunctionality')
    def GetMaxFunctionality(self):
        return self._maxfunctionality

    def __repr__(self):
        return '<Cargo %s x%d (%s)%s>' % (self._content, self._quantity, self._category,
                                          ' mission' if self._mission else '')


def cargo_is_upgrade(c):
    return c._category.startswith('upgrades')


# --------------------------------------------------------------------------
# Flightgroups
# --------------------------------------------------------------------------

class Flightgroup:
    def __init__(self, name, faction, ftype='', ainame='default'):
        self.name = name
        self.faction = faction
        self.type = ftype
        self.ainame = ainame
        self.directive = 'b'
        self.leader = None
        self.nr_ships = 0
        self.nr_ships_left = 0
        self.nr_waves_left = 0

    def __repr__(self):
        return '<FG %s/%s>' % (self.name, self.faction)


# --------------------------------------------------------------------------
# Engine-side unit
# --------------------------------------------------------------------------

PLANET_KINDS = ('planet', 'jump', 'sun', 'nav')


class MockUnit:
    def __init__(self, engine, name, fullname, faction, kind='ship'):
        self.engine = engine
        self.serial = next_serial()
        self.name = name
        self.fullname = fullname
        self.faction = faction
        self.kind = kind
        self.system = None
        self.position = [0.0, 0.0, 0.0]
        self.velocity = [0.0, 0.0, 0.0]
        self.radius = 20.0
        self.hull = 1.0
        self.max_hull = 1.0
        self.killed = False
        self.flightgroup = None
        self.fg_subnumber = 0
        self.target = None
        self.threat = None
        self.velocity_ref = None
        self.ai_script = 'default'
        self.python_ai = None
        self.cargo = []
        self.mounts = []
        self.upgrades = []
        self.docked_to = None
        self.docked_units = []
        self.docking_ports = False
        self.dock_order = None       # (unit, actually_dock)
        self.jump_drive = False
        self.jump_active = False
        self.jump_to_system = None   # pending JumpTo destination
        self.destinations = []       # for jump points
        self.lights = False          # suns
        self.mission_relevant = False
        self.combat_role = 'INERT'
        self.unit_role = 'INERT'
        self.attack_preference = 'INERT'
        self.max_speed = 300.0
        self.max_ab_speed = 600.0
        self.set_speed = None
        self.cloaked = False
        self.cargo_volume = 0.0
        self.upgrade_volume = 0.0
        self.damage = 1.0            # damage per combat turn
        self.weapon_range = 1500.0
        self.anger = {}
        self.target_fgs = ('', '', '')
        self.moveto = None           # explicit destination (player autopilot etc)
        self.planet_file = ''
        self.hostile_spawn = False
        self.last_attacker = None
        self.spawn_time = 0.0
        self.orbit = None
        self.launched_by = ''         # mission type that launched this unit

    # -- helpers ----------------------------------------------------------
    def alive(self):
        return not self.killed

    def is_planet(self):
        return self.kind in PLANET_KINDS

    def is_jumppoint(self):
        return len(self.destinations) > 0

    def is_sun(self):
        if self.is_jumppoint():
            return False
        return self.is_planet() and self.lights

    def fg_name(self):
        return self.flightgroup.name if self.flightgroup else ''

    def is_significant(self):
        typ_ok = self.is_planet() or self.kind in ('asteroid', 'nebula') or self.fg_name() == 'Base'
        return typ_ok and not self.is_sun()

    def is_unitptr(self):
        return self.kind in ('ship', 'base', 'eject')

    def is_dockable(self):
        ok = ((self.is_planet() and not self.is_sun() and self.is_significant() and not self.is_jumppoint())
              or self.is_unitptr() or self.fg_name() == 'Base')
        return ok and self.docking_ports

    def rsize(self):
        return self.radius

    def distance_to(self, other):
        dx = self.position[0] - other.position[0]
        dy = self.position[1] - other.position[1]
        dz = self.position[2] - other.position[2]
        return math.sqrt(dx * dx + dy * dy + dz * dz) - self.radius - other.radius

    def significant_distance_to(self, sig):
        dist = self.distance_to(sig)
        pct = self.engine.planet_radius_percent()
        if sig.is_planet():
            dist -= sig.radius * pct
        if self.is_planet():
            dist -= self.radius * pct
        return dist

    def system_file(self):
        return self.system.name if self.system else ''

    def is_player(self):
        return self.engine.player is self

    def __repr__(self):
        return '<Unit #%d %s "%s" %s %s%s>' % (self.serial, self.name, self.fullname, self.faction,
                                               self.system_file(), ' DEAD' if self.killed else '')


# --------------------------------------------------------------------------
# Iterator (exported as VS.un_iter)
# --------------------------------------------------------------------------

class UnIter:
    """Snapshot iterator over a star system's unit list.

    The engine's iterator walks the live list; iterating a snapshot and
    skipping killed units is close enough for scripts, which only advance
    it and read ``current()``.
    """

    def __init__(self, units=None):
        self._units = list(units) if units is not None else []
        self._pos = 0
        self._skip_dead()

    def _skip_dead(self):
        while self._pos < len(self._units) and self._units[self._pos].killed:
            self._pos += 1

    def _cur(self):
        self._skip_dead()
        if self._pos < len(self._units):
            return self._units[self._pos]
        return None

    @method('un_iter.current')
    def current(self):
        return wrap(self._cur())

    @method('un_iter.isDone')
    def isDone(self):
        return self._cur() is None

    @method('un_iter.notDone')
    def notDone(self):
        return self._cur() is not None

    @method('un_iter.advance')
    def advance(self):
        if self._cur() is not None:
            self._pos += 1
        self._skip_dead()

    def _advance_until(self, pred):
        self.advance()
        while True:
            u = self._cur()
            if u is None or pred(u):
                return
            self._pos += 1

    def _advance_n_until(self, n, pred):
        # advanceN* skips to the n-th unit (0 based) that satisfies pred
        while True:
            u = self._cur()
            if u is None:
                return
            if pred(u):
                if n <= 0:
                    return
                n -= 1
            self._pos += 1

    @method('un_iter.advanceN', B.INT)
    def advanceN(self, n):
        for _ in range(n):
            self.advance()

    @method('un_iter.advanceSignificant')
    def advanceSignificant(self):
        self._advance_until(lambda u: u.is_significant())

    @method('un_iter.advanceNSignificant', B.INT)
    def advanceNSignificant(self, n):
        self._advance_n_until(n, lambda u: u.is_significant())

    @method('un_iter.advanceInsignificant')
    def advanceInsignificant(self):
        self._advance_until(lambda u: not u.is_significant())

    @method('un_iter.advanceNInsignificant', B.INT)
    def advanceNInsignificant(self, n):
        self._advance_n_until(n, lambda u: not u.is_significant())

    @method('un_iter.advancePlanet')
    def advancePlanet(self):
        self._advance_until(lambda u: u.is_planet())

    @method('un_iter.advanceNPlanet', B.INT)
    def advanceNPlanet(self, n):
        self._advance_n_until(n, lambda u: u.is_planet())

    @method('un_iter.advanceJumppoint')
    def advanceJumppoint(self):
        self._advance_until(lambda u: u.is_jumppoint())

    @method('un_iter.advanceNJumppoint', B.INT)
    def advanceNJumppoint(self, n):
        self._advance_n_until(n, lambda u: u.is_jumppoint())

    @method('un_iter.next')
    def next(self):
        self.advance()
        return wrap(self._cur())

    @method('un_iter.remove')
    def remove(self):
        u = self._cur()
        if u is not None and u.system is not None:
            u.system.remove(u)
            self._pos += 1

    @method('un_iter.preinsert', B.UNITPTR)
    def preinsert(self, un):
        u = un._get() if un is not None else None
        if u is not None:
            self._units.insert(self._pos, u)


# --------------------------------------------------------------------------
# Python visible unit (exported as VS.Unit)
# --------------------------------------------------------------------------

def wrap(unit):
    w = UnitWrapper()
    w._unit = unit
    return w


def _size_flags(size):
    toks = []
    for t in size.replace(',', ' ').split():
        t = t.upper().replace('-', '')
        if t in ('LIGHT', 'MEDIUM', 'HEAVY', 'CAPSHIPLIGHT', 'CAPSHIPHEAVY', 'SPECIAL',
                 'LIGHTMISSILE', 'MEDIUMMISSILE', 'HEAVYMISSILE', 'CAPSHIPLIGHTMISSILE',
                 'CAPSHIPHEAVYMISSILE', 'SPECIALMISSILE', 'AUTOTRACKING'):
            if t not in toks:
                toks.append(t)
    order = ['LIGHT', 'MEDIUM', 'HEAVY', 'CAPSHIPLIGHT', 'CAPSHIPHEAVY', 'SPECIAL', 'LIGHTMISSILE',
             'MEDIUMMISSILE', 'HEAVYMISSILE', 'CAPSHIPLIGHTMISSILE', 'CAPSHIPHEAVYMISSILE',
             'SPECIALMISSILE', 'AUTOTRACKING']
    toks.sort(key=order.index)
    return ' '.join(toks) if toks else 'NOWEAP'


def _vec(v):
    return (float(v[0]), float(v[1]), float(v[2]))


class UnitWrapper:
    """Mirror of the engine's UnitWrapper/UnitContainer."""

    def __init__(self, *args):
        if args:
            raise B.signature_error('Unit.__init__', (), args)
        self._unit = None

    # ------------------------------------------------------------------
    def _get(self):
        u = self._unit
        if u is None:
            return None
        if u.killed:
            self._unit = None
            return None
        return u

    def _me(self, what):
        u = self._get()
        if u is None:
            eng = _engine()
            if eng is not None:
                eng.note_null_unit_call(what)
        return u

    @property
    def _eng(self):
        return _engine()

    def __repr__(self):
        u = self._unit
        return '<VS.Unit %r>' % (u,)

    # -- odd functions -------------------------------------------------
    @method('Unit.isNull')
    def isNull(self):
        u = self._get()
        if u is None:
            return True
        return u.hull <= 0

    @method('Unit.setNull')
    def setNull(self):
        self._unit = None

    def __bool__(self):
        return not self.isNull()

    # The engine exports __nonzero__ as well; keep it for scripts that call
    # it explicitly.
    def __nonzero__(self):
        return not self.isNull()

    def __eq__(self, other):
        if not isinstance(other, UnitWrapper):
            raise B.signature_error('Unit.__eq__', (B.UNIT,), (other,))
        return self._get() is other._get()

    def __ne__(self, other):
        if not isinstance(other, UnitWrapper):
            raise B.signature_error('Unit.__ne__', (B.UNIT,), (other,))
        return self._get() is not other._get()

    # Boost.Python adds __eq__ after class creation, which leaves the
    # default identity based hash in place (and so do we).
    __hash__ = object.__hash__

    @method('Unit.Kill')
    def Kill(self):
        u = self._me('Kill')
        if u is not None:
            self._eng.kill_unit(u, None)

    @method('Unit.SetTarget', B.UNIT)
    def SetTarget(self, targ):
        u = self._me('SetTarget')
        if u is not None:
            u.target = targ._get()

    @method('Unit.GetTarget')
    def GetTarget(self):
        u = self._me('GetTarget')
        if u is None:
            return UnitWrapper()
        return wrap(u.target if (u.target and not u.target.killed) else None)

    @method('Unit.SetVelocityReference', B.UNIT)
    def SetVelocityReference(self, targ):
        u = self._me('SetVelocityReference')
        if u is not None:
            u.velocity_ref = targ._get()

    @method('Unit.GetVelocityReference')
    def GetVelocityReference(self):
        u = self._me('GetVelocityReference')
        return wrap(u.velocity_ref if u else None)

    @method('Unit.GetOrientation')
    def GetOrientation(self):
        return ((1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0))

    @method('Unit.queryBSP', B.VEC, B.VEC, B.BOOL)
    def queryBSP(self, st, en, shield):
        return (UnitWrapper(), (0.0, 0.0, 1.0), 0.0)

    @method('Unit.cosAngleTo', B.UNIT)
    def cosAngleTo(self, targ):
        u = self._me('cosAngleTo')
        t = targ._get()
        if u is None or t is None:
            return (0.0, 0.0)
        return (1.0, u.distance_to(t))

    @method('Unit.cosAngleToITTS', B.UNIT, B.FLOAT, B.FLOAT)
    def cosAngleToITTS(self, targ, speed, rng):
        return self.cosAngleTo(targ)

    @method('Unit.cosAngleFromMountTo', B.UNIT)
    def cosAngleFromMountTo(self, targ):
        return self.cosAngleTo(targ)

    @method('Unit.getAverageGunSpeed')
    def getAverageGunSpeed(self):
        u = self._me('getAverageGunSpeed')
        if u is None:
            return (1.0, 0.0, 0.0)
        return (1000.0, u.weapon_range, 0.0)

    @method('Unit.InsideCollideTree', B.UNIT)
    def InsideCollideTree(self, smaller):
        z = (0.0, 0.0, 0.0)
        return (z, z, z, z)

    @method('Unit.GetVelocityDifficultyMult')
    def GetVelocityDifficultyMult(self):
        return 1.0

    @method('Unit.GetJumpStatus')
    def GetJumpStatus(self):
        u = self._me('GetJumpStatus')
        if u is None:
            return -1
        return 0 if u.jump_active else -1

    @method('Unit.ApplyDamage', B.VEC, B.VEC, B.FLOAT, B.UNIT, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT)
    def ApplyDamage(self, pnt, normal, amt, dealer, phase, r, g, b, a):
        u = self._me('ApplyDamage')
        if u is not None:
            self._eng.damage_unit(u, amt, dealer._get())

    @method('Unit.GetMountInfo', B.INT)
    def GetMountInfo(self, index):
        u = self._me('GetMountInfo')
        if u is None or index < 0 or index >= len(u.mounts):
            return {'empty': True, 'status': 'UNDEFINED'}
        m = u.mounts[index]
        empty = m['status'] in ('UNCHOSEN', 'DESTROYED') or not m['weapon']
        rv = {'position': (0.0, 0.0, 0.0), 'orientation': (0.0, 0.0, 0.0, 1.0),
              'scale': (1.0, 1.0, 1.0), 'empty': empty, 'volume': float(m.get('volume', -1)),
              'ammo': int(m.get('ammo', -1)), 'size': 0, 'size_flags': _size_flags(m.get('size', '')),
              'bank': False, 'functionality': 1.0, 'maxfunctionality': 1.0,
              'status': m['status']}
        if not empty:
            wi = self._eng.data.weapons.weapons.get(m['weapon'])
            info = dict(wi) if wi else {'type': 'BOLT', 'speed': 1000.0, 'range': 2000.0,
                                        'damage': 1.0, 'phaseDamage': 0.0, 'stability': 1.0,
                                        'longRange': 1.0, 'lockTime': 0.0, 'energyRate': 1.0,
                                        'refire': 1.0, 'volume': 0.0}
            info.pop('mountsize', None)
            info['name'] = m['weapon']
            rv['weapon_info'] = info
        return rv

    @method('Unit.getSubUnits')
    def getSubUnits(self):
        return UnIter([])

    # -- automatic wrap (python_unit_wrap.h) -----------------------------
    @method('Unit.AutoPilotTo', B.UNIT, B.BOOL)
    def AutoPilotTo(self, un, ignore_energy):
        u = self._me('AutoPilotTo')
        t = un._get()
        if u is None or t is None:
            return False
        u.moveto = ('unit', t, self._eng.opts.cruise_speed, 100.0)
        return True

    @method('Unit.SetTurretAI')
    def SetTurretAI(self):
        pass

    @method('Unit.DisableTurretAI')
    def DisableTurretAI(self):
        pass

    @method('Unit.leach', B.FLOAT, B.FLOAT, B.FLOAT)
    def leach(self, a, b, c):
        pass

    @method('Unit.getFgSubnumber')
    def getFgSubnumber(self):
        u = self._me('getFgSubnumber')
        return u.fg_subnumber if u else -1

    @method('Unit.getFgID')
    def getFgID(self):
        u = self._me('getFgID')
        if u is None:
            return ''
        if u.flightgroup is not None:
            return '%s-%d' % (u.flightgroup.name, u.fg_subnumber)
        return u.fullname

    @method('Unit.setFullname', B.STR)
    def setFullname(self, name):
        u = self._me('setFullname')
        if u is not None:
            u.fullname = name

    @method('Unit.getFullname')
    def getFullname(self):
        u = self._me('getFullname')
        return u.fullname if u else ''

    @method('Unit.getFullAIDescription')
    def getFullAIDescription(self):
        u = self._me('getFullAIDescription')
        return u.ai_script if u else ''

    @method('Unit.setTargetFg', B.STR, B.STR, B.STR)
    def setTargetFg(self, a, b, c):
        u = self._me('setTargetFg')
        if u is not None:
            u.target_fgs = (a, b, c)

    @method('Unit.ReTargetFg', B.INT)
    def ReTargetFg(self, which):
        pass

    @method('Unit.isStarShip')
    def isStarShip(self):
        u = self._me('isStarShip')
        return bool(u and u.is_unitptr())

    @method('Unit.isPlanet')
    def isPlanet(self):
        u = self._me('isPlanet')
        return bool(u and u.is_planet())

    @method('Unit.isJumppoint')
    def isJumppoint(self):
        u = self._me('isJumppoint')
        return bool(u and u.is_jumppoint())

    @method('Unit.isEnemy', B.UNIT)
    def isEnemy(self, other):
        u = self._me('isEnemy')
        o = other._get()
        if u is None or o is None:
            return False
        return self._eng.unit_relation(u, o) < 0.0

    @method('Unit.isFriend', B.UNIT)
    def isFriend(self, other):
        u = self._me('isFriend')
        o = other._get()
        if u is None or o is None:
            return False
        return self._eng.unit_relation(u, o) > 0.0

    @method('Unit.isNeutral', B.UNIT)
    def isNeutral(self, other):
        u = self._me('isNeutral')
        o = other._get()
        if u is None or o is None:
            return False
        return self._eng.unit_relation(u, o) == 0.0

    @method('Unit.getRelation', B.UNIT)
    def getRelation(self, other):
        u = self._me('getRelation')
        o = other._get()
        if u is None or o is None:
            return 0.0
        return self._eng.unit_relation(u, o)

    @method('Unit.ToggleWeapon', B.BOOL)
    def ToggleWeapon(self, missile):
        pass

    @method('Unit.SelectAllWeapon', B.BOOL)
    def SelectAllWeapon(self, missile):
        pass

    @method('Unit.Split', B.INT)
    def Split(self, level):
        pass

    @method('Unit.Init')
    def Init(self):
        pass

    @method('Unit.ActivateJumpDrive', B.INT)
    def ActivateJumpDrive(self, dest):
        u = self._me('ActivateJumpDrive')
        if u is not None:
            u.jump_active = True

    @method('Unit.DeactivateJumpDrive')
    def DeactivateJumpDrive(self):
        u = self._me('DeactivateJumpDrive')
        if u is not None:
            u.jump_active = False

    @method('Unit.Destroy')
    def Destroy(self):
        u = self._me('Destroy')
        if u is not None:
            self._eng.kill_unit(u, None)

    @method('Unit.LocalCoordinates', B.UNIT)
    def LocalCoordinates(self, un):
        u = self._me('LocalCoordinates')
        o = un._get()
        if u is None or o is None:
            return (0.0, 0.0, 0.0)
        return tuple(o.position[i] - u.position[i] for i in range(3))

    @method('Unit.InRange', B.UNIT, B.BOOL, B.BOOL)
    def InRange(self, target, cone, cap):
        u = self._me('InRange')
        t = target._get()
        if u is None or t is None:
            return False
        return u.distance_to(t) < 15000.0

    @method('Unit.CloakVisible')
    def CloakVisible(self):
        u = self._me('CloakVisible')
        return 0.0 if (u and u.cloaked) else 1.0

    @method('Unit.Cloak', B.BOOL)
    def Cloak(self, c):
        u = self._me('Cloak')
        if u is not None:
            u.cloaked = c

    @method('Unit.RemoveFromSystem')
    def RemoveFromSystem(self):
        u = self._me('RemoveFromSystem')
        if u is not None and u.system is not None:
            u.system.remove(u)

    @method('Unit.PositionITTS', B.VEC, B.VEC, B.FLOAT, B.BOOL)
    def PositionITTS(self, pos, vel, speed, steady):
        u = self._me('PositionITTS')
        return _vec(u.position) if u else (0.0, 0.0, 0.0)

    @method('Unit.Position')
    def Position(self):
        u = self._me('Position')
        return _vec(u.position) if u else (0.0, 0.0, 0.0)

    @method('Unit.LocalPosition')
    def LocalPosition(self):
        u = self._me('LocalPosition')
        return _vec(u.position) if u else (0.0, 0.0, 0.0)

    @method('Unit.Threat')
    def Threat(self):
        u = self._me('Threat')
        return wrap(u.threat if u else None)

    @method('Unit.TargetTurret', B.UNIT)
    def TargetTurret(self, targ):
        pass

    @method('Unit.Threaten', B.UNIT, B.FLOAT)
    def Threaten(self, targ, danger):
        u = self._me('Threaten')
        if u is not None:
            u.threat = targ._get()

    @method('Unit.ResetThreatLevel')
    def ResetThreatLevel(self):
        u = self._me('ResetThreatLevel')
        if u is not None:
            u.threat = None

    @method('Unit.Fire', B.UINT, B.BOOL)
    def Fire(self, missile, only_target):
        pass

    @method('Unit.UnFire')
    def UnFire(self):
        pass

    @method('Unit.computeLockingPercent')
    def computeLockingPercent(self):
        return 0.0

    def _shield(self, what):
        u = self._me(what)
        return 1.0 if u else 0.0

    @method('Unit.FShieldData')
    def FShieldData(self):
        return self._shield('FShieldData')

    @method('Unit.RShieldData')
    def RShieldData(self):
        return self._shield('RShieldData')

    @method('Unit.LShieldData')
    def LShieldData(self):
        return self._shield('LShieldData')

    @method('Unit.BShieldData')
    def BShieldData(self):
        return self._shield('BShieldData')

    @method('Unit.FuelData')
    def FuelData(self):
        return self._shield('FuelData')

    @method('Unit.EnergyData')
    def EnergyData(self):
        return self._shield('EnergyData')

    @method('Unit.GetHull')
    def GetHull(self):
        u = self._me('GetHull')
        return float(u.hull) if u else 0.0

    @method('Unit.GetHullPercent')
    def GetHullPercent(self):
        u = self._me('GetHullPercent')
        if u is None:
            return 0.0
        return float(u.hull) / u.max_hull if u.max_hull else 1.0

    @method('Unit.rSize')
    def rSize(self):
        u = self._me('rSize')
        return float(u.radius) if u else 0.0

    @method('Unit.getMinDis', B.VEC)
    def getMinDis(self, pnt):
        u = self._me('getMinDis')
        if u is None:
            return 0.0
        d = math.sqrt(sum((u.position[i] - pnt[i]) ** 2 for i in range(3)))
        return d - u.radius

    @method('Unit.querySphere', B.VEC, B.VEC, B.FLOAT)
    def querySphere(self, start, end, rad):
        return 0.0

    @method('Unit.queryBoundingBox', B.VEC, B.VEC, B.FLOAT)
    def queryBoundingBox(self, origin, direction, err):
        return 0

    @method('Unit.PrimeOrders')
    def PrimeOrders(self):
        u = self._me('PrimeOrders')
        if u is not None:
            u.dock_order = None
            u.moveto = None

    @method('Unit.LoadAIScript', B.STR)
    def LoadAIScript(self, script):
        u = self._me('LoadAIScript')
        if u is not None:
            self._eng.set_ai_script(u, script)

    @method('Unit.LoadLastPythonAIScript')
    def LoadLastPythonAIScript(self):
        return False

    @method('Unit.EnqueueLastPythonAIScript')
    def EnqueueLastPythonAIScript(self):
        return False

    @method('Unit.SetPosition', B.VEC)
    def SetPosition(self, pos):
        u = self._me('SetPosition')
        if u is not None:
            u.position = list(pos)

    @method('Unit.SetCurPosition', B.VEC)
    def SetCurPosition(self, pos):
        self.SetPosition(pos)

    @method('Unit.SetPosAndCumPos', B.VEC)
    def SetPosAndCumPos(self, pos):
        self.SetPosition(pos)

    @method('Unit.Rotate', B.VEC)
    def Rotate(self, axis):
        pass

    @method('Unit.ApplyForce', B.VEC)
    def ApplyForce(self, f):
        pass

    @method('Unit.ApplyLocalForce', B.VEC)
    def ApplyLocalForce(self, f):
        pass

    @method('Unit.Accelerate', B.VEC)
    def Accelerate(self, f):
        pass

    @method('Unit.ApplyTorque', B.VEC, B.VEC)
    def ApplyTorque(self, f, loc):
        pass

    @method('Unit.ApplyBalancedLocalTorque', B.VEC, B.VEC)
    def ApplyBalancedLocalTorque(self, f, loc):
        pass

    @method('Unit.ApplyLocalTorque', B.VEC)
    def ApplyLocalTorque(self, t):
        pass

    @method('Unit.DealDamageToHull', B.VEC, B.FLOAT)
    def DealDamageToHull(self, pnt, dmg):
        u = self._me('DealDamageToHull')
        if u is None:
            return 0.0
        self._eng.damage_unit(u, dmg, None)
        return float(u.hull)

    @method('Unit.ClampThrust', B.VEC, B.BOOL)
    def ClampThrust(self, thrust, ab):
        return thrust

    @method('Unit.Thrust', B.VEC, B.BOOL)
    def Thrust(self, amt, ab):
        pass

    @method('Unit.LateralThrust', B.FLOAT)
    def LateralThrust(self, amt):
        pass

    @method('Unit.VerticalThrust', B.FLOAT)
    def VerticalThrust(self, amt):
        pass

    @method('Unit.LongitudinalThrust', B.FLOAT)
    def LongitudinalThrust(self, amt):
        pass

    @method('Unit.ClampVelocity', B.VEC, B.BOOL)
    def ClampVelocity(self, v, ab):
        return v

    @method('Unit.ClampAngVel', B.VEC)
    def ClampAngVel(self, v):
        return v

    @method('Unit.ClampTorque', B.VEC)
    def ClampTorque(self, v):
        return v

    @method('Unit.SetOrientation', B.VEC, B.VEC)
    def SetOrientation(self, q, r):
        pass

    @method('Unit.UpCoordinateLevel', B.VEC)
    def UpCoordinateLevel(self, v):
        return v

    @method('Unit.DownCoordinateLevel', B.VEC)
    def DownCoordinateLevel(self, v):
        return v

    @method('Unit.ToLocalCoordinates', B.VEC)
    def ToLocalCoordinates(self, v):
        return v

    @method('Unit.ToWorldCoordinates', B.VEC)
    def ToWorldCoordinates(self, v):
        return v

    @method('Unit.GetAngularVelocity')
    def GetAngularVelocity(self):
        return (0.0, 0.0, 0.0)

    @method('Unit.GetVelocity')
    def GetVelocity(self):
        u = self._me('GetVelocity')
        return _vec(u.velocity) if u else (0.0, 0.0, 0.0)

    @method('Unit.SetVelocity', B.VEC)
    def SetVelocity(self, v):
        u = self._me('SetVelocity')
        if u is not None:
            u.velocity = list(v)

    @method('Unit.SetAngularVelocity', B.VEC)
    def SetAngularVelocity(self, v):
        pass

    @method('Unit.GetMoment')
    def GetMoment(self):
        return 1.0

    @method('Unit.GetMass')
    def GetMass(self):
        return 1.0

    @method('Unit.LockMissile')
    def LockMissile(self):
        return 0

    @method('Unit.EjectCargo', B.INT)
    def EjectCargo(self, index):
        u = self._me('EjectCargo')
        if u is not None:
            self._eng.eject_cargo(u, index)

    @method('Unit.PriceCargo', B.STR)
    def PriceCargo(self, s):
        u = self._me('PriceCargo')
        if u is None:
            return 0.0
        for c in u.cargo:
            if c._content == s:
                return c._price
        part = self._eng.data.parts.by_name.get(s)
        return float(part['price']) if part else 0.0

    @method('Unit.numCargo')
    def numCargo(self):
        u = self._me('numCargo')
        return len(u.cargo) if u else 0

    @method('Unit.IsCleared', B.UNIT)
    def IsCleared(self, docking):
        return True

    @method('Unit.ImportPartList', B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT)
    def ImportPartList(self, cat, price, pricedev, quant, quantdev):
        pass

    @method('Unit.RequestClearance', B.UNIT)
    def RequestClearance(self, docking):
        return True

    @method('Unit.isDocked', B.UNIT)
    def isDocked(self, other):
        u = self._me('isDocked')
        o = other._get()
        if u is None or o is None:
            return False
        # this->isDocked(other): is `other` docked to this unit?
        return o.docked_to is u

    @method('Unit.Dock', B.UNIT)
    def Dock(self, other):
        u = self._me('Dock')
        o = other._get()
        if u is None or o is None:
            return False
        return self._eng.dock(u, o, check_distance=True)

    @method('Unit.setCombatRole', B.STR)
    def setCombatRole(self, role):
        u = self._me('setCombatRole')
        if u is not None:
            u.combat_role = role

    @method('Unit.getCombatRole')
    def getCombatRole(self):
        u = self._me('getCombatRole')
        return u.combat_role if u else 'INERT'

    @method('Unit.setAttackPreference', B.STR)
    def setAttackPreference(self, role):
        u = self._me('setAttackPreference')
        if u is not None:
            u.attack_preference = role

    @method('Unit.getAttackPreference')
    def getAttackPreference(self):
        u = self._me('getAttackPreference')
        return u.attack_preference if u else 'INERT'

    @method('Unit.setUnitRole', B.STR)
    def setUnitRole(self, role):
        u = self._me('setUnitRole')
        if u is not None:
            u.unit_role = role

    @method('Unit.getUnitRole')
    def getUnitRole(self):
        u = self._me('getUnitRole')
        return u.unit_role if u else 'INERT'

    @method('Unit.UnDock', B.UNIT)
    def UnDock(self, other):
        u = self._me('UnDock')
        o = other._get()
        if u is None or o is None:
            return False
        return self._eng.undock(u, o)

    @method('Unit.DockedOrDocking')
    def DockedOrDocking(self):
        u = self._me('DockedOrDocking')
        if u is None:
            return 0
        v = 0
        if u.docked_to is not None:
            v |= 2
        if u.docked_units:
            v |= 4
        return v

    @method('Unit.GetNumMounts')
    def GetNumMounts(self):
        u = self._me('GetNumMounts')
        return len(u.mounts) if u else 0

    # -- UnitUtil exports -------------------------------------------------
    @method('Unit.owner')
    def owner(self):
        return wrap(None)

    @method('Unit.performDockingOperations', B.UNIT, B.INT)
    def performDockingOperations(self, other, actually):
        u = self._me('performDockingOperations')
        o = other._get()
        if u is not None and o is not None:
            u.dock_order = (o, bool(actually))
            u.moveto = None

    @method('Unit.getFactionName')
    def getFactionName(self):
        u = self._get()
        return u.faction if u else ''

    @method('Unit.getFactionIndex')
    def getFactionIndex(self):
        u = self._get()
        return self._eng.factions.ensure(u.faction) if u else 0

    @method('Unit.setFactionIndex', B.INT)
    def setFactionIndex(self, idx):
        u = self._get()
        if u is not None and 0 <= idx < len(self._eng.factions.names):
            u.faction = self._eng.factions.names[idx]

    @method('Unit.setFactionName', B.STR)
    def setFactionName(self, name):
        u = self._get()
        if u is not None:
            if self._eng.factions.get_index(name) < 0:
                name = 'neutral'  # unknown faction name maps to index 0
            u.faction = name

    @method('Unit.getFactionRelation', B.UNIT)
    def getFactionRelation(self, other):
        u = self._get()
        o = other._get()
        if u is None or o is None:
            return 0.0
        return self._eng.faction_relation_units(u, o)

    @method('Unit.getRelationToFaction', B.INT)
    def getRelationToFaction(self, fac):
        u = self._get()
        if u is None:
            return 0.0
        return self._eng.relation_to_faction(u, fac)

    @method('Unit.getRelationFromFaction', B.INT)
    def getRelationFromFaction(self, fac):
        u = self._get()
        if u is None:
            return 0.0
        return self._eng.relation_from_faction(u, fac)

    @method('Unit.getName')
    def getName(self):
        u = self._get()
        return u.name if u else ''

    @method('Unit.setName', B.STR)
    def setName(self, name):
        u = self._get()
        if u is not None:
            u.name = name

    @method('Unit.SetHull', B.FLOAT)
    def SetHull(self, hull):
        u = self._get()
        if u is not None:
            u.hull = hull
            if hull > u.max_hull:
                u.max_hull = hull

    @method('Unit.getFlightgroupName')
    def getFlightgroupName(self):
        u = self._get()
        return u.fg_name() if u else ''

    @method('Unit.getFlightgroupLeader')
    def getFlightgroupLeader(self):
        u = self._get()
        if u is None:
            return wrap(None)
        if u.flightgroup is None:
            return wrap(u)
        lead = u.flightgroup.leader
        return wrap(lead if lead is not None and not lead.killed else None)

    @method('Unit.setFlightgroupLeader', B.UNIT)
    def setFlightgroupLeader(self, un):
        u = self._get()
        o = un._get()
        if u is None or o is None:
            return False
        if u.flightgroup is not None:
            u.flightgroup.leader = o
            return True
        return False

    @method('Unit.getFgDirective')
    def getFgDirective(self):
        u = self._get()
        if u is None:
            return ''
        if u.flightgroup is not None:
            return u.flightgroup.directive
        return 'b'

    @method('Unit.RecomputeUnitUpgrades')
    def RecomputeUnitUpgrades(self):
        pass

    @method('Unit.setFgDirective', B.STR)
    def setFgDirective(self, inp):
        u = self._get()
        if u is None:
            return False
        if u.flightgroup is not None:
            u.flightgroup.directive = inp
            return True
        return False

    @method('Unit.removeCargo', B.STR, B.INT, B.BOOL)
    def removeCargo(self, s, quantity, erasezero):
        u = self._get()
        if u is None:
            return 0
        return self._eng.remove_cargo(u, s, quantity, erasezero)

    @method('Unit.removeWeapon', B.STR, B.INT, B.BOOL)
    def removeWeapon(self, name, offset, loop):
        u = self._get()
        if u is None:
            return -1
        maxmount = len(u.mounts)
        for loopi in range(offset, maxmount + offset):
            i = loopi % maxmount
            m = u.mounts[i]
            if m['weapon'] == name and m['status'] in ('ACTIVE', 'INACTIVE'):
                m['status'] = 'UNCHOSEN'
                return i
        return -1

    @method('Unit.upgrade', B.STR, B.INT, B.INT, B.BOOL, B.BOOL)
    def upgrade(self, file, mountoffset, subunitoffset, force, loop):
        u = self._get()
        if u is None:
            return 0.0
        return self._eng.upgrade_unit(u, file, mountoffset, subunitoffset, force, loop)

    @method('Unit.addCargo', B.CARGO)
    def addCargo(self, carg):
        u = self._get()
        if u is None:
            return 0
        return self._eng.add_cargo(u, carg.copy(), force=False)

    @method('Unit.forceAddCargo', B.CARGO)
    def forceAddCargo(self, carg):
        u = self._get()
        if u is None:
            return 0
        return self._eng.add_cargo(u, carg.copy(), force=True)

    @method('Unit.incrementCargo', B.FLOAT, B.INT)
    def incrementCargo(self, pct, quantity):
        u = self._get()
        if u is None or not u.cargo:
            return False
        idx = self._eng.rng.randrange(len(u.cargo))
        c = u.cargo[idx].copy()
        c._quantity = quantity
        if c._price != 0 and self._eng.can_add_cargo(u, c):
            self._eng.add_cargo(u, c, force=True)
            u.cargo[idx]._price *= pct
            return True
        return False

    @method('Unit.decrementCargo', B.FLOAT)
    def decrementCargo(self, pct):
        u = self._get()
        if u is None or not u.cargo:
            return False
        idx = self._eng.rng.randrange(len(u.cargo))
        c = u.cargo[idx]
        if c._quantity > 0:
            c._quantity -= 1
            c._price *= pct
        return True

    @method('Unit.getDistance', B.UNIT)
    def getDistance(self, un):
        u = self._get()
        o = un._get()
        if u is None or o is None:
            return 3.4028234663852886e+38
        return u.distance_to(o)

    @method('Unit.getSignificantDistance', B.UNIT)
    def getSignificantDistance(self, sig):
        u = self._get()
        o = sig._get()
        if u is None or o is None:
            return 3.4028234663852886e+38
        return u.significant_distance_to(o)

    @method('Unit.hasCargo', B.STR)
    def hasCargo(self, name):
        u = self._get()
        if u is None:
            return 0
        for c in u.cargo:
            if c._content == name:
                return c._quantity
        return 0

    @method('Unit.GetCargoIndex', B.INT)
    def GetCargoIndex(self, index):
        u = self._get()
        if u is not None and 0 <= index < len(u.cargo):
            return u.cargo[index].copy()
        c = Cargo()
        c._quantity = 0
        return c

    @method('Unit.GetCargo', B.STR)
    def GetCargo(self, name):
        u = self._get()
        if u is not None:
            for c in u.cargo:
                if c._content == name:
                    return c.copy()
        c = Cargo()
        c._quantity = 0
        return c

    @method('Unit.getUnitSystemFile')
    def getUnitSystemFile(self):
        u = self._get()
        if u is None:
            return self._eng.active_system_name()
        return u.system_file()

    @method('Unit.getCredits')
    def getCredits(self):
        u = self._get()
        if u is None:
            return 0.0
        return self._eng.get_credits(u)

    @method('Unit.addCredits', B.FLOAT)
    def addCredits(self, credits):
        u = self._get()
        if u is not None:
            self._eng.add_credits(u, credits)

    @method('Unit.isSignificant')
    def isSignificant(self):
        u = self._get()
        return bool(u and u.is_significant())

    @method('Unit.isCapitalShip')
    def isCapitalShip(self):
        u = self._get()
        return bool(u and u.unit_role in ('ESCORTCAP', 'CAPITAL', 'CARRIER', 'BASE', 'TROOP'))

    @method('Unit.isDockableUnit')
    def isDockableUnit(self):
        u = self._get()
        return bool(u and u.is_dockable())

    @method('Unit.isSun')
    def isSun(self):
        u = self._get()
        return bool(u and u.is_sun())

    @method('Unit.isAsteroid')
    def isAsteroid(self):
        u = self._get()
        return bool(u and (u.kind == 'asteroid' or u.fg_name() == 'Asteroid'))

    @method('Unit.switchFg', B.STR)
    def switchFg(self, arg):
        u = self._get()
        if u is not None:
            self._eng.switch_fg(u, arg)

    @method('Unit.communicateTo', B.UNIT, B.FLOAT)
    def communicateTo(self, other, mood):
        return 0

    @method('Unit.commAnimation', B.STR)
    def commAnimation(self, anim):
        return True

    @method('Unit.JumpTo', B.STR)
    def JumpTo(self, system):
        u = self._get()
        if u is None:
            return False
        return self._eng.request_jump_to(u, system)

    @method('Unit.isPlayerStarship')
    def isPlayerStarship(self):
        u = self._get()
        if u is not None and u is self._eng.player:
            return 0
        return -1

    @method('Unit.setECM', B.INT)
    def setECM(self, ecm):
        pass

    @method('Unit.getECM')
    def getECM(self):
        return 0

    @method('Unit.setSpeed', B.FLOAT)
    def setSpeed(self, speed):
        u = self._get()
        if u is not None:
            u.set_speed = speed

    @method('Unit.maxSpeed')
    def maxSpeed(self):
        u = self._get()
        return float(u.max_speed) if u else 0.0

    @method('Unit.maxAfterburnerSpeed')
    def maxAfterburnerSpeed(self):
        u = self._get()
        return float(u.max_ab_speed) if u else 0.0

    @method('Unit.setMissionRelevant')
    def setMissionRelevant(self):
        u = self._get()
        if u is not None:
            u.mission_relevant = True
            self._eng.note_mission_relevant(u)

    @method('Unit.orbit', B.UNIT, B.FLOAT, B.VEC, B.VEC, B.VEC)
    def orbit(self, orbitee, speed, R, S, center):
        u = self._get()
        if u is not None:
            u.orbit = (orbitee._get(), speed, R, S, center)

    @method('Unit.RepairCost')
    def RepairCost(self):
        return 0

    @method('Unit.RepairUpgrade')
    def RepairUpgrade(self):
        return 0

    @method('Unit.PercentOperational', B.STR, B.STR, B.BOOL)
    def PercentOperational(self, content, category, hull_armor_full):
        return 1.0

    @method('Unit.MountPercentOperational', B.INT)
    def MountPercentOperational(self, which):
        u = self._get()
        if u is None or which < 0 or which >= len(u.mounts):
            return (-1.0, -1.0, -1.0)
        return (1.0, 1.0, 1.0)


def _engine():
    from . import engine as _e
    return _e.CURRENT


B.register_unit_types(UnitWrapper, Cargo)
