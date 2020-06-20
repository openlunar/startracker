import os.path
import time as time

import numpy as np
import cv2

from ruamel.yaml import YAML
yaml = YAML()

import warnings

from .starlib import StarDatabase
from .starlib import Star

class Camera(object):
    """Represents a configuration for a specific camera.

    This class was abstracted from config.h in openstartracker.

    Instead of using environment variables, however, it uses a YAML
    configuration file.
    """
    
    kdbucket_scale = 3.5 * (360 / np.pi)**2 # 3.5 is default value, not sure where it came from
    
    def __init__(self, filename):
        """Constructor for loading a camera configuration into a Python
        object.

        Args:
            filename   location of YAML configuration file to load

        """
        
        f = open(filename, 'r')
        y = yaml.load(f)

        if 'kdbucket_scale' in y:
            self.kdbucket_scale = y['kdbucket_scale']

        w = int(y['image_width']) # in pixels
        h = int(y['image_height']) # in pixels

        # Get radians per pixel
        radians_per_pixel = y['pixel_arcseconds'] * np.pi * 0.5 / (3600.0 * 180)

        self.max_fov = radians_per_pixel * np.sqrt(w * w + h * h)
        self.min_fov = radians_per_pixel * h

        # TODO: What is this?
        self.match_value = 4 * np.log(1.0 / (w * h)) + np.log(2 * np.pi)

        # These are the "distance" to the celestial sphere in pixels:
        self.pixel_x_tangent = 2 * np.tan(w * radians_per_pixel) / w
        self.pixel_y_tangent = 2 * np.tan(h * radians_per_pixel) / h

        # Get size of a kd-tree bucket
        self.kdbucket_size = int((w * radians_per_pixel) * (h * radians_per_pixel) * self.kdbucket_scale)

        self.image_width           = w
        self.image_height          = h
        self.radians_per_pixel     = radians_per_pixel
        self.position_error_sigma  = float(y['position_error_sigma'])
        self.min_position_variance = float(y['min_position_variance']) # for each star
        self.image_variance        = float(y['image_variance'])
        self.threshold_factor      = float(y['threshold_factor'])
        self.double_star_pixels    = float(y['double_star_pixels'])
        self.max_false_stars       = int(y['max_false_stars'])
        self.min_stars_per_fov     = int(y['min_stars_per_fov']) # was required_stars
        self.db_redundancy         = int(y['db_redundancy'])
        self.base_flux             = float(y['base_flux'])

        if 'median_image_path' in y:
            if os.path.isfile(y['median_image_path']):
                self.median_image          = cv2.imread(y['median_image_path'])
            else:
                raise FileNotFoundError("No such file or directory: '{}'".format(y['median_image_path']))
        else:
            warnings.warn("{}: Need median_image_path configuration option in order to load median image".format(filename))
            self.median_image          = None
        

        # tan(image radians) = (w / 2) / dist
        # 2 * tan(image radians) / w = dist
        # tan(w * s)

        f.close()

    @property
    def double_star_angle(self):
        return self.double_star_pixels * self.radians_per_pixel

    @property
    def min_observable_flux(self):
        return self.threshold_factor * self.image_variance

    def load_catalog(self, year,
                     filename   = 'data/hip_main.dat',
                     epoch      = 1991.25,
                     stop_after = 118219):
        """Load a star catalog, such as hip_main.dat.
        
        Updates star positions in the catalog to the provided year
        (all positions are given relative to 1991.25).

        References:

        [0] https://heasarc.gsfc.nasa.gov/W3Browse/all/hipparcos.html

        [1] O'Connell. Magnitude and color systems. ASTR 511 Lec 14.
            <http://web.ipac.caltech.edu/staff/fmasci/home/astro_refs/magsystems.pdf>
            Retrieved 16 Apr 2020.

        Args:
            year        decimal year image was taken
            filename    location of catalog (default is data/hip_main.dat)
            epoch       epoch year for catalog (default is 1991.25)
            stop_after  catalog index after which to stop (this defaults to
                        past the end of the catalog, but can be set lower
                        if one doesn't wish to spend the time needed to load
                        the entire catalog, e.g. for testing)

        Returns:
            A database object.

        """
        database = StarDatabase(self.min_position_variance)
        
        year_diff = year - epoch

        f = open(filename, 'r')
        for line in f:
            fields = line.split('|')

            star_identifier = int(fields[1])
            
            if star_identifier > stop_after: # This is mainly here for debugging purposes.
                break

            try:
                vmag            = float(fields[5])
                var_flag        = fields[6]

                # Compute declination at present date
                dec_proper_motion_deg_per_year = float(fields[13]) / 3600000.0
                dec_deg_epoch                  = float(fields[9])
                dec_deg                        = year_diff * dec_proper_motion_deg_per_year + dec_deg_epoch
                dec                            = dec_deg * np.pi / 180.0 # in radians
                cos_dec                        = np.cos(dec)

                # Compute right ascension at present date
                ra_proper_motion_deg_per_year = float(fields[12]) / (cos_dec * 3600000.0)
                ra_deg_epoch                  = float(fields[8])
                ra_deg                        = year_diff * ra_proper_motion_deg_per_year + ra_deg_epoch
                ra                            = ra_deg * np.pi/180.0 # in radians
            except ValueError:
                warnings.warn("unable to read data for catalog star {}".format(star_identifier))
                continue # skip this star
            
            reject_percent  = int(fields[29])
            reliable        = reject_percent in (0, 1) and var_flag != '3'
            star = Star(self.pixel_x_tangent, self.pixel_y_tangent, self.image_variance,
                        np.cos(ra) * cos_dec,
                        np.sin(ra) * cos_dec,
                        np.sin(dec),
                        self.base_flux * 10.0 ** (-vmag / 2.5), # see ref [1]
                        star_identifier,
                        not reliable)
            database += star
            
        f.close()
        
        return database
            
    def solve(self, image_filename,
              lost_in_space = True):

        # Get time of image receipt, attempt to process the image, and
        # then get another timestamp.
        timestamp              = time.time()
        current_image          = Image(self, timestamp, image_filename)
        timestamp_done_loading = time.time()

        # 1. Attempt to match based on brightest stars in image.
        current_image.match_lost_in_space()

        # 2. Attempt to match based on last match.

        # 3. Make a list of things that turned out not to be stars?
        
        # At this point, solve_image() in openstartracker *optionally*
        # (but by default) calls match_lis(). Then, if there's a
        # previous match, it also calls match_rel on that previous
        # match. Next, it calls update_nonstars().
        
