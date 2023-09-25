#include "util/hashing.h"

#include "object.h"
#include "value.h"

// array types
#include "tpl/describe.h"

ARRAY_TYPE(Values, Value, Value);

void printValues(FILE* ios, Values* values) {
  fprintf(ios, "Contents of values: \n");
  for (size_t i=0; i<values->count; i++) {
    fprintf(ios, "    ");
    printValue(ios, values->data[i], -1);
    fprintf(ios, "\n");
  }
}

Type valueType(Value value) {
  switch (value & TAG_MASK) {
    case NUL_TAG:  return UNIT;
    case BOOL_TAG: return BOOLEAN;
    case OBJ_TAG:  return OBJECT;
    default:       return NUMBER;
  }
}

Type rascalType(Value value) {
  Type type = valueType(value);

  if (type == OBJECT)
    type = objectType(AS_OBJ(value));

  return type;
}

Type objectType(Obj* object) {
  assert(object != NULL);

  return object->type;
}

size_t sizeOfType(Type type) {
  size_t out;
  
  switch (type) {
    case NUMBER:  out = sizeof(Number);  break;
    case BOOLEAN: out = sizeof(Boolean); break;
    case UNIT:    out = sizeof(Value);   break;
    case SYMBOL:  out = sizeof(Symbol);  break;
    case BITS:    out = sizeof(Bits);    break;
    case LIST:    out = sizeof(List);    break;
    case TUPLE:   out = sizeof(Tuple);   break;
      /*
        case MAP:     out = sizeof(Map);     break;
        case NODE:    out = sizeof(Node);    break;
        case LEAF:    out = sizeof(Leaf);    break;
        case CHUNK:   out = sizeof(Chunk);   break;
        case CLOSURE: out = sizeof(Closure); break;
        case UPVALUE: out = sizeof(UpValue); break;
        case NATIVE:  out = sizeof(Native);  break;
        case STREAM:  out = sizeof(Stream);  break; */
    default:      out = 0;               break;
  }

  return out;
}

char* nameOfType(Type type) {
  char* out;
  
  switch (type) {
    case NUMBER:  out = "Number";  break;
    case BOOLEAN: out = "Boolean"; break;
    case UNIT:    out = "Unit";    break;
    case SYMBOL:  out = "Symbol";  break;
    case BITS:    out = "Bits";    break;
    case LIST:    out = "List";    break;
    case TUPLE:   out = "Tuple";   break;
      /*
        case MAP:     out = "Map";     break;
        case NODE:    out = "Node";    break;
        case LEAF:    out = "Leaf";    break;
        case CHUNK:   out = "Chunk";   break;
        case CLOSURE: out = "Closure"; break;
        case UPVALUE: out = "Upvalue"; break;
        case NATIVE:  out = "Native";  break;
        case STREAM:  out = "Stream";  break; */
    default:      out = "Term";    break;
  }

  return out;
}

bool equalValues(Value x, Value y) {
  if (x == y)
    return true;

  Type xt = rascalType(x), yt = rascalType(y);

  if (xt != yt || xt < SYMBOL)
    return false;

  return equalObjects(AS_OBJ(x), AS_OBJ(y));
}

static void printNumber(FILE* ios, Number num, int indent) {
  indent = max(indent, 0);
  fprintf(ios, "%.*s%g", indent, "  ", num);
}

static void printBoolean(FILE* ios, Boolean x, int indent) {
  indent = max(indent, 0);
  fprintf(ios, "%.*s%s", indent, "  ", x ? "true" : "false");
}

static void printUnit(FILE* ios, Value x, int indent) {
  (void)x;
  indent = max(indent, 0);
  fprintf(ios, "%.*s%s", indent, "  ", "nul");
}

static void printSymbol(FILE* ios, Symbol* x, int indent) {
  indent = max(indent, 0);
  fprintf(ios, "%.*s:%s", indent, "  ", x->name);
}

static void printBits(FILE* ios, Bits* xs, int indent) {
  indent = max(indent, 0);
  if (xs->obj.flags)
    fprintf(ios, "%.*s\"%s\"", indent, "  ", (char*)xs->data);

  else {
    fprintf(ios, "%.*s<<", indent, "  ");

    for (size_t i=0; i<xs->arity; i++) {
      uint8_t byte = ((uint8_t*)xs->data)[i];
      
      fprintf(ios, "%.3d", byte);

      if (i+1 < xs->arity)
        fprintf(ios, ",");
    }

    fprintf(ios, ">>");
  }
}

static void printList(FILE* ios, List* xs, int indent) {
  bool pretty = indent > -1; indent = max(0, indent);
  char* sep = pretty ? "\n" : " ", *term = pretty ? "\n" : "";
  int childIndent = pretty ? indent+1 : -1;

  if (xs->arity == 0)
    fprintf(ios, "%.*s[]", indent, " ");

  else {
    fprintf(ios, "%.*s[%s", indent, " ", term);

    for (; xs->arity; xs=xs->tail ) {
      printValue(ios, xs->head, childIndent);
      
    if (xs->arity > 1)
      fprintf(ios, ",%s", sep);
    }

    fprintf(ios, "%.*s]%s", indent, " ", term);
  }
}

static void printTuple(FILE* ios, Tuple* xs, int indent) {
  bool pretty = indent > -1; indent = max(indent, 0);
  char* sep = pretty ? "\n" : " ", *term = pretty ? "\n" : "";
  int childIndent = pretty ? indent+1 : -1;

  if (xs->arity == 0)
    fprintf(ios, "%.*s()", indent, "  ");

  else if (xs->arity == 1) {
    fprintf(ios, "%.*s(%s", indent, "  ", term);
    printValue(ios, xs->data[0], childIndent);
    fprintf(ios, "%.*s,)%s", indent, "  ", term);
  } else {
    fprintf(ios, "%.*s(%s", indent, "  ", term);

    for (size_t i=0; i<xs->arity; i++) {
      printValue(ios, xs->data[i], childIndent);

      if (i+1 < xs->arity)
        fprintf(ios, ",%s", sep);
    }

    fprintf(ios, "%.*s)%s", indent, "  ", term);
  }
}

static void printTerm(FILE* ios, Value x, int indent) {
  indent = max(indent, 0);
  const char* tName = nameOfType(rascalType(x));

  fprintf(ios, "%.*s#%s<%lx>", indent, "  ", tName, x & VAL_MASK);
}

void printValue(FILE* ios, Value x, int indent) {
  switch (rascalType(x)) {
    case NUMBER:  printNumber(ios, AS_NUM(x), indent); break;
    case BOOLEAN: printBoolean(ios, AS_BOOL(x), indent); break;
    case UNIT:    printUnit(ios, x, indent); break;
    case SYMBOL:  printSymbol(ios, AS_SYMBOL(x), indent); break;
    case BITS:    printBits(ios, AS_BITS(x), indent); break;
    case LIST:    printList(ios, AS_LIST(x), indent); break;
    case TUPLE:   printTuple(ios, AS_TUPLE(x), indent); break;
    default:      printTerm(ios, x, indent); break;
  }
}

uint64_t hashValue(Value x) {
  Type xt = rascalType(x);

  uint64_t out;

  if (xt < SYMBOL)
    out = mixHashes(hashType(xt), hashWord(x));

  else
    out = hashObject(AS_OBJ(x));

  return out;
}

uint64_t hashType(Type type) {
  static uint64_t cache[NUM_TYPES] = {};

  return cache[type] ? : (cache[type] = hashWord(type));
}
