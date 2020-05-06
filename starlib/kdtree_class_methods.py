"""Extra KDTree functions go in this file. These should be added to
KDTree in starlib.i."""


def filter_mask(self, camera,
                mask = None):
    """Produce a mask of stars in the KDTree where True indicates stars to
    be ignored/removed, and False indicates stars to be retained.

    Sufficient conditions for removal/masking:

    1. Not bright enough to be observed.

    2. Unreliable / variable brightness.

    2. Stars which are too close to the query star and less bright.

    """
    from .starlib import BoolVector

    if mask is None:
        # Before we can search, we need to do a sort().
        self.sort()
    
        mask = BoolVector(self.size, False)
        
    for index, star in enumerate(self):

        # Don't use already-masked stars as queries.
        if mask[index]:
            continue
        
        # Don't use unreliable stars as queries.
        if self[index].unreliable:
            continue

        # Don't use stars that are too dim as queries.
        if self[index].flux >= camera.min_observable_flux:
            continue

        # Find all neighbors which are less bright than the query and
        # mask them.
        self.mask_search(star.x, star.y, star.z, camera.double_star_angle,
                         camera.min_observable_flux, self[index].flux,
                         mask)

    return mask


def uniform_density_mask(self, camera,
                         mask = None):
    """For each star, this method finds the n brightest stars in the FOV
    (n is determined by camera parameters) and returns a mask for those
    stars."""
    
    from .starlib import BoolVector, IndexSet
    
    if mask is None:
        # Before we can search, we need to do a sort().
        self.sort()
    
        mask = BoolVector(self.size, False)

    keep_indices = IndexSet()

    # Search the proximity of each star.
    for ii, query_star in enumerate(self):

        query_mask = BoolVector(mask)

        # Don't use already-masked stars as queries.
        if mask[ii]:
            continue
        
        # Find all visible, unmasked stars within the FOV and mark them as True.
        self.mask_search(query_star.x, query_star.y, query_star.z, camera.min_fov * 0.5,
                         camera.min_observable_flux, 0.0,
                         query_mask)

        # Retain only the n brightest (according to the camera
        # configuration). Because we're doing this for each and every
        # query star, we are essentially attempting to retain
        # min_stars_per_fov. Sometimes there might just be a totally
        # empty area of the sky and we can't really do anything about
        # that.
        self.add_n_brightest_mask_indices(query_mask, keep_indices, camera.min_stars_per_fov, True)
                
    # Start with everything masked.
    result_mask = BoolVector(self.size, True)

    # Now go through the indices we got from the above loop and unmask each of them.
    for ii in keep_indices:
        result_mask[ii] = False

    return result_mask
