"""Loaders for the static data in a Vega Strike data directory.

Only what the mock engine needs is parsed: configuration variables, the
galaxy (systems, jumps, factions), star system layouts, faction relations,
the unit table (units.csv), the master part list and save games.
"""

import csv
import os
import re
import xml.etree.ElementTree as ET


# --------------------------------------------------------------------------
# vegastrike.config
# --------------------------------------------------------------------------

_TAG_RE = re.compile(r'<!--.*?-->|<(/?)([A-Za-z_][\w.-]*)((?:\s+[\w.:-]+\s*=\s*"[^"]*")*)\s*(/?)>',
                     re.S)
_ATTR_RE = re.compile(r'([\w.:-]+)\s*=\s*"([^"]*)"')


class VSConfig:
    """Lenient reader for vegastrike.config.

    The engine uses its own forgiving XML parser, and real config files are
    often not well formed (for example ``<-- #end -->``), so this scans tags
    with a regex instead of using a strict XML parser.  Values live in the
    ``<variables>`` element as nested ``<section name=..>`` blocks.
    """

    def __init__(self, path):
        self.vars = {}
        with open(path, encoding='latin-1') as fh:
            text = fh.read()
        stack = []
        in_variables = False
        for m in _TAG_RE.finditer(text):
            if m.group(0).startswith('<!--'):
                continue
            closing, tag, attrs, selfclose = m.group(1), m.group(2), m.group(3), m.group(4)
            attrd = dict(_ATTR_RE.findall(attrs or ''))
            if tag == 'variables':
                in_variables = not closing
                continue
            if not in_variables:
                continue
            if tag == 'section':
                if closing:
                    if stack:
                        stack.pop()
                elif not selfclose:
                    stack.append(attrd.get('name', ''))
            elif tag == 'var' and not closing:
                key = tuple(stack) + (attrd.get('name', ''),)
                self.vars[key] = attrd.get('value', '')

    def get(self, section, name, default):
        return self.vars.get((section, name), default)

    def get_sub(self, section, subsection, name, default):
        return self.vars.get((section, subsection, name), default)


# --------------------------------------------------------------------------
# Galaxy
# --------------------------------------------------------------------------

class Galaxy:
    """universe/<galaxy>.xml: per system variables such as jumps/faction."""

    def __init__(self, path):
        tree = ET.parse(path)
        root = tree.getroot()
        self.root_vars = {}
        self.sector_vars = {}
        self.systems = {}          # 'Gemini/Troy' -> dict of vars
        self.system_order = []
        systems_el = root.find('systems')
        if systems_el is None:
            return
        for v in systems_el.findall('var'):
            self.root_vars[v.get('name')] = v.get('value', '')
        for sector in systems_el.findall('sector'):
            sname = sector.get('name') or ''
            svars = {}
            for v in sector.findall('var'):
                svars[v.get('name')] = v.get('value', '')
            self.sector_vars[sname] = svars
            for system in sector.findall('system'):
                full = sname + '/' + (system.get('name') or '')
                d = {}
                for v in system.findall('var'):
                    d[v.get('name')] = v.get('value', '')
                self.systems[full] = d
                self.system_order.append(full)
        self.original_factions = {k: v.get('faction', '') for k, v in self.systems.items()}

    def lookup(self, sysname, prop):
        """Returns the property or None if missing."""
        d = self.systems.get(sysname)
        if d is not None and prop in d:
            return d[prop]
        sector = sysname.split('/')[0]
        sv = self.sector_vars.get(sector)
        if sv is not None and prop in sv:
            return sv[prop]
        if prop in self.root_vars:
            return self.root_vars[prop]
        return None

    def jumps(self, sysname):
        val = self.lookup(sysname, 'jumps')
        if not val:
            return []
        return val.split()


# --------------------------------------------------------------------------
# Star systems
# --------------------------------------------------------------------------

class SystemObject:
    """One <Planet>/<Unit>/<Nebula>/<Asteroid> element of a .system file."""

    def __init__(self, kind, attrs, parent=None):
        self.kind = kind            # planet | unit | nebula | asteroid | enhancement
        self.attrs = attrs
        self.parent = parent
        self.children = []
        self.position = (0.0, 0.0, 0.0)

    def __repr__(self):
        return '<%s %s>' % (self.kind, self.attrs.get('name'))


