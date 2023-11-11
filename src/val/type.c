#include "util/hashing.h"

#include "environment.h"
#include "function.h"

#include "vm.h"

#include "type.h"

// external API
// accessors
Kind getKind(Type* type) {
  return type->obj.flags & 0x7;
}

// constructors
Type*   newStructType(Type* parent, Symbol* name, List* slots, Tuple* signature);
Type*   newRecordType(Type* parent, Symbol* name, List* slots, Tuple* signature);
Type*   newAbstractType(Type* parent, Symbol* name);
Type*   newUnionType(Type* parent, Symbol* name, size_t count, Type** members);

// utilities
Tuple*  rankTypes(Tuple* sig);
int     orderSigs(Tuple* sigx, Tuple* sigy);

// initialization
static void initializeBuiltinType(char* name, Type* type) {
  type->name       = symbol(name);
  type->obj.hash   = hashWord(type->idno);
  type->obj.hashed = true;

  if (type->ctor != NULL)
    type->ctor->name = type->name;

  Binding* binding = define(NULL, type->name, tag(type), false);

  setFl(binding, CONSTANT); 
}

void initializeBuiltinTypes(void) {
  // numeric
  extern Type FloatType, ArityType, SmallType, BigType,
    NumberType, RealType, RationalType, IntegerType;

  // mutable array types
  extern Type Buffer8Type, Buffer16Type, Buffer32Type, Binary8Type, Binary16Type,
    Binary32Type, AlistType, ObjectsType;

  // mutable table types
  extern Type SymbolTableType, TableType, NameSpaceType;

  // environment
  extern Type SymbolType, EnvironmentType, BindingType, UpValueType;

  // collections
  extern Type BitsType, StringType, TupleType, ListType, VectorType, VecNodeType,
    VecLeafType, MapType, MapNodeType, MapLeafType;

  // functions
  extern Type FunctionType, MethodTableType, MethodMapType, MethodNodeType, MethodType,
    NativeType, ClosureType;

  // execution & control
  extern Type ChunkType, ControlType;

  // miscellaneous
  extern Type PointerType, FuncPtrType, BooleanType, UnitType, GlyphType, StreamType;
  
  struct { char* name; Type* type; } types[] = {
    // fucked up
    [TOP]          = { "Any",         &AnyType         },
    [BOTTOM]       = { "None",        &NoneType        },
    [UNIT]         = { "Unit",        &UnitType        },
    [TERM]         = { "Term",        &TermType        },
    [TYPE]         = { "Type",        &TypeType        },

    // numeric
    [FLOAT]        = { "Float",       &FloatType       },
    [ARITY]        = { "Arity",       &ArityType       },
    [SMALL]        = { "Small",       &SmallType       },
    [BIG]          = { "Big",         &BigType         },
    [NUMBER]       = { "Number",      &NumberType      },
    [REAL]         = { "Real",        &RealType        },
    [RATIONAL]     = { "Rational",    &RationalType    },
    [INTEGER]      = { "Integer",     &IntegerType     },

    // mutable arrays
    [BUFFER8]      = { "Buffer8",     &Buffer8Type     },
    [BUFFER16]     = { "Buffer16",    &Buffer16Type    },
    [BUFFER32]     = { "Buffer32",    &Buffer32Type    },
    [BINARY8]      = { "Binary8",     &Binary8Type     },
    [BINARY16]     = { "Binary16",    &Binary16Type    },
    [BINARY32]     = { "Binary32",    &Binary32Type    },
    [ALIST]        = { "Alist",       &AlistType       },
    [OBJECTS]      = { "Objects",     &ObjectsType     },

    // mutable tables
    [SYMBOL_TABLE] = { "SymbolTable", &SymbolTableType },
    [TABLE]        = { "Table",       &TableType       },
    [NAME_SPACE]   = { "NameSpace",   &NameSpaceType   },

    // environment
    [SYMBOL]       = { "Symbol",      &SymbolType      },
    [ENVIRONMENT]  = { "Environment", &EnvironmentType },
    [BINDING]      = { "Binding",     &BindingType     },
    [UPVALUE]      = { "UpValue",     &UpValueType     },
 
    // collections
    [BITS]         = { "Bits",        &BitsType        },
    [STRING]       = { "String",      &StringType      },
    [TUPLE]        = { "Tuple",       &TupleType       },
    [LIST]         = { "List",        &ListType        },
    [VECTOR]       = { "Vector",      &VectorType      },
    [VEC_NODE]     = { "VecNode",     &VecNodeType     },
    [VEC_LEAF]     = { "VecLeaf",     &VecLeafType     },
    [MAP]          = { "Map",         &MapType         },
    [MAP_NODE]     = { "MapNode",     &MapNodeType     },
    [MAP_LEAF]     = { "MapLeaf",     &MapLeafType     },

    // functions
    [FUNCTION]     = { "Function",    &FunctionType    },
    [METHOD_TABLE] = { "MethodTable", &MethodTableType },
    [METHOD_MAP]   = { "MethodMap",   &MethodMapType   },
    [METHOD_NODE]  = { "MethodNode",  &MethodNodeType  },
    [METHOD]       = { "Method",      &MethodType      },
    [NATIVE]       = { "Native",      &NativeType      },
    [CLOSURE]      = { "Closure",     &ClosureType     },

    // runtime, execution & control
    [CHUNK]        = { "Chunk",       &ChunkType       },
    [CONTROL]      = { "Control",     &ControlType     },

    // miscellaneous
    [POINTER]      = { "Pointer",     &PointerType     },
    [FUNCPTR]      = { "FuncPtr",     &FuncPtrType     },
    [BOOLEAN]      = { "Boolean",     &BooleanType     },
    [GLYPH]        = { "Glyph",       &GlyphType       },
    [STREAM]       = { "Stream",      &StreamType      },
  };

  for (size_t i=0;i <= INTEGER; i++)
    initializeBuiltinType(types[i].name, types[i].type);
}
