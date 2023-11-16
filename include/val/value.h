#ifndef rascal_val_value_h
#define rascal_val_value_h

#include "common.h"

/* C types */

/* Value types. Every  value is either a 64-bit double or a
   tagged word that can be safely cast to one of the types below. */
typedef int          Small;
typedef bool         Boolean;
typedef char32_t     Glyph;
typedef void*        Pointer;
typedef funcptr_t    FuncPtr;
typedef double       Float;
typedef struct Obj   Obj;

// core user object types
typedef struct Big    Big;
typedef struct Symbol Symbol;
typedef struct Type   Type;
typedef struct Func   Func;
typedef struct List   List;
typedef struct String String;
typedef struct Binary Binary;
typedef struct Vector Vector;
typedef struct Dict   Dict;
typedef struct Set    Set;
typedef struct Stream Stream;

// utility mutable object types
typedef struct MutList  MutList;
typedef struct MutDict  MutDict;
typedef struct MutSet   MutSet;
typedef struct MutVec   MutVec;
typedef struct MutStr   MutStr;
typedef struct MutBin   MutBin;

// internal object types
typedef struct Objects   Objects;
typedef struct Envt      Envt;
typedef struct Chunk     Chunk;
typedef struct Closure   Closure;
typedef struct Native    Native;
typedef struct MethTable MethTable;
typedef struct Binding   Binding;
typedef struct UpValue   UpValue;
typedef struct Seq       Seq;

/* Globals */
extern struct MutDict ValueMeta;

// tags and such
#define SYS_TAG     0x7ffc000000000000UL // internal value with no external representation
#define SMALL_TAG   0x7ffd000000000000UL
#define BOOL_TAG    0x7ffe000000000000UL
#define NUL_TAG     0x7fff000000000000UL
#define GLYPH_TAG   0xfffc000000000000UL
#define PTR_TAG     0xfffd000000000000UL
#define FPTR_TAG    0xfffe000000000000UL
#define OBJ_TAG     0xffff000000000000UL

#define TAG_MASK    0xffff000000000000UL
#define VAL_MASK    0x0000ffffffffffffUL
#define SMALL_MASK  0x00000000ffffffffUL

#define TRUE        (BOOL_TAG  | 1UL)
#define FALSE       (BOOL_TAG  | 0UL)
#define NUL         (NUL_TAG   | 0UL)
#define NOTHING     (SYS_TAG   | 1UL) // invalid value marker
#define TOMBSTONE   (SYS_TAG   | 2ul) // indicates deleted table entry
#define ZERO        (SMALL_TAG | 0UL)
#define ONE         (SMALL_TAG | 1UL)
#define NULL_OBJ    (OBJ_TAG   | 0UL)

/* external APIs */
// utility macros
#define untag48(v)    ((v) & VAL_MASK)
#define untag32(v)    ((v) & SMALL_MASK)
#define tag_of(x)     ((x) & TAG_MASK)
#define has_tag(x, t) (tag_of(x) == (t))
#define as(T, c, x)   ((T)c(x))

// forward declarations & generics
extern Type*  type_of_obj(void* obj);
extern size_t size_of_obj(void* obj);
extern bool   has_type_obj(void* obj, Type* type);
extern void   mark_obj(void* obj);
extern void   trace_obj(void* obj);
extern bool   get_mfl_obj(void* obj, flags_t mfl);
extern bool   set_mfl_obj(void* obj, flags_t mfl);
extern bool   del_mfl_obj(void* obj, flags_t mfl);
extern bool   get_fl_obj(void* obj, flags_t fl);
extern bool   set_fl_obj(void* obj, flags_t fl);
extern bool   del_fl_obj(void* obj, flags_t fl);
extern Dict*  get_meta_dict_obj(void* obj);
extern Dict*  set_meta_dict_obj(void* obj, Dict* meta);
extern Value  get_meta_obj(void* obj, Value key);
extern Value  set_meta_obj(void* obj, Value key, Value val);
extern Dict*  join_meta_obj(void* obj, Dict* meta);

Type*  type_of_val(Value val);
size_t size_of_val(Value val);
bool   has_type_val(Value val, Type* type);
void   mark_val(Value val);
void   trace_val(Value val);
bool   get_mfl_val(Value val, flags_t mfl);
bool   set_mfl_val(Value val, flags_t mfl);
bool   del_mfl_val(Value val, flags_t mfl);
bool   get_fl_val(Value val, flags_t fl);
bool   set_fl_val(Value val, flags_t fl);
bool   del_fl_val(Value val, flags_t fl);
Dict*  get_meta_dict_val(Value val);
Dict*  set_meta_dict_val(Value val, Dict* meta);
Value  get_meta_val(Value val, Value key);
Value  set_meta_val(Value val, Value key, Value kval);
Dict*  join_meta_val(Value val, Dict* meta);

#define type_of(x)          generic2(type_of, x, x)
#define size_of(x)          generic2(size_of, x, x)
#define has_type(x, T)      generic2(has_type, x, x, T)
#define get_mfl(x, fl)      generic2(get_mfl, x, x, fl)
#define set_mfl(x, fl)      generic2(set_mfl, x, x, fl)
#define del_mfl(x, fl)      generic2(del_mfl, x, x, fl)
#define get_fl(x, fl)       generic2(get_fl, x, x, fl)
#define set_fl(x, fl)       generic2(set_fl, x, x, fl)
#define del_fl(x, fl)       generic2(del_fl, x, x, fl)
#define get_meta_dict(x)    generic2(get_meta_dict, x, x)
#define set_meta_dict(x, d) generic2(set_meta_dict, x, x, d)
#define get_meta(x, k)      generic2(get_meta, x, x, k)
#define set_meta(x, k, v)   generic2(set_meta, x, x, k, v)
#define join_meta(x, d)     generic2(join_meta, x, x, d)

Value tag_float(Float f);
Value tag_small(Small s);
Value tag_bool(Boolean b);
Value tag_glyph(Glyph gl);
Value tag_ptr(Pointer p);
Value tag_fptr(FuncPtr f);
Value tag_obj(void* obj);

#define tag(x)                                  \
  generic((x),                                  \
          Float:tag_float,                      \
          Small:tag_small,                      \
          Boolean:tag_bool,                     \
          Glyph:tag_glyph,                      \
          Pointer:tag_ptr,                      \
          FuncPtr:tag_fptr,                     \
          default:tag_obj)(x)

#endif
