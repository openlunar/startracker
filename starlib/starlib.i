%include <attribute.i>

%module starlib
%{
#include "star.hpp"
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
%attribute(Star, hash_t, hash, get_hash);
%attribute(Star, float, variance, get_variance);
%attribute(Star, bool, unreliable, get_unreliable);

%include "star.hpp"

