#ifndef rascal_common_h
#define rascal_common_h

// utility typedefs -----------------------------------------------------------
typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

// common macros --------------------------------------------------------------
#define HEADER object_t base

#define tag(x)    ((x)&TMASK)
#define tagp(x,t) (((value_t)(x))|(t))
#define tagc(x,t) (((value_t)(x))|CHARACTER|(((value_t)(t))<<32))
#define pval(x)   ((void*)((x)&PMASK))
#define dval(x)   (((ieee64_t)(x)).fp)
#define cval(x)   ((int)((x)&CMASK))
#define uval(x)   ((uint)((x)&CMASK))
#define cflg(x)   (((x)>>32)&UINT16_MAX)

#define asa(ctype, x, cnvt)          ((ctype)cnvt(x))
#define getf(ctype, x, field)				\
  (_Generic((x),					\
	    value_t:((ctype)pval((value_t)(x))),	\
	    default:((ctype)(x))			\
	    )->field)

#define getf_s(type, x, field, func) (to##type(x, func)->field)

#define tag_p(type, TAG)      bool is_##type(value_t x) { return tag(x) == TAG; }
#define value_p(value, VALUE) bool is_##value(value_t x) { return x == VALUE; }
#define non_empty_tag_p(type, TAG)			\
  bool is_##type(value_t x)				\
  {							\
    return tag(x) == TAG && !(x&7) && !!(x&PMASK);	\
  }

#define ob_flag_p(flag, TAG, FLAG)			\
  bool is_##flag(value_t x)				\
  {							\
    return is_object(x)					\
      && tag(x) == TAG					\
      && !!(ob_flags(x)&FLAG);				\
  }

#define im_flag_p(flag, TAG, FLAG)		\
  bool is_##flag(value_t x)			\
  {						\
    return is_immediate(x)			\
      && tag(x) == TAG				\
      && !!(cflg(x)&FLAG);			\
  }

#define flag_p(flag, TAG, FLAG)			\
  bool is_##flag(value_t x)			\
  {						\
    if (tag(x) != TAG)				\
      return false;				\
    else if (is_immediate(x))			\
      return !!(cflg(x)&FLAG);			\
    else					\
      return !!(ob_flags(x)&FLAG);		\
  }

#define safe_cast(ctype, type, cnvt)		\
  ctype to##type(value_t x, char *fname)	\
  {						\
    require(is##type(x),			\
	    x,					\
	    fname,				\
	    "expected type %s, got ",		\
	    #type,				\
	    x );				\
    return (ctype)cnvt(x);			\
  }

#define safe_cast_ptr(ctype, type, cnvt)	\
  ctype to##type(value_t x, char *fname)	\
  {						\
    require(is##type(x) && is_object(x),	\
	    x,					\
	    fname,				\
	    "expected reference to a %s, got ",	\
	    #type,				\
	    x );				\
    return (ctype)cnvt(x);			\
  }


#endif
