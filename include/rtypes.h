#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>
#include "common.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

typedef struct object_t   object_t;

typedef struct symbol_t   symbol_t;
typedef struct pair_t     pair_t;
typedef struct tuple_t    tuple_t;
typedef struct string_t   string_t;
typedef struct binary_t   binary_t;
typedef struct table_t    table_t;
typedef struct function_t function_t;
typedef struct port_t     port_t;
typedef struct error_t    error_t;

typedef uint_t   integer_t;
typedef bool_t   boolean_t;
typedef char_t   character_t;

typedef tuple_t  vector_t;

typedef table_t  dict_t;
typedef table_t  set_t;

typedef enum  type_t        type_t;
typedef enum  symfl_t       symfl_t;
typedef enum  memfl_t       memfl_t;
typedef enum  obfl_t        obfl_t;
typedef enum  pairfl_t      pairfl_t;
typedef enum  builtin_t     builtin_t;
typedef enum  form_t        form_t;
typedef enum  opcode_t      opcode_t;
typedef union instruction_t instruction_t;

typedef value_t   (*Cbuiltin_t)( value_t *args, arity_t nargs );
typedef value_t   (*ensure_t)( value_t *args, arity_t nargs );

typedef int_t     (*init_t)(type_t x, uint_t f, size_t n, size_t s, value_t i, void *spc );
typedef object_t* (*construct_t)(type_t t, uint_t f, size_t n, size_t s );
typedef value_t   (*trace_t)( value_t xv );
typedef void      (*relocate_t)(value_t *b, uchar_t **s, uchar_t **m, size_t *u );
typedef void      (*untrace_t)(value_t val);
typedef size_t    (*print_t)(FILE *ios, value_t val);
typedef hash_t    (*mk_hash_t)(value_t val);
typedef size_t    (*sizeof_t)(value_t val);
typedef void      (*finalize_t)(object_t *ob);

#endif
