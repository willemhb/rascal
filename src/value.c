#include "util/hashing.h"

#include "object.h"
#include "value.h"

// generics
#include "tpl/describe.h"

ARRAY_TYPE(Values, Value, Value, false);

// external API
Value tagFloat(Float x) {
  return doubleToWord(x);
}

Value tagArity(Arity x) {
  return (x & VAL_MASK) | ARITY_TAG;
}

Value tagSmall(Small x) {
  return ((Value)x) | SMALL_TAG;
}

Value tagBoolean(Boolean x) {
  return x ? TRUE : FALSE;
}

Value tagGlyph(Glyph x) {
  return ((Value)x) | GLYPH_TAG;
}

Value tagObj(void* x) {
  return ((Value)x) | OBJ_TAG;
}


Type* typeOfVal(Value value) {
  Type* out;
  
  switch (value & TAG_MASK) {
    case ARITY_TAG: out = &ArityType;               break;
    case SMALL_TAG: out = &SmallType;               break;
    case NUL_TAG:   out = &UnitType;                break;
    case BOOL_TAG:  out = &BooleanType;             break;
    case GLYPH_TAG: out = &GlyphType;               break;
    case OBJ_TAG:   out = typeOfObj(AS_PTR(value)); break;
    default:        out = &FloatType;               break;
  }

  return out;
}

Type* typeOfObj(void* ptr) {
  assert(ptr != NULL);
  Obj* obj = ptr;
  Type* out;

  switch(obj->type) {
    case SYMBOL:       out = &SymbolType;      break;
    case FUNCTION:     out = &FunctionType;    break;
    case TYPE:         out = &TypeType;        break;
    case BINDING:      out = &BindingType;     break;
    case STREAM:       out = &StreamType;      break;
    case BIG:          out = &BigType;         break;
    case BITS:         out = &BitsType;        break;
    case LIST:         out = &ListType;        break;
    case VECTOR:{
      Vector* vec = ptr;
      out = vec->sType ? : &VectorType;
      break;
    }
    case MAP:{
      Map* map = ptr;
      out = map->rType ? : &MapType;
      break;
    }
    case METHOD_TABLE: out = &MethodTableType; break;
    case NATIVE:       out = &NativeType;      break;
    case CHUNK:        out = &ChunkType;       break;
    case CLOSURE:      out = &ClosureType;     break;
    case CONTROL:      out = &ControlType;     break;
    case NAMESPACE:    out = &NameSpaceType;   break;
    case UPVALUE:      out = &UpValueType;     break;
    case VEC_NODE:     out = &VecNodeType;     break;
    case VEC_LEAF:     out = &VecLeafType;     break;
    case MAP_NODE:     out = &MapNodeType;     break;
    case MAP_LEAF:     out = &MapLeafType;     break;
    default:           /* unreachable */       break;
  }

  return out;
}

bool equalValues(Value x, Value y) {
  if (x == y)
    return true;

  Type* xt = typeOf(x), * yt = typeOf(y);

  if (xt != yt || xt->code < SYMBOL)
    return false;

  return equalObjects(AS_OBJ(x), AS_OBJ(y));
}

static void printFloat(FILE* ios, Float num) {
  fprintf(ios, "%g", num);
}

static void printBoolean(FILE* ios, Boolean x) {
  fprintf(ios, x ? "true" : "false");
}

static void printUnit(FILE* ios, Value x) {
  (void)x;
  fprintf(ios, "nul");
}

static void printSymbol(FILE* ios, Symbol* x) {
  fprintf(ios, "%s", x->name);
}

static void printBits(FILE* ios, Bits* xs) {
  if (xs->obj.flags)
    fprintf(ios, "\"%s\"", (char*)xs->data);

  else {
    fprintf(ios, "<<");

    for (size_t i=0; i<xs->arity; i++) {
      uint8_t byte = ((uint8_t*)xs->data)[i];
      
      fprintf(ios, "%.3d", byte);

      if (i+1 < xs->arity)
        fprintf(ios, " ");
    }

    fprintf(ios, ">>");
  }
}

static void printList(FILE* ios, List* xs) {
  fprintf(ios, "(");
  
  for (; xs->arity > 0; xs=xs->tail ) {
    printValue(ios, xs->head);

    if (xs->arity > 1)
      fprintf(ios, " ");
  }

  fprintf(ios, ")");
}

static void printTerm(FILE* ios, Value x) {
  const char* tName = typeOf(x)->name->name;

  fprintf(ios, "#%s<%lx>", tName, x & VAL_MASK);
}

void printValue(FILE* ios, Value x) {
  switch (typeCode(x)) {
    case FLOAT:   printFloat(ios, AS_FLOAT(x)); break;
    case BOOLEAN: printBoolean(ios, AS_BOOL(x)); break;
    case UNIT:    printUnit(ios, x); break;
    case SYMBOL:  printSymbol(ios, AS_SYMBOL(x)); break;
    case BITS:    printBits(ios, AS_BITS(x)); break;
    case LIST:    printList(ios, AS_LIST(x)); break;
    default:      printTerm(ios, x); break;
  }
}

uint64_t hashValue(Value x) {
  Type* xt = typeOf(x);

  uint64_t out;

  if (xt->code < SYMBOL)
    out = mixHashes(xt->obj.hash, hashWord(x));

  else
    out = hashObject(AS_OBJ(x));

  return out;
}
