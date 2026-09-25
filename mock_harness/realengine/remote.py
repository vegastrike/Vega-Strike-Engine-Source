"""Start the real engine with the remote-control agent and talk to it."""

import json
import re
import os
import shutil
import socket
import subprocess
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
AGENT = os.path.join(HERE, 'vs_remote.py')


_EXC_LINE = re.compile(r'^([A-Za-z_][\w.]*(Error|Exception|Exit|Interrupt)|SyntaxError)\b')


class RemoteError(Exception):
    pass


class EngineGone(RemoteError):
    pass


class Engine:
    """A running vegastrike process (under Xvfb unless ``display`` is given)."""

    def __init__(self, binary, datadir, workdir=None, port=None, display=None,
                 xvfb=True, extra_env=None):
        self.workdir = workdir or tempfile.mkdtemp(prefix='vs_real_')
        self.home = os.path.join(self.workdir, 'home')
        os.makedirs(self.home, exist_ok=True)
        self.port = port or _free_port()
        env = dict(os.environ)
        env.update({'VS_REMOTE_CONTROL': AGENT, 'VS_REMOTE_PORT': str(self.port),
                    'VS_HOMEDIR': self.home,
                    'PYTHONDONTWRITEBYTECODE': '1'})   # keep __pycache__ out of the data
        env.update(extra_env or {})
        cmd = [binary]
        if display:
            env['DISPLAY'] = display
        elif xvfb:
            cmd = ['xvfb-run', '-a', '-s', '-screen 0 1024x768x24'] + cmd
        self.stdout_path = os.path.join(self.workdir, 'stdout.txt')
        self.stderr_path = os.path.join(self.workdir, 'stderr.txt')
        self.proc = subprocess.Popen(cmd, cwd=datadir, env=env,
                                     stdout=open(self.stdout_path, 'w'),
                                     stderr=open(self.stderr_path, 'w'),
                                     start_new_session=True)
        self.sock = None
        self.buf = b''
        self.next_id = 0

    def alive(self):
        return self.proc.poll() is None

    def connect(self, timeout=180.0):
        t0 = time.time()
        while time.time() - t0 < timeout:
            if not self.alive():
                raise EngineGone('engine exited with %s before accepting connections' % self.proc.returncode)
            try:
                self.sock = socket.create_connection(('127.0.0.1', self.port), timeout=5.0)
                self.sock.settimeout(60.0)
                return
            except OSError:
                time.sleep(0.5)
        raise RemoteError('could not connect to the engine in %.0fs' % timeout)

    def _request(self, req):
        self.next_id += 1
        req['id'] = self.next_id
        try:
            self.sock.sendall(json.dumps(req).encode('utf-8') + b'\n')
            while b'\n' not in self.buf:
                data = self.sock.recv(1 << 20)
                if not data:
                    raise EngineGone('engine closed the connection (exit status %s)' % self.proc.poll())
                self.buf += data
        except socket.timeout:
            raise RemoteError('engine did not answer within 60s (hung? frame loop stopped?)')
        except OSError as e:
            raise EngineGone('connection lost: %r (exit status %s)' % (e, self.proc.poll()))
        line, self.buf = self.buf.split(b'\n', 1)
        rep = json.loads(line)
        if not rep.get('ok'):
            raise RemoteError(rep.get('error'))
        return rep.get('result')

    def eval(self, expr):
        return self._request({'eval': expr})

    def exec(self, code):
        return self._request({'exec': code})

    def wait_until(self, expr, timeout=60.0, poll=0.1):
        """Evaluate ``expr`` until it is truthy; returns its value."""
        t0 = time.time()
        while True:
            v = self.eval(expr)
            if v:
                return v
            if time.time() - t0 > timeout:
                raise RemoteError('timed out after %.0fs waiting for %s' % (timeout, expr))
            time.sleep(poll)

    def stop(self):
        if self.sock:
            try:
                self.sock.close()
            except OSError:
                pass
        if self.alive():
            try:
                os.killpg(self.proc.pid, 15)
            except OSError:
                pass
            try:
                self.proc.wait(10)
            except subprocess.TimeoutExpired:
                os.killpg(self.proc.pid, 9)

    def python_errors(self):
        """Python exceptions the engine printed (data pack errors).  Stack
        dumps of debug.warn() (no exception line) are not errors."""
        out = []
        for path in (self.stdout_path, self.stderr_path):
            with open(path, errors='replace') as fh:
                lines = fh.read().splitlines()
            i = 0
            while i < len(lines):
                if lines[i].startswith('Traceback (most recent call last)') or \
                        (lines[i].startswith('  File "') and (i == 0 or not lines[i - 1].startswith('  '))):
                    j = i + 1
                    while j < len(lines) and (lines[j].startswith(' ') or lines[j].startswith('\t')):
                        j += 1
                    if j < len(lines) and _EXC_LINE.match(lines[j]):
                        out.append('\n'.join(lines[i:j + 1]))
                    i = j + 1
                else:
                    i += 1
        return out


def _free_port():
    s = socket.socket()
    s.bind(('127.0.0.1', 0))
    port = s.getsockname()[1]
    s.close()
    return port
