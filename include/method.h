#ifndef rascal_method_h
#define rascal_method_h

#include "table.h"

struct metht_t
{
  obj_t obj;
  TABLE_SLOTS(meth_t*);

  arity_t   min_argc;
  arity_t   max_argc;
  func_t   *func;
  meth_t   *tree;
};

typedef enum
  {
    meth_fl_native=0x01,
  } meth_fl_t;

struct meth_t
{
  obj_t obj;
  TABLE_SLOTS(meth_t*);
  ENTRY_SLOTS(types_t *, signature, invoke_t, invoke );
  arity_t argc;
  bool    vargs;
  metht_t *metht;
};

typedef enum
  {
   types_fl_fsig=1,
  } types_fl_t;

struct types_t
{
  obj_t obj;
  ENTRY_SLOTS( type_t *, types, type_t, type );
  arity_t arity;
  flags_t typesfl;
};

struct typet_t
{
  obj_t obj;
  ORDERED_TABLE_SLOTS(types_t *);
  idno_t idno;
};

// globals
extern typet_t Types;

// forward declarations
metht_t *new_metht(void);
void     init_metht(metht_t *metht);
void     mark_metht(obj_t *obj);
void     free_metht(obj_t *obj);
void     resize_metht(metht_t *metht, arity_t newl);
void     rehash_metht(meth_t **old, arity_t oldc, meth_t **new, arity_t newc);
bool     metht_put(metht_t *metht, types_t *types, meth_t **buf);
bool     metht_get(metht_t *metht, types_t *types, meth_t **buf);

meth_t  *new_meth(void);
void     init_meth(meth_t *meth, metht_t *metht, types_t *types);
void     mark_meth(obj_t *obj);
void     free_meth(obj_t *obj);
void     resize_meth(meth_t *meth, arity_t newl);
void     rehash_meth(meth_t **old, arity_t oldc, meth_t **new, arity_t newc);
bool     meth_put(meth_t *meth, types_t *types, meth_t **buf);
bool     meth_get(meth_t *meth, types_t *types, meth_t **buf);

types_t *new_types(void);
void     init_types(types_t *types, type_t *signature);
void     free_types(obj_t *obj);

void     init_typet(typet_t *typet);
void     mark_typet(obj_t *obj);
void     free_typet(obj_t *obj);
void     resize_typet(typet_t *typet, arity_t newl);
void     rehash_typet(types_t **types, arity_t len, arity_t cap, ords_t ords);
void     typet_get(typet_t *typet, types_t *types, types_t **buf);
void     typet_get(typet_t *typet, types_t *types, types_t **buf);

// initialization
void     method_init( void );

#endif
