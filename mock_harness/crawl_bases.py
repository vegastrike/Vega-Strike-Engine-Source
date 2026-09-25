#!/usr/bin/env python3
"""Dock at every base in Gemini and click every link/button in it.

    python3 mock_harness/crawl_bases.py [--systems Gemini/Troy,Gemini/Oxford]

Reports every Python exception raised by the data pack's base code.
"""
import argparse
import os
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from mockvs.engine import Options          # noqa: E402
from driver.crawler import Crawler         # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--data', default=os.path.join(os.path.dirname(HERE), 'Privateer'))
    ap.add_argument('--systems', help='comma separated list (default: all of Gemini)')
    ap.add_argument('--max-clicks', type=int, default=250, help='per base')
    ap.add_argument('--script-log')
    ap.add_argument('-v', '--verbose', type=int, default=1)
    args = ap.parse_args()
    opts = Options(verbose=args.verbose, save_dir=tempfile.mkdtemp(prefix='mockvs_saves_'),
                   check_division=True)
    script_log = open(args.script_log, 'w') if args.script_log else None
    systems = args.systems.split(',') if args.systems else None
    t0 = time.time()
    c = Crawler(args.data, opts, script_log=script_log, max_clicks=args.max_clicks,
                systems=systems, verbose=args.verbose)
    c.run()
    eng = c.eng
    print('crawled %d base visits (%d distinct scripts), %d clicks in %.1fs'
          % (sum(c.visited_files.values()), len(c.visited_files), c.clicks, time.time() - t0),
          file=c.game.real_stdout)
    print('python errors: %d distinct' % len(eng.errors), file=c.game.real_stdout)
    for e in eng.errors:
        lines = e.traceback.strip().splitlines()
        where = [l.strip() for l in lines if l.strip().startswith('File') and 'mock_harness' not in l]
        print('  %s\n      at %s' % (e.summary(), where[-1] if where else '?'), file=c.game.real_stdout)
    from mockvs import divcheck
    rep = divcheck.report(os.path.abspath(args.data))
    print('python2-style int division changes observed: %d sites' % len(rep), file=c.game.real_stdout)
    for r in rep:
        print('  ' + r, file=c.game.real_stdout)
    return 0 if not eng.errors else 1


if __name__ == '__main__':
    sys.exit(main())
