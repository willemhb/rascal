#ifndef rascal_array_h
#define rascal_array_h


#define TUPLE(T, V)				\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    obj_t *spec;				\
    V slots[0];					\
  }


#endif
