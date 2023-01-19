#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* value types */
typedef uword   Value;  // unitype
typedef double  Real;
typedef int32_t Small;
typedef bool    Bool;
typedef ascii   Glyph;
typedef uchar  *Object;

/* object types */
/* user objects */
typedef ascii           *Symbol;
typedef struct Function *Function;
typedef struct List     *List;
typedef struct Pair     *Pair;
typedef Value           *Tuple;
typedef Glyph           *String;

/* internal object types */
typedef ushort              *ByteCode;
typedef struct NameSpc      *NameSpc;
typedef struct Environ      *Environ;
typedef struct Method       *Method;
typedef struct UserMethod   *UserMethod;
typedef struct NativeMethod *NativeMethod;

/* builtin types */
typedef enum {
  NoType,             // not a type (used as flag when a type is missing or optional)
  RealType,
  SmallType,
  UnitType,
  BoolType,
  GlyphType,
  SymbolType,
  FunctionType,
  ListType,
  PairType,
  TupleType,
  StringType,
  ByteCodeType,
  NameSpcType,
  EnvironType,
  MethodType,
  UserMethodType,
  NativeMethodType,
  NoneType,
  AnyType
} RlType;

#define NUM_TYPES (AnyType+1)

typedef uintptr_t RlTag;

typedef enum {
  NoError,
  ReadError,
  EvalError,
  ApplyError,
  CompileError,
  RuntimeError,
  SystemError
} RlError;

/* internal types */
typedef struct Reader      Reader;
typedef struct Compiler    Compiler;
typedef struct Interpreter Interpreter;

extern struct Reader      RlReader;
extern struct Compiler    RlCompiler;
extern struct Interpreter RlInterpreter;

typedef struct ObjectInit ObjectInit;

/* vm function pointer types */
typedef RlError (*ReadFn)(Reader *state, char dispatch);
typedef RlError (*CompFn)(Compiler* state, Value form);
typedef RlError (*EvalFn)(Interpreter *state, List args);
typedef RlError (*ExecFn)(Interpreter *state, Value *args, int nArgs);

/* dispatch methods for core operations/object model */
typedef void  (*PrintFn)(Value x);
typedef void *(*AllocFn)(ObjectInit *args);
typedef void  (*InitFn)(void *self, ObjectInit *args);
typedef void  (*FreeFn)(Object self);

#endif
