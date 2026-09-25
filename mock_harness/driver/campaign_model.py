"""Read-only model of the campaign trees built by ``campaign_lib``.

The automated player never hard-codes the storyline.  Instead it looks at
each campaign's current node (reconstructed from the save data exactly
like ``Campaign.readPositionFromSavegame``) and simulates - without side
effects - what ``Campaign.getCurrentNode`` would do if the player docked
at a given base.  Bases where that simulation changes the campaign state
or reveals a fixer worth clicking are the next destinations.
"""

import collections


class Loc:
    """A place the player can dock: (system, unit name, unit full name)."""

    __slots__ = ('system', 'name', 'fullname', 'kind')

    def __init__(self, system, name, fullname, kind):
        self.system = system
        self.name = name
        self.fullname = fullname
        self.kind = kind

    def key(self):
        return (self.system, self.name, self.fullname)

    def __eq__(self, other):
        return isinstance(other, Loc) and self.key() == other.key()

    def __hash__(self):
        return hash(self.key())

    def __repr__(self):
        return '%s:%s(%s)' % (self.system, self.fullname or self.name, self.name)


def static_dockables(eng):
    """All dockable things in every system, from the .system files.

    Uses the loaded star system when available (so dynamic ownership and
    renamed units are respected)."""
    out = collections.defaultdict(list)
    from mockvs.engine import cargo_unit_name
    for sysname in eng.data.all_system_names():
        s = eng.systems.get(sysname)
        if s is not None:
            for u in s.units:
                if not u.killed and u.is_dockable() and u is not eng.player:
                    out[sysname].append(Loc(sysname, u.name, u.fullname, 'loaded'))
            continue
        lay = eng.data.system_layout(sysname)
        if lay is None:
            continue
        for obj in lay.objects:
            a = obj.attrs
            if obj.kind == 'planet':
                if a.get('destination') or 'light' in a:
                    continue
                fn = cargo_unit_name(a.get('file', ''))
                if fn == 'invisible':
                    continue
                out[sysname].append(Loc(sysname, a.get('name', ''), fn, 'planet'))
            elif obj.kind == 'unit':
                typ = a.get('file', '')
                row = eng.data.units.lookup(typ, a.get('faction', ''))
                if row is None:
                    continue
                role = (row.get('Combat_Role') or '').upper()
                if (row.get('Dock') or '').strip() or role == 'BASE':
                    out[sysname].append(Loc(sysname, typ, a.get('name', ''), 'unit'))
    return out


def _norm(s):
    return (s or '').replace(' ', '_').lower()


