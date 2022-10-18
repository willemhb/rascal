#ifndef rascal_globals_h
#define rascal_globals_h

// all globals used by the vm go here
#include "rl/value.h"
#include "obj/type.h"
#include "vm/memory.h"
#include "rl/read.h"

// types and type objects
// type lookup table
type_t *BuiltinTypes[N_REPR];

// primitive types
type_t *SymbolType, *ConsType, *FunctionType, *StreamType;
dtype_t SymbolTypeOb, ConsTypeOb, FunctionTypeOb, StreamTypeOb;

// internal types
type_t *ControlType, *EntryType, *MappingType, *VariableType, *SlotType, *SyntaxType,
  *DispatchType;

// record types
type_t *BytecodeType, *ClosureType, *EnvironmentType, *NamespaceType, *ModuleType,
  *ReaderType, *EnvironmentType, *ReaderType, *RatioType, *ComplexType, *TypeType,
  *DataType, *UnionType, *ClassType;

// array types
type_t *StringType, *AlistType, *StackType, *OpcodesType, *BytesType, *TextType;

// table types
type_t *SlotsType, *SyntaxesType, *DispatchesType, *VariablesType, *MembersType,
  *ConstantsType, *MethodsType, *SymbolsType, *GensymsType, *BackrefsType;

// hamt types
type_t *VectorType, *DictType, *SetType;

// cvalue types
// bignums
type_t *UInt64Type, *Int64Type, *BigIntType;

// native functions
type_t *IsaType, *HasType, *ThunkType, *UnaryType, *BinaryType, *TernaryType,
  *NaryType, *CompareType, *HashofType, *SizeofType, *ReadType, *PrintType,
  *InternType, *ConstructType, *ObInitType, *TraceType, *FreeType, *ResizeType,
  *CVInitType, *UnboxType, *InvokeType;

// immediate types
type_t *BooleanType, *PrimitiveType, *CharacterType, *OpcodeType, *ReprType, *CtypeType;

// big immediate types
type_t *RealType, *FixnumType, *PointerType;

// fucked up types
type_t *AnyType, *NoneType, *NulType;

static inline void init_types( void )
{
  // set pointers to correct backing object
  
  BuiltinTypes[RECORD]    = NULL;
  BuiltinTypes[ARRAY]     = NULL;
  BuiltinTypes[HAMT]      = NULL;
  BuiltinTypes[CVALUE]    = NULL;
  BuiltinTypes[SYMBOL]    = SymbolType;
  BuiltinTypes[CONS]      = ConsType;
  BuiltinTypes[FUNCTION]  = StreamType;
  BuiltinTypes[CONTROL]   = ControlType;
  BuiltinTypes[ENTRY]     = EntryType;
  BuiltinTypes[MAPPING]   = MappingType;
  BuiltinTypes[VARIABLE]  = VariableType;
  BuiltinTypes[SLOT]      = SlotType;
  BuiltinTypes[SYNTAX]    = DispatchType;
  BuiltinTypes[BOOLEAN]   = BooleanType;
  BuiltinTypes[PRIMITIVE] = PrimitiveType;
  BuiltinTypes[CHARACTER] = CharacterType;
  BuiltinTypes[NUL]       = NulType;
  BuiltinTypes[REPR]      = ReprType;
  BuiltinTypes[CTYPE]     = CtypeType;
  BuiltinTypes[POINTER]   = PointerType;
  BuiltinTypes[FIXNUM]    = FixnumType;
  BuiltinTypes[REAL]      = RealType;
}



#endif
