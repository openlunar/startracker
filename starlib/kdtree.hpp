#ifndef KDTREE_HPP
# define KDTREE_HPP

#include <vector>
#include <list>
#include <set>
#include <iostream>

#include "star_database.hpp"

#define KDBUCKET_SIZE

class flux_greater_t {
public:
  bool operator()(const Star * const lhs, const Star * const rhs) const {
    return (lhs->get_flux() > rhs->get_flux());
  }
};


/** @brief Array-based 3D kd-tree for storing stars.
 *
 * This is basically star_query from openstartracker, but with the
 * code cleaned up.
 *
 * The KDTree can be used to filter a star database:
 *
 * 1. Create a StarDatabase from a catalog.
 *
 * 2. Create a KDTree from the StarDatabase.
 *
 * 3. Mask the stars in the KDTree based on certain criteria (like
 *    brightness, variability, uniform density, etc.). Multiple mask
 *    functions may be applied in sequence.
 *
 * 4. Produce a new StarDatabase from the filtered KDTree results.
 *
 * It can also be used to search the filtered star database,
 * e.g. using find_nearest().
 */
class KDTree {
protected:
  int kdbucket_size;              /* (cnt) Size of a KDTree bucket */
  StarDatabase* db;               /* (--) StarDatabase to filter */
  std::vector<Star*> elements;    /* (--) Array of pointers to the stars in the StarDatabase that we want to filter */
  bool sorted;
  
public:

  
  KDTree(StarDatabase* db_, const int& kdbucket_size_)
    : kdbucket_size(kdbucket_size_)
    , db(db_)
    , elements(db_->size())
    , sorted(false)
  {
    for (star_id_t ii = 0; ii < elements.size(); ++ii) {
      elements[ii] = db->get_star(ii);
    }
  }


  KDTree(StarDatabase* db_, const int& kdbucket_size_, const std::vector<Star*>& found_elements)
    : kdbucket_size(kdbucket_size_)
    , db(db_)
    , elements(found_elements.begin(), found_elements.end()) // don't reserve extra space
    , sorted(false)
  {
  }


  /** @brief Perform a KDTree sort.
   */
  void sort() {
    if (!sorted) {
      // Start recursive sort
      sort_dim<0>(elements.begin(), elements.end());
      
      sorted = true;
    }
  }


  KDTree search_sorted(const float& x, const float& y, const float& z, const float& radius, const float& min_flux) const {
    if (!sorted) {
      std::cerr << "Error: attempted to search an unsorted tree" << std::endl;
      throw;
    }
    
    std::vector<Star*> found;
    
    search_dim<0>(found, elements.begin(), elements.end(), x, y, z, radius, min_flux);
    std::cerr << "found size = " << found.size() << std::endl;

    // Create a KDTree from the found stars.
    return KDTree(db, kdbucket_size, found);
  }

  
  KDTree search(const float& x, const float& y, const float& z, const float& radius, const float& min_flux) {
    sort();
    return search_sorted(x, y, z, radius, min_flux);
  }
  


  // Methods for accessing contents of elements
  Star* operator[](const size_t& index) const { return elements[index]; }
  Star*& operator[](const size_t& index) { return elements[index]; }
  Star* at(const size_t& index) { return elements[index]; }
  size_t size() const { return elements.size(); }


  /*
  KDTree find_k_nearest(const float& x, const float& y, const float& z, const float& r,
			const float& min_flux,
			const int& min, const int& max,
			const int dim) {
			} */
  

protected:

  /** @brief Perform a KDTree sort with buckets at the leaves, where
   **        the branches are sorted on the star positions and within
   **        the buckets the stars are sorted by flux.
   *
   * @param  min  iterator pointing to where to start the recursive sort
   *              in the elements vector
   * @param  max  iterator pointing to just after the end of the region
   *              to sort in the elements vector
   */
  template <int Dim>
  void sort_dim(std::vector<Star*>::iterator min, std::vector<Star*>::iterator max) {
    std::vector<Star*>::iterator mid = min + (max - min) / 2;
    
    if (min + 1 < max) {
      // Sort the first half of the elements by star x position.
      std::nth_element(min, mid, max, star_ptr_rx_less);

      // Sort the first half of the list by star y position or by star
      // flux (depending on number of stars in this portion).
      if (mid - min > kdbucket_size) { 
	sort_dim<(Dim + 1) % 3>(min, mid); // binary recusion
      } else {
	std::sort(min, mid, star_ptr_flux_greater); // leaf
      }

      // Sort the second half of the list by star y position or by
      // star flux (depending on number of stars in this portion).
      if (max - (mid + 1) > kdbucket_size) {
	sort_dim<(Dim + 1) % 3>(mid + 1, max);
      } else {
	std::sort(mid + 1, max, star_ptr_flux_greater);
      }
    }
  }

  
  /** @brief Check whether a star meets the constraints for inclusion.
   *
   * This replaces kdcheck() from openstartracker. Unlike that
   * function, it does not insert the star into the results or unmask
   * it. Those must be done based on the result of this function.
   *
   * It could still use some simplification and improvement. It really
   * ought to use Star::vector_squared_distance() to avoid code
   * duplication.
   */
  void search_check(std::vector<Star*>& result,
		    std::vector<Star*>::const_iterator it,
		    float x,
		    float y,
		    float z,
		    const float& r,
		    const float& min_flux) const {
    x -= (*it)->x();
    y -= (*it)->y();
    z -= (*it)->z();
    
    if ((x - r <= 0 && 0 <= x + r) && (y - r <= 0 && 0 <= y + r) && (z - r <= 0 && 0 <= z + r) &&
	((*it)->get_flux() >= min_flux) &&
	(x * x + y * y + z * z <= r * r)) {
      result.push_back(*it);
    }
  }

  
  template <int Dim>
  void search_dim(std::vector<Star*>& result,
		  std::vector<Star*>::const_iterator min,
		  std::vector<Star*>::const_iterator max,
		  const float& x,
		  const float& y,
		  const float& z,
		  const float& radius,
		  const float& min_flux) const {
    std::vector<Star*>::const_iterator mid = min + (max - min) / 2;

    // Get bounds
    float center;
    if (Dim == 0)      center = x;
    else if (Dim == 1) center = y;
    else	       center = z;

    // Search the left half
    if (min < mid && center - radius <= (*mid)->get_r(Dim)) {
      if (mid - min > kdbucket_size) {
	search_dim<(Dim + 1) % 3>(result, min, mid, x, y, z, radius, min_flux);
      } else { // Search a bucket
	for (std::vector<Star*>::const_iterator it = min; it < mid; ++it) {
	  search_check(result, it, x, y, z, radius, min_flux);
	}
      }
    }

    // Not clear to me why we check the center as part of a recursive
    // search. Why can't this just be included in the search of the
    // right half?
    if (mid < max)
      search_check(result, mid, x, y, z, radius, min_flux);

    // In openstartracker, we here checked to make sure the list of
    // results didn't exceed a maximum, but this check really seems to
    // be absurdly unlikely (it's like SIZE_T_MAX or something), so
    // I'm leaving it out.

    // Search the right half
    if (mid + 1 < max && (*mid)->get_r(Dim) <= center + radius) {
      if (max - (mid + 1) > kdbucket_size) {
	search_dim<(Dim + 1) % 3>(result, mid + 1, max, x, y, z, radius, min_flux);
      } else {  // Search a bucket
	for (std::vector<Star*>::const_iterator it = mid + 1; it < max; ++it) {
	  search_check(result, it, x, y, z, radius, min_flux);
	}
      }
    }
    
  }

};

#endif // KDTREE_HPP
