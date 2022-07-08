#ifndef describe_object_h
#define describe_object_h

/* helper macros for boilerplate implementing object types */

#define DeclareConstruct(_type)						\
  object_t *construct_##_type(type_t t, flags_t f, size_t n, size_t s )

#define DeclareInit(_type)						\
  int_t init_##_type(type_t xt, flags_t f, size_t n, size_t s, value_t i, void *spc )

#define DeclareTrace(_type)			\
  value_t trace_##_type( value_t x, flags_t f )

#define DeclareRelocate(_type)						\
  void relocate_##_type( value_t *b, uchar_t **s, uchar_t **m, size_t *u )

#define DeclarePrint(_type)			\
  size_t print_##_type( FILE *ios, value_t x )

#define DeclareHash(_type)			\
  hash_t hash_##_type( value_t x )

#define DeclareSizeof(_type)			\
  size_t sizeof_##_type( value_t x )

#define DeclareFinalize(_type)			\
  void   finalize_##_type( object_t *ob )

#endif
