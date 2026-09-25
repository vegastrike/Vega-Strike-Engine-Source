"""The automated player: plays the campaigns end to end on the mock engine."""

import collections
import time

from .game import Game, Stuck
from .pilot import Pilot
from .campaign_model import CampaignModel, _norm
from .strategies import Strategies


class Report:
    def __init__(self):
        self.lines = []
        self.phase_results = []
        self.mission_log = []       # (time, what)
        self.interactions = []
        self.problems = []

    def add(self, msg):
        self.lines.append(msg)


class CampaignDriver:
    def __init__(self, datadir, scenario, options, log=None, script_log=None, verbose=1):
        self.scenario = scenario
        self.game = Game(datadir, options, log=log, script_log=script_log)
        self.log = self.game.log
        self.eng = self.game.eng
        self.pilot = Pilot(self.game, log=self.log)
        self.strategies = Strategies(scenario, log=self.log)
        self.model = None
        self.report = Report()
        self.verbose = verbose
        self.refused = set()
        self.fail_memo = collections.Counter()
        self.loc_states = {}
        self.visits = collections.Counter()
        self.phase = None
        self.bonus_unlocked = False
        self.started = time.time()

    # ------------------------------------------------------------------
    def say(self, msg):
        self.report.add('[t=%7.0f] %s' % (self.eng.game_time_total(), msg))
        if self.verbose:
            self.log('[t=%7.0f] %s' % (self.eng.game_time_total(), msg))

    def node_state(self):
        return tuple(id(self.model.current_node(c)) for c in self.model.campaigns())

    def campaign_state(self):
        return tuple((c.name, tuple(self.eng.floats.get(c.name, []))) for c in self.model.campaigns())

    def signature(self):
        sig = []
        for c in self.model.campaigns():
            sig.append((c.name, tuple(self.eng.floats.get(c.name, []))))
        sig.append(('missions', tuple(m.id for m in self.eng.missions)))
        return tuple(sig)

    # ------------------------------------------------------------------
    # base interaction
    # ------------------------------------------------------------------
    def choice_links(self):
        return self.game.find_links(lambda l: 'campaign_lib.clickChoice' in (l.pythonfile or ''))

    def fixer_links(self):
        """Campaign fixer links, limited to fixers of campaigns in the
        current phase (a bar may also show fixers of other campaigns)."""
        links = self.game.find_links(lambda l: 'campaign_lib.clickFixer' in (l.pythonfile or ''))
        if self.phase is None or self.model is None:
            return links
        wanted = set()
        other = set()
        for c in self.model.campaigns():
            node = self.model.current_node(c)
            texts = set()
            for n in [node] + ([node.contingency] if node.contingency else []) + list(node.subnodes):
                if n is not None and n.spritelink:
                    texts.add(n.spritelink[1])
            if c.name in self.phase.campaigns:
                wanted |= texts
            else:
                other |= texts
        out = []
        for ri, l in links:
            if l.kind == 'python' and l.text in other and l.text not in wanted:
                continue
            out.append((ri, l))
        return out

    def pick_choice(self, links):
        texts = [l.text for _, l in links]
        explicit = [(ri, l) for ri, l in links if self.scenario.choice(l.text)
                    or any(self.scenario.choice(k) is True and l.text.startswith(k)
                           for k in self.scenario.choices if k[:1].isupper())]
        if explicit:
            return explicit[0]
        node_key = self.node_state()
        yes = [(ri, l) for ri, l in links if l.text.startswith(('Accept', 'Offer'))]
        no = [(ri, l) for ri, l in links if l.text.startswith(('Refuse', 'Reject'))]
        if self.scenario.refuse_first and no and node_key not in self.refused:
            self.refused.add(node_key)
            return no[0]
        if yes:
            return yes[0]
        return links[-1]

    def last_dialog(self):
        if self.eng.base is None:
            return ''
        msgs = self.eng.base.last_messages(1)
        return msgs[-1] if msgs else ''

    def handle_base(self):
        """Talk to every campaign fixer here until nothing changes."""
        base = self.eng.base
        if base is None:
            return False
        progressed = False
        tried = set()
        seen_nodes = {self.node_state()}
        for _ in range(60):
            if self.eng.base is not base:
                break
            before = self.signature()
            choices = self.choice_links()
            if choices:
                ri, l = self.pick_choice(choices)
                self.say('  choose %s' % l.text)
                self.game.goto_room(ri)
                self.game.click(l)
                self.report.interactions.append((self.eng.game_time_total(), base.basefile, l.text))
                progressed = True
                continue
            clicked = False
            for ri, l in self.fixer_links():
                # re-clicking the same fixer only makes sense if the campaign
                # state moved; a click that merely (re)loads a mission is done
                key = (ri, l.index, l.text, self.campaign_state())
                if key in tried:
                    continue
                tried.add(key)
                self.game.goto_room(ri)
                self.game.click(l)
                after = self.signature()
                ns = self.node_state()
                if ns in seen_nodes and not self.choice_links():
                    # a loop in the campaign tree (e.g. Syrai's small talk)
                    continue
                seen_nodes.add(ns)
                if after != before or self.choice_links():
                    self.say('  talk: %s -> %s' % (l.text, self.last_dialog()[:120].replace('\n', ' | ')))
                    self.report.interactions.append((self.eng.game_time_total(), base.basefile, l.text))
                    progressed = True
                    clicked = True
                    break
            if not clicked:
                break
        return progressed

    def apply_doom(self):
        now = self.eng.game_time_total()
        for mtype, fgp, delay in self.scenario.doomed:
            for s in self.eng.systems.values():
                for u in s.units:
                    if (not u.killed and u.launched_by == mtype and u.fg_name().startswith(fgp)
                            and now - u.spawn_time >= delay):
                        self.say('scenario: destroying %r (doomed)' % (u,))
                        self.eng.kill_unit(u, None)

    def loot_base(self):
        """Pick up the Steltek gun on the derelict fighter."""
        if not self.scenario.choice('collect_steltek_gun', True):
            return
        for ri, l in self.game.find_links(lambda l: l.text == 'Remove Derelict Weapon'):
            self.game.goto_room(ri)
            self.game.click(l)
            self.say('  took the derelict Steltek weapon')

    def opportunistic(self):
        if not self.scenario.choice('collect_steltek_gun', True):
            return
        if self.eng.floats.get('have_the_gun', [0])[0] == 1:
            return
        for u in self.game.units_here(lambda u: u.name == 'derelict' and u.is_dockable()):
            self.say('found the derelict fighter; docking')
            try:
                self.pilot.dock_at(u)
            except Stuck as e:
                self.say('  could not dock at derelict: %s' % e)
            return

    # ------------------------------------------------------------------
    # planning
    # ------------------------------------------------------------------
    def active_campaigns(self):
        wanted = set(self.phase.campaigns)
        return [c for c in self.model.campaigns() if c.name in wanted]

    def mission_failures(self):
        """Campaign variables set to -1 (the campaign_lib failure marker)."""
        if not hasattr(self, '_campaign_vars'):
            self._campaign_vars = self.model.campaign_variables()
        return [e for e in self.eng.events if e['kind'] == 'var_failed' and e['time'] >= self.phase_start
                and e['key'] in self._campaign_vars]

    def phase_done(self):
        by_name = {c.name: c for c in self.model.campaigns()}
        for n in self.phase.required:
            c = by_name.get(n)
            if c is None or not self.model.is_end(self.model.current_node(c)):
                return False
        return True

    def mission_goal(self):
        for m in list(self.eng.missions[1:]):
            try:
                g = self.strategies.goal(self.eng, m)
            except Exception as e:
                self.say('strategy error for %r: %r' % (m, e))
                g = None
            if g is not None:
                return m, g
        return None, None

    def jumps_to(self, sysname):
        cur = self.eng.player.system_file()
        if cur == sysname:
            return 0
        r = self.pilot.route(cur, sysname)
        return len(r) if r is not None else 999

    def hint_goal(self):
        for c in self.active_campaigns():
            if self.model.is_end(self.model.current_node(c)):
                continue
            for sysname in self.model.hints(c):
                if self.eng.player.system_file() != sysname and self.pilot.route(
                        self.eng.player.system_file(), sysname) is not None:
                    return sysname
        return None

    def campaign_goal(self):
        options = []
        for c in self.active_campaigns():
            node = self.model.current_node(c)
            if self.model.is_end(node):
                continue
            maxv = self.phase.optional_visits.get(c.name)
            for loc, res, why in self.model.candidates(c):
                memo = (c.name, id(node), loc.key())
                if self.fail_memo[memo] >= 1:
                    continue
                if maxv is not None and self.visits[c.name] >= maxv:
                    continue
                prio = self.phase.campaigns.index(c.name)
                options.append((self.jumps_to(loc.system), prio, loc, c, node, why))
        if not options:
            return None
        options.sort(key=lambda o: (o[0], o[1]))
        return options[0]

    # ------------------------------------------------------------------
    # execution
    # ------------------------------------------------------------------
    def ensure_flying(self):
        if self.game.docked():
            self.game.launch()

    def find_loc_unit(self, loc):
        for u in self.game.units_here(lambda u: u.is_dockable()):
            if u.name == loc.name and u.fullname == loc.fullname:
                return u
        for u in self.game.units_here(lambda u: u.is_dockable()):
            if _norm(u.name) == _norm(loc.name) or _norm(u.fullname) == _norm(loc.fullname):
                return u
        return None

    def go_dock(self, loc):
        self.ensure_flying()
        self.pilot.goto_system(loc.system)
        if self.game.docked():
            return
        u = self.find_loc_unit(loc)
        if u is None:
            raise Stuck('cannot find %r in %s' % (loc, loc.system))
        self.pilot.dock_at(u)

    def execute(self, goal):
        kind = goal[0]
        if kind == 'dock' and self.game.docked() and self.eng.player.docked_to is goal[1]:
            return
        self.ensure_flying()
        if kind in ('near', 'dock', 'tractor') and goal[1].system is not None \
                and goal[1].system is not self.eng.player.system:
            self.pilot.goto_system(goal[1].system.name)
            return
        if kind == 'system':
            self.pilot.goto_system(goal[1])
        elif kind == 'near':
            arrived = self.pilot.fly_to(goal[1], goal[2], significant=goal[3], interrupt=self.goal_changed(goal))
            if arrived and len(goal) > 4:
                self.pilot.wait(goal[4], interrupt=self.goal_changed(goal))
        elif kind == 'dock':
            self.pilot.dock_at(goal[1])
        elif kind == 'attack':
            ts = [t for t in goal[1] if not t.killed]
            if ts:
                p = self.eng.player
                ts.sort(key=lambda t: p.distance_to(t))
                if ts[0].system is not p.system:
                    self.pilot.goto_system(ts[0].system.name)
                else:
                    self.pilot.attack(ts[0], interrupt=self.goal_changed(goal))
        elif kind == 'tractor':
            self.pilot.tractor(goal[1])
        elif kind == 'eject':
            p = self.eng.player
            for _ in range(1000):
                idx = [i for i, c in enumerate(p.cargo) if c._content == goal[1] and c._quantity > 0]
                if not idx:
                    break
                self.eng.eject_cargo(p, idx[0])
            self.pilot.wait(2.0)
        elif kind == 'wait':
            self.pilot.wait(goal[1], interrupt=self.goal_changed(goal))
        else:
            raise Stuck('unknown goal %r' % (goal,))

    def goal_changed(self, goal):
        def check():
            m, g = self.mission_goal()
            if g is None:
                return False
            if g[0] != goal[0]:
                return True
            if g[0] == 'wait' and goal[0] == 'wait':
                return False
            if g[0] == 'attack':
                return set(map(id, g[1])) != set(map(id, goal[1]))
            return len(g) > 1 and len(goal) > 1 and g[1] is not goal[1]
        return check

    def unlock_bonus_campaign(self):
        """campaign_bonus gates its first node on a base named 'DoNotEnter'
        (i.e. it is switched off in the shipped game).  Point it at Munchen
        so the bonus missions can be exercised."""
        if self.bonus_unlocked:
            return
        import campaign_lib
        n = 0
        for c in self.model.campaigns():
            if c.name != 'freetrader_campaign':
                continue
            seen = set()
            stack = [c.root]
            while stack:
                node = stack.pop()
                if id(node) in seen:
                    continue
                seen.add(id(node))
                for cond in node.preconditions or []:
                    if isinstance(cond, campaign_lib.InSystemCondition) and cond.dockedshipname == 'donotenter':
                        cond.dockedshipname = 'munchen'
                        n += 1
                stack.extend(node.subnodes)
                if node.contingency:
                    stack.append(node.contingency)
        self.bonus_unlocked = True
        self.say('unlocked bonus campaign (%d DoNotEnter conditions -> Munchen)' % n)

    # ------------------------------------------------------------------
    def run(self):
        g = self.game
        g.start_new_game()
        self.model = CampaignModel(self.eng, log=self.log)
        self.say('new game: docked at %r in %s' % (self.eng.player.docked_to, g.system()))
        for phase in self.scenario.phases:
            self.phase = phase
            if phase.name.startswith('Bonus') and self.scenario.unlock_bonus:
                self.unlock_bonus_campaign()
            ok = self.run_phase()
            self.report.phase_results.append((phase.name, ok, self.eng.game_time_total()))
            self.say('=== phase %s %s ===' % (phase.name, 'COMPLETE' if ok else 'NOT COMPLETED'))
            if not ok:
                return False
            if self.scenario.stop_after_phase == phase.name:
                break
        return True

    def run_phase(self):
        self.say('=== phase %s ===' % self.phase.name)
        self.phase_start = self.eng.game_time_total()
        idle_rounds = 0
        spin = 0
        last_t = None
        while True:
            t = self.eng.game_time_total()
            spin = spin + 1 if t == last_t else 0
            last_t = t
            if spin > 50:
                m, goal = self.mission_goal()
                self.say('STUCK: no time passes; mission goal %r of %r; docked=%s at %r' % (
                    goal, m, self.game.docked(), self.eng.player.docked_to))
                self.say('  player in %s at %r' % (self.eng.player.system_file(), self.eng.player.position))
                if m is not None and m.pyobj is not None:
                    self.say('  mission state: %r' % {k: v for k, v in vars(m.pyobj).items()
                                                      if not k.startswith('_')})
                    adj = getattr(m.pyobj, 'adjsys', None)
                    if adj is not None:
                        self.say('  adjsys state: %r' % vars(adj))
                return False
            if self.eng.game_time_total() > self.scenario.max_game_seconds:
                self.say('game time budget exhausted')
                return False
            if self.eng.player is None or self.eng.player.killed:
                self.say('PLAYER DIED')
                return False
            if self.game.docked():
                self.handle_base()
                self.loot_base()
            else:
                self.opportunistic()
                self.apply_doom()
            if self.phase_done():
                fails = self.mission_failures()
                allowed = dict(self.scenario.choice('allowed_failures', {}) or {})
                unexpected = []
                for f in fails:
                    k = (f.get('by', ''), f['key'])
                    if allowed.get(k, 0) > 0:
                        allowed[k] -= 1
                    else:
                        unexpected.append(f)
                if fails and not unexpected:
                    self.say('expected branch failures: %s' % ', '.join('%s by %s' % (f['key'], f.get('by')) for f in fails))
                fails = unexpected
                if fails:
                    self.say('campaign ended but %d mission(s) failed: %s' % (
                        len(fails), ', '.join('%s (set by %s)' % (f['key'], f.get('by')) for f in fails)))
                    return False
                return True
            m, goal = self.mission_goal()
            if self.eng.frame - getattr(self, '_last_status', 0) >= 4000:
                self._last_status = self.eng.frame
                p = self.eng.player
                self.say('status: frame %d, %s, %d units here (%d total), missions %s, goal %s' % (
                    self.eng.frame, p.system_file(), len(p.system.units) if p.system else 0,
                    sum(len(x.units) for x in self.eng.systems.values()),
                    [mm.type_name() for mm in self.eng.missions[1:]], goal and goal[:2]))
            if goal is not None:
                key = (m.id, goal[0], id(goal[1]) if len(goal) > 1 and not isinstance(goal[1], (int, float, str)) else goal[1:2])
                cstate = self.campaign_state()
                if key != getattr(self, '_stall_key', None) or cstate != getattr(self, '_stall_state', None):
                    self._stall_key, self._stall_state, self._stall_since = key, cstate, self.eng.game_time_total()
                elif self.eng.game_time_total() - self._stall_since > self.scenario.choice('stall_seconds', 3000.0):
                    self.say('STUCK: mission %s has wanted %r for %.0fs without progress' % (
                        m.type_name(), goal[:2], self.eng.game_time_total() - self._stall_since))
                    self.say('  mission: %r' % {k: v for k, v in vars(m.pyobj).items() if not k.startswith('_')})
                    return False
                self.pilot.escort_targets = self.strategies.protect(self.eng, m)
                if self.verbose >= 2:
                    self.say('mission %s: goal %s' % (m.type_name(), goal[0]))
                try:
                    t0 = self.eng.game_time_total()
                    self.execute(goal)
                    if self.eng.game_time_total() == t0 and not self.game.docked():
                        self.game.step()   # let the mission see the new situation
                except Stuck as e:
                    self.say('STUCK in mission %s: %s' % (m.type_name(), e))
                    self.say('  units here: %s' % [(u.name, u.fullname) for u in self.game.units_here()][:40])
                    self.say('  mission: %r' % {k: v for k, v in vars(m.pyobj).items() if not k.startswith('_')})
                    return False
                idle_rounds = 0
                continue
            cg = self.campaign_goal()
            if cg is None:
                hint = self.hint_goal()
                if hint is not None:
                    self.say('hint: visit %s' % hint)
                    try:
                        self.ensure_flying()
                        self.pilot.goto_system(hint)
                        self.pilot.wait(5.0)
                        self.opportunistic()
                    except Stuck as e:
                        self.say('STUCK travelling: %s' % e)
                        return False
                    continue
                idle_rounds += 1
                if self.eng.missions[1:] and idle_rounds < 200:
                    self.ensure_flying()
                    self.pilot.wait(10.0)
                    continue
                self.say('no campaign destination; state: %s' % '; '.join(
                    self.model.describe(c) for c in self.active_campaigns()))
                for c in self.active_campaigns():
                    cur = self.model.current_node(c)
                    for cond in self.model.conditions_in_subtree(cur, depth=4):
                        locs = self.model.expand(cond)
                        self.say('  cond %s/%s -> %d locs %s' % (cond.system, cond.dockedshipname, len(locs),
                                 [(l, type(self.model.sim_current(cur, l)[0]).__name__,
                                   self.model.sim_current(cur, l)[1]) for l in locs[:3]]))
                        for l in locs[:1]:
                            for line in self.model.explain(c, l):
                                self.say('      ' + line)
                return False
            dist, prio, loc, camp, node, why = cg
            self.say('%s: go to %r (%s)' % (camp.name, loc, why))
            before = self.signature()
            before_nodes = self.node_state()
            try:
                if self.game.docked() and self.find_loc_unit(loc) is self.eng.player.docked_to:
                    # relaunch and land again so the base (bar) is rebuilt
                    self.ensure_flying()
                self.go_dock(loc)
            except Stuck as e:
                self.say('STUCK travelling: %s' % e)
                return False
            self.handle_base()
            after_nodes = self.node_state()
            seen = self.loc_states.setdefault(loc.key(), set())
            seen.add(before_nodes)
            missions_changed = self.signature()[-1] != before[-1]
            if (after_nodes in seen or after_nodes == before_nodes) and not missions_changed:
                self.fail_memo[(camp.name, id(node), loc.key())] += 1
                self.say('  (no progress at %r)' % (loc,))
            else:
                seen.add(after_nodes)
                self.visits[camp.name] += 1
