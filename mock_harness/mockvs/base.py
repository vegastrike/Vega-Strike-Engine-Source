"""Base (docked) interface: rooms, links, sprites and the ``Base`` module."""

import os

from . import boostargs as B
from .boostargs import exported

SIMULATION_ATOM = 0.06


class EngineCrash(RuntimeError):
    """Something that would crash (segfault) the real engine."""


class Link:
    def __init__(self, kind, index, pythonfile):
        self.kind = kind          # goto | launch | comp | python | eject
        self.index = index
        self.pythonfile = pythonfile
        self.text = ''
        self.x = self.y = self.w = self.h = 0.0
        self.to = -1
        self.modes = ''
        self.mask = 'c'

    def __repr__(self):
        extra = ' ->%d' % self.to if self.kind == 'goto' else ''
        return '<Link %s %r %r%s>' % (self.kind, self.index, self.text, extra)


class Obj:
    def __init__(self, kind, index, **kw):
        self.kind = kind          # texture | ship | text | talk | script | video
        self.index = index
        self.__dict__.update(kw)

    def __repr__(self):
        return '<Obj %s %r>' % (self.kind, self.index)


class Room:
    def __init__(self, text):
        self.text = text
        self.links = []
        self.objs = []


class BaseInterface:
    def __init__(self, eng, basefile, baseun, caller):
        self.eng = eng
        self.basefile = basefile
        self.baseun = baseun
        self.caller = caller
        self.rooms = []
        self.curroom = 0
        self.python_kbhandler = ''
        self.messages = []        # (room, text) in order
        self.terminated = False
        self.computer_opened = []

    # -- introspection for the driver ------------------------------------
    def room(self, i=None):
        if i is None:
            i = self.curroom
        if 0 <= i < len(self.rooms):
            return self.rooms[i]
        return None

    def links(self, i=None):
        r = self.room(i)
        return list(r.links) if r else []

    def describe(self):
        out = []
        for i, r in enumerate(self.rooms):
            out.append('%d %s%s' % (i, r.text, ' *' if i == self.curroom else ''))
            for l in r.links:
                out.append('    %r' % l)
        return '\n'.join(out)

    def last_messages(self, n=5):
        return [t for (_, t) in self.messages[-n:]]


def _base():
    from . import engine as _e
    eng = _e.CURRENT
    return eng, (eng.base if eng is not None else None)


def _room(room):
    eng, base = _base()
    if base is None:
        return None
    if room < 0 or room >= len(base.rooms):
        return None
    return base.rooms[room]


# --------------------------------------------------------------------------
# loading / terminating
# --------------------------------------------------------------------------

def load_base_interface(eng, basefile, baseun, caller, compilename):
    if eng.base is not None:
        terminate_base(eng, launch=False)
    base = BaseInterface(eng, basefile, baseun, caller)
    eng.base = base
    eng.event('base_loaded', file=basefile, base=repr(baseun))
    if basefile is None:
        eng.warn('no base script found for %r' % (baseun,))
    else:
        eng.run_file_simple(eng.data.path(basefile), 'base script %s' % basefile)
    if baseun is not None and caller is not None and caller is eng.player:
        eng.save_string_list('unit_to_dock_with', [baseun.name])
    if not base.rooms:
        eng.warn('base %s has no rooms' % basefile)
        r = Room('ERROR: No rooms specified...')
        l = Link('launch', 'default room', '')
        l.text = 'ERROR: No rooms specified... - Launch'
        r.links.append(l)
        base.rooms.append(r)
    goto_room(eng, 0)
    for _ in range(16):
        eng.execute_director(SIMULATION_ATOM)
    return base


def terminate_base(eng, launch=True):
    base = eng.base
    if base is None:
        return
    base.terminated = True
    un = base.caller
    if un is not None and un is eng.player:
        eng.save_string_list('unit_to_dock_with', [''])
    eng.base = None
    eng.event('base_terminated', file=base.basefile, launch=launch)


