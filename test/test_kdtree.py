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
        
    def test_kdtree_search(self):
        radius = 0.5
        
        tree = KDTree(self.db, self.camera.kdbucket_size)
        tree.sort()

        tree_items = []
        for ii in range(0, tree.size):
            tree_items.append(tree[ii])

        # Build a smaller tree using a search.
        smaller_tree = tree.search(1.0, 0.0, 0.0, radius, 0.0)

        smaller_tree_items = []
        for ii in range(0, smaller_tree.size):
            smaller_tree_items.append(smaller_tree[ii])

        # Test that all stars found are acceptable
        for star in smaller_tree_items:
            self.assertLess(star.vector_squared_distance(1.0, 0.0, 0.0), radius ** 2)

        # Test that all stars not found are unacceptable
        #
        # FIXME: Really slow.
        for star in tree_items:
            if star not in smaller_tree_items:
                self.assertGreater(star.vector_squared_distance(1.0, 0.0, 0.0), radius ** 2)

        
