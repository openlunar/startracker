import unittest

from .context import Database, Star

class TestDatabase(unittest.TestCase):

    def test_database(self):
        """Creation of databases appropriately increases the database count"""
        db = Database()

        self.assertEqual(db.count, 1)

        db2 = Database()
        self.assertEqual(db.count, 2)
        self.assertEqual(db.count, db2.count)

    def test_database_add(self):
        pass
