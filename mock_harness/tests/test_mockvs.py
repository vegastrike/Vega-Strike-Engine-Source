"""Unit tests for the mock engine.  Run with:

    python3 -m unittest discover -s mock_harness/tests

The data-driven tests are skipped when ../Privateer is not present.
"""

import os
import sys
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, ROOT)
DATA = os.path.join(os.path.dirname(ROOT), 'Privateer')

from mockvs import boostargs as B          # noqa: E402
from mockvs import data as vsdata          # noqa: E402


class BoostArgsTest(unittest.TestCase):
    def test_int_rejects_float(self):
        f = B.exported('VS.getUnit', B.INT)(lambda i: i)
        self.assertEqual(f(3), 3)
        with self.assertRaises(B.ArgumentError):
            f(3 / 2)             # the classic Python 3 true-division bug

    def test_uint_rejects_negative(self):
        f = B.exported('Director.getSaveData', B.INT, B.STR, B.UINT)(lambda a, b, c: c)
        with self.assertRaises(OverflowError):
            f(0, 'x', -1)

    def test_float_accepts_int(self):
        f = B.exported('VS.sqrt', B.FLOAT)(lambda x: x)
        self.assertIsInstance(f(4), float)

    def test_vector_must_be_tuple(self):
        f = B.exported('VS.SafeEntrancePoint', B.VEC, B.FLOAT)(lambda v, r: v)
        self.assertEqual(f((1, 2, 3), 1.0), (1.0, 2.0, 3.0))
        with self.assertRaises(B.ArgumentError):
            f([1, 2, 3], 1.0)

    def test_arity(self):
        f = B.exported('VS.GetGameTime')(lambda: 0.0)
        with self.assertRaises(B.ArgumentError):
            f(1)


class ConfigTest(unittest.TestCase):
    def test_lenient_config(self):
        import tempfile
        text = ('<vegaconfig><variables><section name="graphics">'
                '<var name="a" value="1"/><!-- <var name="a" value="2"/> -->'
                '<-- broken comment --> <section name="sub"><var name="b" value="x"/></section>'
                '</section></variables></vegaconfig>')
        with tempfile.NamedTemporaryFile('w', suffix='.config', delete=False) as fh:
            fh.write(text)
        try:
            cfg = vsdata.VSConfig(fh.name)
            self.assertEqual(cfg.get('graphics', 'a', '?'), '1')
            self.assertEqual(cfg.get_sub('graphics', 'sub', 'b', '?'), 'x')
        finally:
            os.unlink(fh.name)


@unittest.skipUnless(os.path.isdir(DATA), 'Privateer data not present')
class DataTest(unittest.TestCase):
    def test_new_game_save(self):
        sg = vsdata.SaveGame.load(os.path.join(DATA, 'New_Game'))
        self.assertEqual(sg.system, 'Gemini/Troy')
        self.assertIn('unit_to_dock_with', sg.floats)
        self.assertTrue(any(k.startswith('FG:') for k in sg.strings))

    def test_galaxy(self):
        d = vsdata.DataDir(DATA)
        self.assertIn('Gemini/Pyrenees', d.galaxy.jumps('Gemini/Troy'))


if __name__ == '__main__':
    unittest.main()
