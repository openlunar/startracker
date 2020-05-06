%include <attribute.i>
%include <exception.i>

%module starlib
%{
#include "star.hpp"
#include "star_database.hpp"
#include "kdtree.hpp"
#include "py_kdtree_iterator.hpp"

size_t StarDatabase::count = 0;
static int kdtree_iterator_error = 0;
%}

// FIXME: This next line is a bit fragile
%include "types.hpp"
%apply unsigned long long { hash_t }

// Getter/setter methods for Star
%attribute(Star, float, px, px);
%attribute(Star, float, py, py);
%attribute(Star, float, x, x);
%attribute(Star, float, y, y);
%attribute(Star, float, z, z);
%attribute(Star, float, flux, get_flux);
%attribute(Star, star_id_t, index, get_index, set_index);
%attribute(Star, int, id, get_id);
%attribute(Star, hash_t, hash, get_hash);
%attribute(Star, float, variance, get_variance);
%attribute(Star, bool, unreliable, get_unreliable);

%extend Star {
%pythoncode {
  def __repr__(self): return "Star({}, {}, r=({}, {}, {}), p=({}, {}), flux={}, index={}, variance={}, unreliable={})".format(self.id, self.hash, self.x, self.y, self.z, self.px, self.py, self.flux, self.index, self.variance, self.unreliable)
}};

%include "star.hpp"

%attribute(StarDatabase, size_t, size, size);
%attribute(StarDatabase, float, max_variance, get_max_variance);

%include "star_database.hpp"
   
%extend StarDatabase {
%pythoncode {
       def __repr__(self): return "StarDatabase(size={}, max_variance={})".format(self.size, self.max_variance)
}};

%attribute(KDTree, size_t, size, size);

%include "kdtree.hpp"
%include "py_kdtree_iterator.hpp"

%extend KDTreeIterator {
  KDTreeIterator* __iter__() {
    return $self;
  }

  // This next bit is in pythoncode because to return StopIteration,
  // it needs to return a PyObject* instead of a Star*, and I'm not
  // quite sure how to write that in swiggable C++ without getting a
  // segfault.
%pythoncode %{
  def __next__(self):
    if self.pos >= self.list.size:
      raise StopIteration()
    ret = self.list.at(self.pos)
    self.pos += 1
    return ret
%}
}

%exception KDTree::__getitem__ {
  assert(!kdtree_iterator_error);
  $action
  if (kdtree_iterator_error) {
    kdtree_iterator_error = 0; // clear flag
    SWIG_exception(SWIG_IndexError, "index out of bounds");
  }
}

%exception KDTreeIterator::__next__ {
  assert(!kdtree_iterator_error);
  $action
  if (kdtree_iterator_error) {
    kdtree_iterator_error = 0; // clear flag
    PyErr_SetNone(PyExc_StopIteration, "end of list");
  }
}


%extend KDTree {
  Star* __getitem__(size_t index) {
    if (index >= $self->size() || index < 0) {
      kdtree_iterator_error = 1;
      return NULL;
    }
    return $self->at(index);
  }

  KDTreeIterator __iter__() {
    KDTreeIterator ret = { $self, 0 };
    return ret;
  }

%pythoncode %{
  def filter_catalog(self):
    for star in self:
      print("star index = {}".format(star.index))
%}

};


