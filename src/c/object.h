#ifndef object_h
#define object_h

#include "common.h"

#include "util/number.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct object {
  object_t* next;
  type_t* type;
  uint64 hash  : 48;
  uint64 flags : 14;
  uint64 black :  1;
  uint64 gray  :  1;
};

#define HEADER object_t obj

// user object types
struct symbol {
  HEADER;
  char* name;
  value_t constant;
  uint64 idno;
  symbol_t* left, * right;
};

struct cons {
  HEADER;
  value_t car, cdr;
};

struct binary {
  HEADER;
  void* data;
  usize cnt, cap;
};

struct vector {
  HEADER;
  value_t* data;
  usize cnt, cap;
};

struct table {
  HEADER;
  value_t* data;
  usize cnt, cap;
};

struct record {
  HEADER;
  value_t slots;
};

struct function {
  HEADER;
  value_t name;
  value_t template;
};

struct method_table {
  HEADER;
  value_t cache;
  value_t thunk;
  value_t varMethods;
  value_t fixMethods;
};

struct method {
  HEADER;
  value_t signature;
  value_t handler;
};

struct type {
  HEADER;
  value_t name;
  uint64 idno;
  value_t ctor;
  value_t slots;
};

struct chunk {
  HEADER;
  value_t name;
  value_t namespc;
  value_t vals;
  value_t instr;
  value_t envt;
};

struct control {
  HEADER;
  // copy of main registers
  int ip, bp, fp, pp;
  value_t stack;
};

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// object flags ---------------------------------------------------------------
enum {
  // general flags
  NOMANAGE =0x2000, // don't trace
  NODEALLOC=0x1000, // don't free object
  NOFREE   =0x0800, // don't free object data
  FROZEN   =0x0400, // immutable
  HASHED   =0x0200, // cached hash

  // symbol flags
  INTERNED =0x0100,
  LITERAL  =0x0080,

  // cons flags
  DOTTED   =0x0100,

  // binary flags
  ISENCODED=0x0100,

  // table flags
  HASHID   =0x0100,

  // function/method flags
  VARIADIC =0x0100
};

// builtin type codes ---------------------------------------------------------
enum {
  NUMBER=1,
  GLYPH,
  PORT,
  POINTER,

  SYMBOL,
  CONS,
  BINARY,
  VECTOR,
  TABLE,
  RECORD,

  FUNCTION,
  METHOD_TABLE,
  METHOD,

  TYPE,
  CHUNK,
  CONTROL,

  UNIT,
  BOTTOM,
  TOP
};

// builtin types --------------------------------------------------------------
extern type_t NumberType, GlyphType, PortType, PointerType,
  SymbolType, ConsType, BinaryType, VectorType, TableType, RecordType,
  FunctionType, MethodTableType, MethodTYpe,
  TypeType, ChunkType, ControlType,
  UnitType, BottomType, TopType;

// tags -----------------------------------------------------------------------
#define QNAN     0x7ff8000000000000ul
#define SIGN     0x8000000000000000ul

#define NUMTAG  0x0000000000000000ul // dummy
#define CHRTAG  0x7ffc000000000000ul
#define IOSTAG  0x7ffd000000000000ul
#define NILTAG  0x7ffe000000000000ul
#define PTRTAG  0x7fff000000000000ul
#define FIXTAG  0xfffc000000000000ul
#define OBJTAG  0xffff000000000000ul

#define TAGMASK  0xffff000000000000ul
#define VALMASK  0x0000fffffffffffful

#define NIL       (NILTAG|0)
#define NOTFOUND  (NILTAG|1)
#define UNDEFINED (NILTAG|3)
#define UNBOUND   (NILTAG|5)

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// cast/access/test macros ----------------------------------------------------
#define asnum(x) (((ieee64_t)(x)).dbl)
#define aschr(x) ((glyph_t)((x) & VALMASK))
#define asport(x) ((port_t)((x) & VALMASK))
#define asptr(x) ((pointer_t)((x) & VALMASK))
#define asob(x) ((object_t*)((x) & VALMASK))
#define assym(x) ((symbol_t*)((x) & VALMASK))
#define ascons(x) ((cons_t*)((x) & VALMASK))
#define asbin(x) ((binary_t*)((x) & VALMASK))
#define asvec(x) ((vector_t*)((x) & VALMASK))
#define astable(x) ((table_t*)((x) & VALMASK))
#define asrecord(x) ((record_t*)((x) & VALMASK))
#define asfunc(x) ((function_t*)((x) & VALMASK))
#define asmt(x) ((method_table_t*)((x) & VALMASK))
#define asmethod(x) ((method_t*)((x) & VALMASK))
#define astype(x) ((type_t*)((x) & VALMASK))
#define aschunk(x) ((chunk_t*)((x) & VALMASK))
#define asctl(x) ((control_t*)((x) & VALMASK))

