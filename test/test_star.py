import unittest

from .context import Star

class TestStar(unittest.TestCase):

    def test_star_from_xyz(self):
        """Star initializes from camera coordinates and has accessible properties (tests SWIG configuration)"""
        star = Star(5.0, 4.0, 1.0, 1.0, 2.0, 3.0, 5.5, 0, False)

        # Test properties
        x = star.px
        y = star.py
        x = star.x
        y = star.y
        z = star.z
        f = star.flux
        var = star.variance
        self.assertEqual(star.unreliable, False)
        h = star.hash
        idx = star.index
        self.assertEqual(idx, -1)

        star.index = 5
        self.assertEqual(star.index, 5)

    def test_star_from_pxy(self):
        """Star initializes from focal plane array coordinates"""
        star = Star(5.0, 4.0, 1.0, 1.0, 2.0, 3.0, 0, False)
