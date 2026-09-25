#!/usr/bin/env python3
"""Play real missions end to end in the real engine through the
remote-control interface (see README.md).

    python3 mock_harness/realengine/run_real_missions.py --binary build/vegastrike [--mission cargo]

The engine runs under Xvfb with its own home directory, so the user's
saves are never touched.  Exit status 0 means every step worked and the
data pack raised no Python exception.
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


def new_game(s):
    s.wait_base(need='New Game', timeout=240)
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


def launch(s):
    s.goto_link(kind='launch')
    s.click(kind='launch')
    s.wait_space()
    time.sleep(1.0)
    log('launched in %s' % s.system())


def destination_base(s):
    """The base the mission wants: the dockable unit named in an objective
    or a recent message."""
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
    text = offers_until(s, r'CARGO MISSION')
    if not text:
        report['result'] = 'no cargo mission offered'
        return False
    dest = None
    for m in re.finditer(r'((?:[A-Z][\w-]*\s+){0,3})([A-Z][\w-]*) system', text):
        words = m.group(1).split() + [m.group(2)]
        for k in range(len(words)):          # longest name first
            dest = full_system_name(s, ' '.join(words[k:]))
            if dest:
                break
        if dest:
            break
    if not dest:
        report['result'] = 'could not parse destination from %r' % text
        return False
    credits0 = s.e.eval('VS.getPlayer().getCredits()')
    s.click('Accept this Mission')
    time.sleep(1.0)
    cargo = s.e.eval('player_cargo()')
    mission_cargo = [c for c in cargo if c[3]]
    log('accepted; mission cargo %s; objectives %s' % (mission_cargo, s.e.eval('VSRemote.GetObjectives()')))
    report['accepted'] = text
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


MISSIONS = {'cargo': cargo_mission}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--binary', required=True, help='vegastrike executable built with remote control')
    ap.add_argument('--data', default=os.path.join(os.path.dirname(os.path.dirname(HERE)), 'Privateer'))
    ap.add_argument('--mission', default='cargo', choices=sorted(MISSIONS))
    ap.add_argument('--workdir', help='keep logs/home here (default: a temp dir)')
    ap.add_argument('--display', help='use this X display instead of Xvfb (to watch)')
    args = ap.parse_args()
    workdir = args.workdir or tempfile.mkdtemp(prefix='vs_real_')
    e = Engine(os.path.abspath(args.binary), args.data, workdir=workdir, display=args.display)
    s = Session(e, log=log)
    report = {}
    ok = False
    try:
        e.connect()
        ok = MISSIONS[args.mission](s, report)
    except EngineGone as ex:
        report['result'] = 'ENGINE DIED: %s' % ex
    except RemoteError as ex:
        report['result'] = 'stuck: %s' % ex
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
