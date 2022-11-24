#ifndef rascal_tpl_array_h
#define rascal_tpl_array_h

// common array header
#define ARRAY(V)				\
  OBJHEAD;					\
  V      *data;					\
  size_t  capacity;				\
  size_t  count

#endif
