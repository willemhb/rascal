#ifndef rl_tpl_decl_generic_h
#define rl_tpl_decl_generic_h

#define TYPE(T)        T##_t
#define HEAD(T)        T##_head_t
#define STRUCT(T)      struct TYPE(T)
#define STRUCT_HEAD(T) struct HEAD(T)
#define METHOD(T, M) T##_##M

#define GENERIC_CALL_2( m, t1, t2, x, ...)                           \
  _Generic((x),                                                      \
           TYPE(t1):METHOD(t1, m),                                   \
           TYPE(t2):METHOD(t2, m))((x) __VA_OPT__(,) __VA_ARGS__)

#define GENERIC_CALL_2_PLUS( m, t1, t2, x, ...)                         \
  _Generic((x),                                                         \
           TYPE(t1):METHOD(t1, m),                                      \
           TYPE(t2):METHOD(t2, m),                                      \
           default:METHOD(fallback, m))((x) __VA_OPT__(,) __VA_ARGS__)

#define GENERIC_CALL_3( m, t1, t2, t3, x, ...)                       \
  _Generic((x),                                                     \
           TYPE(t1):METHOD(t1, m),                                  \
           TYPE(t2):METHOD(t2, m),                                  \
           TYPE(t3):METHOD(t3, m))((x) __VA_OPT__(,) __VA_ARGS__)

#define GENERIC_CALL_4(m, t1, t2, t3, t4, x, ...)                   \
  _Generic((x),                                                     \
           TYPE(t1):METHOD(t1, m),                                  \
           TYPE(t2):METHOD(t2, m),                                  \
           TYPE(t3):METHOD(t3, m),                                  \
           TYPE(t4):METHOD(t4, m))((x) __VA_OPT__(,) __VA_ARGS__)

#define GENERIC_CALL_5(m, t1, t2, t3, t4, t5, x, ...)               \
  _Generic((x),                                                     \
           TYPE(t1):METHOD(t1, m),                                  \
           TYPE(t2):METHOD(t2, m),                                  \
           TYPE(t3):METHOD(t3, m),                                  \
           TYPE(t4):METHOD(t4, m),                                  \
           TYPE(t5):METHOD(t5, m))((x) __VA_OPT__(,) __VA_ARGS__)

#define GETF_2( f, cnvt, t1, t2, x )            \
  (GENERIC_CALL_2(cnvt, t1, t2, x)->f)
#define GETF_3( f, cnvt, t1, t2, t3, x )        \
  (GENERIC_CALL_3(cnvt, t1, t2, t3, x)->f)
#define GETF_4( f, cnvt, t1, t2, t3, t4, x )    \
  (GENERIC_CALL_4(cnvt, t1, t2, t3, t4, x)->f)
#define GETF_5( f, cnvt, t1, t2, t3, t4, t5, x )    \
  (GENERIC_CALL_5(cnvt, t1, t2, t3, t4, t5, x)->f)

#define GENERIC_ISA(type)                       \
  bool val_is_##type(val_t val);                \
  bool obj_is_##type(obj_t val)

#define GENERIC_ASA(type)                       \
  TYPE(type) val_as_##type(val_t val);          \
  TYPE(type) obj_as_##type(obj_t obj)

#define GENERIC_HEAD(type)                          \
  STRUCT_HEAD(type) val_##type##_head(val_t val);   \
  STRUCT_HEAD(type) obj_##type##_head(obj_t obj)

#endif