def goto_room(eng, index):
    base = eng.base
    if base is None:
        return
    if 0 <= index < len(base.rooms):
        base.curroom = index


def set_mouse_event(eng, evtype, x, y, buttons):
    d = eng.event_data
    d['type'] = evtype
    d['mousex'] = float(x)
    d['mousey'] = float(y)
    d['mousebuttons'] = int(buttons)
    d['modifiers'] = 0
    d['alt'] = False
    d['shift'] = False
    d['ctrl'] = False


def click_link(eng, link, button=1):
    """Simulate a full mouse click (down then up) on a link."""
    base = eng.base
    if base is None:
        raise RuntimeError('not docked')
    cx = link.x + link.w / 2.0
    cy = link.y + link.h / 2.0
    ctx = 'click %s %r in room %d (%s)' % (link.kind, link.text, base.curroom, base.basefile)
    eng.event('click', link=link.text, index=link.index, linkkind=link.kind, room=base.curroom)
    mask = link.mask
    if link.kind in ('python', 'talk'):
        if 'd' in mask:
            set_mouse_event(eng, 'down', cx, cy, button)
            eng.run_link_python(link.pythonfile, ctx + ' (down)')
    if eng.base is not base:
        return
    if 'u' in mask:
        set_mouse_event(eng, 'up', cx, cy, 0)
        eng.run_link_python(link.pythonfile, ctx + ' (up)')
    if eng.base is not base:
        return
    if 'c' in mask:
        set_mouse_event(eng, 'click', cx, cy, 0)
        eng.run_link_python(link.pythonfile, ctx)
    if eng.base is not base:
        return
    if link.kind == 'goto':
        goto_room(eng, link.to)
    elif link.kind == 'launch':
        launch_from_base(eng)
    elif link.kind == 'eject':
        launch_from_base(eng)
    elif link.kind == 'comp':
        base.computer_opened.append(link.modes)
        eng.event('computer', modes=link.modes)


def launch_from_base(eng):
    base = eng.base
    if base is None:
        return
    plr = eng.player
    if plr is not None and base.baseun is not None and plr.docked_to is base.baseun:
        eng.undock(plr, base.baseun)
    terminate_base(eng, launch=True)


def run_due_scripts(eng, seconds):
    """Let time pass inside a base: RunScript objects in the current room."""
    base = eng.base
    if base is None:
        return
    r = base.room()
    if r is None:
        return
    for o in list(r.objs):
        if o.kind != 'script':
            continue
        o.timeleft += seconds
        if o.timeleft >= o.maxtime:
            o.timeleft = 0.0
            eng.run_link_python(o.pythonfile, 'RunScript %s' % o.index)
            if eng.base is not base:
                return


# --------------------------------------------------------------------------
# The Base module
# --------------------------------------------------------------------------

def _add_link(room, link, x, y, w, h, text, front=False):
    link.x, link.y, link.w, link.h, link.text = x, y, w, h, text
    if front:
        room.links.insert(0, link)
    else:
        room.links.append(link)


@exported('Base.Room', B.STR)
def Room_(text):
    eng, base = _base()
    if base is None:
        return -1
    base.rooms.append(Room(text))
    return len(base.rooms) - 1


@exported('Base.Texture', B.INT, B.STR, B.STR, B.FLOAT, B.FLOAT)
def Texture(room, index, file, x, y):
    r = _room(room)
    if r is None:
        return
    r.objs.append(Obj('texture', index, file=file, x=x, y=y))


@exported('Base.Video', B.INT, B.STR, B.STR, B.STR, B.FLOAT, B.FLOAT)
def Video(room, index, vfile, afile, x, y):
    r = _room(room)
    if r is None:
        return
    r.objs.append(Obj('video', index, file=vfile, audio=afile, x=x, y=y))


@exported('Base.VideoStream', B.INT, B.STR, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT)
def VideoStream(room, index, file, x, y, w, h):
    r = _room(room)
    if r is None:
        return
    r.objs.append(Obj('video', index, file=file, x=x, y=y))


