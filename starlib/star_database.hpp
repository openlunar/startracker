#ifndef STAR_DATABASE_HPP
# define STAR_DATABASE_HPP

#include <unordered_map>
#include <set>
#include <vector>
#include <map>

#include "types.hpp"
#include "star.hpp"


/** @brief Hashed database of stars
 *
 * Instead of using a regular STL map, this uses an unordered_map and
 * an ordered STL set. The advantage is that we can search for
 * specific stars in constant time or get stars in a neighborhood in
 * logarithmic time.
 *
 * It's also possible to look up the star by index or using a search
 * of stars with similar fluxes (using the star_indices vector and the
 * flux_map multimap).
 *
 * This database class is more or less as implemented in the original
 * openstartracker, but with clearer code and comments.
 */
class StarDatabase {
protected:

  std::unordered_map<hash_t,Star> hash_map; /* (--) hash from star hash value to star */
  std::set<hash_t> hash_set;                /* (--) sorted set of hash values */
  std::vector<hash_t> indices;              /* (--) vector of star indices to hash values */
  std::multimap<float,hash_t> flux_map;     /* (--) map star flux value to star hash value */
  
  float max_variance;                       /* (--) maximum position variance of all stars in the database */

public:
  // FIXME: This should not be public
  static size_t count;                      /* (--) number of active databases */
  
  StarDatabase(float max_variance_ = 0.0)
    : max_variance(max_variance_)
  {
    StarDatabase::count++;
  }

  ~StarDatabase()
  {
    StarDatabase::count--;
  }

  float get_max_variance() const { return max_variance; }

  size_t size() const {
    return hash_map.size();
  }

  /** @brief Checks to see if a star is in the database. */
  bool contains(const Star& star) {
    return hash_map.count(star.get_hash()) == 1;
  }


  /** @brief Add a star to the database (this version creates a copy).
   */
  StarDatabase& operator+=(const Star& star) {
    if (!contains(star)) {
      Star temp(star);
      temp.set_index(size());
      add_internal(temp);
    }
    return *this;
  }

  /** @brief Add a star to the database.
   */
  StarDatabase& operator+=(Star& star) {
    if (!contains(star)) {
      add_internal(star);
    }
    return *this;
  }

  /** @brief Load stars from a catalog
   *
   * @param filename   path to catalog file (e.g. hip_main.dat)
   * @param year       update star positions to the specified year
   */
  //void load(const char* filename, float year) {
  //  std::ifstream fin(filename, 'r');
  //}

  /** @brief Lookup a star by its hash
   *
   * @param hash  hash key
   *
   * @return A pointer to a Star in the hash.
   */
  Star* get_star_by_hash(const hash_t& hash) {
    return &(hash_map.at(hash));
  }

  /** @brief Get star by order of when it was added to the database
   *
   * @param index  order star was added
   *
   * @return A pointer to a Star in the hash.
   */
  Star* get_star(const star_id_t& index) {
    if (size() > 0) {
      return get_star_by_hash(indices[index]);
    } else {
      return NULL;
    }
  }


protected:
  /** @brief Add a star to the database without checking to see if it already exists.
   *
   * Helper function for operator+=.
   */
  void add_internal(Star& star) {
    if (max_variance < star.get_variance()) {
      max_variance = star.get_variance();
    }

    star.set_index(size());

    hash_t hash = star.get_hash();
    
    // Add to the hash
    hash_map.emplace(hash, star);

    // Add to the sorted set of hash keys
    hash_set.insert(hash);

    // Add the flux to the database
    flux_map.emplace(star.get_flux(), hash);

    // Also add the star by its index of addition
    indices.push_back(hash);
  }
};

#endif // STAR_DATABASE_HPP
