#ifndef STAR_HPP
# define STAR_HPP

#include <string>
#include <iostream>

#include "types.hpp"
#include "kdhash.hpp"

/** @brief Star database entry
 *
 * Abstracted from stars.h in openstartracker.
 */
class Star {
protected:
  int id;               /* (--) Hipparcos catalog ID */
  float r[3];           /* (--) star position in equatorial coordinate system, normalized */
  float p[2];           /* (--) px and py */
  float flux;           /* (--) */
  bool unreliable;      /* (--) */
  star_id_t index;      /* (--) how many stars were inserted before this one */
  float variance;       /* (--) variance */
  hash_t hash;          /* (--) value in the hash table */

public:

  /** @brief Create star from catalog
   */
  Star(const float& pixel_x_tangent, // from CameraConfig
       const float& pixel_y_tangent, // from CameraConfig
       const float& position_variance, // from CameraConfig
       const float& x_,
       const float& y_,
       const float& z_,
       const float& flux_,
       const int& id_,
       bool unreliable_ = false)
    : id(id_)
    , r{x_, y_, z_}
    , p{y_ / (x_ * pixel_x_tangent), z_ / (x_ * pixel_y_tangent)}
    , flux(flux_)
    , unreliable(unreliable_)
    , index(-1)
    , variance(position_variance)
    , hash(kdhash_3f::hash(x_, y_, z_))
  {
  }

  /** @brief Create star from image, using camera frame coordinates
   *
   * @param px_   (px) x position relative to camera center
   * @param py_   (px) y position relative to camera center
   * @param flux_ brightness of pixel
   */
  Star(const float& pixel_x_tangent, // from CameraConfig
       const float& pixel_y_tangent, // from CameraConfig
       const float& image_variance,  // from CameraConfig
       const float& px_,
       const float& py_,
       const float& flux_,
       const int& id_,
       bool unreliable_ = false)
    : id(id_)
    , p{px_, py_}
    , flux(flux_)
    , unreliable(unreliable_)
    , index(-1)
    , variance(image_variance / flux_)
  {
    // Convert from camera frame into celestial sphere coordinates (I think)
    float j = pixel_x_tangent * p[0]; // j = y / x
    float k = pixel_y_tangent * p[1]; // k = z / x

    r[0] = 1.0 / sqrt(j * j + k * k + 1);
    r[1] = j * r[0];
    r[2] = k * r[0];

    hash = kdhash_3f::hash(r[0], r[1], r[2]);
  }


  /*  Star(const Star& rhs)
    : r{rhs.r[0], rhs.r[1], rhs.r[2]}
    , flux(rhs.flux)
    , id(rhs.id)
    , p{rhs.p[0], rhs.p[1]}
    , unreliable(rhs.unreliable)
    , index(rhs.index)
    , variance(rhs.variance)
    , hash(rhs.hash)
    {} */
  

  // Fast accessors for position
  float x() const { return r[0]; }
  float y() const { return r[1]; }
  float z() const { return r[2]; }
  float get_r(size_t index) const { return r[index]; }

  // Fast accessors for focal plane array position
  float px() const { return p[0]; }
  float py() const { return p[1]; }
  float get_p(size_t index) const { return p[index]; }

  float get_flux() const { return flux; }

  int get_id() const { return id; }


  /** @brief Comparison operator */
  bool operator==(const Star& rhs) const {
    return hash == rhs.hash;
  }

  
  /** @brief Numerically stable method to calculate distance between
   **        stars, assumes a small angle approximation.
   *
   * @param rhs  star to get distance from
   *
   * @return Angular separation in radians
   */
  float approximate_distance(const Star& rhs) const {
    return approximate_distance(rhs.r[0], rhs.r[1], rhs.r[2]);
  }


  /** @brief "Numerically stable method to calculate distance between
   **        stars" from openstartracker.
   *
   * This is dist_arcsec in openstartracker, but this version uses
   * radians instead.
   */
  float approximate_distance(const float& x, const float& y, const float& z) const {
    float dot_product = r[0] * x + r[1] * y + r[2] * z;

    float a = r[0] * y - x * r[1];
    float b = r[0] * z - x * r[2];
    float c = r[1] * z - y * r[2];

    if (dot_product >= 0) {
      return asin(sqrt(a * a + b * b + c * c));
    } else {
      std::cerr << "Warning: Angles are too far apart. distance() is unstable; use exact_distance()." << std::endl;
      return M_PI - asin(sqrt(a * a + b * b + c * c));
    }
  }
  