def _for_objs(room, index):
    r = _room(room)
    if r is None:
        return []
    return [o for o in r.objs if o.index == index]


@exported('Base.SetTexture', B.INT, B.STR, B.STR)
def SetTexture(room, index, file):
    for o in _for_objs(room, index):
        if o.kind not in ('texture', 'video'):
            raise EngineCrash('SetTexture on non-sprite object %r (engine would crash)' % index)
        o.file = file


@exported('Base.SetTextureSize', B.INT, B.STR, B.FLOAT, B.FLOAT)
def SetTextureSize(room, index, w, h):
    for o in _for_objs(room, index):
        if o.kind not in ('texture', 'video'):
            raise EngineCrash('SetTextureSize on non-sprite object %r (engine would crash)' % index)
        o.w, o.h = w, h


@exported('Base.SetTexturePos', B.INT, B.STR, B.FLOAT, B.FLOAT)
def SetTexturePos(room, index, x, y):
    for o in _for_objs(room, index):
        if o.kind not in ('texture', 'video'):
            raise EngineCrash('SetTexturePos on non-sprite object %r (engine would crash)' % index)
        o.x, o.y = x, y


@exported('Base.Ship', B.INT, B.STR, B.VEC, B.VEC, B.VEC)
def Ship(room, index, pos, q, r_):
    r = _room(room)
    if r is None:
        return
    r.objs.append(Obj('ship', index, pos=pos))


@exported('Base.RunScript', B.INT, B.STR, B.STR, B.FLOAT)
def RunScript(room, index, pythonfile, time):
    r = _room(room)
    if r is None:
        return
    r.objs.append(Obj('script', index, pythonfile=pythonfile, maxtime=time, timeleft=0.0))


@exported('Base.TextBox', B.INT, B.STR, B.STR, B.FLOAT, B.FLOAT, B.VEC, B.VEC, B.FLOAT, B.VEC)
def TextBox(room, index, text, x, y, widheimult, backcol, backalp, forecol):
    r = _room(room)
    if r is None:
        return
    r.objs.append(Obj('text', index, text=text, x=x, y=y))


@exported('Base.SetTextBoxText', B.INT, B.STR, B.STR)
def SetTextBoxText(room, index, text):
    for o in _for_objs(room, index):
        if o.kind != 'text':
            raise EngineCrash('SetTextBoxText on non-text object %r (engine would crash)' % index)
        o.text = text


def _for_links(room, index):
    r = _room(room)
    if r is None:
        return []
    return [l for l in r.links if l.index == index]


@exported('Base.SetLinkArea', B.INT, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT)
def SetLinkArea(room, index, x, y, w, h):
    for l in _for_links(room, index):
        l.x, l.y, l.w, l.h = x, y, w, h


@exported('Base.SetLinkText', B.INT, B.STR, B.STR)
def SetLinkText(room, index, text):
    for l in _for_links(room, index):
        l.text = text


@exported('Base.SetLinkPython', B.INT, B.STR, B.STR)
def SetLinkPython(room, index, python):
    for l in _for_links(room, index):
        l.pythonfile = python


@exported('Base.SetLinkRoom', B.INT, B.STR, B.INT)
def SetLinkRoom(room, index, to):
    for l in _for_links(room, index):
        if l.kind != 'goto':
            raise EngineCrash('SetLinkRoom on non-goto link %r (engine would crash)' % index)
        l.to = to


@exported('Base.SetLinkEventMask', B.INT, B.STR, B.STR)
def SetLinkEventMask(room, index, maskdef):
    mask = ''.join(ch for ch in maskdef.lower() if ch in 'cudel')
    for l in _for_links(room, index):
        l.mask = mask


@exported('Base.Link', B.INT, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR, B.INT)
def Link_(room, index, x, y, w, h, text, to):
    LinkPython(room, index, '', x, y, w, h, text, to)


@exported('Base.LinkPython', B.INT, B.STR, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR, B.INT)
def LinkPython(room, index, pythonfile, x, y, w, h, text, to):
    r = _room(room)
    if r is None:
        return
    l = Link('goto', index, pythonfile)
    l.to = to
    _add_link(r, l, x, y, w, h, text)


