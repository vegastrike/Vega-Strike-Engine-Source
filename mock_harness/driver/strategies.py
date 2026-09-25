"""Per mission-type flight strategies.

Each strategy inspects a running mission's Python object and returns the
next goal for the pilot, or None when the mission needs nothing from the
player right now.  Goals:

    ('system', name)              travel to a star system
    ('near', unit, dist, sig)     fly within ``dist`` of a unit
    ('dock', unit)                dock with a unit
    ('attack', [units])           destroy these units
    ('tractor', unit)             pick up an ejected pilot / cargo pod
    ('eject', cargo_name)         jettison all of a cargo
    ('wait', seconds)             loiter (fighting anything that attacks)
"""


def _live(u):
    return u is not None and not u.killed


def _unit(w):
    """Mission objects hold VS.Unit wrappers; get the engine unit (or None)."""
    if w is None:
        return None
    try:
        return w._get()
    except AttributeError:
        return None


def cls_names(obj):
    return [c.__name__ for c in type(obj).__mro__]


def adjsys_goal(adj, player):
    """Goal for a go_to_adjacent_systems / go_somewhere_significant helper."""
    if adj is None:
        return None
    names = cls_names(adj)
    if 'go_to_adjacent_systems' in names:
        if adj.arrivedsys:
            return None
        jumps = list(adj.jumps)
        if jumps:
            return ('system', jumps[0])
        return None
    if 'go_somewhere_significant' in names:
        if adj.arrivedarea:
            return None
        sig = _unit(adj.significantun)
        if _live(sig):
            if sig.system is not player.system:
                return ('system', sig.system.name)
            return ('near', sig, max(adj.distfrombase * 0.8, 1.0), True)
    return None


