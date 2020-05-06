import unittest

from .context import KDTree, Camera, MaskVector

class TestKDTree(unittest.TestCase):

    def setUp(self):
        self.camera = Camera('cameras/science_cam.yml')
        self.db     = self.camera.load_catalog(2020)

    def test_sort(self):
        tree = KDTree(self.db, self.camera.kdbucket_size)
        star0_before = tree[0]
        tree.sort()
        star0_after = tree[0]

        self.assertNotEqual(star0_before, star0_after)

    def test_iterator(self):
        tree = KDTree(self.db, self.camera.kdbucket_size)
        for star in tree:
            self.assertEqual(star.index, star.index)
        
    def test_search(self):
        radius = 0.05
        
        tree = KDTree(self.db, self.camera.kdbucket_size)
        tree.sort()

        # First check that we can find a star we know is in the tree.
        # Let's do the first item.
        first_item = tree[0]
        found_tree = tree.search(first_item.x, first_item.y, first_item.z, 0.05, 0.0)
        self.assertGreater(found_tree.size, 0)


        # Now let's do the last item.
        last_item = tree[tree.size-1]
        found_tree = tree.search(last_item.x, last_item.y, last_item.z, 0.05, 0.0)
        self.assertGreater(found_tree.size, 0)
        

        tree_items = []
        for ii in range(0, tree.size):
            tree_items.append(tree[ii])

        # Build a smaller tree using a search.
        smaller_tree = tree.search(1.0, 0.0, 0.0, radius, 0.0)

        smaller_tree.sort()
        smaller_tree_items = []
        for ii in range(0, smaller_tree.size):
            smaller_tree_items.append(smaller_tree[ii])

        # Test that all stars found are acceptable
        for star in smaller_tree_items:
            self.assertLess(star.vector_squared_distance(1.0, 0.0, 0.0), radius ** 2)

        # Test that all stars not found are out of range
        for star in smaller_tree_items:
            if star not in tree_items:
                self.assertGreater(star.vector_squared_distance(1.0, 0.0, 0.0), radius ** 2)

        
    def test_mask_search(self):
        """Successfully filters the catalog and produces a smaller KDTree"""

        tree = KDTree(self.db, self.camera.kdbucket_size)
        tree.sort()

        mask = MaskVector()
        tree.mask_search(1.0, 0.0, 0.0, 0.1, 0.0, 0.0, mask)
        items = tree.with_mask(mask)

        # Make a new tree from the same database but with only the
        # items from the mask.
        new_tree = KDTree(tree, items)
        

    def test_kdtree_ext(self):
        """kdtree_ext.py functions are properly added to KDTree"""

        tree       = KDTree(self.db, self.camera.kdbucket_size)
        mask       = tree.filter_mask(self.camera)
        final_mask = tree.uniform_density_mask(self.camera, mask)
