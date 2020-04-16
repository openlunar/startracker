import numpy as np

from ruamel.yaml import YAML
yaml = YAML()

class Camera(object):
    """Represents a configuration for a specific camera.

    This class was abstracted from config.h in openstartracker.

    Instead of using environment variables, however, it uses a YAML
    configuration file.
    """
    
    kdbucket_scale = 3.5 # default value, not sure where it came from
    
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
        self.kdbucket_size = w * radians_per_pixel * (h * radians_per_pixel) * self.kdbucket_scale

        self.image_width = w
        self.image_height = h
        self.radians_per_pixel = radians_per_pixel
        self.position_error_sigma  = float(y['position_error_sigma'])
        self.min_position_variance = float(y['min_position_variance']) # for each star
        self.image_variance        = float(y['image_variance'])
        self.threshold_factor      = float(y['threshold_factor'])
        self.double_star_pixels    = float(y['double_star_pixels'])
        self.max_false_stars       = int(y['max_false_stars'])
        self.db_redundancy         = int(y['db_redundancy'])
        self.base_flux             = float(y['base_flux'])
        

        # tan(image radians) = (w / 2) / dist
        # 2 * tan(image radians) / w = dist
        # tan(w * s)

    def load_catalog(self, filename, year,
                     from_year = 1991.25,
                     base_flux = None):
        """Load a star catalog, such as hip_main.dat.
        
        Updates star positions in the catalog to the provided year
        (all positions are given relative to 1991.25).

        """
        star_database = StarDatabase(self.min_position_variance)
        
        year_diff = year - from_year

        f = open(filename, 'r')
        for line in f:
            fields = line.split('|')
            mag = float(fields[5])
            dec_deg = year_diff * float(fields[13]) / 3600000.0 + float(fields[9])
            dec = dec_deg * np.pi / 180.0
            cos_dec = np.cos(dec)
            ra_deg = year_diff * float(fields[12]) / (cos_dec * 3600000.0) + float(fields[8])
            ra = ra_deg * np.pi/180.0

            star_id = int(fields[1])
            unreliable = not ((int(fields[29]) == 0 or int(fields[29]) == 1) and int(fields[6]) != 3)

            star = Star(np.cos(ra) * cos_dec,
                        np.sin(ra) * cos_dec,
                        np.sin(dec),
                        self.base_flux * 10.0 ** (-mag / 2.5),
                        star_id,
                        unreliable)

            star_database += star

        f.close()
        
        return star_database
            
