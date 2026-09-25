"""High level control of the mock engine: the "hands" of the automated player.

``Game`` owns an engine instance and offers the actions a human player
performs: clicking base links, launching, flying to things, jumping,
fighting and docking.  Everything goes through the same code paths the
real game uses (base links run their Python, docking loads the base
script, jumping marks systems visited, ...).
"""

import collections
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))

from mockvs import engine as engmod          # noqa: E402
from mockvs import modules as modmod         # noqa: E402
from mockvs import base as basemod           # noqa: E402
from mockvs.units import wrap                # noqa: E402


class Stuck(Exception):
    pass


class Game:
    def __init__(self, datadir, options=None, log=None, script_log=None):
        """``script_log``: file object receiving everything the data pack
        prints (it is very chatty); defaults to a null sink."""
        self.datadir = os.path.abspath(datadir)
        self.real_stdout = sys.stdout
        if log is None:
            def log(msg, _out=self.real_stdout):
                _out.write(msg + '\n')
                _out.flush()
        self.log = log
        self.script_log = script_log if script_log is not None else open(os.devnull, 'w')
        sys.stdout = self.script_log
        sys.stderr = self.script_log   # the data pack's debug.warn prints stack traces here
        self.eng = engmod.Engine(self.datadir, options, log=log)
        if getattr(self.eng.opts, 'check_division', False):
            from mockvs import divcheck
            divcheck.enable(self.datadir)
        modmod.install(self.datadir, self.eng.main_globals)
        os.chdir(self.datadir)   # the engine runs with the data dir as cwd
        self.max_frames_per_action = 20000

    # ------------------------------------------------------------------
    # state
    # ------------------------------------------------------------------
    @property
    def player(self):
        return self.eng.player

    @property
    def base(self):
        return self.eng.base

    def docked(self):
        return self.eng.base is not None

    def system(self):
        p = self.player
        return p.system.name if p is not None and p.system else None

    # ------------------------------------------------------------------
    # game start
    # ------------------------------------------------------------------
    def start_new_game(self):
        self.eng.start_mission_file(self.eng.data.config.get('general', 'default_mission', 'main_menu.mission'))
        if self.base is None:
            raise Stuck('main menu did not load')
        link = self.find_link(lambda l: l.index == 'New_Game')
        if link is None:
            raise Stuck('no New Game button on the main menu')
        self.click(link)
        self.eng.process_pending_load()
        if not self.docked():
            raise Stuck('new game did not dock the player')

    # ------------------------------------------------------------------
    # base interaction
    # ------------------------------------------------------------------
    def rooms(self):
        return self.base.rooms if self.base else []

    def find_link(self, pred, room=None):
        if self.base is None:
            return None
        rooms = [room] if room is not None else [self.base.curroom] + list(range(len(self.base.rooms)))
        for ri in rooms:
            r = self.base.room(ri)
            if r is None:
                continue
            for l in r.links:
                if pred(l):
                    return l
        return None

    def find_links(self, pred):
        out = []
        if self.base is None:
            return out
        for ri, r in enumerate(self.base.rooms):
            for l in r.links:
                if pred(l):
                    out.append((ri, l))
        return out

    def goto_room(self, index):
        """Walk to a room the way a player would (via goto links); fall back
        to jumping straight there when no path exists."""
        base = self.base
        if base is None or base.curroom == index:
            return
        # BFS over goto links
        prev = {base.curroom: None}
        q = collections.deque([base.curroom])
        while q:
            r = q.popleft()
            if r == index:
                break
            room = base.room(r)
            if room is None:
                continue
            for l in room.links:
                if l.kind == 'goto' and l.to not in prev and 0 <= l.to < len(base.rooms):
                    prev[l.to] = (r, l)
                    q.append(l.to)
        if index not in prev:
            basemod.goto_room(self.eng, index)
            return
        path = []
        cur = index
        while prev[cur] is not None:
            r, l = prev[cur]
            path.append((r, l))
            cur = r
        for r, l in reversed(path):
            if self.base is not base:
                return
            basemod.goto_room(self.eng, r)
            self.click(l)

    def click(self, link):
        basemod.click_link(self.eng, link)
        self.eng.process_pending_load()

    def launch(self):
        if not self.docked():
            return
        ri_links = self.find_links(lambda l: l.kind == 'launch')
        if not ri_links:
            raise Stuck('no launch link in base %s' % self.base.basefile)
        ri, l = ri_links[0]
        self.goto_room(ri)
        self.click(l)
        if self.docked():
            raise Stuck('launch did not leave the base')

    # ------------------------------------------------------------------
    # flight
    # ------------------------------------------------------------------
    def step(self, n=1):
        for _ in range(n):
            if self.docked():
                return
            self.eng.step()

    def units_here(self, pred=None):
        p = self.player
        if p is None or p.system is None:
            return []
        return [u for u in p.system.units if not u.killed and (pred is None or pred(u))]

    def find_unit(self, name):
        name = name.lower().replace(' ', '_')
        for u in self.units_here():
            if u.name.lower().replace(' ', '_') == name or u.fullname.lower().replace(' ', '_') == name:
                return u
        return None
