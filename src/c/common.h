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
// tagged value ---------------------------------------------------------------
typedef uword value_t;

// immediate types ------------------------------------------------------------
typedef double number_t;
typedef uword fixnum_t;
typedef int glyph_t;
typedef bool bool_t;
typedef FILE* port_t;
typedef value_t (*native_t)(usize n, value_t* args);
typedef void* pointer_t;

// common object type ---------------------------------------------------------
typedef struct object object_t;

// user object types ----------------------------------------------------------
typedef struct symbol symbol_t;
typedef struct list list_t;

// internal object types ------------------------------------------------------
typedef struct environment envt_t;
typedef struct namespace ns_t;
typedef struct chunk chunk_t;
typedef struct closure closure_t;
typedef struct control control_t;

// internal types (not objects) -----------------------------------------------
typedef struct values values_t;
typedef struct objects objects_t;
typedef struct buffer buffer_t;
typedef struct table table_t;
typedef enum token token_t;
typedef enum datatype datatype_t;
typedef struct frame frame_t;
typedef struct vm vm_t;

// miscellaneous macros -------------------------------------------------------
#define unreachable __builtin_unreachable
#define generic _Generic

#endif
