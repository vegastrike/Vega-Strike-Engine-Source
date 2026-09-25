#!/usr/bin/env python3
"""Play real missions end to end in the real engine through the
remote-control interface (see README.md).

    python3 mock_harness/realengine/run_real_missions.py --binary build/vegastrike [--mission cargo]

The engine runs under Xvfb with its own home directory, so the user's
saves are never touched.  Exit status 0 means the mission reached an
outcome (completed, or lost because the player was shot down) and the data
pack raised no Python exception; being stuck, an engine crash or a Python
exception is a failure.
"""

import argparse
import os
import re
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from remote import Engine, RemoteError, EngineGone   # noqa: E402
from session import Session                           # noqa: E402


def log(msg):
    print('[%7.1fs] %s' % (time.time() - T0, msg), flush=True)


T0 = time.time()


SEED = None


def new_game(s):
    s.wait_base(need='New Game', timeout=240)
    if SEED is not None:
        # the data pack's random numbers (mission offers, encounters...)
        s.e.exec('import vsrandom, random\nvsrandom.seed(%d)\nrandom.seed(%d)' % (SEED, SEED))
    s.click('New Game')
    s.wait_base(need='Launch', timeout=240)
    log('new game: docked in %s, %s credits' % (s.system(), s.e.eval('VS.getPlayer().getCredits()')))


def ensure_jump_drive(s, report):
    """tarsus.begin has no jump drive.  Top up the credits (a test cheat)
    and buy one through the upgrade room GUI, like a player would."""
    if s.e.eval('VS.getPlayer().GetJumpStatus()') != -2:
        return True
    credits = s.e.eval('VS.getPlayer().getCredits()')
    if credits < 10000:
        s.e.eval('VS.getPlayer().addCredits(%f)' % (10000 - credits))
        report.setdefault('cheats', []).append('added %.0f credits for the jump drive' % (10000 - credits))
    msg = s.buy_upgrade('Jump_Drive')
    status = s.e.eval('VS.getPlayer().GetJumpStatus()')
    log('bought jump drive: %r, jump status now %s, credits %s'
        % (msg, status, s.e.eval('VS.getPlayer().getCredits()')))
    return status != -2


def offers_until(s, pattern, max_offers=40):
    """Page through the mission computer until an offer matches ``pattern``."""
    if not s.goto_link('Mission_Computer'):
        return None
    s.click('Mission_Computer')
    s.click('Activate_Computer')
    seen = []
    for _ in range(max_offers):
        if not s.find_link('View next Mission Description'):
            break
        s.click('View next Mission Description')
        text = s.texts().get('miscompbox', '')
        head = text.split('\n\n', 1)[-1]
        if head in seen:
            break
        seen.append(head)
        log('offer: %s' % head.replace('\n', ' ')[:150])
        if re.search(pattern, text):
            return text
    return None


def full_system_name(s, short):
    """'New Detroit' -> 'Gemini/New_Detroit' (systems reachable by jumps)."""
    key = short.replace(' ', '_').lower()
    for name in s.all_systems():
        if name.split('/')[-1].lower() == key:
            return name
    return None


PLAYER_SHIP = []


def launch(s):
    s.goto_link(kind='launch')
    s.click(kind='launch')
    s.wait_space()
    if not PLAYER_SHIP:
        PLAYER_SHIP.append(s.e.eval('VS.getPlayer().getName()'))
    time.sleep(1.0)
    log('launched in %s' % s.system())


def destination_base(s):
    """The base the mission wants: the unit of a 'Visit <name>' objective
    (go_somewhere_significant), else a dockable unit named in a message."""
    for o in s.e.eval('VSRemote.GetObjectives()'):
        if o[1].startswith('Visit ') and o[2] < 1.0:
            name = o[1][len('Visit '):].strip()
            for cut in (name, ' '.join(name.split(' ')[:2]), name.split(' ')[0]):
                if s.e.eval('find_by_display_name(%r) is not None' % cut):
                    return ({'name': cut}, 'find_by_display_name(%r)' % cut)
    words = ' '.join([o[1] for o in s.e.eval('VSRemote.GetObjectives()')] +
                     [m[3] for m in s.e.eval('VSRemote.GetMessages(20)')])
    best = None
    for u in s.e.eval('system_units()'):
        if not u['dockable'] or u['jump']:
            continue
        for nm in (u['fullname'], u['name']):
            if nm and len(nm) > 2 and nm.replace('_', ' ').lower() in words.replace('_', ' ').lower():
                if best is None or len(nm) > len(best[1]):
                    best = (u, nm)
    return best


