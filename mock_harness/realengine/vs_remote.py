"""Runs inside the real engine (VS_REMOTE_CONTROL=<this file>).

The engine calls tick(docked) at the start of every frame.  tick() serves a
tiny line based JSON protocol on 127.0.0.1:$VS_REMOTE_PORT so an external
test driver can look at the game and act on it:

    request:  {"id": 1, "eval": "VS.getPlayer().getName()"}
              {"id": 2, "exec": "x = 3"}        (statements; '_' is returned)
    reply:    {"id": 1, "ok": true, "result": ..., "frame": 123}
              {"id": 2, "ok": false, "error": "<traceback>"}

Code runs in a namespace with VS, Base, Director, VSRemote and the helpers
below.  Everything a request does happens inside one engine frame.
"""

import json
import os
import socket
import sys
import traceback

import VS
import VSRemote
try:
    import Base
except ImportError:          # not in every build
    Base = None
import Director

PORT = int(os.environ.get('VS_REMOTE_PORT', '47017'))

_server = None
_clients = []                # [socket, bytearray]
frame = 0
docked = False
_ns = {}


def _jsonable(x):
    try:
        json.dumps(x)
        return x
    except (TypeError, ValueError):
        if isinstance(x, (list, tuple)):
            return [_jsonable(i) for i in x]
        if isinstance(x, dict):
            return {str(k): _jsonable(v) for k, v in x.items()}
        return repr(x)


# ---- helpers available to requests ---------------------------------------

def unit_info(u):
    if u is None or u.isNull():
        return None
    p = u.Position()
    return {'name': u.getName(), 'fullname': u.getFullname(), 'faction': u.getFactionName(),
            'pos': [p[0], p[1], p[2]], 'hull': u.GetHull(), 'dockable': bool(u.isDockableUnit()),
            'jump': bool(u.isJumppoint()), 'planet': bool(u.isPlanet()), 'radius': u.rSize()}


def system_units():
    out = []
    i = VS.getUnitList()
    while i.notDone():
        u = i.current()
        if u and not u.isNull():
            info = unit_info(u)
            info['i'] = len(out)
            out.append(info)
        i.advance()
    return out


def find_unit(name):
    """First unit whose name or fullname is ``name``."""
    i = VS.getUnitList()
    while i.notDone():
        u = i.current()
        if u and not u.isNull() and name in (u.getName(), u.getFullname()):
            return u
        i.advance()
    return None


def find_all_by_display_name(text):
    """Units that the data pack's unit.getUnitFullName() calls ``text`` (the
    name missions put in objectives such as 'Scan <name>'); a base and its
    planet can share one."""
    import unit as vsunit
    out = []
    i = VS.getUnitList()
    while i.notDone():
        u = i.current()
        if u and not u.isNull():
            try:
                if vsunit.getUnitFullName(u) == text or vsunit.getUnitFullName(u, True) == text:
                    out.append(u)
            except Exception:
                pass
        i.advance()
    return out


def find_by_display_name(text, which=0):
    units = find_all_by_display_name(text)
    return units[which % len(units)] if units else None


def player_cargo():
    p = VS.getPlayer()
    out = []
    for k in range(p.numCargo()):
        c = p.GetCargoIndex(k)
        out.append((c.GetContent(), c.GetCategory(), c.GetQuantity(), c.GetMissionFlag()))
    return out


def state():
    p = VS.getPlayer()
    st = {'frame': frame, 'docked': docked, 'in_base': VSRemote.InBase(),
          'time': VS.GetGameTime(), 'system': VS.getSystemFile(),
          'credits': VS.getPlayer().getCredits() if p else None,
          'player': unit_info(p) if p else None,
          'objectives': VSRemote.GetObjectives(),
          'messages': VSRemote.GetMessages(8)}
    if st['in_base'] and Base is not None:
        st['room'] = Base.GetCurRoom()
        st['links'] = VSRemote.GetLinks()
        st['texts'] = VSRemote.GetTexts()
    return st


def teleport_near(u, dist):
    """Put the player ``dist`` beyond ``u``'s surface, at rest."""
    p = VS.getPlayer()
    t = u.Position()
    r = u.rSize() + p.rSize() + dist
    q = p.Position()
    d = [q[0] - t[0], q[1] - t[1], q[2] - t[2]]
    n = (d[0] ** 2 + d[1] ** 2 + d[2] ** 2) ** .5 or 1.0
    p.SetCurPosition((t[0] + d[0] / n * r, t[1] + d[1] / n * r, t[2] + d[2] / n * r))
    p.SetVelocity((0.0, 0.0, 0.0))


# ---- server -----------------------------------------------------------------

def _setup():
    global _server
    _server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    _server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    _server.bind(('127.0.0.1', PORT))
    _server.listen(4)
    _server.setblocking(False)
    _ns.update({'VS': VS, 'Base': Base, 'Director': Director, 'VSRemote': VSRemote,
                'unit_info': unit_info, 'system_units': system_units, 'find_unit': find_unit,
                'player_cargo': player_cargo, 'state': state, 'teleport_near': teleport_near,
                'find_by_display_name': find_by_display_name,
                'json': json, 'sys': sys})
    sys.stderr.write('vs_remote: listening on 127.0.0.1:%d\n' % PORT)


def _handle(line):
    try:
        req = json.loads(line)
    except ValueError as e:
        return {'ok': False, 'error': 'bad request: %s' % e}
    rid = req.get('id')
    _ns['frame'] = frame
    try:
        if 'eval' in req:
            result = eval(req['eval'], _ns)
        else:
            _ns['_'] = None
            exec(req['exec'], _ns)
            result = _ns.get('_')
        return {'id': rid, 'ok': True, 'result': _jsonable(result), 'frame': frame}
    except BaseException:
        return {'id': rid, 'ok': False, 'error': traceback.format_exc(), 'frame': frame}


def tick(dock):
    global frame, docked
    frame += 1
    docked = bool(dock)
    if _server is None:
        _setup()
    while True:
        try:
            conn, _addr = _server.accept()
        except (BlockingIOError, InterruptedError):
            break
        conn.setblocking(False)
        _clients.append([conn, bytearray()])
    for c in list(_clients):
        conn, buf = c
        try:
            while True:
                data = conn.recv(65536)
                if not data:
                    raise ConnectionError('closed')
                buf.extend(data)
        except (BlockingIOError, InterruptedError):
            pass
        except OSError:
            _clients.remove(c)
            conn.close()
            continue
        while b'\n' in buf:
            i = buf.index(b'\n')
            line = bytes(buf[:i]).decode('utf-8')
            del buf[:i + 1]
            reply = json.dumps(_handle(line)).encode('utf-8') + b'\n'
            try:
                conn.setblocking(True)
                conn.sendall(reply)
                conn.setblocking(False)
            except OSError:
                if c in _clients:
                    _clients.remove(c)
                conn.close()
                break
