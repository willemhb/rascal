 #ifndef common_h
#define common_h

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// standard types -------------------------------------------------------------
typedef uint8_t     ubyte;
typedef uint32_t    uint;
typedef uint16_t    ushort;
typedef uintptr_t   uword;
typedef uintptr_t   uhash;
typedef size_t      usize;
typedef void      (*funcptr)(void);
typedef uint32_t    flags;

typedef uint8_t  uint8;
typedef int8_t   sint8;
typedef uint16_t uint16;
typedef int16_t  sint16;
typedef uint32_t uint32;
typedef int32_t  sint32;
typedef uint64_t uint64;
typedef int64_t  sint64;

// rascal types ---------------------------------------------------------------
typedef uword           value_t;
typedef double          number_t;
typedef char            glyph_t;
typedef FILE*           port_t;
typedef struct object   object_t;

typedef struct symbol   symbol_t;
typedef struct cons     cons_t;
typedef struct binary   binary_t;
typedef struct vector   vector_t;
typedef struct table    table_t;
typedef struct function function_t;

// internal types -------------------------------------------------------------
typedef struct control  control_t;
typedef struct toplevel toplevel_t;
typedef struct heap     heap_t;
typedef struct reader   reader_t;
typedef struct error    error_t;

typedef struct vm       vm_t;

// miscellaneous macros -------------------------------------------------------
#define unreachable __builtin_unreachable

#endif
