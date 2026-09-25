"""Detect Python 2 -> 3 integer division changes at runtime.

In Python 2, ``a / b`` with two ints floors; in Python 3 it returns a
float.  2to3 does not touch ``/``, so converted code can silently change
behaviour (or crash when the float is used as an index/count).  With the
checker enabled every ``/`` and ``/=`` in the data pack is rewritten into
a call that records, per source location, whether it ever divided two
ints with a non-integral result (a place where Python 2 would have
floored) and whether it ever saw float operands (where ``//`` would be the
wrong fix).
"""

import ast
import builtins
import importlib.abc
import importlib.util
import os
import sys


class Site:
    __slots__ = ('filename', 'lineno', 'col', 'calls', 'int_int', 'int_int_nonintegral',
                 'non_int', 'example', 'int_wrapped')

    def __init__(self, filename, lineno, col):
        self.filename = filename
        self.lineno = lineno
        self.col = col
        self.calls = 0
        self.int_int = 0
        self.int_int_nonintegral = 0
        self.non_int = 0
        self.example = None
        self.int_wrapped = False   # appears as int(a / b): truncation matches Py2 for a,b >= 0


SITES = []


def _vs_div(a, b, site):
    s = SITES[site]
    s.calls += 1
    if type(a) is int and type(b) is int:
        s.int_int += 1
        if b != 0 and a % b != 0:
            s.int_int_nonintegral += 1
            if s.example is None:
                s.example = (a, b)
    else:
        s.non_int += 1
    return a / b


class _Transformer(ast.NodeTransformer):
    def __init__(self, filename):
        self.filename = filename
        self.wrapped = set()

    def _site(self, node):
        site = Site(self.filename, node.lineno, node.col_offset)
        site.int_wrapped = id(node) in self.wrapped
        SITES.append(site)
        return len(SITES) - 1

    def visit_Call(self, node):
        if (isinstance(node.func, ast.Name) and node.func.id == 'int' and len(node.args) == 1
                and isinstance(node.args[0], ast.BinOp) and isinstance(node.args[0].op, ast.Div)):
            self.wrapped.add(id(node.args[0]))
        self.generic_visit(node)
        return node

    def visit_BinOp(self, node):
        self.generic_visit(node)
        if isinstance(node.op, ast.Div):
            call = ast.Call(func=ast.Name(id='__vs_div__', ctx=ast.Load()),
                            args=[node.left, node.right, ast.Constant(self._site(node))],
                            keywords=[])
            return ast.copy_location(call, node)
        return node

    def visit_AugAssign(self, node):
        self.generic_visit(node)
        if isinstance(node.op, ast.Div) and isinstance(node.target, ast.Name):
            load = ast.Name(id=node.target.id, ctx=ast.Load())
            call = ast.Call(func=ast.Name(id='__vs_div__', ctx=ast.Load()),
                            args=[load, node.value, ast.Constant(self._site(node))], keywords=[])
            new = ast.Assign(targets=[node.target], value=call)
            return ast.copy_location(new, node)
        return node


def transform_compile(source, filename):
    tree = ast.parse(source, filename)
    tree = _Transformer(filename).visit(tree)
    ast.fix_missing_locations(tree)
    return compile(tree, filename, 'exec')


class _Loader(importlib.abc.SourceLoader):
    def __init__(self, fullname, path):
        self.fullname = fullname
        self.path = path

    def get_filename(self, fullname):
        return self.path

    def get_data(self, path):
        with open(path, 'rb') as fh:
            return fh.read()

    def source_to_code(self, data, path, *, _optimize=-1):
        return transform_compile(data.decode('latin-1'), path)


class _Finder(importlib.abc.MetaPathFinder):
    def __init__(self, roots):
        self.roots = [os.path.abspath(r) for r in roots]

    def find_spec(self, fullname, path, target=None):
        if '.' in fullname:
            return None
        for root in self.roots:
            p = os.path.join(root, fullname + '.py')
            if os.path.isfile(p):
                return importlib.util.spec_from_file_location(fullname, p, loader=_Loader(fullname, p))
        return None


ENABLED = False


def enable(data_root):
    """Install the hook for modules living in the engine's python paths."""
    global ENABLED
    ENABLED = True
    builtins.__vs_div__ = _vs_div
    roots = [os.path.join(data_root, d) for d in
             ('modules/quests', 'modules/missions', 'modules/ai', 'modules', 'bases')]
    # Must come before the normal path finder, but the engine's stdlib-first
    # ordering is preserved because we only claim files under the data dir
    # that a normal import would have found *after* the stdlib - so check
    # the stdlib first.
    finder = _Finder(roots)

    class StdlibFirst(importlib.abc.MetaPathFinder):
        def find_spec(self, fullname, path, target=None):
            if '.' in fullname or fullname in sys.builtin_module_names:
                return None
            # Let the regular machinery find stdlib modules first.
            for entry in sys.path:
                if os.path.abspath(entry) in finder.roots:
                    continue
                if not entry:
                    continue
                for cand in (os.path.join(entry, fullname + '.py'),
                             os.path.join(entry, fullname, '__init__.py')):
                    if os.path.exists(cand):
                        return None
            return finder.find_spec(fullname, path, target)

    sys.meta_path.insert(0, StdlibFirst())


def report(data_root, only_suspicious=True):
    rows = []
    for s in SITES:
        if only_suspicious and (not s.int_int_nonintegral or s.int_wrapped):
            continue
        rows.append(s)
    rows.sort(key=lambda s: (s.filename, s.lineno))
    out = []
    for s in rows:
        rel = os.path.relpath(s.filename, data_root) if s.filename.startswith(data_root) else s.filename
        out.append('%s:%d: int/int non-integral %d of %d calls%s (e.g. %s)' % (
            rel, s.lineno, s.int_int_nonintegral, s.calls,
            ', ALSO float operands %d times' % s.non_int if s.non_int else '', s.example))
    return out