@exported('Base.Launch', B.INT, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR)
def Launch(room, index, x, y, w, h, text):
    LaunchPython(room, index, '', x, y, w, h, text)


@exported('Base.LaunchPython', B.INT, B.STR, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR)
def LaunchPython(room, index, pythonfile, x, y, w, h, text):
    r = _room(room)
    if r is None:
        return
    _add_link(r, Link('launch', index, pythonfile), x, y, w, h, text)


@exported('Base.Comp', B.INT, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR, B.STR)
def Comp(room, index, x, y, w, h, text, modes):
    CompPython(room, index, '', x, y, w, h, text, modes)


@exported('Base.CompPython', B.INT, B.STR, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR, B.STR)
def CompPython(room, index, pythonfile, x, y, w, h, text, modes):
    r = _room(room)
    if r is None:
        return
    l = Link('comp', index, pythonfile)
    l.modes = modes
    _add_link(r, l, x, y, w, h, text)


@exported('Base.Python', B.INT, B.STR, B.FLOAT, B.FLOAT, B.FLOAT, B.FLOAT, B.STR, B.STR, B.BOOL)
def Python(room, index, x, y, w, h, text, pythonfile, front):
    r = _room(room)
    if r is None:
        return
    _add_link(r, Link('python', index, pythonfile), x, y, w, h, text, front)


@exported('Base.GlobalKeyPython', B.STR)
def GlobalKeyPython(pythonfile):
    eng, base = _base()
    if base is not None:
        base.python_kbhandler = pythonfile


def _message(room, text, enqueue):
    eng, base = _base()
    if base is None:
        return
    if room < 0 or room >= len(base.rooms):
        raise EngineCrash('MessageToRoom(%d): room out of range (engine indexes rooms[] unchecked)' % room)
    base.messages.append((room, text))
    base.rooms[room].objs.append(Obj('talk', 'currentmsg', text=text, enqueue=enqueue))
    if eng.opts.verbose >= 3:
        eng.log('[base msg] ' + text.replace('\n', ' | ')[:300])


@exported('Base.MessageToRoom', B.INT, B.STR)
def MessageToRoom(room, text):
    _message(room, text, False)


@exported('Base.EnqueueMessageToRoom', B.INT, B.STR)
def EnqueueMessageToRoom(room, text):
    _message(room, text, True)


@exported('Base.Message', B.STR)
def Message(text):
    eng, base = _base()
    if base is None:
        return
    _message(base.curroom, text, False)


@exported('Base.EnqueueMessage', B.STR)
def EnqueueMessage(text):
    eng, base = _base()
    if base is None:
        return
    _message(base.curroom, text, True)


@exported('Base.EraseLink', B.INT, B.STR)
def EraseLink(room, index):
    r = _room(room)
    if r is None:
        return
    r.links = [l for l in r.links if l.index != index]


@exported('Base.EraseObj', B.INT, B.STR)
def EraseObj(room, index):
    r = _room(room)
    if r is None:
        return
    r.objs = [o for o in r.objs if o.index != index]


@exported('Base.GetCurRoom')
def GetCurRoom():
    eng, base = _base()
    if base is None:
        return -1
    return base.curroom


@exported('Base.SetCurRoom', B.INT)
def SetCurRoom(room):
    eng, base = _base()
    if base is None or _room(room) is None:
        return
    goto_room(eng, room)


@exported('Base.GetNumRoom')
def GetNumRoom():
    eng, base = _base()
    if base is None:
        return -1
    return len(base.rooms)


@exported('Base.BuyShip', B.STR, B.BOOL, B.BOOL)
def BuyShip(name, my_fleet, force_base_inventory):
    eng, base = _base()
    if base is None or eng.player is None:
        return False
    eng.event('buy_ship', name=name)
    plr = eng.player
    row = eng.data.units.lookup(name, plr.faction)
    if row is None:
        eng.warn('BuyShip: unknown ship %s' % name)
        return False
    plr.name = name
    plr.fullname = row.get('Name') or name
    return True


