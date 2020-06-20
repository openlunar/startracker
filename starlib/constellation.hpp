#ifndef CONSTELLATION_HPP
# define CONSTELLATION_HPP

#include "types.hpp"
#include "star.hpp"

class Constellation {
protected:
  constellation_id_t index;
  Star* stars[2];
  float d;
  
public:
  Constellation(size_t index_, Star* s1, Star* s2, const float& distance)
    : index(index_)
    , stars{s1, s2}
    , d(distance)
  { }

  Constellation(size_t index_, Star* s1, Star* s2)
    : Constellation(index_, s1, s2, s1->approximate_distance(*s2))
  { }

  float get_distance() const {
    return d;
  }
  
  constellation_id_t get_index() const {
    return index;
  }

  Star* const get_star(size_t ii) const {
    if (ii > 2) return nullptr;
    return stars[ii];
  }
  

  bool operator<(const Constellation& rhs) const {
    if (d == rhs.d) {
      if (stars[0]->get_hash() == rhs.stars[0]->get_hash()) {
	return stars[1]->get_hash() < rhs.stars[1]->get_hash();
      } else {
	return stars[0]->get_hash() < rhs.stars[0]->get_hash();
      }
    } else {
      return d < rhs.d;
    }
  }
  
};


bool constellation_ptr_less(const Constellation* const lhs,
			    const Constellation* const rhs)
{
  return *lhs < *rhs;
}

bool constellation_less(const Constellation& lhs,
			const Constellation& rhs)
{
  return lhs < rhs;
}



#endif // CONSTELLATION_HPP