#define intval(x) ((long)asnum(x))
#define wrdval(x) ((uword)asnum(x))

#define tagptr(x) (((value_t)(x)) | PTRTAG)
#define tagob(x) (((value_t)(x)) | OBJTAG)
#define tagchr(x) (((value_t)(x)) | CHRTAG)
#define tagnum(x) (((ieee64_t)((number_t)(x))).word)

#define obtype(x) (asob(x)->type)
#define obhash(x) (asob(x)->hash)
#define obfl(x) (asob(x)->flags)
#define hasfl(x, fl) (!!(obfl(x) & (fl)))
#define setfl(x, fl) (obfl(x) |= (fl))
#define sethash(x, h)                            \
  do {                                           \
    value_t _x = x;                              \
    obhash(_x) = h;                              \
    obfl(_x) |= HASHED;                          \
  } while (false)

#define symname(x) (assym(x)->name)
#define symconst(x) (assym(x)->constant)
#define symid(x) (assym(x)->idno)
#define setconst(x, v)                          \
  do {                                          \
    value_t _x = x;                             \
    symconst(_x) = v;                           \
    obfl(_x) |= CONSTANT;                       \
  } while (false)

#define car(x) (ascons(x)->car)
#define cdr(x) (ascons(x)->cdr)

#define elsize(x) (obfl(x) & 0xff)
#define bdata(x) (asbin(x)->data)
#define bcnt(x) (asbin(x)->cnt)
#define bcap(x) (asbin(x)->cap)

#define vdata(x) (asvec(x)->data)
#define vcnt(x) (asvec(x)->cnt)
#define vcap(x) (asvec(x)->cap)

#define tbdata(x) (astable(x)->data)
#define tbcnt(x) (astable(x)->cnt)
#define tbcap(x) (astable(x)->cap)

#define rslots(x) (asrecord(x)->slots)

#define fname(x) (asfunc(x)->name)
#define ftemplate(x) (asfunc(x)->template)

#define mtcache(x) (asmt(x)->cache)
#define mtthunk(x) (asmt(x)->thunk)
#define mtmethods(x) (asmt(x)->methods)

#define msig(x) (asmethod(x)->signature)
#define mhandler(x) (asmethod(x)->handler)

#define tname(x) (astype(x)->name)
#define tidno(x) (astype(x)->idno)
#define ttag(x)  (astype(x)->tag)
#define tsize(x) (astype(x)->size)
#define tslots(x) (astype(x)->slots)
#define tctor(x) (astype(x)->ctor)
#define tisa(x) (astype(x)->isa)
#define ttrace(x) (astype(x)->trace)
#define tinit(x) (astype(x)->init)
#define tfree(x) (astype(x)->free)
#define thash(x) (astype(x)->hash)
#define tcompare(x) (astype(x)->compare)

#define cname(x) (aschunk(x)->name)
#define cnamespc(x) (aschunk(x)->namespc)
#define cvals(x) (aschunk(x)->vals)
#define cinstr(x) (aschunk(x)->instr)
#define cenvt(x) (aschunk(x)->envt)

#define ctlip(x) (asctl(x)->ip)
#define ctlbp(x) (asctl(x)->bp)
#define ctlfp(x) (asctl(x)->fp)
#define ctlpp(x) (asctl(x)->pp)
#define ctlstk(x) (asctl(x)->stk)

type_t* type_of( value_t x );
usize size_of( value_t x );
bool rl_isa( value_t x, type_t* type );

value_t make_object( type_t* type, flags fl );
void init_object( void* slf, type_t* type, flags fl );
void mark_object( void* slf );
void mark_value( value_t slf );
void mark_values( value_t* vals, usize n );
void free_object( void* slf );

// working with methods
value_t make_signature( usize n, ... );
void add_method( value_t func, value_t sig, value_t handler, flags fl );

#endif
