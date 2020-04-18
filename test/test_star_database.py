"""This file tests mainly the creation of the database. Loading a
catalog is tested in test_camera.py."""

import unittest

from .context import StarDatabase, Star

class TestDatabase(unittest.TestCase):

    def test_database(self):
        """Creation of databases appropriately increases the database count"""
        db = StarDatabase()

        self.assertEqual(db.count, 1)

        db2 = StarDatabase()
        self.assertEqual(db.count, 2)
        self.assertEqual(db.count, db2.count)
