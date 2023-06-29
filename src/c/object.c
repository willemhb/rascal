#include <stdarg.h>

#include "object.h"
#include "runtime.h"

#include "util/hashing.h"
#include "util/string.h"

// misc helpers ---------------------------------------------------------------
// APIs -----------------------------------------------------------------------
type_t* type_of( value_t x ) {
  switch ( x & TAGMASK ) {
    case CHRTAG: return &GlyphType;
    case IOSTAG: return &PortType;
    case NILTAG: return &UnitType;
    case PTRTAG: return &PointerType;
    case OBJTAG: return obtype(x);
    default:     return &NumberType;
  }
}

usize size_of( value_t x ) {
  return type_of(x)->size;
}

bool rl_isa( value_t x, type_t* type ) {
  return type->isa(x, type);
}

bool datatype_isa( value_t x, type_t* type ) {
  return type_of(x) == type;
}

bool bottom_isa( value_t x, type_t* type ) {
  (void)x;
  (void)type;
  return false;
}

bool top_isa( value_t x, type_t* type ) {
  (void)x;
  (void)type;
  return true;
}

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
extern int compare_numbers(value_t x, value_t y, void* state, flags fl);
extern uhash hash_number(value_t slf, void* state, flags fl);

type_t NumberType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =NUMBER,
  .tag    =NUMTAG,
  .size   =sizeof(number_t),
  .isa    =datatype_isa,
  .compare=compare_numbers,
  .hash   =hash_number
};

extern int compare_glyphs(value_t x, value_t y, void* state, flags fl);
extern uhash hash_glyph(value_t slf, void* state, flags fl);

type_t GlyphType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =GLYPH,
  .tag    =CHRTAG,
  .size   =sizeof(glyph_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .compare=compare_glyphs,
  .hash   =hash_glyph
};

extern int compare_ports(value_t x, value_t y, void* state, flags fl);
extern uhash hash_port(value_t slf, void* state, flags fl);

type_t PortType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =PORT,
  .tag    =IOSTAG,
  .size   =sizeof(port_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .compare=compare_ports,
  .hash   =hash_port
};


extern int compare_ptrs(value_t x, value_t y, void* state, flags fl);
extern uhash hash_ptr(value_t slf, void* state, flags fl);

type_t PointerType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =POINTER,
  .tag    =PTRTAG,
  .size   =sizeof(pointer_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .compare=compare_ptrs,
  .hash   =hash_ptr
};

extern void trace_symbol(value_t slf);
extern void init_symbol(void* spc, type_t* type, flags fl);
extern void free_symbol(value_t slf);
extern int compare_symbols(value_t x, value_t y, void* state, flags fl);
extern uhash hash_symbol(value_t slf, void* state, flags fl);

type_t SymbolType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =SYMBOL,
  .tag    =OBJTAG,
  .size   =sizeof(symbol_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_symbol,
  .init   =init_symbol,
  .free   =free_symbol,
  .compare=compare_symbols,
  .hash   =hash_symbol
};

extern void trace_cons(value_t slf);
extern void init_cons(void* spc, type_t* type, flags fl);
extern int compare_conses(value_t x, value_t y, void* state, flags fl);
extern uhash hash_cons(value_t slf, void* state, flags fl);

type_t ConsType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =CONS,
  .tag    =OBJTAG,
  .size   =sizeof(cons_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_cons,
  .init   =init_cons,
  .compare=compare_conses,
  .hash   =hash_cons
};

extern void trace_binary(value_t slf);
extern void init_binary(void* spc, type_t* type, flags fl);
extern void free_binary(value_t slf);
extern int compare_binaries(value_t x, value_t y, void* state, flags fl);
extern uhash hash_binary(value_t slf, void* state, flags fl);

type_t BinaryType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =BINARY,
  .tag    =OBJTAG,
  .size   =sizeof(binary_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_binary,
  .init   =init_binary,
  .free   =free_binary,
  .compare=compare_binaries,
  .hash   =hash_binary
};

extern void trace_vector(value_t slf);
extern void init_vector(void* spc, type_t* type, flags fl);
extern void free_vector(value_t slf);
extern int compare_vectors(value_t x, value_t y, void* state, flags fl);
extern uhash hash_vector(value_t slf, void* state, flags fl);

type_t VectorType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =VECTOR,
  .tag    =OBJTAG,
  .size   =sizeof(vector_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_vector,
  .init   =init_vector,
  .free   =free_vector,
  .compare=compare_vectors,
  .hash   =hash_vector
};

extern void trace_table(value_t slf);
extern void init_table(void* spc, type_t* type, flags fl);
extern void free_table(value_t slf);
extern int compare_tables(value_t x, value_t y, void* state, flags fl);
extern uhash hash_table(value_t slf, void* state, flags fl);

type_t TableType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =TABLE,
  .tag    =OBJTAG,
  .size   =sizeof(table_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_table,
  .init   =init_table,
  .free   =free_table,
  .compare=compare_tables,
  .hash   =hash_table
};

extern void trace_record(value_t slf);
extern void init_record(void* spc, type_t* type, flags fl);
extern void free_record(value_t slf);
extern int compare_records(value_t x, value_t y, void* state, flags fl);
extern uhash hash_record(value_t slf, void* state, flags fl);

type_t RecordType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =RECORD,
  .tag    =OBJTAG,
  .size   =sizeof(record_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_record,
  .init   =init_record,
  .compare=compare_records,
  .hash   =hash_record
};

