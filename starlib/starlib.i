%include <attribute.i>

%module starlib
%{
#include "star.hpp"
#include "star_database.hpp"
#include "kdtree.hpp"

size_t StarDatabase::count = 0;
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

   
%extend KDTree {
%pythoncode {
  def __getitem__(self, index):
    if index >= 0 and index < self.size:
      return self.at(index)
    else:
      raise KeyError("out of bounds")
}};
   


   //%attribute(ConstellationDatabase, size_t, size, size);

   //%include "constellation_database.hpp"
