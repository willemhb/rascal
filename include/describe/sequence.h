#ifndef rascal_describe_sequence_h
#define rascal_describe_sequence_h

// utilities for working lists and such
#define for_cons(c, x)					\
  for (;consp(*c) && ((x=car(*c))||1); *c = cdr(*c))

#define for_vec(v, i, x)				\
  for (i=0;i<vlen(*v) && ((x=vdata(*v)[(i)])||1); i++)

#endif
