#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

// value types and tags
typedef uword                 value_t;

// core object types are defined here
typedef struct object_t       object_t;

// immediate types
typedef void                 *nul_t;
typedef bool                  bool_t;
typedef double                real_t;
typedef value_t             (*primitive_t)(value_t *args, size_t nargs);

// object types
typedef struct type_t         type_t;
typedef struct cons_t         cons_t;
typedef struct vector_t       vector_t;
typedef struct instructions_t instructions_t;
typedef struct atom_t         atom_t;
typedef struct lambda_t       lambda_t;
typedef struct control_t      control_t;
typedef struct closure_t      closure_t;
typedef struct environment_t  environment_t;

// globals
extern type_t TypeType, NulType, BoolType, RealType, PrimitiveType, ConsType,
  VectorType, InstructionsType, AtomType, LambdaType, ControlType, ClosureType,
  EnvironmentType;

#endif
