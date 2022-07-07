#ifndef rascal_safety_h
#define rascal_safety_h

#define TypeP(x, type) isap(x, type_##type )

#define ObFlagP( _name, _flag, _linkage )	\
  _linkage##_ObFlagP( _name, _flag )

#define Static_ObFlagP( _name, _flag )			\
  static inline bool_t v##_name##p( value_t x )		\
  {							\
    return flagp( x, _flag );				\
  }							\
  static inline bool_t o##_name##p( object_t *o )	\
  {							\
    return flagp( o, _flag );				\
  }

#define Auto_ObFlagP( _name, _flag )			\
  inline bool_t v##_name##p( value_t x )		\
  {							\
    return flagp( x, _flag );				\
  }							\
  inline bool_t o##_name##p( object_t *o )		\
  {							\
    return flagp( o, _flag );				\
  }

#define DeclareObFlagP( _name )			\
  bool_t v##_name##p( value_t x );		\
  bool_t o##_name##p( object_t *ob )

#define GenericObFlagP( x, _name )		\
  _Generic((x),					\
	   object_t*:o##_name##p,		\
	   value_t:v##_name##p)(x)

#define SafeCast(_type, _ctype, _cnvt)					\
  _ctype valto##_type(value_t val)					\
  {									\
    require( TypeP( val, _type ),					\
	    (value_t)val,						\
	    "expected type %s, got ",					\
	    #_type );							\
    return (_ctype)_cnvt(bal);						\
  }									\
  _ctype obto##_type(object_t *ob)					\
  {									\
    require(TypeP( val, _type ),					\
	    (value_t)ob,						\
	    "expected type %s, got ",					\
	    #_type );							\
    return (_ctype)_cnvt(ob);						\
  }

#define SafeGet(_x, _type, _field) (to##_type(_x)->_field)

#define DeclareSafeCast(_type, _ctype)		\
  _ctype valto##_type(value_t val);		\
  _ctype obto##_type(object_t *ob)

#define GenericSafeCast(_type, _x)		\
  Generic((_x),					\
	  value_t: valto##_type,		\
	  object_t*: obto##_type)(_x)

#endif
