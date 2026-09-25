"""Base crawler: dock everywhere and click everything.

For every dockable unit in the galaxy the crawler docks the player (by
teleporting next to it - flying is not the point here) and then walks the
base interface breadth first, clicking every link in every room: rooms,
GUI buttons (commodity exchange, ship dealer, upgrade and repair screens,
Quine computer, mission computer, guilds), bartenders and fixers.  Every
Python exception raised by the data pack is recorded by the engine.
"""

import collections

from .game import Game, Stuck
from .campaign_model import static_dockables

# Links that would end the crawl of this base or the whole game.
SKIP_TEXT = ('Quit', 'XXXQuit', 'XXXLoad', 'Load_Game', 'XXXNew Game', 'New_Game')


class Crawler:
    def __init__(self, datadir, options, log=None, script_log=None, max_clicks=250,
                 systems=None, verbose=1, fuzz_clicks=40, credits=5.0e6):
        self.game = Game(datadir, options, log=log, script_log=script_log)
        self.eng = self.game.eng
        self.log = self.game.log
        self.max_clicks = max_clicks
        self.systems = systems
        self.verbose = verbose
        self.visited_files = collections.Counter()
        self.clicks = 0
        self.fuzz_clicks = fuzz_clicks
        self.credits = credits
        import random
        self.rng = random.Random(1234)

    def run(self):
        g = self.game
        g.start_new_game()
        self.eng.credits = self.credits
        dock = static_dockables(self.eng)
        names = sorted(dock) if self.systems is None else list(self.systems)
        for sysname in names:
            if not sysname.startswith('Gemini/'):
                continue
            s = self.eng.get_system(sysname)
            for u in list(s.units):
                if u.killed or not u.is_dockable() or u is self.eng.player:
                    continue
                try:
                    self.crawl_base(u)
                except Stuck as e:
                    self.log('[crawler] stuck at %r: %s' % (u, e))
        return True

    def teleport_and_dock(self, u):
        eng = self.eng
        p = eng.player
        if eng.base is not None:
            self.game.launch()
        if p.system is not u.system:
            eng.transfer_unit(p, u.system.name)
        p.position = [u.position[0] + u.radius + 50.0, u.position[1], u.position[2]]
        if not eng.dock(p, u, check_distance=False):
            raise Stuck('could not dock at %r' % u)

    def crawl_base(self, u):
        self.teleport_and_dock(u)
        base = self.eng.base
        f = base.basefile
        self.visited_files[f] += 1
        if self.verbose:
            self.log('[crawler] %s: %r (%s)' % (u.system.name, u, f))
        done = set()
        clicks = 0
        while clicks < self.max_clicks:
            if self.eng.base is None or self.eng.base.baseun is not u:
                self.teleport_and_dock(u)
            base = self.eng.base
            todo = None
            for ri, r in enumerate(base.rooms):
                for l in r.links:
                    key = (ri, r.text, l.index, l.text, l.kind)
                    if key in done or l.kind == 'launch' or l.text.startswith(SKIP_TEXT) \
                            or l.index.startswith(SKIP_TEXT):
                        continue
                    todo = (ri, l, key)
                    break
                if todo:
                    break
            if todo is None:
                break
            ri, l, key = todo
            done.add(key)
            try:
                self.game.goto_room(ri)
                if self.eng.base is base:
                    self.game.click(l)
            except Exception as e:     # driver-side problems, not data errors
                self.log('[crawler] click failed %r: %r' % (l, e))
            clicks += 1
            self.clicks += 1
        self.fuzz(u)
        if self.eng.base is not None:
            self.game.launch()

    def fuzz(self, u):
        """Random click sequences inside rooms with GUI buttons (modes,
        next/prev item, select = buy/sell/repair)."""
        if self.eng.base is None or self.eng.base.baseun is not u:
            self.teleport_and_dock(u)
        base = self.eng.base
        gui_rooms = [i for i, r in enumerate(base.rooms)
                     if sum(1 for l in r.links if l.kind == 'python' and l.mask != 'c') >= 3]
        for ri in gui_rooms:
            for _ in range(self.fuzz_clicks):
                if self.eng.base is None or self.eng.base.baseun is not u:
                    self.teleport_and_dock(u)
                base = self.eng.base
                if ri >= len(base.rooms):
                    break
                self.game.goto_room(ri)
                if self.eng.base is not base or base.curroom != ri:
                    break
                links = [l for l in base.rooms[ri].links if l.kind == 'python'
                         and not l.text.startswith(SKIP_TEXT) and not l.index.startswith(SKIP_TEXT)]
                if not links:
                    break
                l = self.rng.choice(links)
                try:
                    self.game.click(l)
                except Exception as e:
                    self.log('[crawler] click failed %r: %r' % (l, e))
                self.clicks += 1
