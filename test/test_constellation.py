import unittest

from scipy.linalg import norm
import numpy as np

from .context import Constellation, Star

class TestConstellation(unittest.TestCase):

    def test_constellation(self):
        """Constellation initializes from two stars"""

        # Make sure to normalize coordinates of the made-up stars or
        # the hash will be the same for all of them.
        p1 = np.array([1.0, 2.0, 3.0])
        p2 = np.array([1.0, 2.01, 3.0])
        p3 = np.array([1.01, 2.0, 3.0])
        p1 /= norm(p1)
        p2 /= norm(p2)
        p3 /= norm(p3)
        
        s1 = Star(5.0, 4.0, 1.0, *p1, 5.5, 0, False)
        s2 = Star(5.0, 4.1, 1.0, *p2, 5.0, 1, False)
        s3 = Star(5.0, 3.9, 1.0, *p3, 5.4, 2, False)
        c1 = Constellation(0, s1, s2)
        c2 = Constellation(1, s1, s3)

        self.assertEqual(c1.distance, s1.approximate_distance(s2))

        # Test operator<
        self.assertLess(c1, c2)
        self.assertLess(c1.distance, c2.distance)
        
        # Test __repr__
        print(c1)
