"""Flight primitives for the automated player: travel, jumping, combat, docking."""

import collections
import math

from .game import Stuck


def dist(a, b):
    return math.sqrt(sum((a.position[i] - b.position[i]) ** 2 for i in range(3)))


class Pilot:
    def __init__(self, game, log=print):
        self.game = game
        self.eng = game.eng
        self.log = log
        self.combat_speed = 1200.0
        self.threat_range = 4000.0
        self.max_fight_hull = 2000.0      # don't duel "immortal" NPCs
        self.kills = 0
        self.fight_timeout = 600.0        # game seconds per engagement
        self.escort_targets = []          # friendlies whose attackers we engage
        self.ignore_threats = False

    @property
    def player(self):
        return self.eng.player

    def now(self):
        return self.eng.game_time_total()

    # ------------------------------------------------------------------
    # threats & combat
    # ------------------------------------------------------------------
    def threats(self, extra=()):
        p = self.player
        out = []
        protect = [p] + [u for u in self.escort_targets if not u.killed]
        for u in self.game.units_here():
            if u is p or u.killed or u.docked_to is not None or u.is_planet():
                continue
            if u.kind not in ('ship',):
                continue
            if u.hull > self.max_fight_hull and u not in extra:
                continue
            ct = getattr(u, 'combat_target', None)
            targeting_us = ct in protect or u.target in protect
            hostile = self.eng.unit_relation(u, p) < 0
            if u in extra or (targeting_us and (hostile or ct in protect)) or \
                    (hostile and u.hostile_spawn and dist(u, p) < self.threat_range):
                out.append(u)
        out.sort(key=lambda u: (u not in extra, dist(u, p)))
        return out

    def attack(self, target, timeout=None, interrupt=None):
        """Close in on and destroy ``target``.  Returns True if it died."""
        p = self.player
        start = self.now()
        timeout = timeout or self.fight_timeout
        polls = 0
        while not target.killed and self.player is p and not p.killed:
            polls += 1
            if interrupt is not None and polls % 4 == 0 and interrupt():
                break
            if self.game.docked():
                return False
            if target.system is not p.system or target.docked_to is not None:
                return False
            if self.now() - start > timeout:
                self.log('[pilot] giving up on %r after %.0fs' % (target, timeout))
                return False
            p.target = target
            p.fire_target = target
            d = dist(p, target) - p.radius - target.radius
            if d > p.weapon_range * 0.8:
                p.moveto = ('unit', target, self.combat_speed, p.weapon_range * 0.5)
            else:
                p.moveto = None
            self.game.step()
        if target.killed:
            self.kills += 1
        p.target = None
        p.fire_target = None
        return target.killed

    def fight(self, priority=(), max_rounds=200):
        """Deal with everything attacking us (and ``priority`` targets)."""
        rounds = 0
        while rounds < max_rounds:
            rounds += 1
            if self.game.docked() or self.player is None or self.player.killed:
                return
            ts = self.threats(extra=[u for u in priority if not u.killed])
            if not ts:
                return
            self.attack(ts[0])

    # ------------------------------------------------------------------
    # travel
    # ------------------------------------------------------------------
    def fly_to(self, unit, within=100.0, fight=True, timeout=3600.0, significant=False, interrupt=None):
        """Fly until within range.  ``interrupt()`` is polled every few
        frames; when it returns True we stop early so the caller re-plans."""
        p = self.player
        start = self.now()
        polls = 0
        while True:
            polls += 1
            if interrupt is not None and polls % 8 == 0 and interrupt():
                p.moveto = None
                return False
            if self.game.docked() or self.player is not p or p.killed:
                return False
            if unit.killed or unit.system is not p.system:
                return False
            d = p.significant_distance_to(unit) if significant else p.distance_to(unit)
            if d <= within:
                p.moveto = None
                return True
            if self.now() - start > timeout:
                raise Stuck('could not reach %r (distance %.0f)' % (unit, d))
            if fight and not self.ignore_threats:
                ts = self.threats()
                if ts:
                    self.attack(ts[0], timeout=30.0, interrupt=interrupt)
                    continue
            p.moveto = ('unit', unit, self.eng.opts.cruise_speed, max(within - 10.0, 0.0) * 0.5)
            self.game.step()

    def wait(self, seconds, fight=True, interrupt=None):
        """Loiter for ``seconds``, shooting back at anything attacking us."""
        end = self.now() + seconds
        p = self.player
        while self.now() < end:
            if self.game.docked() or self.player is not p:
                return
            if interrupt is not None and interrupt():
                return
            if fight and not self.ignore_threats:
                ts = self.threats()
                if ts:
                    self.attack(ts[0], timeout=max(end - self.now(), 0.5), interrupt=interrupt)
                    continue
            p.moveto = None
            self.game.step()

    def route(self, src, dst):
        links = self.eng.all_jump_links()
        prev = {src: None}
        q = collections.deque([src])
        while q:
            s = q.popleft()
            if s == dst:
                break
            for d in links.get(s, {}):
                if d not in prev:
                    prev[d] = s
                    q.append(d)
        if dst not in prev:
            return None
        path = []
        cur = dst
        while cur != src:
            path.append(cur)
            cur = prev[cur]
        path.reverse()
        return path

    def jump_to_adjacent(self, dest):
        p = self.player
        if not p.jump_drive:
            raise Stuck('no jump drive (cannot jump to %s)' % dest)
        jp = None
        for u in self.game.units_here(lambda u: u.is_jumppoint()):
            if dest in u.destinations:
                jp = u
                break
        if jp is None:
            raise Stuck('no jump point from %s to %s' % (p.system_file(), dest))
        if not self.fly_to(jp, within=5.0):
            if p.system_file() == dest:
                return True
            raise Stuck('could not reach jump point %r' % jp)
        self.eng.pending_jumps.append((p, dest, jp))
        self.game.step()
        if p.system_file() != dest:
            raise Stuck('jump to %s failed' % dest)
        return True

    def goto_system(self, dest, via=()):
        """Travel to ``dest``, optionally passing through ``via`` systems."""
        for waypoint in list(via) + [dest]:
            while self.player.system_file() != waypoint:
                path = self.route(self.player.system_file(), waypoint)
                if not path:
                    raise Stuck('no route from %s to %s' % (self.player.system_file(), waypoint))
                self.jump_to_adjacent(path[0])
                if self.game.docked():
                    return False
        return True

    def dock_at(self, unit):
        p = self.player
        if not unit.is_dockable():
            raise Stuck('%r is not dockable' % unit)
        target = max(unit.radius, 300.0) - 10.0
        if not self.fly_to(unit, within=target, significant=unit.is_planet()):
            return False
        ok = self.eng.dock(p, unit, check_distance=True)
        if not ok:
            raise Stuck('dock with %r refused' % unit)
        return True

    def tractor(self, unit):
        if not self.fly_to(unit, within=50.0, fight=False):
            return False
        return self.eng.tractor(self.player, unit)
