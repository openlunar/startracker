import unittest

from test import test_star
from test import test_star_database
from test import test_camera
from test import test_image

def starlib_test_suite():
    """
    Load unit tests from each file for automatic running using
    `python setup.py test`.
    """

    loader = unittest.TestLoader()
    suite  = unittest.TestSuite()
    
    suite.addTests(loader.loadTestsFromModule(test_star))
    suite.addTests(loader.loadTestsFromModule(test_camera))
    suite.addTests(loader.loadTestsFromModule(test_image))
    suite.addTests(loader.loadTestsFromModule(test_star_database))

    return suite