def _float(attrs, key, default=0.0):
    try:
        return float(attrs.get(key, default))
    except (TypeError, ValueError):
        return default


class SystemLayout:
    def __init__(self, sysname, path):
        self.name = sysname
        self.path = path
        self.objects = []           # flattened list in document order
        tree = ET.parse(path)
        root = tree.getroot()
        self.attrs = dict(root.attrib)
        self._walk(root, None, (0.0, 0.0, 0.0))

    def _walk(self, el, parent, origin):
        for child in el:
            tag = child.tag.lower()
            if tag not in ('planet', 'unit', 'nebula', 'asteroid', 'enhancement'):
                # Lights, atmospheres, fog etc. are irrelevant to the mock.
                continue
            attrs = {k.lower(): v for k, v in child.attrib.items()}
            obj = SystemObject(tag, attrs, parent)
            # Positions are given either with x/y/z or with an orbit
            # (ri,rj,rk / si,sj,sk).  Use the orbit's R vector at t=0.
            if any(k in attrs for k in ('x', 'y', 'z')):
                pos = (_float(attrs, 'x'), _float(attrs, 'y'), _float(attrs, 'z'))
            else:
                pos = (_float(attrs, 'ri'), _float(attrs, 'rj'), _float(attrs, 'rk'))
            obj.position = (origin[0] + pos[0], origin[1] + pos[1], origin[2] + pos[2])
            self.objects.append(obj)
            if parent is not None:
                parent.children.append(obj)
            self._walk(child, obj, obj.position)


# --------------------------------------------------------------------------
# Factions
# --------------------------------------------------------------------------

class Factions:
    def __init__(self, path):
        tree = ET.parse(path)
        root = tree.getroot()
        self.names = []
        self.relations = {}
        self.contraband = {}
        for fac in root.findall('Faction'):
            name = fac.get('name')
            self.names.append(name)
            self.contraband[name] = fac.get('contraband', '')
            for st in fac.findall('stats'):
                try:
                    rel = float(st.get('relation', '0'))
                except ValueError:
                    rel = 0.0
                self.relations[(name, st.get('name'))] = rel
        # Mirror the engine: a few factions always exist.
        for extra in ('neutral', 'upgrades', 'planets'):
            if extra not in self.names:
                self.names.append(extra)
        self.index = {n: i for i, n in enumerate(self.names)}

    def get_index(self, name):
        return self.index.get(name, -1)

    def ensure(self, name):
        """The engine creates unknown factions on demand; do the same."""
        if name not in self.index:
            self.index[name] = len(self.names)
            self.names.append(name)
        return self.index[name]

    def relation(self, a, b):
        if a == b:
            return 1.0 if a not in ('neutral', 'upgrades', 'planets') else 0.0
        return self.relations.get((a, b), 0.0)


# --------------------------------------------------------------------------
# units.csv
# --------------------------------------------------------------------------

_MOUNT_RE = re.compile(r'\{([^{}]*)\}')


def parse_mounts(text):
    """Parse the Mounts column: {weapon;ammo;volume;size;...}."""
    mounts = []
    for body in _MOUNT_RE.findall(text or ''):
        fields = body.split(';')
        weapon = fields[0].strip() if fields else ''
        size = fields[3].strip() if len(fields) > 3 else ''
        try:
            ammo = int(float(fields[1])) if len(fields) > 1 and fields[1].strip() else -1
        except ValueError:
            ammo = -1
        try:
            volume = float(fields[2]) if len(fields) > 2 and fields[2].strip() else -1.0
        except ValueError:
            volume = -1.0
        mounts.append({'weapon': weapon, 'size': size, 'ammo': ammo, 'volume': volume})
    return mounts


def parse_cargo_import(text):
    """Parse Cargo_Import: {name;price;pricedev;quantity;quantdev}."""
    out = []
    for body in _MOUNT_RE.findall(text or ''):
        fields = body.split(';')
        out.append(fields)
    return out


