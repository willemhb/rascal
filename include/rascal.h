#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* tagged value type */
typedef uword value_t;

/* immediate types */
typedef double real_t;
typedef FILE  *stream_t;

/* object types */
/* generic object type (more type information before header) */
typedef uchar *object_t;

/* generic array type (extended object) */
typedef uchar *array_t;

/* vm object types */
typedef struct lambda_data_t *lambda_t;
typedef struct control_data_t *control_t;
typedef struct type_data_t *type_t;

/* array types */
typedef value_t *vector_t;
typedef ushort *bytecode_t;
typedef char *buffer_t;

/* internal structure types (no rascal representation) */
typedef struct vm_t  vm_t;
typedef struct reader_t reader_t;

#endif
