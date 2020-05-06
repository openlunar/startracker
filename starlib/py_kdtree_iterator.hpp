#ifndef PY_KDTREE_ITERATOR_HPP
# define PY_KDTREE_ITERATOR_HPP

#include "kdtree.hpp"

struct KDTreeIterator {
  KDTree *list;
  size_t pos;
};

#endif