def cargo_mission(s, report):
    new_game(s)
    if not ensure_jump_drive(s, report):
        report['result'] = 'could not buy a jump drive'
        return False
    credits0 = s.e.eval('VS.getPlayer().getCredits()')
    dest = accept_offer(s, report, r'CARGO MISSION')
    if not dest:
        return False
    mission_cargo = [c for c in s.e.eval('player_cargo()') if c[3]]
    log('mission cargo %s' % (mission_cargo,))
    if not mission_cargo:
        report['result'] = 'accepted but no mission cargo was loaded'
        return False
    log('destination system %s' % dest)
    launch(s)
    s.goto_system(dest)
    log('arrived in %s; objectives %s' % (s.system(), s.e.eval('VSRemote.GetObjectives()')))
    t0 = time.time()
    base = None
    while time.time() - t0 < 30 and base is None:
        base = destination_base(s)
        time.sleep(1.0)
    if base is None:
        report['result'] = 'no destination base named in objectives/messages: %s' % (
            s.e.eval('VSRemote.GetMessages(10)'),)
        return False
    log('destination base %s' % (base[1],))
    s.dock(base[1])
    time.sleep(2.0)
    credits1 = s.e.eval('VS.getPlayer().getCredits()')
    left = [c for c in s.e.eval('player_cargo()') if c[3]]
    log('docked at %s; credits %s -> %s; mission cargo left %s' % (base[1], credits0, credits1, left))
    for msg in s.e.eval('VSRemote.GetMessages(6)'):
        log('  message: %s: %s' % (msg[1], msg[3]))
    report['credits'] = (credits0, credits1)
    ok = credits1 > credits0 and not left
    report['result'] = 'delivered and paid' if ok else 'docked but not paid/cargo still aboard'
    return ok


def accept_offer(s, report, pattern, max_bases=4):
    """Accept the first offer matching ``pattern`` (trying the other bases of
    the system when this one has none); returns its destination system."""
    text = offers_until(s, pattern)
    tried = set()
    def by_distance():
        me = s.e.eval('VS.getPlayer().Position()')
        bases = [u for u in s.e.eval('system_units()') if u['dockable'] and not u['jump']]
        return sorted(bases, key=lambda u: sum((a - b) ** 2 for a, b in zip(u['pos'], me)))
    while not text and len(tried) < max_bases:
        launch(s)
        bases = by_distance()
        if bases and not tried:
            tried.add(bases[0]['name'])       # the base we just left
        others = [u for u in bases if u['name'] not in tried]
        if not others:
            break
        other = others[0]
        tried.add(other['name'])
        log('no offer here; trying %s' % (other['fullname'] or other['name']))
        s.dock(other['name'])
        text = offers_until(s, pattern)
    if not text:
        report['result'] = 'no offer matching %r' % pattern
        return None
    dest = None
    for m in re.finditer(r'((?:[A-Z][\w-]*\s+){0,3})([A-Z][\w-]*) system', text):
        words = m.group(1).split() + [m.group(2)]
        for k in range(len(words)):
            dest = full_system_name(s, ' '.join(words[k:]))
            if dest:
                break
        if dest:
            break
    if not dest:
        report['result'] = 'could not parse destination from %r' % text
        return None
    s.click('Accept this Mission')
    time.sleep(1.0)
    report['accepted'] = text
    log('accepted; objectives %s' % s.e.eval('VSRemote.GetObjectives()'))
    return dest


class PlayerDestroyed(Exception):
    pass


def check_alive(s, ship):
    """After the player's ship is destroyed the cockpit gets a new unit
    (an ejected pilot / dumbfire), so the ship name changes."""
    if s.e.eval('VS.getPlayer().getName()') != ship:
        raise PlayerDestroyed('player ship destroyed (%s)' % s.e.eval('VSRemote.GetMessages(1)'))


def keep_alive(s, report, hull0):
    """Test cheat: top the hull up so a lost fight does not end the run."""
    hull = s.e.eval('VS.getPlayer().GetHull()')
    if hull < hull0 * 0.5:
        s.e.eval('VS.getPlayer().SetHull(%f)' % hull0)
        report.setdefault('cheats', []).append('hull topped up from %.1f' % hull)