class UnitDB:
    def __init__(self, path):
        self.rows = {}
        self.columns = []
        with open(path, encoding='latin-1', newline='') as fh:
            reader = csv.reader(fh)
            header = next(reader)
            self.columns = header
            next(reader, None)       # second row documents the types
            for row in reader:
                if not row or not row[0]:
                    continue
                d = {}
                for i, col in enumerate(header):
                    d[col] = row[i] if i < len(row) else ''
                self.rows[row[0]] = d

    def lookup(self, name, faction=''):
        """Mirror the engine's key lookup: name__faction, then name."""
        if faction:
            row = self.rows.get(name + '__' + faction)
            if row is not None:
                return row
        return self.rows.get(name)

    def stat(self, name, faction, stat):
        row = self.lookup(name, faction)
        if row is None:
            return ''
        return row.get(stat, '')


class WeaponList:
    """weapon_list.xml: name -> dict of stats used by GetMountInfo."""

    def __init__(self, path):
        self.weapons = {}
        if not os.path.exists(path):
            return
        root = ET.parse(path).getroot()
        for el in root:
            name = el.get('name')
            if not name:
                continue
            en = el.find('Energy')
            dm = el.find('Damage')
            di = el.find('Distance')

            def g(e, k, d=0.0):
                try:
                    return float(e.get(k, d)) if e is not None else d
                except ValueError:
                    return d
            self.weapons[name] = {
                'type': el.tag.upper(), 'speed': g(di, 'speed'), 'range': g(di, 'range'),
                'damage': g(dm, 'rate'), 'phaseDamage': g(dm, 'phasedamage'),
                'stability': g(en, 'stability'), 'longRange': g(dm, 'longrange'),
                'lockTime': g(di, 'locktime'), 'energyRate': g(en, 'rate'),
                'refire': g(en, 'refire'), 'volume': g(di, 'volume'), 'name': name,
                'mountsize': el.get('mountsize', '')}


# --------------------------------------------------------------------------
# master_part_list.csv
# --------------------------------------------------------------------------

class PartList:
    def __init__(self, path):
        self.parts = []            # list of dicts in file order
        self.by_name = {}
        with open(path, encoding='latin-1', newline='') as fh:
            reader = csv.DictReader(fh)
            for row in reader:
                name = row.get('file', '')
                if not name:
                    continue
                try:
                    price = float(row.get('price') or 0)
                except ValueError:
                    price = 0.0
                try:
                    mass = float(row.get('mass') or 0)
                except ValueError:
                    mass = 0.0
                try:
                    volume = float(row.get('volume') or 0)
                except ValueError:
                    volume = 0.0
                part = {'content': name, 'category': row.get('categoryname', ''),
                        'price': price, 'mass': mass, 'volume': volume,
                        'description': row.get('description', '')}
                self.parts.append(part)
                self.by_name.setdefault(name, part)


# --------------------------------------------------------------------------
# Save games
# --------------------------------------------------------------------------

