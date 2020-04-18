import unittest

from .context import Camera

class TestCamera(unittest.TestCase):

    def setUp(self):
        self.camera = Camera('cameras/flircam_jan2020.yml')
    
    def test_camera(self):
        """Camera constructor loads a yaml configuration"""
        pass # actually, this is tested by setUp()
        
    def test_load_catalog(self):
        """Loads star catalog"""
        db = self.camera.load_catalog(2020, stop_after = 1000)
        self.assertLess(db.size, 1000)
        self.assertGreater(db.size, 0)

