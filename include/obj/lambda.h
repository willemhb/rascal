#ifndef rascal_function_h
#define rascal_function_h

#include "table.h"

/* C types */
/* lexical environment representation */
typedef enum
  {
   variable_name=1,
   macro_name   =2,
   function_name=3,
  } name_type_t;

struct namespace_t
{
  OBJHEAD;

  namespace_t *parent;
  table_t     *locals;
};

/* user function representation */
struct lambda_t
{
  OBJHEAD;

  namespace_t *namespace;   // lexical environment
  vector_t    *constants;   // constant store
  bytecode_t  *bytecode;    // instruction sequence
};

/* apis and utilities */
/* lambda */
lambda_t *make_lambda( void );
void      init_lambda( lambda_t *lambda, namespace_t *namespace );

/* runtime interaction with constant store */
value_t   lmb_get_value( lambda_t *lambda, size_t n );
value_t   lmb_get_constant( lambda_t *lambda, size_t n );
value_t   lmb_set_constant( lambda_t *lambda, size_t n, value_t x );

/* namespace */
namespace_t *make_namespace( void );
void         init_namespace( namespace_t *namespace, namespace_t *next );
void         free_namespace( namespace_t *namespace );

bool         ns_lookup( namespace_t *namespace, size_t *i, size_t *j, object_t **macro );
void         ns_defvar( namespace_t *namespace, value_t name );
void         ns_defmac( namespace_t *namespace, value_t name );


/* convenience macros */
#define is_lambda( x ) value_is_type(x, &LambdaType)
#define as_lambda( x ) ((lambda_t*)as_obj(x))

#endif