class CampaignModel:
    def __init__(self, eng, log=None):
        self.eng = eng
        self.log = log or (lambda *a: None)
        self._dockables = None
        import campaign_lib
        self.cl = campaign_lib

    # ------------------------------------------------------------------
    def campaigns(self):
        return self.cl.getCampaignList()

    def dockables(self, refresh=False):
        if self._dockables is None or refresh:
            self._dockables = static_dockables(self.eng)
        return self._dockables

    def save_path(self, campaign):
        return list(self.eng.floats.get(campaign.name, []))

    def current_node(self, campaign):
        node = campaign.root
        for v in self.save_path(campaign):
            v = int(v)
            if v >= 0:
                if v >= len(node.subnodes):
                    return node
                node = node.subnodes[v]
            elif v == -2:
                if not node.contingency:
                    return node
                node = node.contingency
        return node

    def is_end(self, node):
        """True for the never-satisfiable node CampaignEndNode leads to."""
        for c in (node.preconditions or []):
            if isinstance(c, self.cl.InSystemCondition) and c.system and c.system[0] == 'neverneverland':
                return True
        return False

    def is_click(self, node):
        return isinstance(node, self.cl.CampaignClickNode)

    def is_choice(self, node):
        return isinstance(node, self.cl.CampaignChoiceNode)

    # ------------------------------------------------------------------
    # hypothetical condition evaluation
    # ------------------------------------------------------------------
    def insystem_matches(self, cond, loc):
        if loc is None:
            return False
        if cond.system:
            parts = loc.system.split('/')
            for i in range(-1, -min(len(cond.system), len(parts)) - 1, -1):
                if parts[i].lower() != cond.system[i]:
                    return False
        if cond.dockedshipname:
            dn = cond.dockedshipname
            return _norm(loc.name) == dn or _norm(loc.fullname) == dn
        return True

    def ev(self, cond, loc):
        cl = self.cl
        name = type(cond).__name__
        if name == 'StealGun':
            if not self.insystem_matches(cond, loc):
                import quest
                return bool(quest.checkSaveValue(0, 'removed_' + cond.guntype, 1))
            p = self.eng.player
            has = p is not None and any(m['weapon'] == cond.guntype and m['status'] in ('ACTIVE', 'INACTIVE')
                                        for m in p.mounts)
            if has:
                return True
            import quest
            return bool(quest.checkSaveValue(0, 'removed_' + cond.guntype, 1))
        if isinstance(cond, cl.InSystemCondition):
            return self.insystem_matches(cond, loc)
        if isinstance(cond, cl.OrCondition):
            return any(self.ev(c, loc) for c in cond.conds)
        if isinstance(cond, cl.AndCondition):
            return all(self.ev(c, loc) for c in cond.conds)
        if isinstance(cond, cl.InvertCondition):
            return not self.ev(cond.cond, loc)
        if isinstance(cond, cl.HasUndocked):
            return True
        try:
            return bool(cond())
        except Exception:
            return False

    def ev_all(self, conds, loc):
        for c in conds or []:
            if not self.ev(c, loc):
                return False
        return True

    # ------------------------------------------------------------------
    # script prediction
    # ------------------------------------------------------------------
    def predict_script(self, script, node, loc):
        """What would ``script(room, subnodes)`` return?  None = unknown."""
        cl = self.cl
        if script is None:
            return None
        name = type(script).__name__
        subs = node.subnodes
        if isinstance(script, cl.TrueBackwardsSubnode):
            for i in range(len(subs) - 1, -1, -1):
                if self.ev_all(subs[i].preconditions, loc):
                    return i
            return -1
        if isinstance(script, cl.TrueSubnode):
            for i, s in enumerate(subs):
                if self.ev_all(s.preconditions, loc):
                    return i
            return -1
        if isinstance(script, cl.GoToSubnodeIfTrue):
            ok = self.predict_bool(script.nextscript)
            return script.iftrue if ok else script.iffalse
        if isinstance(script, cl.GoToSubnode):
            return script.const
        r = self.predict_bool(script)
        return int(bool(r))

    def predict_bool(self, script):
        cl = self.cl
        if script is None:
            return False
        if isinstance(script, cl.RemoveCargo):
            p = self.eng.player
            have = 0
            if p is not None:
                for c in p.cargo:
                    if c._content == script.cargname:
                        have += c._quantity
            return have >= script.cargnum and have >= 1
        if isinstance(script, cl.DisplayTextIfTrueScript):
            return self.predict_bool(script.nextscript)
        if isinstance(script, cl.SaveVariableGreaterScript):
            import Director
            n = Director.getSaveDataLength(0, script.var)
            v = Director.getSaveData(0, script.var, 0) if n > 0 else 0
            return v > script.val
        return True

    # ------------------------------------------------------------------
    # simulation of getCurrentNode
    # ------------------------------------------------------------------
    def sim_evaluate(self, node, loc, depth=0):
        if depth > 40:
            return node
        if self.is_click(node) or self.is_choice(node):
            return node
        if not self.ev_all(node.preconditions, loc):
            return node
        num = self.predict_script(node.script, node, loc)
        if num is None:
            return node
        if 0 <= num < len(node.subnodes):
            self._moved = True
            return self.sim_evaluate(node.subnodes[num], loc, depth + 1)
        return node

    def sim_current(self, node, loc):
        """Returns (node_after, active) mirroring Campaign.getCurrentNode.
        After the call ``self._moved`` tells whether any script moved the
        campaign down a subnode (i.e. real progress happened)."""
        self._moved = False
        for _ in range(60):
            if self.ev_all(node.preconditions, loc):
                return self.sim_evaluate(node, loc), True
            if not node.contingency:
                return node, False
            node = self.sim_evaluate(node.contingency, loc)
        return node, False

    def click_useful(self, node, loc):
        """Would clicking this (fixer) node change anything?"""
        if self.is_choice(node):
            return True
        if not node.subnodes:
            return False
        num = self.predict_script(node.script, node, loc)
        return num is not None and 0 <= num < len(node.subnodes)

    # ------------------------------------------------------------------
    # candidate locations
    # ------------------------------------------------------------------
    def conditions_in_subtree(self, node, depth=10):
        seen = set()
        out = []
        stack = [(node, 0)]
        cl = self.cl

        def collect(c):
            if isinstance(c, cl.InSystemCondition):
                out.append(c)
            elif isinstance(c, (cl.OrCondition, cl.AndCondition)):
                for x in c.conds:
                    collect(x)
            elif isinstance(c, cl.InvertCondition):
                pass
        while stack:
            n, d = stack.pop()
            if id(n) in seen or d > depth:
                continue
            seen.add(id(n))
            for c in n.preconditions or []:
                collect(c)
            for s in n.subnodes:
                stack.append((s, d + 1))
            if n.contingency:
                stack.append((n.contingency, d + 1))
        return out

    def expand(self, cond):
        locs = []
        dock = self.dockables()
        for sysname, items in dock.items():
            for loc in items:
                if self.insystem_matches(cond, loc):
                    locs.append(loc)
        return locs

    def candidates(self, campaign):
        """[(loc, result_node, reason)] where docking at loc would matter."""
        cur = self.current_node(campaign)
        if self.is_end(cur):
            return []
        locs = []
        seen = set()
        for c in self.conditions_in_subtree(cur):
            for loc in self.expand(c):
                if loc not in seen:
                    seen.add(loc)
                    locs.append(loc)
        out = []
        for loc in locs:
            node, active = self.sim_current(cur, loc)
            if not active:
                if self._moved and node is not cur and not self.same_state(cur, node):
                    out.append((loc, node, 'advances'))
                continue
            if node is not cur and not self.same_state(cur, node):
                out.append((loc, node, 'advances'))
            elif node.spritelink and self.click_useful(node, loc):
                out.append((loc, node, 'fixer'))
            elif self.click_useful(node, loc) and self.has_click_access(loc):
                out.append((loc, node, 'click'))
        return out

    def campaign_variables(self):
        """Names of save variables the campaign trees test or set."""
        cl = self.cl
        names = set()

        def scan_cond(c):
            if isinstance(c, cl.SaveVariableCondition):
                names.add(c.name)
            elif isinstance(c, (cl.OrCondition, cl.AndCondition)):
                for x in c.conds:
                    scan_cond(x)
            elif isinstance(c, cl.InvertCondition):
                scan_cond(c.cond)

        def scan_script(sc):
            while sc is not None:
                if isinstance(sc, cl.SetSaveVariable):
                    names.add(sc.name)
                if isinstance(sc, cl.LoadMission):
                    names.add(sc.name)
                sc = getattr(sc, 'nextscript', None)
        for camp in self.campaigns():
            seen = set()
            stack = [camp.root]
            while stack:
                n = stack.pop()
                if id(n) in seen:
                    continue
                seen.add(id(n))
                for c in n.preconditions or []:
                    scan_cond(c)
                scan_script(n.script)
                stack.extend(n.subnodes)
                if n.contingency:
                    stack.append(n.contingency)
        return names

    def same_state(self, a, b):
        """Moving between a waiting node and its contingency (or between two
        reminder variants sharing a contingency) is bookkeeping, not progress."""
        if b is a.contingency or a is b.contingency:
            return True
        if a.contingency is not None and a.contingency is b.contingency:
            return True
        return False

    def hints(self, campaign, depth=4):
        """Systems the player is evidently meant to visit: unsatisfied
        SaveVariableCondition('visited_<system>', 1) near the current node."""
        cl = self.cl
        cur = self.current_node(campaign)
        out = []
        seen = set()
        stack = [(cur, 0)]

        def scan(c):
            if isinstance(c, cl.SaveVariableCondition):
                if c.name.startswith('visited_') and c.value == 1 and not c():
                    out.append(c.name[len('visited_'):])
            elif isinstance(c, (cl.OrCondition, cl.AndCondition)):
                for x in c.conds:
                    scan(x)
        while stack:
            n, d = stack.pop()
            if id(n) in seen or d > depth:
                continue
            seen.add(id(n))
            for c in n.preconditions or []:
                scan(c)
            for x in n.subnodes:
                stack.append((x, d + 1))
            if n.contingency:
                stack.append((n.contingency, d + 1))
        return out

    def has_click_access(self, loc):
        """Some bases call campaign_lib.clickFixer from a room link of their
        own (Oxford library, Perry's admiral office, Eden's church) so even
        nodes without a bar fixer sprite can be clicked there."""
        key = loc.key()
        cache = self.__dict__.setdefault('_click_cache', {})
        if key not in cache:
            cache[key] = self._base_calls_clickfixer(loc)
        return cache[key]

    def _base_calls_clickfixer(self, loc):
        import os
        import re
        eng = self.eng
        from mockvs.engine import cargo_unit_name
        basename = cargo_unit_name(loc.fullname) if loc.kind == 'planet' else loc.name
        if loc.kind == 'loaded':
            for u in eng.systems[loc.system].units:
                if u.name == loc.name and u.fullname == loc.fullname:
                    basename = cargo_unit_name(u.fullname) if u.is_planet() else u.name
                    break
        fac = eng.get_galaxy_faction(loc.system)
        files = []
        for tod in ('day', 'night', 'sunset', None):
            for fn in ('bases/%s_%s' % (basename, fac), 'bases/%s' % basename):
                cand = fn + ('_' + tod if tod else '') + '.py'
                if os.path.isfile(eng.data.path(cand)):
                    files.append(cand)
        seen = set()
        while files:
            f = files.pop()
            if f in seen:
                continue
            seen.add(f)
            try:
                text = open(eng.data.path(f), encoding='latin-1').read()
            except OSError:
                continue
            if 'campaign_lib.clickFixer' in text and 'bar_lib' not in f:
                return True
            for mod in re.findall(r'^\s*import\s+(\w+_lib)\b', text, re.M):
                cand = 'bases/%s.py' % mod
                if mod != 'bar_lib' and os.path.isfile(eng.data.path(cand)):
                    files.append(cand)
        return False

    def explain(self, campaign, loc, steps=8):
        """Trace of sim_current for debugging."""
        out = []
        node = self.current_node(campaign)
        for _ in range(steps):
            conds = [(type(c).__name__, self.ev(c, loc)) for c in node.preconditions or []]
            out.append('%s %s conds=%s script=%s subs=%d cont=%s' % (
                type(node).__name__, id(node) % 10000, conds, type(node.script).__name__,
                len(node.subnodes), node.contingency is not None))
            if self.ev_all(node.preconditions, loc):
                out.append(' -> active; predict=%r' % (self.predict_script(node.script, node, loc),))
                break
            if not node.contingency:
                break
            c = node.contingency
            out.append(' contingency %s script=%s predict=%r subs=%s' % (
                id(c) % 10000, type(c.script).__name__, self.predict_script(c.script, c, loc),
                [[(type(x).__name__, self.ev(x, loc)) for x in s.preconditions] for s in c.subnodes]))
            node = self.sim_evaluate(c, loc)
        return out

    def describe(self, campaign):
        cur = self.current_node(campaign)
        text = cur.text
        if isinstance(text, list) and text:
            t = text[0]
            text = t[1] if isinstance(t, tuple) and len(t) > 1 else t
        return '%s @%s %s%s' % (campaign.name, self.save_path(campaign)[-3:], type(cur).__name__,
                                (' "%s"' % str(text)[:60]) if text else '')
