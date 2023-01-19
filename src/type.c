#include "type.h"
#include "object.h"

// global dispatch tables
usize BaseSize[NUM_TYPES] = {
  [SymbolType]        = sizeof(struct Symbol),
  [FunctionType]      = sizeof(struct Object) + sizeof(struct Function),
  [ListType]          = sizeof(struct Object) + sizeof(struct List),
  [PairType]          = sizeof(struct Object) + sizeof(struct Pair),
  [TupleType]         = sizeof(struct Tuple),
  [StringType]        = sizeof(struct String),

  [ByteCodeType]      = sizeof(struct ByteCode),
  [NameSpcType]       = sizeof(struct Object) + sizeof(struct NameSpc),
  [EnvironType]       = sizeof(struct Object) + sizeof(struct Environ),
  [MethodType]        = sizeof(struct Object) + sizeof(struct Method),
  [UserMethodType]    = sizeof(struct Object) + sizeof(struct UserMethod),
  [NativeMethodType]  = sizeof(struct Object) + sizeof(struct NativeMethod)
};

uint16 Offset[NUM_TYPES] = {
    [SymbolType]        = sizeof(struct Symbol),
    [FunctionType]      = sizeof(struct Object),
    [ListType]          = sizeof(struct Object),
    [PairType]          = sizeof(struct Object),
    [TupleType]         = sizeof(struct Tuple),
    [StringType]        = sizeof(struct String),

    [ByteCodeType]      = sizeof(struct ByteCode),
    [NameSpcType]       = sizeof(struct Object),
    [EnvironType]       = sizeof(struct Object),
    [MethodType]        = sizeof(struct Object),
    [UserMethodType]    = sizeof(struct Object),
    [NativeMethodType]  = sizeof(struct Object)
};

extern void printReal(Value x);
extern void printSmall(Value x);
extern void printBool(Value x);
extern void printGlyph(Value x);
extern void printSymbol(Value x);
extern void printFunction(Value x);
extern void printList(Value x);
extern void printPair(Value x);
extern void printTuple(Value x);
extern void printString(Value x);
extern void printByteCode(Value x);
extern void printNameSpc(Value x);

void (*Print[NUM_TYPES])(Value x) = {
  [RealType]         = printReal,
  [SmallType]        = printSmall,
  [BoolType]         = printBool,
  [GlyphType]        = printGlyph,
  [SymbolType]       = printSymbol,
  [FunctionType]     = printFunction,
  [ListType]         = printList,
  [PairType]         = printPair,
  [TupleType]        = printTuple,
  [StringType]       = printString,
  [ByteCodeType]     = printByteCode,
  [NameSpcType]      = printNameSpc
};

// object model methods
extern usize allocSymbol(RlType type, void *args, void **dst);

extern usize allocList(RlType type, void *args, void **dst);
extern usize allocTuple(RlType type, void *args, void **dst);
extern usize allocString(RlType type, void *args, void **dst);
extern usize allocByteCode(RlType type, void *args, void **dst);

usize (*Alloc[NUM_TYPES])(RlType type, void *args, void **dst) = {
  [SymbolType]   = allocSymbol,
  [ListType]     = allocList,
  [TupleType]    = allocTuple,
  [StringType]   = allocString,
  [ByteCodeType] = allocByteCode
};

extern void initSymbol(void *self, ObjectInit *args);
extern void initFunction(void *self, ObjectInit *args);
extern void initList(void *self, ObjectInit *args);
extern void initPair(void *self, ObjectInit *args);
extern void init
