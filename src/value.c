#include "util/hashing.h"

#include "object.h"
#include "value.h"

// array types
#include "tpl/describe.h"

ARRAY_TYPE(Values, Value, Value, false);

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
    case NATIVE:  out = sizeof(Native);  break;
    case CHUNK:   out = sizeof(Chunk);   break;
    case BITS:    out = sizeof(Bits);    break;
    case LIST:    out = sizeof(List);    break;
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
    case NATIVE:  out = "Native";  break;
    case CHUNK:   out = "Chunk";   break;
    case BITS:    out = "Bits";    break;
    case LIST:    out = "List";    break;
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

static void printNumber(FILE* ios, Number num) {
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
  const char* tName = nameOfType(rascalType(x));

  fprintf(ios, "#%s<%lx>", tName, x & VAL_MASK);
}

void printValue(FILE* ios, Value x) {
  switch (rascalType(x)) {
    case NUMBER:  printNumber(ios, AS_NUM(x)); break;
    case BOOLEAN: printBoolean(ios, AS_BOOL(x)); break;
    case UNIT:    printUnit(ios, x); break;
    case SYMBOL:  printSymbol(ios, AS_SYMBOL(x)); break;
    case BITS:    printBits(ios, AS_BITS(x)); break;
    case LIST:    printList(ios, AS_LIST(x)); break;
    default:      printTerm(ios, x); break;
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
