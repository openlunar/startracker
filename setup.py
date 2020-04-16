#!/usr/bin/env python
"""Open Star Tracker.

See:
https://github.com/openlunar/openstartracker
"""

from setuptools import setup, find_packages
from setuptools.extension import Extension
import os
import numpy

MAJOR = 0
MINOR = 0
TINY  = 1
version='%d.%d.%d' % (MAJOR, MINOR, TINY)


c_starlib = Extension('starlib._starlib',
                      ['starlib/starlib.i'],
                      swig_opts = ['-Wall', '-c++'],
                      extra_compile_args = ['-std=c++11', '-lstdc++'],
                      #extra_link_args = ['-v', '-g'],
                      define_macros = [('MAJOR_VERSION', MAJOR),
                                       ('MINOR_VERSION', MINOR),
                                       ('TINY_VERSION',  TINY)])
                    
setup(name='startracker',
      version=version,
      description="Open source star tracker",
      author='John O. "Juno" Woods, Ph.D.',
      author_email='john.o.woods@gmail.com',
      url='https://github.com/openlunar/startracker',
      download_url='https://github.com/openlunar/startracker/archive/v' + version + '.tar.gz',
      include_package_data=True,
      package_dir={'starlib': 'starlib'},
      ext_modules=[c_starlib],
      test_suite='test.starlib_test_suite',
      license='MIT',
      install_requires=['numpy', 'ruamel.yaml']
)