@exported('Base.SellShip', B.STR)
def SellShip(name):
    eng, base = _base()
    eng.event('sell_ship', name=name)
    return True


@exported('Base.SetEventData', B.DICT)
def SetEventData(d):
    eng, base = _base()
    eng.event_data = d


@exported('Base.SetMouseEventData', B.STR, B.FLOAT, B.FLOAT, B.INT)
def SetMouseEventData(typ, x, y, buttons):
    eng, base = _base()
    set_mouse_event(eng, typ, x, y, buttons)


@exported('Base.GetEventData')
def GetEventData():
    eng, base = _base()
    return eng.event_data


@exported('Base.GetTextWidth', B.STR, B.VEC)
def GetTextWidth(text, mult):
    return 0.0


@exported('Base.GetTextHeight', B.STR, B.VEC)
def GetTextHeight(text, mult):
    return 0.05


@exported('Base.LoadBaseInterface', B.STR)
def LoadBaseInterface(name):
    eng, base = _base()
    plr = eng.player
    fac = plr.faction if plr is not None else 'neutral'
    if fac == 'neutral' and plr is not None:
        fac = eng.get_galaxy_faction(plr.system_file())
    f = None
    for cand in ('bases/%s_%s_day.py' % (name, fac), 'bases/%s_%s.py' % (name, fac),
                 'bases/%s_day.py' % name, 'bases/%s.py' % name):
        if os.path.isfile(eng.data.path(cand)):
            f = cand
            break
    load_base_interface(eng, f, plr, plr, name)


@exported('Base.ExitGame')
def ExitGame():
    eng, base = _base()
    eng.event('exit_game')
    eng.quit_requested = True


@exported('Base.GetRandomBarMessage')
def GetRandomBarMessage():
    eng, base = _base()
    msgs = [m for m in eng.io_messages if m[2] == 'bar']
    if not msgs:
        return ('', '')
    m = msgs[eng.rng.randrange(len(msgs))][3]
    snd = ''
    if '[' in m and m.find('[') + 1 < len(m):
        first = m.find('[')
        last = m.find(']')
        snd = m[first + 1:last]
        m = m[:first]
    return (m, snd)


BASE_FUNCTIONS = {
    'Room': Room_, 'SetCurRoom': SetCurRoom, 'GetCurRoom': GetCurRoom, 'GetNumRoom': GetNumRoom,
    'Comp': Comp, 'CompPython': CompPython, 'Launch': Launch, 'LaunchPython': LaunchPython,
    'Link': Link_, 'LinkPython': LinkPython, 'Python': Python, 'EraseLink': EraseLink,
    'Ship': Ship, 'Texture': Texture, 'Video': Video, 'VideoStream': VideoStream,
    'SetTexture': SetTexture, 'SetTextureSize': SetTextureSize, 'SetTexturePos': SetTexturePos,
    'EnqueueMessageToRoom': EnqueueMessageToRoom, 'MessageToRoom': MessageToRoom,
    'EnqueueMessage': EnqueueMessage, 'Message': Message, 'EraseObj': EraseObj,
    'GetRandomBarMessage': GetRandomBarMessage, 'RunScript': RunScript, 'TextBox': TextBox,
    'SetTextBoxText': SetTextBoxText, 'GlobalKeyPython': GlobalKeyPython,
    'SetLinkArea': SetLinkArea, 'SetLinkText': SetLinkText, 'SetLinkPython': SetLinkPython,
    'SetLinkRoom': SetLinkRoom, 'SetLinkEventMask': SetLinkEventMask, 'BuyShip': BuyShip,
    'SellShip': SellShip, 'SetEventData': SetEventData, 'SetMouseEventData': SetMouseEventData,
    'GetEventData': GetEventData, 'GetTextWidth': GetTextWidth, 'GetTextHeight': GetTextHeight,
    'LoadBaseInterface': LoadBaseInterface, 'ExitGame': ExitGame,
}
