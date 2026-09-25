"""The mock engine core.

This models just enough of Vega Strike for the Python data pack to run:
star systems populated from the .system files, units with trivial
combat stats, missions whose ``Execute`` is called every frame, the
save-game key/value store (``Director.*SaveData``), faction relations, the
base interface and a simple frame loop that moves ships and resolves
combat once per game second ("turn").
"""

import builtins
import collections
import math
import os
import random
import re
import struct
import sys
import traceback
import xml.etree.ElementTree as ET

from . import data as vsdata
from .units import MockUnit, Flightgroup, Cargo, UnitWrapper, wrap, cargo_is_upgrade

CURRENT = None


def f32(v):
    """Round to a C float, like the engine's save data storage."""
    try:
        return struct.unpack('f', struct.pack('f', v))[0]
    except (OverflowError, struct.error):
        return float('inf') if v > 0 else float('-inf')


def cargo_unit_name(filename):
    """getCargoUnitName: strip directories, keep leading [A-Za-z_]."""
    base = filename.replace('\\', '/').split('/')[-1]
    m = re.match(r'[A-Za-z_]*', base)
    return m.group(0) if m else ''


class Options:
    """Scenario knobs.  Everything the user may want to tweak per run."""

    def __init__(self, **kw):
        self.seed = 1
        self.dt = 0.25                 # seconds of game time per frame
        self.turn = 1.0                # seconds per combat turn
        self.player_hp = 10000.0
        self.enemy_hp = 20.0           # ships hostile to the player at launch
        self.npc_hp = 10000.0          # everybody else
        self.damage = 1.0              # damage per turn per ship
        self.player_damage = 5.0
        self.weapon_range = 1500.0
        self.detect_range = 12000.0
        self.cruise_speed = 3000.0     # player autopilot speed
        self.hp_overrides = []         # list of (predicate(unit)->bool, hp)
        self.verbose = 1
        self.save_dir = None
        self.new_game = 'New_Game'
        self.max_same_error = 3        # how many identical tracebacks to print
        self.repair_on_dock = True     # the player's hull is restored when docking
        self.__dict__.update(kw)


class ScriptError:
    def __init__(self, context, exc_type, message, tb_text, gametime):
        self.context = context
        self.exc_type = exc_type
        self.message = message
        self.traceback = tb_text
        self.count = 1
        self.first_time = gametime

    def key(self):
        # dedupe on the innermost frame + message
        last = [l for l in self.traceback.strip().splitlines() if l.strip().startswith('File')]
        return (self.exc_type, self.message, last[-1] if last else '')

    def summary(self):
        return '%s: %s: %s (x%d)' % (self.context, self.exc_type, self.message, self.count)


class StarSystem:
    def __init__(self, engine, name):
        self.engine = engine
        self.name = name
        self.units = []
        self.suns = []
        self._static_fgs = {}

    def add(self, unit):
        # the engine prepends to its draw list
        if unit.system is not None and unit.system is not self:
            unit.system.remove(unit)
        if unit not in self.units:
            self.units.insert(0, unit)
        unit.system = self

    def remove(self, unit):
        try:
            self.units.remove(unit)
        except ValueError:
            pass

    def live_units(self):
        return [u for u in self.units if not u.killed]

    def __repr__(self):
        return '<StarSystem %s (%d units)>' % (self.name, len(self.units))


class Mission:
    _ids = 0

    def __init__(self, engine, name, script):
        Mission._ids += 1
        self.id = Mission._ids
        self.engine = engine
        self.name = name
        self.script = script
        self.pyobj = None
        self.gametime = 0.0
        self.objectives = []
        self.player_num = 0
        self.flightgroups = {}
        self.terminated = False
        self.won = None

    def type_name(self):
        return type(self.pyobj).__name__ if self.pyobj is not None else '<none>'

    def __repr__(self):
        return '<Mission #%d %s %s>' % (self.id, self.name, self.type_name())


class Objective:
    def __init__(self, text):
        self.text = text
        self.completeness = 0.0
        self.owner = None


