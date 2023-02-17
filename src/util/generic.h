#ifndef util_generic_h
#define util_generic_h

#include "common.h"

// dispatch signatures
#define BASE_DISPATCH(method)			\
  Val:val_##method,				\
  Obj*:obj_##method

#define IMM_DISPATCH(method)			\
  Real:real_##method,				\
  Int:int_##method,				\
  Bool:bool_##method,				\
  Glyph:glyph_##method

#define OBJ_DISPATCH(method)						\
  Bin*:bin_##method,							\
    Sym*:sym_##method,							\
    Stream*:stream_##method,						\
    Func*:func_##method,						\
    Cons*:cons_##method,						\
    Vec*:vec_##method,							\
    Table*:table_##method

#define OBJ2_DISPATCH(method)			\
  Obj*:obj_##method,				\
  OBJ_DISPATCH(method)

// generic declarations -------------------------------------------------------
#define GENERIC_2(method, dispatch, ...)				\
  generic((dispatch),							\
	  BASE_DISPATCH(method) )((dispatch) __VA_OPT__(,) __VA_ARGS__)

#define GENERIC_3(method, type, Type, dispatch, ...)			\
  generic((dispatch),							\
	  BASE_DISPATCH(method),					\
	  Type:type##_##method   )((dispatch) __VA_OPT__(,) __VA_ARGS__)

#define BASE_SIGS(m, rtn, ...)				\
  extern rtn m##_val(Val v __VA_OPT__(,) __VA_ARGS__);	\
  extern rtn m##_obj(Obj* o __VA_OPT__(,) __VA_ARGS__)

#define IMM_SIGS(m, rtn, ...)					\
  extern rtn m##_real(Real r __VA_OPT__(,) __VA_ARGS__);	\
  extern rtn m##_int(Int i __VA_OPT__(,) __VA_ARGS__);		\
  extern rtn m##_bool(Bool b __VA_OPT__(,) __VA_ARGS__);	\
  extern rtn m##_glyph(Glyph g __VA_OPT__(,) __VA_ARGS__)

#define OBJ_SIGS(m, rtn, ...)					\
  extern rtn m##_bin(Bin* b __VA_OPT__(,) __VA_ARGS__);		\
  extern rtn m##_sym(Sym* s __VA_OPT__(,) __VA_ARGS__);		\
  extern rtn m##_stream(Stream* s __VA_OPT__(,) __VA_ARGS__);	\
  extern rtn m##_func(Func* f __VA_OPT__(,) __VA_ARGS__);	\
  extern rtn m##_cons(Cons* c __VA_OPT__(,) __VA_ARGS__);	\
  extern rtn m##_vec(Vec* v __VA_OPT__(,) __VA_ARGS__);		\
  extern rtn m##_table(Table* t __VA_OPT__(,) __VA_ARGS__)

#define OBJ2_SIGS(m, rtn, ...)				\
  extern rtn m##_obj(Obj* o __VA_OPT__(,) __VA_ARGS__);	\
  OBJ_SIGS(m, rtn __VA_OPT__(,) __VA_ARGS__)

#endif
