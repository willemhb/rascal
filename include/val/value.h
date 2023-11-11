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
typedef struct Vector Vector;
typedef struct Dict   Dict;
typedef struct Set    Set;
typedef struct Stream Stream;

// utility mutable object types
typedef struct MutDict  MutDict;
typedef struct MutSet   MutSet;
typedef struct Alist    Alist;
typedef struct Objects  Objects;
typedef struct Buffer8  Buffer8;
typedef struct Binary16 Binary16;

// internal object types
typedef struct Envt      Envt;
typedef struct Chunk     Chunk;
typedef struct Closure   Closure;
typedef struct Native    Native;
typedef struct MethTable MethTable;
typedef struct Binding   Binding;
typedef struct UpValue   UpValue;

/* Globals */
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
#define untag_48(v)   ((v) & VAL_MASK)
#define untag_32(v)   ((v) & SMALL_MASK)

#define tag_of(x)     ((x) & TAG_MASK)
#define tag_val(p, t) (((Value)(p)) | (t))
#define tag_small(s)  tag_val(s,  SMALL_TAG)
#define tag_glyph(g)  tag_val(g,  GLYPH_TAG)
#define tag_obj(o)    tag_val(o,  OBJ_TAG)
#define tag_fptr(fp)  tag_val(fp, FPTR_TAG)
#define tag_ptr(p)    tag_val(p,  PTR_TAG)

#define as(T, v, c) ((T)c(v))

// forward declarations
Type* type_of(Value x);
void  mark_val(Value x);
void  trace_val(Value x);

#endif
