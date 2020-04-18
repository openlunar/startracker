import unittest
import cv2

from .context import Image, Camera

class TestImage(unittest.TestCase):

    def setUp(self):
        self.camera = Camera('cameras/science_cam.yml')
        
    def test_image(self):
        """Constructs image object successfully"""
        median_image = cv2.imread('images/science_cam_2020-05-08_50ms_gain40/median_image.png')
        image = Image(self.camera, 'images/science_cam_2020-05-08_50ms_gain40/samples/img0.png', median_image,
                      show = True)

        self.assertEqual(image.stars.size, len(image.data.fluxes))
        self.assertEqual(image.stars.size, len(image.data.centroids))
        self.assertEqual(image.stars.size, len(image.data.covariances))
        self.assertEqual(image.stars.size, 23)