extern void trace_function(value_t slf);
extern void init_function(void* spc, type_t* type, flags fl);
extern int compare_functions(value_t x, value_t y, void* state, flags fl);
extern uhash hash_function(value_t slf, void* state, flags fl);

type_t FunctionType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =FUNCTION,
  .tag    =OBJTAG,
  .size   =sizeof(function_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_function,
  .init   =init_function,
  .compare=compare_functions,
  .hash   =hash_function
};


extern void trace_method_table(value_t slf);
extern void init_method_table(void* spc, type_t* type, flags fl);
extern int compare_method_tables(value_t x, value_t y, void* state, flags fl);
extern uhash hash_method_table(value_t slf, void* state, flags fl);

type_t Method_TableType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =METHOD_TABLE,
  .tag    =OBJTAG,
  .size   =sizeof(method_table_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_method_table,
  .init   =init_method_table,
  .compare=compare_method_tables,
  .hash   =hash_method_table
};


extern void trace_method(value_t slf);
extern void init_method(void* spc, type_t* type, flags fl);
extern int compare_methods(value_t x, value_t y, void* state, flags fl);
extern uhash hash_method(value_t slf, void* state, flags fl);

type_t MethodType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =METHOD,
  .tag    =OBJTAG,
  .size   =sizeof(method_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_method,
  .init   =init_method,
  .compare=compare_methods,
  .hash   =hash_method
};

extern void trace_type(value_t slf);
extern void init_type(void* spc, type_t* type, flags fl);
extern int compare_types(value_t x, value_t y, void* state, flags fl);

type_t TypeType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =TYPE,
  .tag    =OBJTAG,
  .size   =sizeof(type_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_type,
  .init   =init_type,
  .compare=compare_types
};


extern void trace_chunk(value_t slf);
extern void init_chunk(void* spc, type_t* type, flags fl);
extern int compare_chunks(value_t x, value_t y, void* state, flags fl);
extern uhash hash_chunk(value_t slf, void* state, flags fl);

type_t ChunkType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =CHUNK,
  .tag    =OBJTAG,
  .size   =sizeof(chunk_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_chunk,
  .init   =init_chunk,
  .compare=compare_chunks,
  .hash   =hash_chunk
};

extern void trace_control(value_t slf);
extern void init_control(void* spc, type_t* type, flags fl);
extern int compare_controls(value_t x, value_t y, void* state, flags fl);
extern uhash hash_control(value_t slf, void* state, flags fl);

type_t ControlType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =CONTROL,
  .tag    =OBJTAG,
  .size   =sizeof(control_t),
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_control,
  .init   =init_control,
  .compare=compare_controls,
  .hash   =hash_control
};

extern void trace_unit(value_t slf);
extern void init_unit(void* spc, type_t* type, flags fl);
extern int compare_units(value_t x, value_t y, void* state, flags fl);
extern uhash hash_unit(value_t slf, void* state, flags fl);

type_t UnitType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =UNIT,
  .tag    =OBJTAG,
  .size   =0,
  .slots  =NIL,
  .isa    =datatype_isa,
  .trace  =trace_unit,
  .init   =init_unit,
  .compare=compare_units,
  .hash   =hash_unit
};

type_t BottomType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =BOTTOM,
  .tag    =NILTAG,
  .size   =0,
  .slots  =NIL,
  .isa    =bottom_isa
};

type_t TopType = {
  .obj={
    .type =&TypeType,
    .flags=NODEALLOC|FROZEN,
    .gray =true
  },
  .idno   =TOP,
  .tag    =NILTAG,
  .size   =0,
  .slots  =NIL,
  .isa    =top_isa
};

// initialization +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void toplevel_init_type( type_t* type, char* name ) {
  sethash(tagob(type), hash_uword(type->idno));
  type->name = symbol(name);
  setconst(type->name, tagob(type));
  type->ctor = function(type->name);
}

void toplevel_add_ctor( type_t* type, native_t ctor, flags fl, usize n, ... ) {
  va_list va;
  va_start(va, n);
  value_t buf[n];

  for ( usize i=0; i<n; i++ )
    buf[i] = tagob(va_arg(va, type_t*));

  va_end(va);
  value_t sig = vector(n, buf);
  value_t func = type->ctor;
  
  add_method(func, sig, tagptr(ctor), fl|NATIVE);
}

void toplevel_init_object( void ) {
  toplevel_init_type(&NumberType, "number");
  toplevel_init_type(&GlyphType, "glyph");
  toplevel_init_type(&PortType, "port");
  toplevel_init_type(&PointerType, "pointer");
  toplevel_init_type(&SymbolType, "symbol");
  toplevel_init_type(&ConsType, "cons");
  toplevel_init_type(&BinaryType, "binary");
  toplevel_init_type(&VectorType, "vector");
  toplevel_init_type(&TableType, "table");
  toplevel_init_type(&RecordType, "record");
  toplevel_init_type(&FunctionType, "function");
  toplevel_init_type(&MethodTableType, "method-table");
  toplevel_init_type(&MethodType, "method");
  toplevel_init_type(&TypeType, "type");
  toplevel_init_type(&ChunkType, "chunk");
  toplevel_init_type(&ControlType, "control");
  toplevel_init_type(&UnitType, "unit");
  toplevel_init_type(&BottomType, "bottom");
  toplevel_init_type(&TopType, "top");
}
