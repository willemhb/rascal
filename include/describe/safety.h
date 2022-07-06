#ifndef rascal_safety_h
#define rascal_safety_h

#define TagP(_tag)							\
  bool_t ob##_tag##p( object_t *ob )					\
  {									\
    return tag_##_tag == tag_object ||					\
      tag_##_tag > tag_immediate &&					\
      ob != NULL &&							\
      obtag(ob) == tag_##_tag;						\
  }									\
  									\
  bool_t val##_tag##p( value_t val )					\
  {									\
    if (tag_##_tag < tag_moved)						\
      return tag(val) == tag_##_tag;					\
    return tag(val) != tag_immediate &&					\
      val != rnull &&							\
      obtag(val) == tag_##_tag;						\
  }

#define DeclareTagP(_tag)			\
  bool_t ob##_tag##p( object_t *ob );		\
  bool_t val##_tag##p( value_t val )

#define GenericTagP(_tag, _x)				\
  _Generic((_x),					\
	   value_t:val##_tag##p,			\
	   object_t*:ob##_tag##p)(_x)

#define TypeP(_type)				\
  bool_t ob##_type##p( object_t *ob )		\
  {						\
    if (type_##_type == type_null)		\
      return ob == NULL;			\
    if (ob == NULL)				\
      return false;				\
    if ((type_##_type && 0x3) == tag_immediate)	\
      return false;				\
    return obtype(ob) == type_##_type;		\
  }						\
  bool_t val##_type##p( value_t val )		\
  {						\
    if (val&1)					\
      return (val&255) == type_##_type;		\
    if (val == rnull)				\
      return type_##_type == type_null;		\
    return obtype( val ) == type_##_type;	\
  }

#define DeclareTypeP(_type)			\
  bool_t ob##_type##p( object_t *ob );		\
  bool_t val##_type##p( value_t val )

#define GenericTypeP(_type, _x)			\
  _Generic((_x),				\
	   object_t*:ob##_type##p,		\
	   value_t:val##_type##p)(_x)

#define SafeCast(_type, _ctype, _cnvt)					\
  _ctype valto##_type(value_t val)					\
  {									\
    require(rtypeof(val) == type_##_type,				\
	    (value_t)val,						\
	    "expected type %s, got ",					\
	    #_type );							\
    return (_ctype)_cnvt(bal);						\
  }									\
  _ctype obto##_type(object_t *ob)					\
  {									\
    require(rtypeof(ob) == type_##_type,				\
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
