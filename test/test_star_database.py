"""This file tests mainly the creation of the database. Loading a
catalog is tested in test_camera.py."""

import unittest

from .context import StarDatabase, Star, Camera

class TestStarDatabase(unittest.TestCase):

    def test_star_database(self):
        """Creation of databases appropriately increases the database count"""
        db = StarDatabase()

        self.assertEqual(db.count, 1)

        db2 = StarDatabase()
        self.assertEqual(db.count, 2)
        self.assertEqual(db.count, db2.count)

    def test_stars_by_greatest_flux(self):
        self.camera = Camera('cameras/science_cam.yml')
        db = self.camera.load_catalog(2020, stop_after = 1000)
        stars = db.stars_by_greatest_flux(100)
        
        # Returns the correct number of stars
        self.assertEqual(len(stars), 100)

        for ii in range(0, 99):
            self.assertGreaterEqual(stars[ii].flux, stars[ii+1].flux)
            self.assertNotEqual(stars[ii].hash, stars[ii+1].hash)
        
