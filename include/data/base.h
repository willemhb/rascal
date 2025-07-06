#ifndef rl_data_base_h
#define rl_data_base_h

/* Globals, macros, and core APIs for Rascal data.

   Includes templates for implementing simple dynamic array and table types. */

// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------
// Expression tags
#define QNAN    0x7ffc000000000000ul

#define NUL_T   0x7ffd000000000000ul
#define BOOL_T  0x7ffe000000000000ul
#define GLYPH_T 0x7fff000000000000ul
#define FIX_T   0xfffc000000000000ul
#define OBJ_T   0xfffd000000000000ul

// masks
#define XTMSK  0xffff000000000000ul
#define XVMSK  0x0000fffffffffffful

// utility macros
#define exp_tag(x) ((x) & XTMSK)
#define exp_val(x) ((x) & XVMSK)

// limits
#define NUM_TYPES (EXP_MAP_LEAF+1)

// common object head macro
#define HEAD                                     \
  Obj* heap;                                     \
  ExpType type;                                  \
  union {                                        \
    flags_t bfields;                             \
    struct {                                     \
      flags_t black   :   1;                     \
      flags_t gray    :   1;                     \
      flags_t nosweep :   1;                     \
      flags_t flags    : 29;                     \
    };                                           \
  }

// array template
#define ALIST_API(A, X, a)                      \
  typedef struct {                              \
    X* vals;                                    \
    int count, max_count;                       \
  } A;                                          \
                                                \
  void init_##a(A* a);                          \
  void free_##a(A* a);                          \
  void grow_##a(A* a);                          \
  void shrink_##a(A* a);                        \
  void resize_##a(A* a, int n);                 \
  void a##_push(A* a, X x);                     \
  X    a##_pop(A* a);                           \
  void a##_write(A* a, X* xs, int n)

ALIST_API(Exprs, Expr, exprs);
ALIST_API(Objs, void*, objs);
ALIST_API(Bin16, ushort_t, bin16);

// table template macro
#define TABLE_API(T, K, V, t)                                    \
  typedef struct {                                               \
    K key;                                                       \
    V val;                                                       \
  } T##KV;                                                       \
                                                                 \
  typedef struct {                                               \
    T##KV* kvs;                                                  \
    int count, max_count;                                        \
  } T;                                                           \
                                                                 \
  typedef void (*T##InternFn)(T* t, T##KV* kv, K k, hash_t h);   \
                                                                 \
  void init_##t(T* t);                                           \
  void free_##t(T* t);                                           \
  bool t##_get(T* t, K k, V* v);                                 \
  bool t##_set(T* t, K k, V v);                                  \
  bool t##_del(T* t, K k, V* v);                                 \
  V    t##_intern(T* t, K k, T##InternFn ifn)

TABLE_API(Strings, char*, Str*, strings);
TABLE_API(EnvMap, Sym*, Ref*, env_map);
TABLE_API(ExpMap, Val, Val, exp_map);

#define head(x)   ((Obj*)as_obj(x))
#define is_obj(x) (exp_tag(x) == OBJ_T)

// C types --------------------------------------------------------------------
// Expression type codes
typedef enum {
  // Immediate types ----------------------------------------------------------
  EXP_NUL=1,
  EXP_NUM,
  EXP_BOOL,
  EXP_GLYPH,
  
  // Vm types -----------------------------------------------------------------
  EXP_CHUNK,
  EXP_REF,
  EXP_ENV,
  EXP_UPV,

  // Language types -----------------------------------------------------------
  EXP_SYM,
  EXP_PORT,
  EXP_FUN,

  // Immutable aggregate types ------------------------------------------------
  EXP_TUPLE,
  EXP_LIST,
  EXP_MAP,
  EXP_STR,
  EXP_BIN,

  // Mutable aggregate types --------------------------------------------------
  EXP_MUT_TUPLE,
  EXP_MUT_LIST,
  EXP_MUT_MAP,
  EXP_MUT_STR,
  EXP_MUT_BIN,

  // Internal types -----------------------------------------------------------
  EXP_LIST_NODE,
  EXP_LIST_LEAF,
  EXP_MAP_NODE,
  EXP_MAP_LEAF,
} ExpType;

// Internal function pointer types --------------------------------------------
typedef void   (*PrintFn)(Port* p, Expr x);
typedef hash_t (*HashFn)(Expr x);
typedef bool   (*EgalFn)(Expr x, Expr y);
typedef size_t (*SizeFn)(void* ob);
typedef void*  (*AllocFn)(ExpType type, flags_t flags, size_t n);
typedef void   (*CloneFn)(void* ob); // called to clone object's owned pointers
typedef void   (*TraceFn)(void* ob);
typedef void   (*FreeFn)(void* ob);

// internal type information struct
typedef struct {
  PrintFn print_fn;
  HashFn  hash_fn;
  EgalFn  egal_fn;
} ExpAPI;

typedef struct {
  size_t  obsize;
  SizeFn  size_fn;
  AllocFn alloc_fn;
  CloneFn clone_fn;
  TraceFn trace_fn;
  FreeFn  free_fn;
} ObjAPI;

typedef struct {
  ExpType type;
  char*   c_name;
  Sym*    rl_name;
  ExpAPI* exp_api;
  ObjAPI* obj_api;
} TypeInfo;

// memory flags
typedef enum {
  FL_BLACK   = 0x80000000,
  FL_GRAY    = 0x40000000,
  FL_NOSWEEP = 0x20000000
} ExpFlags;

// Common Object head type
struct Obj {
  HEAD;
};

// function prototypes --------------------------------------------------------
// expression API -------------------------------------------------------------
ExpType   exp_type(Expr x);
bool      has_type(Expr x, ExpType t);
TypeInfo* type_info(Expr x);
ExpAPI*   exp_api(Expr x);
ObjAPI*   obj_api(void* ptr);
void      mark_exp(Expr x);

// object API -----------------------------------------------------------------
void*  as_obj(Expr x);
Expr   tag_obj(void* ptr);
void*  mk_obj(ExpType type, flags_t flags, size_t n);
size_t obj_size(void* ptr);
void*  clone_obj(void* ptr);
void   mark_obj(void* ptr);
void   unmark_obj(void* ptr);
void   free_obj(void *ptr);

// miscellaneous array helpers ------------------------------------------------
void trace_exprs(Exprs* xs);
void trace_objs(Objs* os);

// globals --------------------------------------------------------------------
extern TypeInfo Types[];

// initialization -------------------------------------------------------------

#endif