class Strategies:
    def __init__(self, scenario, log=print):
        self.scenario = scenario
        self.log = log

    def goal(self, eng, mission):
        obj = mission.pyobj
        if obj is None:
            return None
        names = cls_names(obj)
        p = eng.player
        for n in names:
            fn = getattr(self, 'g_' + n, None)
            if fn is not None:
                return fn(eng, obj, p)
        return self.g_generic(eng, obj, p)

    def protect(self, eng, mission):
        """Friendly units whose attackers the pilot should engage."""
        obj = mission.pyobj
        out = []
        for attr in ('defendee', 'escortee', 'ally', 'eject'):
            u = _unit(getattr(obj, attr, None))
            if _live(u) and u is not eng.player:
                out.append(u)
        return out

    # -- fallbacks ----------------------------------------------------------
    def g_generic(self, eng, m, p):
        return adjsys_goal(getattr(m, 'adjsys', None), p)

    def g_privateer(self, eng, m, p):
        return None

    # -- directions / ambush family -----------------------------------------
    def g_directions_mission(self, eng, m, p):
        names = cls_names(m)
        if 'ambush_scan' in names and self.scenario.choice('dump_contraband', False):
            carrying = any(c._content == m.cargotype and c._quantity > 0 for c in p.cargo)
            if m.systems and carrying:
                if p.system_file() != m.systems[0]:
                    return ('system', m.systems[0])
                return ('eject', m.cargotype)
        if not m.arrived:
            g = adjsys_goal(m.adjsys, p)
            if g is not None:
                return g
            if not m.destination and not m.jumps and 'patrol_ambush' in names:
                # no directions: the job is in the ambush system(s)
                if not m.inescapable:
                    systems = list(m.systems)
                    if systems and p.system_file() not in systems:
                        return ('system', systems[0])
                    return ('wait', 1.0)
            else:
                return None
        if 'patrol_ambush' in names and m.inescapable:
            pts = [_unit(x) for x in m.patrolpoints]
            pts = [x for x in pts if _live(x)]
            if pts:
                pts.sort(key=lambda u: p.significant_distance_to(u))
                return ('near', pts[0], m.distance * 0.8, True, 3.0 + 0.3 * len(pts))
        base = _unit(m.base)
        if m.destination and _live(base):
            if base.is_dockable():
                return ('dock', base)
            return ('near', base, 40.0, True)
        return None

    # -- patrols ----------------------------------------------------------
    def g_patrol(self, eng, m, p):
        g = adjsys_goal(m.adjsys, p)
        if g is not None:
            return g
        if m.quantity > 0:
            return ('wait', 1.0)
        pts = [_unit(x) for x in m.patrolpoints]
        live = [x for x in pts if _live(x)]
        if live:
            live.sort(key=lambda u: p.significant_distance_to(u))
            # patrol checks one point per frame: loiter so our point is seen
            return ('near', live[0], m.distance * 0.8, True, 3.0 + 0.3 * len(live))
        if pts:
            return ('wait', 1.0)
        active = getattr(m, 'activeships', None)
        if active:
            targets = [_unit(s[0]) for s in active]
            targets = [t for t in targets if _live(t)]
            if targets:
                return ('attack', targets)
            return ('wait', 1.0)
        return None

    # -- escorts ------------------------------------------------------------
    def g_escort_local(self, eng, m, p):
        if m.successdelay:
            todock = _unit(m.todock)
            if m.incoming and _live(todock):
                return ('near', todock, 250.0, False)
            return ('wait', 5.0)
        g = adjsys_goal(m.adjsys, p)
        if g is not None:
            return g
        att = [_unit(a) for a in m.attackers]
        att = [a for a in att if _live(a)]
        if att:
            return ('attack', att)
        return ('wait', 2.0)

    def g_escort_mission(self, eng, m, p):
        names = cls_names(m)
        esc = _unit(m.escortee)
        if _live(esc):
            following = esc.flightgroup is not None and esc.flightgroup.directive[:1] in ('F', 'f')
            if esc.system is not p.system:
                return ('system', esc.system.name)      # go back for our charge
            if not following or p.distance_to(esc) > 3000.0:
                if p.distance_to(esc) > 1500.0:
                    return ('near', esc, 500.0, False)
                return ('wait', 2.0)
        if 'wrong_escort' in names and self.scenario.choice('divert_escort', False):
            alt = list(getattr(m, 'alternatesystems', ()))
            if alt and p.system_file() != alt[0]:
                return ('system', alt[0])
        return adjsys_goal(m.adjsys, p)

    # -- defend -------------------------------------------------------------
    def g_defend(self, eng, m, p):
        if not m.arrived:
            return adjsys_goal(m.adjsys, p) or ('wait', 1.0)
        g = adjsys_goal(m.adjsys, p)
        if g is not None:
            return g
        att = [_unit(a) for a in m.attackers]
        att = [a for a in att if _live(a)]
        if att:
            return ('attack', att)
        return ('wait', 2.0)

    # -- bounties -----------------------------------------------------------
    def g_bounty(self, eng, m, p):
        if m.arrived == 0:
            return adjsys_goal(m.adjsys, p) or ('wait', 1.0)
        if m.arrived == 1:
            return adjsys_goal(m.adjsys, p) or ('wait', 1.0)
        enemy = _unit(m.enemy)
        if _live(enemy) and enemy.docked_to is None:
            return ('attack', [enemy])
        return ('wait', 1.0)

    # -- defend drone (end of the main campaign) ----------------------------
    def g_defend_drone(self, eng, m, p):
        if m.arrived == 0:
            if p.system_file() != m.helpsystem:
                return ('system', m.helpsystem)
            return ('wait', 1.0)
        if m.arrived == 1:
            if p.system_file() == m.helpsystem:
                return ('wait', 5.0)     # the Steltek jump us onwards
            return adjsys_goal(m.adjsys, p) or ('wait', 1.0)
        if m.arrived == 2:
            return ('wait', 1.0)
        import quest_drone
        drone = _unit(quest_drone.drone)
        if _live(drone):
            return ('attack', [drone])
        return ('wait', 1.0)

    # -- rescue -------------------------------------------------------------
    def g_rescue(self, eng, m, p):
        if m.arrived in (0, 1):
            # 0: travel; 1: get within 10km of a significant so the pod launches
            return adjsys_goal(m.adjsys, p) or ('wait', 1.0)
        if m.arrived == 2:
            ej = _unit(m.eject)
            if _live(ej):
                return ('tractor', ej)
            return ('wait', 1.0)
        sig = _unit(m.adjsys.SignificantUnit()) if m.adjsys else None
        if _live(sig):
            if sig.is_dockable():
                return ('dock', sig)
            return ('near', sig, 5.0, False)
        return None
