#ifndef rascal_rtypes_h
#define rascal_rtypes_h

#include <stdio.h>
#include "common.h"

// rascal typedefs ------------------------------------------------------------
typedef uintptr_t value_t;

typedef struct object_t   object_t;

// s-expression types
typedef struct symbol_t   symbol_t;

// pair types
typedef struct pair_t     pair_t;
typedef struct assc_t     assc_t;
typedef struct envt_t     envt_t;

// array types
typedef struct vector_t   vector_t;     // mutable, boxed
typedef struct tuple_t    tuple_t;      // immutable, boxed
typedef struct binary_t   binary_t;     // immutable, any binary type
typedef struct string_t   string_t;     // immutable, any character type, encoded
typedef struct buffer_t   buffer_t;     // mutable, any binary type, maybe encoded
typedef struct bytecode_t bytecode_t;

// mapping types
typedef union mapping_t mapping_t;
typedef union table_t   table_t;
typedef union dict_t    dict_t;
typedef union set_t     set_t;

// internal types
typedef struct closure_t  closure_t;
typedef struct port_t     port_t;
typedef struct error_t    error_t;

// immediate types
typedef uint_t   integer_t;
typedef bool_t   boolean_t;
typedef char_t   character_t;
typedef ulong_t  fixnum_t;

// builtin flag and enum types
typedef enum  type_t        type_t;
typedef enum  symfl_t       symfl_t;
typedef enum  memfl_t       memfl_t;
typedef enum  mapfl_t       mapfl_t;
typedef enum  obfl_t        obfl_t;
typedef enum  pairfl_t      pairfl_t;
typedef enum  funfl_t       funfl_t;

typedef enum  builtin_t     builtin_t;
typedef enum  form_t        form_t;
typedef enum  opcode_t      opcode_t;
typedef union instruction_t instruction_t;

// function pointer types
typedef value_t   (*Cbuiltin_t)( value_t *args, arity_t nargs );
typedef value_t   (*ensure_t)( value_t *args, arity_t nargs );

typedef int_t     (*init_t)( type_t t, flags_t f, size_t n, value_t i, void *spc );
typedef object_t* (*construct_t)( type_t t, flags_t f, size_t n );
typedef value_t   (*relocate_t)( value_t xv );
typedef value_t   (*trace_t)( value_t xv );
typedef size_t    (*print_t)( FILE *ios, value_t val );
typedef hash_t    (*mk_hash_t)( value_t val );
typedef size_t    (*sizeof_t)( value_t val );
typedef void      (*finalize_t)( object_t *ob );
typedef int_t     (*order_t)( value_t x, value_t y );

// called when a value with a primitive key must be added to a table
typedef value_t   (*intern_t)( char_t *k, size_t s, hash_t h, index_t l, object_t *t );

#endif
