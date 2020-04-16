import unittest
from test import test_star

def starlib_test_suite():
    """
    Load unit tests from each file for automatic running using
    `python setup.py test`.
    """

    loader = unittest.TestLoader()
    suite  = unittest.TestSuite()
    suite.addTests(loader.loadTestsFromModule(test_star))

    return suite
