#include "util/hashing.h"

#include "type.h"

// external API
// accessors
Kind getKind(Type* type);

// constructors
Type*   newStructType(Type* parent, Symbol* name, List* slots, Tuple* signature);
Type*   newRecordType(Type* parent, Symbol* name, List* slots, Tuple* signature);
Type*   newAbstractType(Type* parent, Symbol* name);
Type*   newUnionType(Type* parent, Symbol* name, size_t count, Type** members);

// utilities
Tuple*  rankTypes(Tuple* sig);
int     orderSigs(Tuple* sigx, Tuple* sigy);

// initialization
#include "vm.h"
#include "environment.h"

void initializeBuiltinTypes(void) {
  // numeric
  extern Type FloatType, ArityType, SmallType, BigType,
    NumberType, RealType, RationalType, IntegerType;

  // environment
  extern Type SymbolType, ScopeType, EnvironmentType, BindingType, UpValueType;

  // collections
  extern Type BitsType, StringType, TupleType, ListType, VectorType, VecNodeType,
    VecLeafType, MapType, MapNodeType, MapLeafType;

  // functions
  extern Type FunctionType, MethodTableType, MethodMapType, MethodNodeType, MethodType,
    NativeType, ClosureType;

  // execution & control
  extern Type ChunkType, ControlType;

  // miscellaneous
  extern Type BooleanType, UnitType, GlyphType, StreamType;
  
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

    // environment
    [SYMBOL]       = { "Symbol",      &SymbolType      },
    [SCOPE]        = { "Scope",       &ScopeType       },
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
    [BOOLEAN]      = { "Boolean",     &BooleanType     },
    [GLYPH]        = { "Glyph",       &GlyphType       },
    [STREAM]       = { "Stream",      &StreamType      },
  };

  for (size_t i=0;i <= INTEGER; i++) {
    char* name = types[i].name;
    Type* type = types[i].type;

    type->name       = symbol(name);
    type->obj.hash   = hashWord(type->idno);
    type->obj.hashed = true;

    defineGlobal(&RlVm, type->name, tag(&type), CONSTANTP);
  }
}
