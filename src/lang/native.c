#include "types/value.h"
#include "types/object.h"
#include "types/number.h"

#include "lang/native.h"
#include "lang/compare.h"
#include "lang/hash.h"

#include "runtime/error.h"

/* globals */
Val True, False;

Val Lt, Gt, Eq;

Val DeepHash;

Val Constructors[NUM_TYPES];

/* internal helpers */
bool type_guard(Val x) {
  return is_func(x) && is_type(as_func(x));
}

bool has_opt(Val opt, List* opts) {
  return list_assc(opts, opt) != &EmptyList;
}

/* api */
// queries & predicates -------------------------------------------------------
Val native_type_of(Val* pos, List* vo, Table* kw, List* va) {
  (void)va;
  (void)vo;
  (void)kw;

  return Constructors[type_of(pos[0])];
}

Val native_isap(Val* pos, List* vo, Table* kw, List *va) {
  (void)va;
  (void)vo;
  (void)kw;

  argtype("isa?", pos[1], FUNC_TYPE);
  argval("isa?", type_guard, pos[1], "not a type");

  return has_type(pos[0], as_func(pos[1])->type->type) ? True : NUL;
}

// comparison -----------------------------------------------------------------
Val native_idp(Val* pos, List* vo, Table* kw, List* va) {
  (void)va;
  (void)vo;
  (void)kw;

  return same(pos[0], pos[1]) ? True : NUL;
}

Val native_eqp(Val* pos, List* vo, Table* kw, List* va) {
  (void)va;
  (void)vo;
  (void)kw;

  return equal(pos[0], pos[1]) ? True : NUL;
}

Val native_ord(Val* pos, List* vo, Table* kw, List* va) {
  (void)va;
  (void)vo;
  (void)kw;

  int o = compare(pos[0], pos[1]);

  if (o < 0)
    return Lt;

  if (o > 0)
    return Gt;

  return Eq;
}

Val native_hash(Val* pos, List* vo, Table* kw, List* va) {
  (void)va;
  (void)kw;

  bool deep = has_opt(DeepHash, vo);
  uhash out = hash(pos[0], deep);

  return tag(out);
}

// arithmetic -----------------------------------------------------------------
Val native_add(Val* pos, List* vo, Table* kw, List* va);
Val native_sub(Val* pos, List* vo, Table* kw, List* va);
Val native_mul(Val* pos, List* vo, Table* kw, List* va);
Val native_div(Val* pos, List* vo, Table* kw, List* va);
Val native_rem(Val* pos, List* vo, Table* kw, List* va);
Val native_neq(Val* pos, List* vo, Table* kw, List* va);
Val native_nlt(Val* pos, List* vo, Table* kw, List* va);

// constructors ---------------------------------------------------------------

// accessors & mutators -------------------------------------------------------

// sequence interface ---------------------------------------------------------

// IO -------------------------------------------------------------------------

// character handling ---------------------------------------------------------

// interpreter ----------------------------------------------------------------

// system ---------------------------------------------------------------------

// initialization -------------------------------------------------------------
void native_init(void) {
  False = symbol(tag("false"));
  True  = symbol(tag("true"));
  Lt    = symbol(tag(":lt"));
  Gt    = symbol(tag(":gt"));
  Eq    = symbol(tag(":eq"));

  set_flag(as_obj(True), LITERAL);
  set_const(as_sym(False), NUL);
}
