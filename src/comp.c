#include "comp.h"
#include "exec.h"
#include "obj/ns.h"
#include "obj/func.h"
#include "obj/atom.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    ref_type_local,
    ref_type_toplevel,
    ref_type_macro,
    ref_type_import,
  } ref_type_t;

typedef struct
{
  ref_type_t ref_type;
  bool       captured;

  arity_t    offset;
  arity_t    index;

  obj_t     *macro;
} ref_buf_t;

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
static void       emit_instr( obj_t *code, instr_t instr, ... );
static ref_type_t resolve_name( val_t name, obj_t *ns, ref_buf_t *buf );
static bool       is_literal( val_t x );

// implementations -------------------------------------------------------------
static bool is_literal( val_t x)
{
  if (is_atom(x))
    return is_keyword(x);

  return !is_cons(x);
}

void do_comp_expr( val_t expr, obj_t *code, obj_t *ns )
{
  
}

// 
void comp_mark( void );

// initialization
void comp_init( void )
{
  
}