class Engine:
    def __init__(self, datadir, options=None, log=None):
        global CURRENT
        CURRENT = self
        self.opts = options or Options()
        self.rng = random.Random(self.opts.seed)
        self.log = log or (lambda *a: None)
        self.data = vsdata.DataDir(datadir)
        self.factions = self.data.factions
        self.galaxy = self.data.galaxy
        self.galaxy_faction = {}
        self.systems = {}
        self.active_stack = []
        self.player = None
        self.player_fg = None
        self.credits = 0.0
        self.floats = {}
        self.strings = {}
        self.save_name = ''
        self.missions = []
        self.current_mission = None
        self.delayed_missions = []
        self.last_python_mission = None
        self.last_python_ai = None
        self.base = None
        self.main_globals = {'__name__': '__main__', '__builtins__': builtins}
        self.errors = []
        self._error_index = {}
        self.warnings = collections.Counter()
        self.io_messages = []
        self.frame = 0
        self.realtime = 0.0
        self.combat_accum = 0.0
        self.pending_load = None
        self.pending_jumps = []
        self.difficulty = float(self.data.config.get('general', 'difficulty', '1') or 1)
        self.scratch_unit = None
        self.scratch_vector = (0.0, 0.0, 0.0)
        self.music_lists = []
        self.event_data = {}
        self.target_label = ''
        self.sounds = []
        self.master_part_list = None
        self.contraband_lists = {}
        self.events = []              # structured event log for the driver/report
        self.mission_relevant = []
        self.null_unit_calls = collections.Counter()
        self.system_jumppoints = None
        self.quit_requested = False
        from . import ai as _ai
        from . import base as _base
        self.ai = _ai
        self.basemod = _base

    # ------------------------------------------------------------------
    # logging / errors
    # ------------------------------------------------------------------
    def event(self, kind, **kw):
        kw['kind'] = kind
        kw['time'] = self.game_time_total()
        self.events.append(kw)
        if self.opts.verbose >= 2:
            self.log('[event] %s %s' % (kind, {k: v for k, v in kw.items() if k not in ('kind',)}))

    def record_error(self, context):
        et, ev, tb = sys.exc_info()
        tb_text = ''.join(traceback.format_exception(et, ev, tb))
        err = ScriptError(context, et.__name__ if et else '?', str(ev), tb_text,
                          self.game_time_total())
        k = err.key()
        if k in self._error_index:
            prev = self._error_index[k]
            prev.count += 1
            if prev.count <= self.opts.max_same_error and self.opts.verbose:
                self.log('[python error again x%d] %s' % (prev.count, prev.summary()))
        else:
            self._error_index[k] = err
            self.errors.append(err)
            if self.opts.verbose:
                self.log('[python error] in %s\n%s' % (context, tb_text))
            self.event('python_error', context=context, error='%s: %s' % (err.exc_type, err.message))
        del tb

    def warn(self, msg):
        if msg not in self.warnings and self.opts.verbose:
            self.log('[warning] ' + msg)
        self.warnings[msg] += 1

    def note_null_unit_call(self, what):
        self.null_unit_calls[what] += 1

    def note_mission_relevant(self, u):
        if u not in self.mission_relevant:
            self.mission_relevant.append(u)

    # ------------------------------------------------------------------
    # python execution helpers (mirror of PyRun_* / CompileRunPython)
    # ------------------------------------------------------------------
    def run_string(self, code, context, filename='<string>'):
        """PyRun_SimpleString: globals=locals=__main__."""
        try:
            compiled = self.compile(code, filename)
        except BaseException:
            self.record_error(context + ' (compile)')
            return False
        try:
            exec(compiled, self.main_globals)
            return True
        except SystemExit:
            self.record_error(context)
            return False
        except BaseException:
            self.record_error(context)
            return False

    def compile(self, code, filename):
        from . import divcheck
        if divcheck.ENABLED:
            return divcheck.transform_compile(code, filename)
        return compile(code, filename, 'exec')

    def resolve_path(self, rel):
        rel = rel.replace('\\', '/')
        candidates = [rel, os.path.join('modules', rel), os.path.join('bases', rel),
                      os.path.join('ai', 'script', rel), os.path.join('ai', rel)]
        for c in candidates:
            p = self.data.path(c)
            if os.path.isfile(p):
                return p
        return None

    def run_file_compiled(self, rel, context):
        """CompileRunPython: globals=__main__, locals=a fresh dict."""
        path = self.resolve_path(rel)
        if path is None:
            self.warn('python file %s not found (%s)' % (rel, context))
            return False
        with open(path, encoding='latin-1') as fh:
            src = fh.read()
        try:
            compiled = self.compile(src, path)
        except BaseException:
            self.record_error(context + ' (compile %s)' % rel)
            return False
        try:
            exec(compiled, self.main_globals, {})
            return True
        except BaseException:
            self.record_error(context)
            return False

    def run_file_simple(self, path, context):
        """PyRun_SimpleFile: globals=locals=__main__."""
        with open(path, encoding='latin-1') as fh:
            src = fh.read()
        return self.run_string(src, context, filename=path)

    def run_link_python(self, pythonfile, context):
        """BaseInterface RunPython(): '#...' is code, otherwise a file."""
        if not pythonfile:
            return True
        if pythonfile[0] == '#' and len(pythonfile) > 1:
            return self.run_string(pythonfile, context)
        return self.run_file_compiled(pythonfile, context)

    # ------------------------------------------------------------------
    # save data (Director module)
    # ------------------------------------------------------------------
    def get_save_data(self, cp, key, num):
        if cp != 0:
            return 0.0
        v = self.floats.get(key)
        if v is None or num >= len(v):
            return 0.0
        return v[num]

    def get_save_data_length(self, cp, key):
        if cp != 0:
            return 0
        return len(self.floats.get(key, ()))

    def push_save_data(self, cp, key, val):
        if cp != 0:
            return 0
        lst = self.floats.setdefault(key, [])
        lst.append(f32(val))
        if val == -1 and len(lst) == 1:
            self.event('var_failed', key=key, by=self._mission_label(), mid=self.current_mission.id if self.current_mission else 0)
        return len(lst) - 1

    def put_save_data(self, cp, key, num, val):
        if cp != 0:
            return
        lst = self.floats.setdefault(key, [])
        if num < len(lst):
            if val == -1 and num == 0 and lst[0] != -1:
                self.event('var_failed', key=key, by=self._mission_label(), mid=self.current_mission.id if self.current_mission else 0)
            lst[num] = f32(val)

    def erase_save_data(self, cp, key, index):
        if cp != 0:
            return 0
        lst = self.floats.setdefault(key, [])
        if index < len(lst):
            del lst[index]
        return len(lst)

    def clear_save_data(self, cp, key):
        if cp != 0:
            return 0
        lst = self.floats.setdefault(key, [])
        n = len(lst)
        del lst[:]
        return n

    def get_save_string(self, cp, key, num):
        if cp != 0:
            return ''
        v = self.strings.get(key)
        if v is None or num >= len(v):
            return ''
        return v[num]

    def get_save_string_length(self, cp, key):
        if cp != 0:
            return 0
        return len(self.strings.get(key, ()))

    def push_save_string(self, cp, key, val):
        if cp != 0:
            return 0
        lst = self.strings.setdefault(key, [])
        lst.append(val)
        return len(lst) - 1

    def put_save_string(self, cp, key, num, val):
        if cp != 0:
            return
        lst = self.strings.setdefault(key, [])
        if num < len(lst):
            lst[num] = val

    def erase_save_string(self, cp, key, index):
        if cp != 0:
            return 0
        lst = self.strings.setdefault(key, [])
        if index < len(lst):
            del lst[index]
        return len(lst)

    def clear_save_string(self, cp, key):
        if cp != 0:
            return 0
        lst = self.strings.setdefault(key, [])
        n = len(lst)
        del lst[:]
        return n

    def save_string_list(self, key, names):
        """saveStringList(): strings encoded as floats."""
        lst = self.floats.setdefault(key, [])
        tot = sum(len(n) + 1 for n in names)
        seq = [float(tot)]
        for n in names:
            seq.extend(float(ord(c)) for c in n)
            seq.append(0.0)
        for i, v in enumerate(seq):
            if i < len(lst):
                lst[i] = v
            else:
                lst.append(v)

    def load_string_list(self, key):
        lst = self.floats.get(key, [])
        if not lst:
            return []
        length = int(lst[0])
        out = []
        cur = ''
        for j in range(length):
            if j + 1 >= len(lst):
                break
            ch = int(lst[j + 1])
            if ch != 0:
                cur += chr(ch)
            else:
                out.append(cur)
                cur = ''
        return out

    # ------------------------------------------------------------------
    # factions / relations
    # ------------------------------------------------------------------
    def player_faction(self):
        return self.player.faction if self.player is not None else None

    def relation_modifier(self, faction):
        v = self.floats.get('Relation_to_' + faction)
        if not v:
            return 0.0
        return min(v[0], 1.0)

    def adjust_relation_modifier(self, faction, delta):
        if delta > 1:
            delta = 1.0
        key = 'Relation_to_' + faction
        if not self.floats.get(key):
            self.floats[key] = [f32(delta)]
        val = self.floats[key][0] + delta
        if val > 1:
            val = 1.0
        self.floats[key][0] = f32(val)

    def fg_relation_modifier(self, fg):
        v = self.floats.get('FG_Relation_' + fg)
        return v[0] if v else 0.0

    def adjust_fg_relation_modifier(self, fg, delta):
        key = 'FG_Relation_' + fg
        if not self.floats.get(key):
            self.floats[key] = [f32(delta)]
            return
        self.floats[key][0] = f32(self.floats[key][0] + delta)

    def get_relation(self, a, b):
        """VS.GetRelation semantics."""
        pf = self.player_faction()
        if pf is None:
            return self.factions.relation(a, b)
        if a == b:
            return 0.0
        if a == pf:
            return self.relation_modifier(b)
        if b == pf:
            return self.relation_modifier(a)
        return self.factions.relation(a, b)

    def adjust_relation(self, a, b, factor, rank):
        pf = self.player_faction()
        if pf is None or a == b:
            return
        if a == pf:
            self.adjust_relation_modifier(b, factor * rank)
        elif b == pf:
            self.adjust_relation_modifier(a, factor * rank)

    def faction_relation_units(self, u, o):
        rel = self.factions.relation(u.faction, o.faction)
        if u is self.player:
            rel += self.relation_modifier(o.faction)
        elif o is self.player:
            rel += self.relation_modifier(u.faction)
        return rel

    def relation_to_faction(self, u, facidx):
        fac = self.factions.names[facidx] if 0 <= facidx < len(self.factions.names) else ''
        rel = self.factions.relation(u.faction, fac)
        if u is self.player:
            rel += self.relation_modifier(fac)
        return rel

    def relation_from_faction(self, u, facidx):
        fac = self.factions.names[facidx] if 0 <= facidx < len(self.factions.names) else ''
        rel = self.factions.relation(fac, u.faction)
        if u is self.player:
            rel += self.relation_modifier(fac)
        return rel

    def unit_relation(self, u, o):
        """Pilot::GetEffectiveRelationship: anger + faction relation."""
        rel = u.anger.get(o.serial, 0.0)
        if o is self.player:
            if 'pirates' in u.faction:
                empty = all(c._quantity == 0 or 'upgrades' in c._category for c in o.cargo)
                if empty:
                    rel += 0.75
            if u.flightgroup is not None:
                rel += self.fg_relation_modifier(u.flightgroup.name)
        if u is self.player and o.flightgroup is not None:
            rel += self.fg_relation_modifier(o.flightgroup.name)
        return rel + self.faction_relation_units(u, o)

    # ------------------------------------------------------------------
    # galaxy
    # ------------------------------------------------------------------
    def galaxy_property(self, sysname, prop):
        if prop == 'faction' and sysname in self.galaxy_faction:
            return self.galaxy_faction[sysname]
        v = self.galaxy.lookup(sysname, prop)
        return v

    def get_galaxy_faction(self, sysname):
        if sysname in self.galaxy_faction:
            return self.galaxy_faction[sysname]
        return self.galaxy.lookup(sysname, 'faction') or ''

    def adjacent_systems(self, sysname):
        return self.galaxy.jumps(sysname)

    def all_jump_links(self):
        """sys -> {dest: jumppoint name} from the .system files."""
        if self.system_jumppoints is None:
            links = {}
            for name in self.data.all_system_names():
                lay = self.data.system_layout(name)
                if lay is None:
                    continue
                d = {}
                for obj in lay.objects:
                    dest = obj.attrs.get('destination')
                    if dest:
                        for dd in dest.split():
                            d.setdefault(dd, obj.attrs.get('name'))
                links[name] = d
            self.system_jumppoints = links
        return self.system_jumppoints

    # ------------------------------------------------------------------
    # star systems
    # ------------------------------------------------------------------
    def get_system(self, name, load=True):
        s = self.systems.get(name)
        if s is None and load:
            s = self.load_system(name)
        return s

    def load_system(self, name):
        s = StarSystem(self, name)
        self.systems[name] = s
        lay = self.data.system_layout(name)
        if lay is None:
            self.warn('star system %s has no .system file' % name)
            return s
        original = self.galaxy.original_factions.get(name, '')
        current = self.get_galaxy_faction(name)
        basefg = Flightgroup('Base', '', '')
        counters = collections.Counter()
        for obj in lay.objects:
            a = obj.attrs
            if obj.kind == 'planet':
                filename = a.get('file', '')
                u = MockUnit(self, a.get('name', 'unknw'), cargo_unit_name(filename), 'neutral', 'planet')
                u.planet_file = filename
                u.radius = float(a.get('radius', '1000') or 1000)
                dests = a.get('destination', '')
                if dests:
                    u.destinations = dests.split()
                    u.kind = 'jump'
                elif 'light' in a:
                    u.lights = True
                    u.kind = 'sun'
                elif u.fullname == 'invisible' or u.name.lower().startswith('nav'):
                    u.kind = 'nav'
                u.docking_ports = not u.destinations and not u.lights and u.fullname != 'invisible'
                if u.docking_ports:
                    prow = self.data.units.lookup(u.fullname, current or 'neutral')
                    if prow is not None:
                        self.import_cargo(u, prow.get('Cargo_Import', ''))
                        u.cargo.sort(key=lambda c: c.sort_key())
                if self.data.config.get('physics', 'planets_always_neutral', 'true') != 'true':
                    u.faction = current or 'neutral'
                u.hull = u.max_hull = 1e12
                u.max_speed = 0.0
            elif obj.kind in ('unit', 'nebula', 'asteroid', 'enhancement'):
                fac = a.get('faction', 'neutral')
                if fac and fac == original and current:
                    fac = current
                typ = a.get('file', '')
                kind = {'unit': 'base', 'nebula': 'nebula', 'asteroid': 'asteroid',
                        'enhancement': 'enhancement'}[obj.kind]
                u = self.create_unit(typ, fac, kind=kind)
                if obj.kind == 'unit':
                    u.fullname = a.get('name', 'unkn-unit')
                    fgname = 'Base'
                elif obj.kind == 'nebula':
                    fgname = 'Nebula'
                else:
                    fgname = 'Asteroid'
                key = (fgname, fac)
                if key not in s._static_fgs:
                    s._static_fgs[key] = Flightgroup(fgname, fac, typ)
                fg = s._static_fgs[key]
                u.flightgroup = fg
                u.fg_subnumber = counters[key]
                counters[key] += 1
                fg.nr_ships += 1
                fg.nr_ships_left += 1
                if fg.leader is None:
                    fg.leader = u
                u.hull = u.max_hull = 1e9
                u.max_speed = 0.0
                dests = a.get('destination', '')
                if dests:
                    u.destinations = dests.split()
            else:
                continue
            u.position = list(obj.position)
            s.add(u)
            if u.kind == 'sun':
                s.suns.append(u)
        del basefg
        return s

    def active_system(self):
        if self.active_stack:
            return self.active_stack[-1]
        if self.player is not None and self.player.system is not None:
            return self.player.system
        return self.get_system('Special/Empty')

    def active_system_name(self):
        s = self.active_system()
        return s.name if s else ''

    # ------------------------------------------------------------------
    # units
    # ------------------------------------------------------------------
    def create_unit(self, typename, faction, fg=None, subnumber=0, kind='ship'):
        row = self.data.units.lookup(typename, faction)
        if row is None:
            u = MockUnit(self, 'LOAD_FAILED', typename, faction, kind)
            self.warn('unit type "%s" (faction %s) not found in units.csv' % (typename, faction))
            u.hull = u.max_hull = 1.0
            u.radius = 1.0
            return u
        u = MockUnit(self, typename, row.get('Name') or typename, faction, kind)
        role = (row.get('Combat_Role') or '').upper()
        u.combat_role = role or 'INERT'
        u.unit_role = role or 'INERT'
        u.attack_preference = role or 'INERT'
        try:
            u.max_speed = float(row.get('Default_Speed_Governor') or 300)
        except ValueError:
            u.max_speed = 300.0
        try:
            u.max_ab_speed = float(row.get('Afterburner_Speed_Governor') or u.max_speed * 2)
        except ValueError:
            u.max_ab_speed = u.max_speed * 2
        try:
            u.cargo_volume = float(row.get('Hold_Volume') or 0)
        except ValueError:
            u.cargo_volume = 0.0
        try:
            u.upgrade_volume = float(row.get('Upgrade_Storage_Volume') or 0)
        except ValueError:
            u.upgrade_volume = 0.0
        u.jump_drive = (row.get('Jump_Drive_Present') or '').strip().lower() in ('1', 'true')
        for m in vsdata.parse_mounts(row.get('Mounts', '')):
            u.mounts.append({'weapon': m['weapon'], 'size': m['size'], 'ammo': m['ammo'],
                             'volume': m['volume'],
                             'status': 'ACTIVE' if m['weapon'] else 'UNCHOSEN'})
        for c in vsdata.parse_cargo_import(row.get('Cargo', '')):
            # {content;category;price;quantity;mass;volume;...}
            try:
                carg = Cargo()
                carg._content = c[0]
                carg._category = c[1] if len(c) > 1 else ''
                carg._price = float(c[2] or 0) if len(c) > 2 else 0.0
                carg._quantity = int(float(c[3] or 0)) if len(c) > 3 else 1
                carg._mass = float(c[4] or 0) if len(c) > 4 else 0.0
                carg._volume = float(c[5] or 0) if len(c) > 5 else 0.0
                u.cargo.append(carg)
            except (ValueError, IndexError):
                pass
        self.import_cargo(u, row.get('Cargo_Import', ''))
        u.cargo.sort(key=lambda c: c.sort_key())
        u.docking_ports = bool((row.get('Dock') or '').strip()) or role == 'BASE'
        if role == 'BASE':
            u.radius = 800.0
        elif role in ('CAPITAL', 'CARRIER', 'ESCORTCAP', 'TROOP'):
            u.radius = 250.0
        elif role in ('SUPPORT',):
            u.radius = 60.0
        else:
            u.radius = 20.0
        try:
            hull = float(row.get('Hull') or 1)
        except ValueError:
            hull = 1.0
        u.hull = u.max_hull = max(hull, 1.0)
        u.flightgroup = fg
        u.fg_subnumber = subnumber
        u.weapon_range = self.opts.weapon_range
        u.damage = self.opts.damage if any(m['weapon'] for m in u.mounts) else 0.0
        u.spawn_time = self.game_time_total()
        return u

    def import_cargo(self, u, spec):
        """Unit::ImportCargo -> ImportPartList for every {cat;price;pricedev;quant;quantdev}."""
        for fields in vsdata.parse_cargo_import(spec):
            if not fields or not fields[0]:
                continue

            def f(i):
                try:
                    return float(fields[i]) if len(fields) > i and fields[i].strip() else 0.0
                except ValueError:
                    return 0.0
            self.import_part_list(u, fields[0], f(1), f(2), f(3), f(4))

    def import_part_list(self, u, category, price, pricedev, quantity, quantdev):
        mpl = self.get_master_part_list()
        cats = [c for c in mpl.cargo if c._category == category]
        if not cats:
            return
        prices = [c._price for c in cats]
        minprice, maxprice = min(prices), max(prices)
        for proto in cats:
            c = proto.copy()
            baseprice = c._price
            q = int(quantity - quantdev)
            c._price *= price - pricedev
            q += int((quantdev * 2 + 1) * self.rng.random())
            c._price += pricedev * 2 * self.rng.random()
            c._price = abs(c._price)
            if q <= 0:
                q = 0
            elif maxprice > minprice + .01:
                renorm = (baseprice - minprice) / (maxprice - minprice) * (5 - 1) + 1
                if renorm > .001:
                    q = int(q / int(renorm)) if int(renorm) else q
                    if q < 1:
                        q = 1
            if c._price < .01:
                c._price = .01
            c._quantity = abs(q)
            u.cargo.append(c)

    def assign_combat_stats(self, u):
        """Simple hit point model: the player has player_hp, everyone else
        starts with npc_hp and is demoted to enemy_hp as soon as it shoots
        at the player or a mission-relevant ship (see ai._maybe_villain)."""
        hp = self.opts.player_hp if u is self.player else self.opts.npc_hp
        for pred, value in self.opts.hp_overrides:
            try:
                if pred(u):
                    hp = value
            except Exception:
                pass
        u.hull = u.max_hull = float(hp)

    def mission_flightgroup(self, name, faction, typ, ai, nships, nwaves):
        mis = self.current_mission
        key = (name, faction)
        store = mis.flightgroups if mis is not None else {}
        fg = store.get(key)
        new_fg = fg is None or (fg.nr_waves_left == 0 and fg.nr_ships_left == 0)
        if fg is None:
            fg = Flightgroup(name, faction, typ, ai)
            fg.directive = 'b'
            store[key] = fg
        if new_fg:
            fg.nr_waves_left = nwaves - 1
        fg.leader = None
        fg.ainame = ai
        fg.type = typ
        fg.nr_ships += nships
        fg.nr_ships_left += nships
        return fg

    def launch(self, name, typ, faction, unittype, ai, nships, nwaves, pos, logo, destinations=''):
        system = self.active_system()
        fg = self.mission_flightgroup(name, faction, typ, ai, nships, nwaves)
        units = []
        for i in range(nships):
            sub = i + fg.nr_ships - nships
            if unittype == 'planet':
                u = MockUnit(self, typ, typ, faction, 'planet')
            else:
                u = self.create_unit(typ, faction, fg, sub)
            u.flightgroup = fg
            u.fg_subnumber = sub
            if destinations:
                u.destinations = destinations.split()
            u.launched_by = self._mission_label()
            u.launched_mid = self.current_mission.id if self.current_mission else 0
            units.append(u)
        fg_radius = units[0].radius if units else 0
        for i, u in enumerate(units):
            u.position = [pos[0] + i * fg_radius * 3, pos[1] + i * fg_radius * 3, pos[2] + i * fg_radius * 3]
            self.assign_combat_stats(u)
            self.set_ai_script(u, ai)
            system.add(u)
            u.target = None
        if not units:
            return None
        if fg.leader is None or fg.leader is not self.player:
            fg.leader = units[0]
        for u in units:
            self.event('launch', unit=repr(u), system=system.name, ai=ai,
                       mission=repr(self.current_mission))
        return units[0]

    def set_ai_script(self, u, script):
        u.ai_script = script
        if script.endswith('.py'):
            self.last_python_ai = None
            self.run_file_compiled(script, 'python AI %s for %r' % (script, u))
            ai = self.last_python_ai
            self.last_python_ai = None
            if ai is not None:
                u.python_ai = ai
                ai._parent = u
                try:
                    ai.init(wrap(u))
                except BaseException:
                    self.record_error('python AI init %s' % script)

    def kill_unit(self, u, killer):
        if u.killed:
            return
        u.killed = True
        u.hull = min(u.hull, 0.0)
        if u.docked_to is not None:
            try:
                u.docked_to.docked_units.remove(u)
            except ValueError:
                pass
        self.event('killed', unit=repr(u), killer=repr(killer) if killer else None)
        if killer is not None and u.kind == 'ship' and killer.kind == 'ship':
            self.score_kill(killer, u)
        if u is self.player:
            self.event('player_died')

    def score_kill(self, killer, killed):
        kill_factor = -float(self.data.config.get('AI', 'kill_factor', '.2') or .2)
        friend_factor = -float(self.data.config.get('AI', 'friend_factor', '.1') or .1)
        if killed is self.player:
            self.adjust_relation_modifier(killer.faction, kill_factor)
        elif killer is self.player:
            self.adjust_relation_modifier(killed.faction, kill_factor)
            for fac in list(self.factions.names):
                if fac != killed.faction and fac != killer.faction:
                    rel = self.factions.relation(fac, killed.faction)
                    if rel:
                        self.adjust_relation_modifier(fac, friend_factor * rel)
            kills = self.floats.setdefault('kills', [])
            n = len(self.factions.names)
            while len(kills) <= n:
                kills.append(0.0)
            idx = self.factions.ensure(killed.faction)
            while len(kills) <= idx + 1:
                kills.append(0.0)
            kills[idx] += 1
            kills[-1] += 1

    def damage_unit(self, u, amount, dealer):
        if u.killed:
            return
        u.hull -= amount
        if dealer is not None:
            u.anger[dealer.serial] = u.anger.get(dealer.serial, 0.0) - 0.1
            u.last_attacker = dealer
            u.threat = dealer
        if u.hull <= 0:
            self.kill_unit(u, dealer)

    # ------------------------------------------------------------------
    # cargo
    # ------------------------------------------------------------------
    def cargo_volume_used(self, u, upgrades):
        return sum(c._quantity * c._volume for c in u.cargo if cargo_is_upgrade(c) == upgrades)

    def can_add_cargo(self, u, c):
        if c._quantity == 0 or c._volume == 0:
            return True
        up = cargo_is_upgrade(c)
        total = c._quantity * c._volume + self.cargo_volume_used(u, up)
        cap = u.upgrade_volume if up else u.cargo_volume
        return total <= cap

    def add_cargo(self, u, c, force):
        if u is self.player:
            self.event('player_cargo_add', content=c._content, quantity=c._quantity, force=force)
        if force:
            u.cargo.append(c)
            u.cargo.sort(key=lambda x: x.sort_key())
            return c._quantity
        # Replicates UnitUtil::addCargo exactly, including its off-by-one:
        # the quantity tested lags the loop counter by one.
        i = c._quantity
        while i > 0 and not self.can_add_cargo(u, c):
            c._quantity = i
            i -= 1
        if i > 0:
            c._quantity = i
            u.cargo.append(c)
            u.cargo.sort(key=lambda x: x.sort_key())
            return i
        return 0

    def remove_cargo(self, u, name, quantity, erasezero):
        if u is self.player:
            have = sum(c._quantity for c in u.cargo if c._content == name)
            self.event('player_cargo_remove', content=name, quantity=quantity, have=have)
        for i, c in enumerate(u.cargo):
            if c._content == name:
                if quantity > c._quantity:
                    quantity = c._quantity
                c._quantity -= quantity
                if c._quantity <= 0 and erasezero:
                    del u.cargo[i]
                return quantity
        return 0

    def get_master_part_list(self):
        if self.master_part_list is None:
            u = MockUnit(self, 'master_part_list', 'master_part_list', 'upgrades', 'ship')
            for p in self.data.parts.parts:
                c = Cargo()
                c._content = p['content']
                c._category = p['category']
                c._price = p['price']
                c._mass = p['mass']
                c._volume = p['volume']
                c._quantity = 1
                c._description = p['description']
                u.cargo.append(c)
            if not any(c._content == 'Pilot' for c in u.cargo):
                c = Cargo('Pilot', 'Contraband', 800.0, 1, .01, 1.0)
                u.cargo.append(c)
            u.cargo.sort(key=lambda x: x.sort_key())
            self.master_part_list = u
        return self.master_part_list

    def eject_cargo(self, u, index):
        if index < 0 or index >= len(u.cargo):
            return
        c = u.cargo[index]
        if c._quantity <= 0:
            return
        name = 'Mission_Cargo' if c._mission else c._content
        cu = MockUnit(self, name, c._content, 'upgrades', 'cargo')
        cu.radius = 5.0
        cu.hull = cu.max_hull = 10.0
        cu.position = [u.position[0] + 50, u.position[1], u.position[2]]
        (u.system or self.active_system()).add(cu)
        c._quantity -= 1
        if c._quantity <= 0:
            del u.cargo[index]
        self.event('eject_cargo', unit=repr(u), content=c._content)

    def tractor(self, u, target):
        """Pick up an ejected pilot or cargo pod."""
        if target.killed:
            return False
        if target.kind not in ('cargo', 'eject') and target.name not in ('eject', 'Pilot'):
            return False
        content = target.fullname if target.kind == 'cargo' else target.name
        if target.name in ('eject', 'Pilot'):
            content = 'Pilot'
        part = self.get_master_part_list()
        proto = None
        for c in part.cargo:
            if c._content == content:
                proto = c
                break
        c = proto.copy() if proto else Cargo(content, 'Contraband', 0.0, 1, .01, 1.0)
        c._quantity = 1
        self.add_cargo(u, c, force=True)
        target.killed = True
        self.event('tractor', unit=repr(target), content=content)
        return True

    # ------------------------------------------------------------------
    # upgrades
    # ------------------------------------------------------------------
    def upgrade_unit(self, u, file, mountoffset, subunitoffset, force, loop):
        row = self.data.units.lookup(file, 'upgrades')
        if row is None:
            self.warn('upgrade "%s" not found in units.csv' % file)
            return 0.0
        u.upgrades.append(file)
        if file == 'jump_drive':
            u.jump_drive = True
        weapons = [m['weapon'] for m in vsdata.parse_mounts(row.get('Mounts', '')) if m['weapon']]
        for w in weapons:
            placed = False
            n = len(u.mounts)
            for k in range(n if loop else 1):
                i = (mountoffset + k) % n if n else 0
                if n == 0:
                    break
                m = u.mounts[i]
                if force or m['status'] == 'UNCHOSEN' or not m['weapon']:
                    m['weapon'] = w
                    m['status'] = 'ACTIVE'
                    placed = True
                    break
            if not placed:
                return 0.0
            if u.damage == 0.0:
                u.damage = self.opts.damage
        return 1.0

    # ------------------------------------------------------------------
    # credits
    # ------------------------------------------------------------------
    def get_credits(self, u):
        return self.credits if u is self.player else 0.0

    def add_credits(self, u, amount):
        if u is self.player:
            self.credits += amount
            self.event('credits', delta=amount, total=self.credits)

    # ------------------------------------------------------------------
    # flightgroups
    # ------------------------------------------------------------------
    def switch_fg(self, u, name):
        fg = self.mission_flightgroup(name, u.faction, u.name, 'default', 1, 1)
        u.flightgroup = fg
        u.fg_subnumber = fg.nr_ships - 1
        if fg.leader is None:
            fg.leader = u

    # ------------------------------------------------------------------
    # missions
    # ------------------------------------------------------------------
    def game_time_total(self):
        return self.missions[0].gametime if self.missions else 0.0

    def register_python_mission(self, obj):
        self.last_python_mission = obj

    def player_missions(self):
        return [m for m in self.missions if m.player_num == 0]

    def load_mission(self, filename, script, title=None):
        """::LoadMission(mission_name, script)."""
        friendly = title if title is not None else filename
        mission_file = filename or 'nothing.mission'
        mpath = self.data.path('mission', mission_file)
        mission_code = None
        if not os.path.isfile(mpath):
            self.warn('mission file %s not found' % mission_file)
            return None
        if not script:
            mission_code = self.parse_mission_python(mpath)
        if self.missions:
            self.push_save_string(0, 'active_scripts', script)
            self.push_save_string(0, 'active_missions', friendly)
        m = Mission(self, friendly, script or mission_code or '')
        self.missions.append(m)
        saved = self.current_mission
        self.current_mission = m
        code = script if script else (mission_code or '')
        self.last_python_mission = None
        if code:
            code = code.replace('\r', '\n')
            self.run_string(code, 'mission script for %s' % friendly)
        m.pyobj = self.last_python_mission
        self.last_python_mission = None
        for ev in self.events[-200:]:
            if ev.get('by') == '<loading>' and ev.get('mid') == m.id:
                ev['by'] = m.type_name()
        for sysobj in self.systems.values():
            for u in sysobj.units:
                if u.launched_by == '<loading>' and getattr(u, 'launched_mid', 0) == m.id:
                    u.launched_by = m.type_name()
                    for pred, value in self.opts.hp_overrides:
                        try:
                            if pred(u):
                                u.hull = u.max_hull = float(value)
                        except Exception:
                            pass
        self.event('mission_loaded', mission=repr(m), name=friendly)
        self.current_mission = self.missions[0] if self.missions else saved
        return m

    def parse_mission_python(self, path):
        with open(path, encoding='latin-1') as fh:
            text = fh.read()
        m = re.search(r'<python>(.*?)</python>', text, re.S)
        if not m:
            return ''
        return m.group(1)

    def _mission_label(self):
        m = self.current_mission
        if m is None:
            return ''
        return m.type_name() if m.pyobj is not None else '<loading>'

    def mission_number(self, mis):
        n = 0
        for m in self.missions:
            if m.player_num == mis.player_num:
                if m is mis:
                    return n
                n += 1
        return -1

    def terminate_mission(self, mis, won=None):
        if mis.terminated:
            return
        queuenum = -1
        if mis in self.missions:
            queuenum = self.mission_number(mis)
            self.missions.remove(mis)
        if queuenum >= 1:
            num = queuenum - 1
            sc = self.strings.setdefault('active_scripts', [])
            if num < len(sc):
                del sc[num]
            ms = self.strings.setdefault('active_missions', [])
            if num < len(ms):
                del ms[num]
        mis.terminated = True
        mis.won = won
        self.event('mission_terminated', mission=repr(mis), won=won)
        obj = mis.pyobj
        mis.pyobj = None
        del obj   # drop the reference; missionhook.__del__ fires here

    def execute_director(self, dt=None):
        if dt is None:
            dt = self.opts.dt
        i = 0
        while i < len(self.missions):
            m = self.missions[i]
            self.current_mission = m
            pushed = False
            if self.player is not None and self.player.system is not None:
                self.active_stack.append(self.player.system)
                pushed = True
            old = m.gametime
            m.gametime += dt
            if m.gametime <= old:
                m.gametime = dt
            if m.pyobj is not None:
                try:
                    m.pyobj.Execute()
                except BaseException:
                    self.record_error('%s.Execute' % m.type_name())
            if pushed and self.active_stack:
                self.active_stack.pop()
            if i < len(self.missions) and self.missions[i] is m:
                i += 1
        self.current_mission = self.missions[0] if self.missions else None
        self.process_delayed_missions()
        i = 1
        while i < len(self.missions):
            m = self.missions[i]
            if m.pyobj is not None:
                i += 1
            else:
                self.terminate_mission(m)

    def process_delayed_missions(self):
        while self.delayed_missions:
            fname, script = self.delayed_missions.pop()
            self.load_mission(fname, script)

    # ------------------------------------------------------------------
    # IO messages
    # ------------------------------------------------------------------
    def io_message(self, delay, frm, to, msg):
        if to == 'news':
            self.push_save_string(0, 'news', '#' + msg)
            return
        self.io_messages.append((self.game_time_total() + delay, frm, to, msg))
        if self.opts.verbose >= 3:
            self.log('[msg] %s -> %s: %s' % (frm, to, msg))

    # ------------------------------------------------------------------
    # docking / bases
    # ------------------------------------------------------------------
    def dock_distance_ok(self, u, base):
        if base.is_planet():
            dis = u.significant_distance_to(base)
        else:
            dis = u.distance_to(base)
        return dis < max(base.radius, 300.0)

    def dock(self, u, base, check_distance=True, load_interface=True):
        if not base.is_dockable():
            return False
        if check_distance and not self.dock_distance_ok(u, base):
            return False
        if u.docked_to is not None:
            self.undock(u, u.docked_to)
        u.docked_to = base
        base.docked_units.append(u)
        u.velocity = [0.0, 0.0, 0.0]
        if u is not self.player:
            # docked ships sit in the hangar: inside the base's radius
            u.position = list(base.position)
        u.moveto = None
        u.dock_order = None
        self.event('docked', unit=repr(u), base=repr(base))
        if u is self.player and self.opts.repair_on_dock:
            u.hull = u.max_hull
        if u is self.player and load_interface:
            self.load_base_interface_for(base)
        return True

    def undock(self, u, base):
        if u.docked_to is not base:
            return False
        u.docked_to = None
        try:
            base.docked_units.remove(u)
        except ValueError:
            pass
        # launch a little outside the base
        u.position = [base.position[0] + base.radius + 300.0, base.position[1], base.position[2]]
        self.event('undocked', unit=repr(u), base=repr(base))
        return True

    def time_of_day(self, base, un):
        s = base.system
        if not s or not s.suns:
            return 'day'
        sun = s.suns[0]
        a = [sun.position[i] - base.position[i] for i in range(3)]
        b = [un.position[i] - base.position[i] for i in range(3)]
        na = math.sqrt(sum(x * x for x in a)) or 1.0
        nb = math.sqrt(sum(x * x for x in b)) or 1.0
        rez = sum(a[i] * b[i] for i in range(3)) / (na * nb)
        if rez > .2:
            return 'day'
        if rez < -.1:
            return 'night'
        return 'sunset'

    def base_file_for(self, base, tod=None):
        if base.is_planet():
            basename = cargo_unit_name(base.fullname)
        else:
            basename = base.name
        fac = base.faction
        if fac == 'neutral':
            fac = self.get_galaxy_faction(base.system_file())
        if tod is None:
            tod = self.time_of_day(base, self.player) if self.player else 'day'

        def with_without(fn):
            for cand in (fn + '_' + tod + '.py', fn + '.py'):
                if os.path.isfile(self.data.path(cand)):
                    return cand
            return None

        def full(fn):
            return with_without(fn + '_' + fac) or with_without(fn)

        f = full('bases/' + basename)
        if f is None:
            f = full('bases/planet' if base.is_planet() else 'bases/unit')
        return f, basename, tod, fac

    def load_base_interface_for(self, base):
        f, basename, tod, fac = self.base_file_for(base)
        self.basemod.load_base_interface(self, f, base, self.player, basename + tod + fac)

    # ------------------------------------------------------------------
    # jumping
    # ------------------------------------------------------------------
    def request_jump_to(self, u, sysname):
        if u.system is None:
            return False
        if sysname not in self.galaxy.systems and self.data.system_layout(sysname) is None:
            return False
        self.pending_jumps.append((u, sysname, None))
        return True

    def transfer_unit(self, u, sysname, via=None):
        old = u.system
        dest = self.get_system(sysname)
        if old is not None:
            old.remove(u)
        u.target = None
        dest.add(u)
        # arrive at the jump point leading back, like the engine
        arrival = None
        if old is not None:
            for x in dest.units:
                if x.is_jumppoint() and old.name in x.destinations:
                    arrival = x
                    break
        if arrival is not None:
            u.position = [arrival.position[0] + arrival.radius + 400.0, arrival.position[1],
                          arrival.position[2]]
        else:
            u.position = [0.0, 0.0, 0.0]
        u.jump_active = False
        if u is self.player:
            self.visit_system(sysname)
            self.event('player_jumped', frm=old.name if old else None, to=sysname)
        return dest

    def visit_system(self, sysname):
        def help_(name, num):
            key = 'visited_' + name
            v = self.floats.setdefault(key, [])
            if not v:
                v.append(num)
            elif v[0] != 1.0 and num == 1:
                v[0] = num
        help_(sysname, 1.0)
        for adj in self.adjacent_systems(sysname):
            help_(adj, 0.0)

    def process_pending_jumps(self):
        jumps = self.pending_jumps
        self.pending_jumps = []
        for u, sysname, via in jumps:
            if u.killed:
                continue
            followers = []
            if u is self.player:
                for x in list(u.system.units):
                    if x is u or x.killed or x.flightgroup is None:
                        continue
                    if (x.flightgroup.leader is u and x.flightgroup.directive[:1] in ('F', 'f')
                            and x.distance_to(u) < 10000.0 and x.docked_to is None):
                        followers.append(x)
            self.transfer_unit(u, sysname, via)
            for x in followers:
                self.transfer_unit(x, sysname, via)
                x.position = [u.position[0] + 100.0, u.position[1] + 100.0, u.position[2]]

    # ------------------------------------------------------------------
    # frame loop
    # ------------------------------------------------------------------
    def step(self, dt=None):
        """Advance the simulation one frame (only while flying)."""
        if dt is None:
            dt = self.opts.dt
        self.frame += 1
        self.execute_director(dt)
        self.ai.update(self, dt)
        self.combat_accum += dt
        while self.combat_accum >= self.opts.turn:
            self.combat_accum -= self.opts.turn
            self.ai.combat_turn(self)
        self.process_pending_jumps()
        self.purge_dead()
        self.run_python_ais()
        self.process_pending_load()

    def run_python_ais(self):
        if self.player is None or self.player.system is None:
            return
        for u in list(self.player.system.units):
            if u.python_ai is not None and not u.killed:
                try:
                    u.python_ai.Execute()
                except BaseException:
                    self.record_error('python AI %s.Execute' % type(u.python_ai).__name__)
                    u.python_ai = None

    def purge_dead(self):
        for s in self.systems.values():
            if any(u.killed for u in s.units):
                s.units = [u for u in s.units if not u.killed]

    # ------------------------------------------------------------------
    # game start / load
    # ------------------------------------------------------------------
    def new_player(self, shiptype, sysname, position, faction='privateer'):
        u = self.create_unit(shiptype, faction)
        u.kind = 'ship'
        self.player = u
        if self.player_fg is None:
            self.player_fg = Flightgroup('Privateer', faction, shiptype)
        u.flightgroup = self.player_fg
        self.player_fg.leader = u
        self.assign_combat_stats(u)
        u.damage = self.opts.player_damage
        u.position = list(position)
        s = self.get_system(sysname)
        s.add(u)
        return u

    def start_mission_file(self, missionfile):
        """Initial mission load, like main.cpp does for default_mission."""
        mpath = self.data.path('mission', missionfile)
        text = open(mpath, encoding='latin-1').read()
        sysm = re.search(r'<var\s+name="system"\s+value="([^"]*)"', text)
        sysname = sysm.group(1) if sysm else 'Gemini/Troy'
        credm = re.search(r'<var\s+name="credits"\s+value="([^"]*)"', text)
        self.credits = float(credm.group(1)) if credm else 0.0
        fgm = re.search(r'<flightgroup[^>]*name="([^"]*)"[^>]*faction="([^"]*)"[^>]*type="([^"]*)"', text)
        shiptype = fgm.group(3) if fgm else 'tarsus.begin'
        faction = fgm.group(2) if fgm else 'privateer'
        if fgm:
            self.player_fg = Flightgroup(fgm.group(1), faction, shiptype)
        self.new_player(shiptype, sysname, (0.0, 0.0, 0.0), faction)
        self.visit_system(sysname)
        code = self.parse_mission_python(mpath)
        m = Mission(self, missionfile, code)
        self.missions.append(m)
        self.current_mission = m
        self.last_python_mission = None
        self.run_string(code.replace('\r', '\n'), 'initial mission %s' % missionfile)
        m.pyobj = self.last_python_mission
        self.last_python_mission = None
        self.event('mission_loaded', mission=repr(m), name=missionfile)
        return m

    def request_load_game(self, savename):
        self.pending_load = savename

    def process_pending_load(self):
        if self.pending_load is None:
            return
        name = self.pending_load
        self.pending_load = None
        self.load_game(name)

    def save_path(self, name):
        if name == self.opts.new_game or not self.opts.save_dir:
            p = self.data.path(name)
            if name == self.opts.new_game and os.path.exists(p):
                return p
        if self.opts.save_dir:
            return os.path.join(self.opts.save_dir, name)
        return self.data.path(name)

    def load_game(self, savename):
        """UniverseUtil::loadGame + respawn: rebuild the player from a save."""
        self.save_name = savename
        path = self.save_path(savename)
        sg = vsdata.SaveGame.load(path)
        if self.base is not None:
            self.basemod.terminate_base(self, launch=False)
        if self.player is not None:
            self.kill_unit(self.player, None)
        self.floats = {k: [f32(x) for x in v] for k, v in sg.floats.items()}
        self.strings = {k: list(v) for k, v in sg.strings.items()}
        self.credits = sg.credits
        self.player_fg = Flightgroup('Privateer', 'privateer', sg.ships[0] if sg.ships else 'tarsus.begin')
        self.new_player(sg.ships[0] if sg.ships else 'tarsus.begin', sg.system, sg.position)
        self.visit_system(sg.system)
        self.event('game_loaded', save=savename, system=sg.system)
        self.load_saved_missions()
        self.dock_to_saved_base()

    def load_saved_missions(self):
        scripts = list(self.strings.get('active_scripts', []))
        names = list(self.strings.get('active_missions', []))
        self.main_globals.setdefault('VS', sys.modules.get('VS'))
        vs = sys.modules.get('VS')
        if vs is not None:
            vs.loading_active_missions = True
        for m in list(self.missions[1:]):
            self.terminate_mission(m)
        self.strings['active_scripts'] = []
        self.strings['active_missions'] = []
        for i in range(min(len(scripts), len(names))):
            if names[i]:
                if names[i].startswith('#'):
                    self.load_mission('', scripts[i], title=names[i][1:])
                else:
                    self.load_mission(names[i], scripts[i])
        if vs is not None:
            vs.loading_active_missions = False

    def dock_to_saved_base(self):
        plr = self.player
        name = self.data.config.get('AI', 'startDockedTo', 'MiningBase')
        strs = self.load_string_list('unit_to_dock_with')
        if strs:
            name = strs[0]
        best = None
        bestd = 0
        for un in plr.system.units:
            if un is plr:
                continue
            if un.name == name or un.fullname == name:
                d = plr.significant_distance_to(un)
                if best is None or d < bestd:
                    best, bestd = un, d
        if best is None:
            self.warn('could not find saved base %r to dock with' % name)
            return None
        if best.is_planet():
            plr.position = list(plr.position)
        else:
            plr.position = list(best.position)
        self.dock(plr, best, check_distance=False)
        return best

    def save_game(self, name):
        if not self.opts.save_dir:
            return
        sg = vsdata.SaveGame()
        sg.floats = self.floats
        sg.strings = self.strings
        path = os.path.join(self.opts.save_dir, name)
        sg.dump(path, self.player.system_file() if self.player else '', self.credits,
                [self.player.name] if self.player else [], self.player.position if self.player else (0, 0, 0))
        self.event('game_saved', save=name)

    # ------------------------------------------------------------------
    # misc
    # ------------------------------------------------------------------
    def planet_radius_percent(self):
        try:
            return float(self.data.config.get('physics', 'auto_pilot_planet_radius_percent', '.75'))
        except ValueError:
            return .75

    def safe_entrance_point(self, pos, radial):
        return tuple(pos)
