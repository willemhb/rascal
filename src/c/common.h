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
typedef uword value_t;
typedef double number_t;
typedef char glyph_t;
typedef FILE* port_t;
typedef void* pointer_t;
typedef struct object object_t;

typedef struct symbol symbol_t;
typedef struct cons cons_t;
typedef struct binary binary_t;
typedef struct vector vector_t;
typedef struct table table_t;
typedef struct record record_t;
typedef struct function function_t;
typedef struct method_table method_table_t;
typedef struct method method_t;
typedef struct type type_t;
typedef struct chunk chunk_t;
typedef struct control control_t;

// internal types -------------------------------------------------------------
typedef value_t (*native_t)(usize n, value_t* args);
typedef enum error error_t;
typedef enum token token_t;
typedef struct vm vm_t;

// miscellaneous macros -------------------------------------------------------
#define unreachable __builtin_unreachable

#endif
