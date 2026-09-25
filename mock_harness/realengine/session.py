"""High level actions on a remote-controlled engine (the "hands")."""

import re
import time

from remote import RemoteError


class Session:
    def __init__(self, engine, log=print):
        self.e = engine
        self.log = log

    # ---- base ---------------------------------------------------------------
    def in_base(self):
        return self.e.eval('VSRemote.InBase()')

    def links(self):
        return self.e.eval("[{k: l[k] for k in ('num','index','text','kind','room')} for l in VSRemote.GetLinks()]")

    def texts(self):
        return dict((k, v) for k, v in self.e.eval('VSRemote.GetTexts()'))

    def room(self):
        return self.e.eval('Base.GetCurRoom()')

    def find_link(self, text=None, index=None, kind=None):
        for l in self.links():
            name = l['text'][3:] if l['text'].startswith('XXX') else l['text']
            if text is not None and text not in (l['text'], name):
                continue
            if index is not None and l['index'] != index:
                continue
            if kind is not None and l['kind'] != kind:
                continue
            return l
        return None

    def click(self, text=None, index=None, kind=None, settle=0.3):
        l = self.find_link(text, index, kind)
        if l is None:
            raise RemoteError('no link text=%r index=%r kind=%r in room %s: %s'
                              % (text, index, kind, self.room(),
                                 [(x['index'], x['text']) for x in self.links()]))
        self.log('  click %r (%s) in room %s' % (l['text'], l['index'], self.room()))
        self.e.eval('VSRemote.ClickLinkNum(%d)' % l['num'])
        time.sleep(settle)
        # loading rooms (e.g. XXXCommodity_Exchange_Loading) have no links
        # and move on by themselves after a moment
        t0 = time.time()
        while time.time() - t0 < 10 and self.in_base() and not self.links():
            time.sleep(0.2)
        return l

    def wait_base(self, timeout=120.0, need=None):
        """Wait until docked in a base (optionally one with link text ``need``)."""
        t0 = time.time()
        while time.time() - t0 < timeout:
            if self.in_base() and (need is None or self.find_link(need)):
                return True
            time.sleep(0.3)
        raise RemoteError('no base (need=%r) after %.0fs' % (need, timeout))

    def wait_space(self, timeout=60.0):
        return self.e.wait_until('not VSRemote.InBase()', timeout)

    def goto_link(self, text=None, kind=None, max_clicks=40):
        """Walk (by clicking goto links) to a room of this base that has a
        link with this text/kind, mapping rooms as they are visited."""
        rooms = {}                     # room -> [links]
        for _ in range(max_clicks):
            cur = self.room()
            rooms[cur] = self.links()
            if self.find_link(text, kind=kind):
                return True
            # BFS over the known goto edges to the nearest unexplored room
            prev = {cur: None}
            todo = [cur]
            goal = None
            while todo and goal is None:
                r = todo.pop(0)
                for l in rooms.get(r, ()):
                    if l['kind'] != 'goto' or l['room'] in prev:
                        continue
                    prev[l['room']] = (r, l)
                    if l['room'] not in rooms:
                        goal = l['room']
                        break
                    todo.append(l['room'])
            if goal is None:
                return False
            step = goal
            while prev[step][0] != cur:
                step = prev[step][0]
            self.click(prev[step][1]['text'], kind='goto')
            if not self.in_base():
                return False
            if self.room() != step:
                # a loading room that moved on by itself: nothing to see there
                rooms.setdefault(step, [])
        return False

    def buy_upgrade(self, item, max_items=80):
        """Buy ``item`` (as the GUI names it, e.g. 'Jump_Drive') in the
        Repair/Upgrade room.  Returns the GUI's message."""
        if not self.goto_link('Buy Mode'):
            raise RemoteError('no upgrade room found in this base')
        self.click('Buy Mode')
        for _ in range(max_items):
            if self.texts().get('txt_name') == item:
                self.click('Select Items')
                time.sleep(0.5)
                return self.texts().get('txt_message')
            self.click('Next Item', settle=0.1)
        raise RemoteError('%s not offered in the upgrade room' % item)

    # ---- space ----------------------------------------------------------------
    def system(self):
        return self.e.eval('VS.getSystemFile()')

    def adjacent(self, sysname):
        return self.e.eval('[VS.GetAdjacentSystem(%r, i) for i in range(VS.GetNumAdjacentSystems(%r))]'
                           % (sysname, sysname))

    def all_systems(self):
        """Every system reachable by jumps from here (cached)."""
        if getattr(self, '_graph', None) is None:
            graph, todo = {}, [self.system()]
            while todo:
                cur = todo.pop()
                if cur in graph:
                    continue
                graph[cur] = self.adjacent(cur)
                todo.extend(n for n in graph[cur] if n not in graph)
            self._graph = graph
        return self._graph

    def route(self, dest):
        start = self.system()
        prev = {start: None}
        todo = [start]
        graph = self.all_systems()
        while todo:
            cur = todo.pop(0)
            if cur == dest:
                break
            for n in graph.get(cur, ()):
                if n not in prev:
                    prev[n] = cur
                    todo.append(n)
        if dest not in prev:
            return None
        path = []
        while dest != start:
            path.append(dest)
            dest = prev[dest]
        return path[::-1]

    @staticmethod
    def _unit(name):
        """Python expression (in the engine) for a unit given by name, or
        an expression already (e.g. "find_by_display_name('drayman 0')")."""
        return name if '(' in name else 'find_unit(%r)' % name

    def fly_to(self, name, near=200.0):
        """Real autopilot first, then close the remaining gap directly (the
        mock-free way to 'fly' until a real flight AI exists)."""
        u = self._unit(name)
        self.e.exec("u = %s\np = VS.getPlayer()\np.SetTarget(u)\n_ = p.AutoPilotTo(u, True)" % u)
        time.sleep(0.5)
        d = self.e.eval('VS.getPlayer().getDistance(%s)' % u)
        if d > near:
            self.e.exec('teleport_near(%s, %f)' % (u, near / 2))
            time.sleep(0.2)
        return self.e.eval('VS.getPlayer().getDistance(%s)' % u)

    def jump_to(self, dest):
        jps = [name for name, dests in self.e.eval('VSRemote.GetJumpPoints()') if dest in dests]
        if not jps:
            raise RemoteError('no jump point to %s in %s' % (dest, self.system()))
        jp = jps[0]
        self.log('  jumping %s -> %s via %s' % (self.system(), dest, jp))
        self.fly_to(jp)
        # into the jump point with the jump drive on (the unit is gone
        # from the list once the jump happened)
        enter = ("u = find_unit(%r)\n"
                 "if u is not None:\n"
                 "    teleport_near(u, -u.rSize() * 0.9)\n"
                 "    VSRemote.KeyCommand('JumpKey', 'tap')\n" % jp)
        self.e.exec(enter)
        t0 = time.time()
        while self.system() != dest:
            if time.time() - t0 > 60:
                raise RemoteError('jump to %s did not happen (jump status %s)'
                                  % (dest, self.e.eval('VS.getPlayer().GetJumpStatus()')))
            if int(time.time() - t0) % 5 == 4:
                self.e.exec(enter)
            time.sleep(0.5)
        time.sleep(1.0)

    def goto_system(self, dest):
        while self.system() != dest:
            path = self.route(dest)
            if not path:
                raise RemoteError('no route to %s' % dest)
            self.jump_to(path[0])

    def dock(self, name, timeout=30.0):
        self.log('  docking at %s' % name)
        u = self._unit(name)
        self.fly_to(name, near=50.0)
        self.e.exec("VS.getPlayer().SetTarget(%s)" % u)
        t0 = time.time()
        tries = 0
        while not self.in_base():
            if time.time() - t0 > timeout:
                raise RemoteError('could not dock at %s (distance %.0f, ports %s)'
                                  % (name, self.e.eval('VS.getPlayer().getDistance(%s)' % u),
                                     self.e.eval('VSRemote.GetDockingPorts(%s)' % u)))
            if tries >= 3:
                # capital ships only dock at their docking ports (Unit::CanDockWithMe)
                self.e.exec("ports = VSRemote.GetDockingPorts(%s)\n"
                            "if ports:\n"
                            "    x, y, z, r = ports[%d %% len(ports)]\n"
                            "    VS.getPlayer().SetCurPosition((x, y, z))\n"
                            "    VS.getPlayer().SetVelocity((0.0, 0.0, 0.0))\n" % (u, tries - 3))
                time.sleep(0.3)
            self.e.eval("VSRemote.KeyCommand('DockKey', 'tap')")
            tries += 1
            time.sleep(1.0)
        return True