def patrol_mission(s, report):
    new_game(s)
    if not ensure_jump_drive(s, report):
        report['result'] = 'could not buy a jump drive'
        return False
    credits0 = s.e.eval('VS.getPlayer().getCredits()')
    # ATTACK missions are patrols of nav points with enemies (patrol_enemies)
    dest = accept_offer(s, report, r'(?s)PATROL MISSION|ATTACK MISSION:.*nav point')
    if not dest:
        return False
    log('destination system %s' % dest)
    launch(s)
    hull0 = s.e.eval('VS.getPlayer().GetHull()')
    ship = s.e.eval('VS.getPlayer().getName()')
    s.goto_system(dest)
    log('arrived in %s; objectives %s' % (s.system(), s.e.eval('VSRemote.GetObjectives()')))
    t0 = time.time()
    attempts = {}
    while time.time() - t0 < 300:
        if s.in_base():
            # got too close to a planet: automatic landing zone
            log('  landed by accident, launching again')
            launch(s)
        check_alive(s, ship)
        keep_alive(s, report, hull0)
        if s.e.eval('VS.getPlayer().getCredits()') > credits0:
            break
        todo = [o[1] for o in s.e.eval('VSRemote.GetObjectives()')
                if o[1].startswith('Scan ') and o[2] < 1.0]
        target = None
        for text in sorted(todo, key=lambda t: attempts.get(t, 0)):
            for prefix in ('Scan Jumppoint ', 'Scan Natural Phenomenon: ', 'Scan '):
                if text.startswith(prefix):
                    name = text[len(prefix):]
                    if s.e.eval('find_by_display_name(%r) is not None' % name):
                        target = name
                    break
            if target:
                break
        if target is None:
            time.sleep(1.0)
            continue
        n = attempts.get(target, 0)
        attempts[target] = n + 1
        if n == 0:
            log('  scanning %s' % target)
        # a name can belong to several units (base and planet): try each;
        # planets have an automatic landing zone, scan them from farther out
        s.e.exec('u = find_by_display_name(%r, %d)\nVS.getPlayer().SetTarget(u)\n'
                 'teleport_near(u, 300.0 if u.isPlanet() and not u.isJumppoint() else 100.0)' % (target, n))
        time.sleep(2.0)
    credits1 = s.e.eval('VS.getPlayer().getCredits()')
    for msg in s.e.eval('VSRemote.GetMessages(6)'):
        log('  message: %s: %s' % (msg[1], msg[3]))
    report['credits'] = (credits0, credits1)
    ok = credits1 > credits0
    report['result'] = ('patrol complete, paid %.0f' % (credits1 - credits0)) if ok else \
        'patrol not completed; objectives %s' % s.e.eval('VSRemote.GetObjectives()')
    return ok


MISSIONS = {'cargo': cargo_mission, 'patrol': patrol_mission}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--binary', required=True, help='vegastrike executable built with remote control')
    ap.add_argument('--data', default=os.path.join(os.path.dirname(os.path.dirname(HERE)), 'Privateer'))
    ap.add_argument('--mission', default='cargo', choices=sorted(MISSIONS))
    ap.add_argument('--workdir', help='keep logs/home here (default: a temp dir)')
    ap.add_argument('--display', help='use this X display instead of Xvfb (to watch)')
    ap.add_argument('--seed', type=int, help='seed the data pack random numbers (mission offers...)')
    args = ap.parse_args()
    global SEED
    SEED = args.seed
    workdir = args.workdir or tempfile.mkdtemp(prefix='vs_real_')
    e = Engine(os.path.abspath(args.binary), args.data, workdir=workdir, display=args.display)
    s = Session(e, log=log)
    report = {}
    ok = False
    try:
        e.connect()
        ok = MISSIONS[args.mission](s, report)
    except PlayerDestroyed as ex:
        # a legitimate way to lose a mission: the engine and scripts worked
        report['result'] = 'lost: %s' % ex
        ok = True
    except EngineGone as ex:
        report['result'] = 'ENGINE DIED: %s' % ex
    except RemoteError as ex:
        report['result'] = 'stuck: %s' % ex
        try:
            if PLAYER_SHIP and e.eval('VS.getPlayer().getName()') != PLAYER_SHIP[0]:
                report['result'] = 'lost: player ship destroyed while: %s' % ex
                ok = True
        except RemoteError:
            pass
    finally:
        e.stop()
    errors = e.python_errors()
    log('mission %s: %s' % (args.mission, report.get('result')))
    log('python errors in engine output: %d' % len(errors))
    for err in errors:
        print(err)
    log('logs in %s' % workdir)
    return 0 if ok and not errors else 1


if __name__ == '__main__':
    sys.exit(main())
