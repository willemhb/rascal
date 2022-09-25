#include <stdio.h>

#include "obj/cons.h"
#include "obj/repr.h"
#include "obj/str.h"
#include "obj/native.h"
#include "mem.h"
#include "rt.h"

// globals --------------------------------------------------------------------
cons_t nil_obj;
str_t cons_name_obj, nil_name_obj, list_name_obj, hd_name_obj, tl_name_obj;

#define NIL_LIST (&nil_obj)
#define NIL_OBJ ((obj_t*)NIL_LIST)

// forward declarations
val_t native_cons(val_t x, val_t y);
val_t native_list(val_t *args, arity_t n);
val_t native_nil(val_t *args,  arity_t n);
val_t native_hd(val_t *args,   arity_t n);
val_t native_tl(val_t *args,   arity_t n);

// implementations
val_t cons(val_t hd, cons_t *tl)
{
  cons_t *out = alloc( sizeof(cons_t) );

  init_obj( (obj_t*)out, cons_type, 0 );

  out->hd     = hd;
  out->tl     = tl;
  out->len    = tl->len+1;
  out->eltype = 0;

  return as_val((obj_t*)out);
}

val_t consn(val_t *args, arity_t n)
{
  if (n == 2)
    return cons( args[0], as_cons( args[1] ) );

  
}

val_t native_cons( val_t x, val_t y)
{
  if (!is_list(y))
    type_error("cons",
	       y,
	       cons_type );

  return cons(x, as_cons(y));
}

void mark_cons( obj_t *o )
{
  cons_t* c = (cons_t*)o;

  while (!c->object.black && c != NIL_LIST)
    {
      mark(c->hd);
      c = c->tl;
    }
}

// initialization -------------------------------------------------------------
repr_t ConsReprObj, NilReprObj;

static void nil_init( void )
{
  *NIL_LIST = (cons_t){
      {
	.next=NULL,
	.type=nil_type,
	.flags=FAMOUS_OBJ|STATIC_OBJ,
	.gray=true,
	.black=false
      },

      .hd=NIL,
      .tl=NIL_LIST,
      .len=0,
      .eltype=0
  };

  FamousObjects[nil_type]   = NIL_OBJ;
}

static void names_init( void )
{
  cons_name_obj = (str_t){
    {
      .next=NULL,
      .type=str_type,
      .flags=STATIC_OBJ|STATIC_DATA,
      .gray=true,
      .black=false
    },
    .vals="cons",
    .len=4,
    .enc=enc_ascii,
    .hash=hash_string( "cons" )
  };

  nil_name_obj = (str_t){
    {
      .next=NULL,
      .type=str_type,
      .flags=STATIC_OBJ|STATIC_DATA,
      .gray=true,
      .black=false
    },
    .vals="nil",
    .len=3,
    .enc=enc_ascii,
    .hash=hash_string( "nil" )
  };
}

static void reprs_init( void )
{
  ConsReprObj = (repr_t){
    {
      .type=repr_type,
      .flags=STATIC_OBJ,
      .gray=true,
    },

    // basic type information
    .name=(obj_t*)&cons_name_obj,
    .base_size=sizeof(cons_t),
    .val_tag=OBJECT,
    .val_type=cons_type,
    .val_Ctype=Ctype_pointer,

    // object methods
    .do_resize=NULL,
    .do_trace=mark_cons,

    // value methods
    .do_cmp=cmp_cons,
    .do_prin=prin_cons
  };

  NilReprObj = (repr_t){
    {
      .type=repr_type,
      .flags=STATIC_OBJ,
      .gray=true,
    },

    // basic type information
    .name=(obj_t*)&nil_name_obj,
    .base_size=sizeof(cons_t),
    .val_tag=OBJECT,
    .val_type=nil_type,
    .val_Ctype=Ctype_pointer,

    // value methods
    .do_cmp=cmp_cons,
    .do_prin=prin_cons,
  };
}

void cons_init( void )
{
  nil_init();
  names_init();
  reprs_init();
}

void cons_init_natives( void )
{
  native( &cons_name_obj, 2, true, &ConsReprObj, (native_fn_t)native_cons );
  native( &nil_name_obj, 1, false, &NilReprObj, (native_fn_t)native_nil );
  native( &list_name_obj, 0, true, NULL, (native_fn_t)native_list );
  native( &hd_name_obj, 1, false, NULL, (native_fn_t)native_hd );
  native( &tl_name_obj, 1, false, NULL, (native_fn_t)native_tl );
}