  /** @brief Numerically stable angular separation method.
   *
   * This method assumes the star's position vector is a unit vector
   * and that <x, y, z> are normalized.
   *
   * The angle between vectors $\mathrm{a}$ and $\mathrm{b}$ is given by
   *
   *    \theta = 2 \arctan( \frac{ \| \mathrm{a} b - a \mathrm{b} \| }{\| \mathrm{a} b + a \mathrm{b} \| }
   *
   * However, we are using unit vectors, so we can simplify this expression quite a bit:
   *
   *    \theta = 2 \arctan( \frac{ \| \mathrm{a} - \mathrm{b} \| }{ \| \mathrm{a} + \mathrm{b} \| }
   *
   * Reference:
   *
   * * Kahan, W. 2006. How futile are mindless assessments of roundoff
   *   in floating-point computation. p. 47. Retrieved from
   *   https://people.eecs.berkeley.edu/~wkahan/Mindless.pdf on 24
   *   April 2020.
   */
  float exact_distance(const float& x, const float& y, const float& z) const {
    float amag = sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
    float bmag = sqrt(x * x + y * y + z * z);

    float nx = r[0] * bmag - amag * x;
    float ny = r[1] * bmag - amag * y;
    float nz = r[2] * bmag - amag * z;

    float dx = r[0] * bmag + amag * x;
    float dy = r[1] * bmag + amag * y;
    float dz = r[2] * bmag + amag * z;

    return 2.0 * atan( sqrt(nx * nx + ny * ny + nz * nz) / sqrt(dx * dx + dy * dy + dz * dz) );
  }


  float exact_distance(const Star& rhs) const {
    return exact_distance(rhs.r[0], rhs.r[1], rhs.r[2]);
  }


  /** @brief Get the squared length of the vector from this star to some point.
   */
  float vector_squared_distance(const float& x, const float& y, const float& z) const {
    float dx = x - r[0];
    float dy = y - r[1];
    float dz = z - r[2];

    return dx * dx + dy * dy + dz * dz;
  }

  
  float vector_squared_distance(const Star& rhs) const {
    return vector_squared_distance(rhs.r[0], rhs.r[1], rhs.r[2]);
  }

  float vector_distance(const float& x, const float& y, const float& z) const {
    return sqrt(vector_squared_distance(x, y, z));
  }

  float vector_distance(const Star& rhs) const {
    return sqrt(vector_squared_distance(rhs));
  }


  void set_index(const star_id_t& new_index) {
    index = new_index;
  }

  star_id_t get_index() const {
    return index;
  }

  float get_variance() const {
    return variance;
  }

  hash_t get_hash() const {
    return hash;
  }

  bool get_unreliable() const {
    return unreliable;
  }
  

  /* Some piece-wise comparison functions */
  bool rx_greater_than(const Star& rhs) const { return r[0] > rhs.r[0]; }
  bool ry_greater_than(const Star& rhs) const { return r[1] > rhs.r[1]; }
  bool rz_greater_than(const Star& rhs) const { return r[2] > rhs.r[2]; }
  bool flux_greater_than(const Star& rhs) const { return flux > rhs.flux; }
  bool rx_less_than(const Star& rhs) const { return r[0] < rhs.r[0]; }
  bool ry_less_than(const Star& rhs) const { return r[1] < rhs.r[1]; }
  bool rz_less_than(const Star& rhs) const { return r[2] < rhs.r[2]; }
  bool flux_less_than(const Star& rhs) const { return flux < rhs.flux; }  
};


bool star_ptr_rx_less(const Star* const lhs, const Star* const rhs) {
  return lhs->x() < rhs->x();
}

bool star_ptr_ry_less(const Star* const lhs, const Star* const rhs) {
  return lhs->y() < rhs->y();
}

bool star_ptr_rz_less(const Star* const lhs, const Star* const rhs) {
  return lhs->z() < rhs->z();
}

bool star_ptr_flux_greater(const Star* const lhs, const Star* const rhs) {
  return lhs->get_flux() > rhs->get_flux();
}


bool star_ptr_unique_flux_greater(const Star* const lhs, const Star* const rhs) {
  if (lhs->get_flux() == rhs->get_flux()) {
    return lhs < rhs;
  } else {
    return star_ptr_flux_greater(lhs, rhs);
  }
}


class UniqueStarPtrFluxGreater {
public:
  bool operator()(const Star* const lhs, const Star* const rhs) const {
    return star_ptr_unique_flux_greater(lhs, rhs);
  }
};


#endif // STAR_HPP
