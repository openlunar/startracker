import unittest
import cv2
import time

from .context import Image, Camera

class TestImage(unittest.TestCase):

    def setUp(self):
        self.camera = Camera('cameras/science_cam.yml')
        
    def test_image(self):
        """Constructs image object successfully"""
        
        image = Image(self.camera,
                      time.time(),
                      'images/science_cam_2018-05-08_50ms_gain40/samples/img0.png',
                      show = False)

        self.assertEqual(image.stars.size, len(image.data.fluxes))
        self.assertEqual(image.stars.size, len(image.data.centroids))
        self.assertEqual(image.stars.size, len(image.data.covariances))
        self.assertEqual(image.stars.size, 23)
