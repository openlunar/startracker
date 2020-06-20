import os.path
import cv2
import numpy as np
import warnings

from .starlib import StarDatabase
from .starlib import Star


def show_contours(image, contours):
    """Display a window showing the image with the contours drawn atop it
    in blue. The window may be closed using the ESC key.

    Reference:

    [0] https://stackoverflow.com/a/28677782/170300
    """
    img1 = image.copy()
    for contour in contours:
        reshaped_contour = contour.reshape(-1, 2)
        for (x, y) in reshaped_contour:
            cv2.circle(img1, (x, y), 1, (255, 0, 0), 1)

    cv2.imshow('contours', img1)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
    

class ImageData(object):
    def __init__(self, camera, timestamp):
        self.camera        = camera
        self.timestamp     = timestamp
        self.centroids     = [] # of stars
        self.covariances   = [] # corresponding to each centroid /
                                # star -- just upper triangular
                                # portion
        self.fluxes        = [] # brightness of center pixel

    def add_blob(self, contour, grayscale_image):
        """Convert a cv2 contour and corresponding image patch into a
        centroid, moments, and a brightness.

        If the area is too large, this will treat it as a planet. If
        the area is 0, it will not be added.

        Reference:

        [0] https://alyssaq.github.io/2015/computing-the-axes-or-orientation-of-a-blob/

        Returns:
            A Star object representing the blob, or None if the blob
        has 0 area.

        """
        m = cv2.moments(contour)
        area = m['m00']

        if area > 100:
            warnings.warn("possible planet")
        elif area == 0:
            return None
        
        c = np.array([m['m10'], m['m01']]) / area
        self.centroids.append(c)

        u11 = m['m11'] / area - c[0] * c[1]
        u20 = m['m20'] / area - c[0] * c[0]
        u02 = m['m02'] / area - c[1] * c[1]
        # cov = np.array([[u20, u11], [u11, u02]]) --- full covariance
        
        self.covariances.append(np.array([u20, u11, u02])) # upper triangle

        centroid = (c[0], c[1])
        flux = float( cv2.getRectSubPix(grayscale_image, (1,1), centroid)[0,0] )
        self.fluxes.append(flux)

        star = Star(self.camera.pixel_x_tangent,
                    self.camera.pixel_y_tangent,
                    self.camera.image_variance,
                    c[0] - self.camera.image_width  / 2.0,
                    c[1] - self.camera.image_height / 2.0,
                    flux,
                    -1)
        return star

class Image(object):
    def __init__(self, camera, timestamp, image_filename,
                 show      = False):
        self.camera = camera
        self.data   = ImageData(camera, timestamp)
        self.stars  = StarDatabase() # Put stars from the image here.

        if os.path.isfile(image_filename):
            image = cv2.imread(image_filename)
            if image is None: # cv2 doesn't raise exceptions, annoyingly
                raise StandardError("opencv was unable to open file '{}' for unknown reasons".format(image_filename))
        else:
            raise FileNotFoundError("No such file or directory: '{}'".format(image_filename))

        image = np.clip(image.astype(np.int16) - camera.median_image, a_min = 0, a_max = 255).astype(np.uint8)
        grayscale_image = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)

        # Black out areas of the image that don't meet our brightness threshold
        _, thresholded_image = cv2.threshold(grayscale_image,
                                             int(camera.threshold_factor * camera.image_variance),
                                             255,
                                             cv2.THRESH_BINARY)

        # Get the contours
        _, contours, _ = cv2.findContours(thresholded_image, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE) # RETR_LIST = 1, CHAIN_APPROX_SIMPLE = 2

        if show:
            show_contours(image, contours)

        print("Found {} contours".format(len(contours)))
        for contour in contours:
            self.add_blob(contour, grayscale_image)
                

    def add_blob(self, *args):
        """Add a star as a blob. Most of the work is done by
        ImageData.add_blob, but this method also adds to the
        StarDatabase for the image.

        See ImageData.add_blob() for arguments and return information.
        """
        star = self.data.add_blob(*args)
        if star is None:
            return False
        else:
            self.stars += star
            return True
        

    def match_lost_in_space(self):
        """Only use the n brightest stars in the image for the first match,
        where

        n = max_false_stars + required_stars

        """

        # Get the brightest stars from this image, take all
        # permutations and stick them in a constellation database, and
        # then see if we can match that database to the global
        # constellation database.
        brightest = self.stars.stars_by_greatest_flux(self.camera.max_false_stars +
                                                      self.camera.min_stars_per_fov)

        import pdb
        pdb.set_trace()
