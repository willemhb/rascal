#ifndef rascal_atom_h
#define rascal_atom_h

#include "table.h"

// symbol and symbol table
typedef enum
  {
    atom_fl_keyword=0x0004,
  } atom_fl_t;

struct atom_t
{
  obj_t obj;
  ENTRY_SLOTS(char*, name, idno_t, idno);
};

struct symt_t
{
  obj_t obj;
  TABLE_SLOTS(atom_t*);
  idno_t idno;
};

// globals
extern symt_t Symbols;

extern val_t Quote, Error;

// forward declarations
atom_t   *new_atom(void);
void      init_atom(atom_t *atom, symt_t *symt, char *name);
void      free_atom(obj_t *obj);
void      prin_atom(port_t *port, val_t val);

void      trace_symt(obj_t *obj);
void      init_symt(symt_t *symt);
void      free_symt(obj_t *obj);
void      clear_symt(symt_t *symt);
void      resize_symt(symt_t *symt, arity_t newl);
void      rehash_symt(atom_t **old, arity_t oldc, atom_t **new, arity_t newc);
bool      symt_put(symt_t *symt, char *name, atom_t **buf);

// external API
val_t     symbol( char *name );
void      atom_init( void );

// convenience
#define is_atom(val)     isa(val, ATOM)
#define as_atom(val)     ((atom_t*)as_ptr(val))

#define atom_hash(val)   (as_atom(val)->hash)
#define atom_name(val)   (as_atom(val)->name)
#define atom_idno(val)   (as_atom(val)->idno)

#endif
