#include <stdarg.h>

#include "object.h"
#include "runtime.h"

#include "util/hashing.h"
#include "util/string.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
type_t NumberType;
type_t GlyphType;
type_t PortType;
type_t UnitType;
type_t PointerType;
type_t SymbolType;
type_t ConsType;
type_t BinaryType;
type_t VectorType;
type_t TableType;
type_t RecordType;
type_t FunctionType;
type_t MethodTableType;
type_t MethodType;
type_t TypeType;
type_t ChunkType;
type_t ControlType;
type_t NoneType;
type_t AnyType;

// APIs +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// misc helpers ---------------------------------------------------------------
void init_object( void* slf, type_t* type, flags fl ) {
  object_t* obj = slf;

  obj->next  = Vm.live;
  obj->type  = type;
  obj->flags = fl;
  obj->gray  = true;
  Vm.live    = obj;
}

value_t make_object( type_t* type, flags fl ) {
  object_t* out = allocate(size_of(type->datatype), true);
  init_object(out, type, fl);
  return tagob(out);
}

type_t* type_of( value_t x ) {
  switch ( x & TAGMASK ) {
    case CHRTAG: return &GlyphType;
    case IOSTAG: return &PortType;
    case NILTAG: return &UnitType;
    case PTRTAG: return &PointerType;
    case OBJTAG: return asob(x)->type;
    default:     return &NumberType;
  }
}

usize size_of_val( value_t x ) {
  return size_of_dtype(type_of(x)->datatype);
}

usize size_of_dtype( datatype_t dt ) {
  switch ( dt ) {
    case NOTYPE:       return 0;
    case NUMBER:       return sizeof(number_t);
    case GLYPH:        return sizeof(glyph_t);
    case PORT:         return sizeof(port_t);
    case UNIT:         return sizeof(value_t);
    case POINTER:      return sizeof(pointer_t);
    case SYMBOL:       return sizeof(symbol_t);
    case CONS:         return sizeof(cons_t);
    case BINARY:       return sizeof(binary_t);
    case VECTOR:       return sizeof(vector_t);
    case TABLE:        return sizeof(table_t);
    case RECORD:       return sizeof(record_t);
    case FUNCTION:     return sizeof(function_t);
    case METHOD_TABLE: return sizeof(method_table_t);
    case METHOD:       return sizeof(method_t);
    case TYPE:         return sizeof(type_t);
    case CHUNK:        return sizeof(chunk_t);
    case CONTROL:      return sizeof(control_t);
  }
  unreachable();
}

void mark_value( value_t slf ) {
  if ( !isob(slf) )
    return;

  if ( asob(slf)->black )
    return;

  if ( hasfl(slf, NOMANAGE) )
    return;

  asob(slf)->black = true;

  if ( type_of(slf)->datatype == BINARY )
    asob(slf)->gray = false;

  else
    vector_push(Vm.grays, slf);
}

void trace_value( value_t slf ) {
  asob(slf)->gray = false;
  
  switch ( asob(slf)->type->datatype ) {
    case SYMBOL:
      mark_value(assym(slf)->left);
      mark_value(assym(slf)->right);
      break;

    case CONS:
      mark_value(ascons(slf)->car);
      mark_value(ascons(slf)->cdr);
      break;

    case VECTOR: {
      value_t *v = asvec(slf)->data;

      for ( usize i=0; i<asvec(slf)->cnt; i++ )
        mark_value(v[i]);

      break;
    }

    case TABLE: {
      value_t* tb = astb(slf)->data;

      for ( usize i=0; i < astb(slf)->cap; i++ ) {
        if ( tb[i*2] == NOTFOUND )
          continue;

        mark_value(tb[i*2]);
        mark_value(tb[i*2+1]);
      }

      break;
    }

    case RECORD:
      mark_value(asrecord(slf)->slots);
      break;

    case FUNCTION:
      mark_value(asfunc(slf)->name);
      mark_value(asfunc(slf)->template);
      break;

    case METHOD_TABLE:
      mark_value(asmt(slf)->cache);
      mark_value(asmt(slf)->thunk);
      mark_value(asmt(slf)->varMethods);
      mark_value(asmt(slf)->fixMethods);
      break;

    case METHOD:
      mark_value(asmethod(slf)->signature);
      mark_value(asmethod(slf)->handler);
      break;

    case TYPE:
      mark_value(astype(slf)->left);
      mark_value(astype(slf)->right);
      mark_value(astype(slf)->name);
      mark_value(astype(slf)->ctor);
      mark_value(astype(slf)->slots);
      break;

    case CHUNK:
      mark_value(aschunk(slf)->name);
      mark_value(aschunk(slf)->namespc);
      mark_value(aschunk(slf)->vals);
      mark_value(aschunk(slf)->instr);
      mark_value(aschunk(slf)->envt);
      break;

    case CONTROL:
      mark_value(asctl(slf)->stack);
      break;

    default:
      unreachable();
  }
}

void free_value( value_t slf ) {
  if ( !isob(slf) )
    return;

  if ( !hasfl(slf, NOFREE) ) {
    switch ( asob(slf)->type->datatype ) {
      case SYMBOL:
        deallocate(assym(slf)->name, 0, false);
        break;

      case BINARY:
        deallocate(asbin(slf)->data, 0, false);
        break;

      case VECTOR:
        deallocate(asvec(slf)->data, 0, false);
        break;

      case TABLE:
        deallocate(astb(slf)->data, 0, false);
        break;

      default:
        break;
    }
  }

  if ( !hasfl(slf, NODEALLOC) )
    deallocate( asob(slf), size_of(slf), true);
}

// high level constructors
value_t cons( value_t car, value_t cdr ) {
  flags fl = FROZEN;

  if ( cdr != NIL && (!iscons(cdr) || hasfl(cdr, DOTTED)) )
    fl |= DOTTED;

  value_t out = make_object(&ConsType, fl);

  ascons(out)->car = car;
  ascons(out)->cdr = cdr;

  return out;
}

value_t* locate_symbol( char* name, value_t* st ) {
  while (*st != NIL ) {
    int o = strcmp(name, assym(*st)->name);

    if ( o < 0 )
      st = &assym(*st)->left;

    else if ( o > 0 )
      st = &assym(*st)->right;

    else
      break;
  }

  return st;
}

value_t make_symbol( char* name, flags fl ) {
  if ( *name == ':' )
    fl |= LITERAL;
  
  value_t out = make_object(&SymbolType, fl|FROZEN);
  assym(out)->left = assym(out)->right = NIL;
  assym(out)->idno = ++Vm.symbolCounter;
  assym(out)->name = duplicates(name, false);
  return out;
}

value_t symbol( char* name ) {
  value_t* spc = locate_symbol(name, &Vm.symbolTable);

  if ( *spc == NIL )
    *spc = make_symbol( name, INTERNED);

  return *spc;
}

value_t gensym( char* name ) {
  if ( name == NULL )
    name = "symbol";

  return make_symbol(name, 0);
}

// initialization
