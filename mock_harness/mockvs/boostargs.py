"""Argument checking that mimics how Boost.Python converts arguments.

The real engine exposes its C++ API through Boost.Python, which is strict
about argument types: a C++ ``int`` parameter only accepts Python ints (a
float such as ``3/2`` raises ``Boost.Python.ArgumentError``), an ``unsigned
int`` also rejects negative numbers, vectors must be *tuples* of three
numbers, and so on.  Python 2 scripts that were mechanically converted to
Python 3 frequently break exactly here (``/`` now produces floats), so the
mock engine applies the same rules to every exported function.
"""

import functools


class ArgumentError(TypeError):
    """Stand-in for ``Boost.Python.ArgumentError`` (also a TypeError)."""


class Spec:
    """A named argument type understood by :func:`convert`."""

    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return self.name


INT = Spec('int')
UINT = Spec('unsigned int')
FLOAT = Spec('float')
STR = Spec('std::string')
BOOL = Spec('bool')
VEC = Spec('Vector')          # tuple of three numbers
UNIT = Spec('UnitWrapper')    # VS.Unit instance, None not allowed
UNITPTR = Spec('Unit*')       # VS.Unit instance or None
CARGO = Spec('Cargo')
DICT = Spec('dict')
ANY = Spec('object')

_UNIT_TYPES = ()
_CARGO_TYPES = ()


def register_unit_types(unit_type, cargo_type):
    global _UNIT_TYPES, _CARGO_TYPES
    _UNIT_TYPES = (unit_type,)
    _CARGO_TYPES = (cargo_type,)


def _pytype(value):
    return type(value).__name__


def _accepts(spec, value):
    if spec is ANY:
        return True
    if spec is INT:
        return isinstance(value, int)
    if spec is UINT:
        return isinstance(value, int)
    if spec is FLOAT:
        return isinstance(value, (int, float))
    if spec is STR:
        return isinstance(value, (str, bytes))
    if spec is BOOL:
        return value is None or isinstance(value, int)
    if spec is VEC:
        return isinstance(value, tuple)
    if spec is UNIT:
        return isinstance(value, _UNIT_TYPES)
    if spec is UNITPTR:
        return value is None or isinstance(value, _UNIT_TYPES)
    if spec is CARGO:
        return isinstance(value, _CARGO_TYPES)
    if spec is DICT:
        return isinstance(value, dict)
    raise AssertionError('unknown spec %r' % (spec,))


def _convert(spec, value):
    if spec is UINT:
        if value < 0:
            raise OverflowError("can't convert negative value to unsigned int")
        return int(value)
    if spec is INT:
        if value > 2147483647 or value < -2147483648:
            raise OverflowError('Python int too large to convert to C long')
        return int(value)
    if spec is FLOAT:
        return float(value)
    if spec is STR:
        if isinstance(value, bytes):
            return value.decode('latin-1')
        return value
    if spec is BOOL:
        return bool(value)
    if spec is VEC:
        if len(value) != 3:
            raise TypeError('function takes exactly 3 arguments (%d given)' % len(value))
        out = []
        for v in value:
            if not isinstance(v, (int, float)):
                raise TypeError('must be real number, not %s' % _pytype(v))
            out.append(float(v))
        return tuple(out)
    return value


def signature_error(qualname, specs, args):
    return ArgumentError(
        'Python argument types in\n    %s(%s)\ndid not match C++ signature:\n    %s(%s)'
        % (qualname, ', '.join(_pytype(a) for a in args),
           qualname.split('.')[-1], ', '.join(s.name for s in specs)))


def convert_args(qualname, specs, args, defaults=()):
    """Validate/convert ``args`` against ``specs``.

    ``defaults`` supplies trailing default values (Boost.Python only honours
    them when the binding declared overloads, which is rare in this engine).
    """
    nspecs = len(specs)
    if len(args) > nspecs or len(args) < nspecs - len(defaults):
        raise signature_error(qualname, specs, args)
    for spec, value in zip(specs, args):
        if not _accepts(spec, value):
            raise signature_error(qualname, specs, args)
    out = [_convert(spec, value) for spec, value in zip(specs, args)]
    if len(out) < nspecs:
        out.extend(defaults[len(defaults) - (nspecs - len(out)):])
    return out


def exported(qualname, *specs, defaults=()):
    """Decorator for a free function exported to Python."""
    def wrap(fn):
        @functools.wraps(fn)
        def inner(*args):
            return fn(*convert_args(qualname, specs, args, defaults))
        return inner
    return wrap


def method(qualname, *specs, defaults=()):
    """Decorator for a bound method exported to Python (self is not checked)."""
    def wrap(fn):
        @functools.wraps(fn)
        def inner(self, *args):
            return fn(self, *convert_args(qualname, specs, args, defaults))
        return inner
    return wrap
