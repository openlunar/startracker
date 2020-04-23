import unittest

from .context import KDTree, Camera

class TestKDTree(unittest.TestCase):

    def setUp(self):
        self.camera = Camera('cameras/science_cam.yml')
        self.db     = self.camera.load_catalog(2020)

    def test_kdtree_sort(self):
        tree = KDTree(self.db, self.camera.kdbucket_size)
        star0_before = tree[0]
        tree.sort()
        star0_after = tree[0]

        self.assertNotEqual(star0_before, star0_after)
        
