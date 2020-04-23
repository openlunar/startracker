#ifndef KDTREE_HPP
# define KDTREE_HPP

#include <vector>
#include <list>
#include <set>

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


  /** @brief Perform a KDTree sort.
   */
  void sort() {
    if (!sorted) {
      // Start recursive sort
      sort_dim<0>(elements.begin(), elements.end());
      
      sorted = true;
    }
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



  /** @brief Check whether a star meets the constraints for inclusion.
   *
   * This replaces kdcheck() from openstartracker. Unlike that
   * function, it does not insert the star into the results or unmask
   * it. Those must be done based on the result of this function.
   */
  bool check(size_t index, float x, float y, float z, const float& r, const float& min_flux) {
    x -= elements[index]->x();
    y -= elements[index]->y();
    z -= elements[index]->z();
    
    if ((x - r <= 0 && 0 <= x + r) && (y - r <= 0 && 0 <= y + r) && (z - r <= 0 && 0 <= z + r) &&
	(elements[index]->get_flux() >= min_flux) &&
	(x * x + y * y + z * z <= r * r)) {

      return true;
    }
    return false;
  }
  

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

};

#endif // KDTREE_HPP
