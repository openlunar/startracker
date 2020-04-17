#ifndef STAR_HPP
# define STAR_HPP

#include <string>

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

  // Fast accessors for focal plane array position
  float px() const { return p[0]; }
  float py() const { return p[1]; }

  float get_flux() const { return flux; }

  int get_id() const { return id; }


  /** @brief Comparison operator */
  bool operator==(const Star& rhs) const {
    return hash == rhs.hash;
  }

  /** @brief Numerically stable method to calculate distance between stars
   *
   * @param rhs  star to get distance from
   *
   * @return Angular separation in radians
   */
  float distance(const Star& rhs) const {
    float a = r[0] * rhs.r[1] - rhs.r[0] * r[1];
    float b = r[0] * rhs.r[2] - rhs.r[0] * r[2];
    float c = r[1] * rhs.r[2] - rhs.r[1] * r[2];

    return asin(sqrt(a*a + b*b + c*c));
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


#endif // STAR_HPP
