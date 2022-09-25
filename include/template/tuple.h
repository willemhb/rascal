#ifndef rascal_tuple_template_h
#define rascal_tuple_template_h

#define TUPLE(T, V)				\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    obj_t *spec;				\
    V slots[0];					\
  }


#endif
