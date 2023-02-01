#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* value types (set of distinct first class representation) */
typedef uintptr_t           Value;  // unitype
typedef uintptr_t           FixNum; // 48-bit unsigned integer
typedef double              Real;
typedef int                 Small;
typedef bool                Bool;
typedef ascii               Glyph;
typedef struct Object       Object;

/* object types */
// user object types
typedef struct Symbol      *Symbol;
typedef struct List        *List;
typedef struct Pair        *Pair;
typedef struct Tuple       *Tuple;
typedef struct Map         *Map;
typedef struct String      *String;
typedef struct Binary      *Binary;
typedef struct Port        *Port;

// internal object types
typedef struct MapNode     *MapNode;
typedef struct Chunk       *Chunk;
typedef struct Closure     *Closure;
typedef struct Native      *Native;
typedef struct SpecialForm *SpecialForm;
typedef struct UpValue     *UpValue;

/* internal struct types */
typedef struct Objects Objects;
typedef struct Values  Values;

/* internal enum types */
typedef enum ValType   ValType;
typedef enum ObjType   ObjType;
typedef enum RlType    RlType;
typedef enum OpCode    OpCode;
typedef enum RlError   RlError;
typedef enum ReadState ReadState;

/* global runtime & interpreter objects */
extern struct Reader      Reader;           // essential reader state
extern struct Compiler    Compiler;         // essential compiler state
extern struct Interpreter Interpreter;      // essential execution state
extern struct Heap        Heap;             // memory state
extern struct Errors      Errors;           // error and synchronization state

#endif
