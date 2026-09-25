"""Deliberately simple ship behaviour for the mock engine.

Ships move in straight lines at their governor speed and fight in
"turns": once per game second every armed ship within weapon range of its
combat target deals ``damage`` hit points.  That is all the fidelity the
campaign scripts need - they only look at who is alive, where, and whom
they are targeting.
"""

import math


def _dist_pos(a, b):
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3)))


def move_towards(u, pos, speed, dt, stop_at=0.0):
    d = _dist_pos(u.position, pos)
    if d <= stop_at or d == 0:
        u.velocity = [0.0, 0.0, 0.0]
        return True
    step = min(speed * dt, d - stop_at)
    for i in range(3):
        v = (pos[i] - u.position[i]) / d
        u.position[i] += v * step
        u.velocity[i] = v * speed
    return _dist_pos(u.position, pos) <= stop_at + 1e-6


def _alive_here(u, t):
    return t is not None and not t.killed and t.system is u.system and t.docked_to is None


def choose_target(eng, u):
    """Pick the unit ``u`` wants to shoot at, or None."""
    directive = u.flightgroup.directive if u.flightgroup else 'b'
    t = u.target
    if _alive_here(u, t) and not t.is_planet():
        if directive[:1] in ('A', 'B') or eng.unit_relation(u, t) < 0:
            return t
    # retaliate / find the nearest enemy
    best = None
    bestd = eng.opts.detect_range
    for o in u.system.units:
        if o is u or o.killed or o.docked_to is not None:
            continue
        if o.is_planet() or o.kind in ('cargo', 'nebula', 'asteroid', 'enhancement'):
            continue
        d = u.distance_to(o)
        if d < bestd and eng.unit_relation(u, o) < 0:
            best, bestd = o, d
    return best


def update(eng, dt):
    plr = eng.player
    if plr is None or plr.system is None:
        return
    systems = [plr.system]
    for s in systems:
        for u in list(s.units):
            if u.killed or u.docked_to is not None:
                continue
            if u is plr:
                _move_player(eng, u, dt)
                continue
            if u.max_speed <= 0 or u.is_planet() or u.kind in ('cargo', 'nebula', 'asteroid'):
                continue
            _move_npc(eng, u, dt)


def _speed(u):
    if u.set_speed is not None and u.set_speed >= 0:
        return min(max(u.set_speed, 0.0), max(u.max_ab_speed, u.max_speed))
    return u.max_speed


def _move_player(eng, u, dt):
    mv = u.moveto
    if mv is None:
        u.velocity = [0.0, 0.0, 0.0]
        return
    kind, what, speed, stop = mv
    if kind == 'unit':
        if what.killed or what.system is not u.system:
            u.moveto = None
            return
        pos = what.position
        stop = stop + u.radius + what.radius
    else:
        pos = what
    if move_towards(u, pos, speed, dt, stop):
        u.moveto = None


def _move_npc(eng, u, dt):
    speed = _speed(u)
    if speed <= 0:
        return
    # 1. docking orders
    if u.dock_order is not None:
        base, actually = u.dock_order
        if base.killed or base.system is not u.system:
            u.dock_order = None
        else:
            if move_towards(u, base.position, speed, dt, base.radius + u.radius + 50.0) or \
                    eng.dock_distance_ok(u, base):
                u.dock_order = None
                if actually and base.is_dockable():
                    eng.dock(u, base, check_distance=False, load_interface=False)
                else:
                    u.velocity = [0.0, 0.0, 0.0]
            return
    # 2. jumping out
    t = u.target
    if u.jump_active and t is not None and not t.killed and t.system is u.system and t.is_jumppoint():
        if move_towards(u, t.position, speed, dt, t.radius):
            eng.pending_jumps.append((u, t.destinations[0], t))
        return
    # 3. combat
    ct = getattr(u, 'combat_target', None)
    if ct is not None and _alive_here(u, ct) and u.damage > 0:
        move_towards(u, ct.position, speed, dt, 0.6 * u.weapon_range + u.radius + ct.radius)
        return
    # 4. formation
    fg = u.flightgroup
    if fg is not None and fg.directive[:1] in ('F', 'f') and fg.leader is not None and \
            fg.leader is not u and not fg.leader.killed:
        lead = fg.leader
        if lead.system is u.system and lead.docked_to is None:
            offset = 150.0 + 60.0 * (u.fg_subnumber % 5)
            follow_speed = max(speed, _dist_pos(u.position, lead.position) / max(dt, 1e-3))
            move_towards(u, lead.position, follow_speed, dt, offset)
        return
    # 5. explicit move orders (AutoPilotTo etc)
    if u.moveto is not None:
        kind, what = u.moveto[0], u.moveto[1]
        pos = what.position if kind == 'unit' else what
        if move_towards(u, pos, speed, dt, 100.0):
            u.moveto = None
        return
    # 6. drift towards an explicit (non hostile) target so escorts etc. make progress
    if t is not None and _alive_here(u, t) and not t.is_planet() and u.damage == 0:
        return
    u.velocity = [0.0, 0.0, 0.0]


def combat_turn(eng):
    plr = eng.player
    if plr is None or plr.system is None:
        return
    s = plr.system
    units = [u for u in s.units if not u.killed and u.docked_to is None]
    # choose targets first so fights resolve simultaneously
    for u in units:
        if u is plr:
            # the player only fires at what the pilot (the "human") chose,
            # not at whatever a script passed to SetTarget
            t = getattr(u, 'fire_target', None)
            u.combat_target = t if _alive_here(u, t) else None
            continue
        if u.damage <= 0 or u.is_planet() or u.kind in ('cargo', 'nebula', 'asteroid', 'enhancement', 'eject', 'base'):
            u.combat_target = None
            continue
        u.combat_target = choose_target(eng, u)
    hits = []
    for u in units:
        t = getattr(u, 'combat_target', None)
        if t is None or u.damage <= 0:
            continue
        if not _alive_here(u, t):
            continue
        if u.distance_to(t) <= u.weapon_range:
            hits.append((t, u.damage, u))
    for t, dmg, dealer in hits:
        if not t.killed:
            _maybe_villain(eng, dealer, t)
            eng.damage_unit(t, dmg, dealer)


def _maybe_villain(eng, dealer, target):
    """Anyone shooting at the player or at a mission-relevant ship is a bad
    guy for the purposes of the simple hit point model."""
    if target.kind == 'base' or dealer.kind == 'base':
        return      # stations are scenery in the mock: never demoted
    if dealer is eng.player:
        if not target.hostile_spawn and not target.is_planet():
            target.hostile_spawn = True
            if target.hull > eng.opts.enemy_hp:
                target.hull = target.max_hull = eng.opts.enemy_hp
            eng.event('villain', unit=repr(target), target='player shot it')
        return
    if dealer.hostile_spawn or dealer.mission_relevant:
        return
    if target is eng.player or target.mission_relevant:
        dealer.hostile_spawn = True
        if dealer.hull > eng.opts.enemy_hp:
            dealer.hull = dealer.max_hull = eng.opts.enemy_hp
        eng.event('villain', unit=repr(dealer), target=repr(target))
