import unittest

from test import test_star
from test import test_database
from test import test_camera

def starlib_test_suite():
    """
    Load unit tests from each file for automatic running using
    `python setup.py test`.
    """

    loader = unittest.TestLoader()
    suite  = unittest.TestSuite()
    
    suite.addTests(loader.loadTestsFromModule(test_star))
    suite.addTests(loader.loadTestsFromModule(test_camera))
    suite.addTests(loader.loadTestsFromModule(test_database))

    return suite