class SaveGame:
    """Enough of the engine's save format for the mock engine.

    Layout (see src/savegame.cpp):
      line 1: ``system^credits^ship1|ship2...^... x y z``
      then packets "0 mission data N", "0 missionstring data N",
      "0 python data", "0 news data", "0 stardate data", "0 factions begin".
    """

    def __init__(self):
        self.system = 'Gemini/Troy'
        self.credits = 0.0
        self.ships = ['tarsus.begin']
        self.position = (0.0, 0.0, 0.0)
        self.floats = {}
        self.strings = {}
        self.raw_tail = ''

    @staticmethod
    def load(path):
        sg = SaveGame()
        with open(path, 'rb') as fh:
            data = fh.read().decode('latin-1')
        first, _, rest = data.partition('\n')
        parts = first.split('^')
        sg.system = parts[0]
        if len(parts) > 1:
            try:
                sg.credits = float(parts[1])
            except ValueError:
                sg.credits = 0.0
        if len(parts) > 2:
            shipfield = parts[2]
            tokens = shipfield.split(' ')
            sg.ships = [s for s in tokens[0].split('|') if s]
            nums = [t for t in tokens[1:] if t]
            try:
                sg.position = tuple(float(x) for x in nums[:3])
            except ValueError:
                pass
        pos = 0
        while pos < len(rest):
            m = re.compile(r'\s*(-?\d+)\s+(\S+)\s+(\S+)').match(rest, pos)
            if not m:
                break
            tag = (m.group(2), m.group(3))
            pos = m.end()
            if tag == ('mission', 'data'):
                pos = sg._read_floats(rest, pos)
            elif tag == ('missionstring', 'data'):
                pos = sg._read_strings(rest, pos)
            else:
                sg.raw_tail = rest[m.start():]
                break
        return sg

    def _read_floats(self, text, pos):
        tok = re.compile(r'\s*(\S+)')

        def nxt():
            nonlocal pos
            m = tok.match(text, pos)
            pos = m.end()
            return m.group(1)
        count = int(nxt())
        for _ in range(count):
            key = nxt()
            n = int(nxt())
            vals = [float(nxt()) for _ in range(n)]
            self.floats[key] = vals
        return pos

    def _read_strings(self, text, pos):
        def any_string():
            nonlocal pos
            # AnyStringScanInString: digits (skipping others) up to a space
            size = 0
            found = False
            while pos < len(text) and (text[pos] != ' ' or not found):
                if text[pos].isdigit():
                    size = size * 10 + int(text[pos])
                    found = True
                pos += 1
            if pos < len(text):
                pos += 1
            s = text[pos:pos + size]
            pos += size
            return s

        m = re.compile(r'\s*(\d+)').match(text, pos)
        count = int(m.group(1))
        pos = m.end()
        for _ in range(count):
            key = any_string()
            m = re.compile(r'\s*(\d+)').match(text, pos)
            n = int(m.group(1))
            pos = m.end()
            # the engine skips exactly one space after the count
            vals = [any_string() for _ in range(n)]
            self.strings[key] = vals
        return pos

    def dump(self, path, system, credits, ships, position):
        def fmt_float(v):
            return '%f' % v
        out = ['%s^%s^%s %s' % (system, fmt_float(credits), '|'.join(ships),
                                ' '.join(fmt_float(p) for p in position))]
        floats = {k: v for k, v in self.floats.items() if v}
        out.append('0 mission data  %d' % len(floats))
        for k in floats:
            out.append('%s %d %s ' % (k, len(floats[k]), ' '.join(fmt_float(x) for x in floats[k])))
        strings = {k: v for k, v in self.strings.items() if v}
        body = ['%d' % len(strings)]
        for k, vals in strings.items():
            body.append('\n%d %s%d ' % (len(k), k, len(vals)))
            for s in vals:
                body.append('%d %s' % (len(s), s))
        out.append('0 missionstring data ' + ''.join(body))
        with open(path, 'w', encoding='latin-1') as fh:
            fh.write('\n'.join(out) + '\n')


# --------------------------------------------------------------------------
# Convenience bundle
# --------------------------------------------------------------------------

class DataDir:
    def __init__(self, root):
        self.root = os.path.abspath(root)
        self.config = VSConfig(self.path('vegastrike.config'))
        galaxy_name = self.config.get('general', 'galaxy', 'milky_way.xml')
        univ = self.config.get('data', 'universe_path', 'universe')
        self.galaxy = Galaxy(self.path(univ, galaxy_name))
        self.factions = Factions(self.path('factions.xml'))
        self.units = UnitDB(self.path('units', 'units.csv'))
        self.parts = PartList(self.path('master_part_list.csv'))
        self.weapons = WeaponList(self.path('weapon_list.xml'))
        self._layouts = {}

    def path(self, *parts):
        return os.path.join(self.root, *parts)

    def exists(self, *parts):
        return os.path.exists(self.path(*parts))

    def system_layout(self, sysname):
        if sysname not in self._layouts:
            p = self.path('sectors', sysname + '.system')
            if os.path.exists(p):
                self._layouts[sysname] = SystemLayout(sysname, p)
            else:
                self._layouts[sysname] = None
        return self._layouts[sysname]

    def all_system_names(self):
        names = []
        secdir = self.path('sectors')
        for sector in sorted(os.listdir(secdir)):
            sp = os.path.join(secdir, sector)
            if not os.path.isdir(sp):
                continue
            for f in sorted(os.listdir(sp)):
                if f.endswith('.system'):
                    names.append(sector + '/' + f[:-len('.system')])
        return names
