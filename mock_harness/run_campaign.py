#!/usr/bin/env python3
"""Play the Privateer campaigns end to end on the mock Vega Strike engine.

Example:
    python3 mock_harness/run_campaign.py --data Privateer --scenario full

Exit status is 0 when every phase of the scenario completed and no Python
errors were raised by the data pack, 1 otherwise.
"""

import argparse
import faulthandler
import signal
import os
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from mockvs.engine import Options                       # noqa: E402
from driver.campaign_driver import CampaignDriver       # noqa: E402
from driver.scenario import SCENARIOS                   # noqa: E402


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--data', default=os.path.join(os.path.dirname(HERE), 'Privateer'),
                    help='game data directory (default: ../Privateer)')
    ap.add_argument('--scenario', default='full', choices=sorted(SCENARIOS) + ['all'],
                    help='scenario to play; "all" runs every scenario in its own process')
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--dt', type=float, default=0.25, help='game seconds per frame')
    ap.add_argument('--player-hp', type=float, default=10000.0)
    ap.add_argument('--enemy-hp', type=float, default=20.0)
    ap.add_argument('--npc-hp', type=float, default=10000.0)
    ap.add_argument('--damage', type=float, default=1.0, help='damage per ship per turn')
    ap.add_argument('--player-damage', type=float, default=5.0)
    ap.add_argument('--fragile', action='append', default=[],
                    help='give ships of this type/name 1 hp (e.g. drayman.toth) to test failures')
    ap.add_argument('--no-division-check', action='store_true',
                    help='do not instrument "/" to find Python 2 integer division changes')
    ap.add_argument('--script-log', help='file receiving everything the data pack prints')
    ap.add_argument('--save-dir', help='directory for save games (default: temporary)')
    ap.add_argument('--report', help='write a text report here')
    ap.add_argument('--events', help='dump the engine event log here')
    ap.add_argument('--watchdog', type=float, default=0, help='abort with a stack dump after this many wall seconds')
    ap.add_argument('-v', '--verbose', type=int, default=1)
    args = ap.parse_args(argv)
    faulthandler.register(signal.SIGUSR1, all_threads=False)
    if args.watchdog:
        _wd = open('/tmp/claude-1000/-home-danielrh-dev-vegastrike/fd2eb83c-4504-4653-96a5-8985ca73512b/scratchpad/watchdog.txt' if os.environ.get('MOCKVS_WD') else os.devnull, 'w')
        faulthandler.dump_traceback_later(args.watchdog, repeat=True, file=_wd if os.environ.get('MOCKVS_WD') else sys.__stderr__)

    if args.scenario == 'all':
        return run_all(args, argv)
    scenario = SCENARIOS[args.scenario]
    save_dir = args.save_dir or tempfile.mkdtemp(prefix='mockvs_saves_')
    fragile = set(args.fragile)
    overrides = scenario.hp_overrides()
    if fragile:
        overrides.append((lambda u: u.name in fragile or u.fullname in fragile, 1.0))
    opts = Options(seed=args.seed, dt=args.dt, player_hp=args.player_hp, enemy_hp=args.enemy_hp,
                   npc_hp=args.npc_hp, damage=args.damage, player_damage=args.player_damage,
                   hp_overrides=overrides, verbose=args.verbose, save_dir=save_dir,
                   check_division=not args.no_division_check, **scenario.engine_options)
    script_log = open(args.script_log, 'w') if args.script_log else None
    t0 = time.time()
    drv = CampaignDriver(args.data, scenario, opts, script_log=script_log, verbose=args.verbose)
    ok = False
    try:
        ok = drv.run()
    except Exception:
        import traceback
        drv.log('DRIVER CRASH:\n' + traceback.format_exc())
    eng = drv.eng
    lines = []
    lines.append('scenario %s: %s in %.1fs wall, %.0f game seconds, %d frames'
                 % (scenario.name, 'SUCCESS' if ok else 'FAILED', time.time() - t0,
                    eng.game_time_total(), eng.frame))
    for name, pok, t in drv.report.phase_results:
        lines.append('  phase %-16s %s at t=%.0f' % (name, 'complete' if pok else 'NOT complete', t))
    lines.append('credits: %.0f  kills: %d' % (eng.credits, drv.pilot.kills))
    lines.append('python errors: %d distinct' % len(eng.errors))
    for e in eng.errors:
        lines.append('  ' + e.summary())
        lines.append('    ' + e.traceback.strip().splitlines()[-2].strip() if len(e.traceback.strip().splitlines()) > 1 else '')
    lines.append('warnings: %d distinct' % len(eng.warnings))
    for w, n in sorted(eng.warnings.items()):
        lines.append('  %s (x%d)' % (w, n))
    if not args.no_division_check:
        from mockvs import divcheck
        rep = divcheck.report(os.path.abspath(args.data))
        lines.append('python2-style int division changes observed: %d sites' % len(rep))
        for r in rep:
            lines.append('  ' + r)
    if args.events:
        with open(args.events, 'w') as fh:
            for ev in eng.events:
                fh.write('%8.1f %s\n' % (ev.get('time', 0), {k: v for k, v in ev.items() if k != 'time'}))
    text = '\n'.join(lines)
    drv.log(text)
    if args.report:
        with open(args.report, 'w') as fh:
            fh.write(text + '\n\n' + '\n'.join(drv.report.lines) + '\n')
    return 0 if ok and not eng.errors else 1


def run_all(args, argv):
    import subprocess
    base = [a for a in (argv if argv is not None else sys.argv[1:])]
    # drop "--scenario all" and per-run output files
    cleaned = []
    skip = False
    for a in base:
        if skip:
            skip = False
            continue
        if a in ('--scenario', '--report', '--events', '--script-log'):
            skip = True
            continue
        if a.startswith(('--scenario=', '--report=', '--events=', '--script-log=')):
            continue
        cleaned.append(a)
    results = []
    for name in sorted(SCENARIOS):
        t0 = time.time()
        proc = subprocess.run([sys.executable, os.path.abspath(__file__), '--scenario', name, '-v', '0'] + cleaned,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        summary = [l for l in proc.stdout.splitlines() if l.startswith(('scenario ', '  phase ', 'python errors'))]
        results.append((name, proc.returncode, time.time() - t0, summary))
        print('\n'.join(summary) or proc.stdout[-2000:])
        sys.stdout.flush()
    print('\n=== suite summary ===')
    for name, rc, dt, _ in results:
        print('  %-22s %s (%.0fs)' % (name, 'PASS' if rc == 0 else 'FAIL', dt))
    return 0 if all(rc == 0 for _, rc, _, _ in results) else 1


if __name__ == '__main__':
    sys.exit(main())
