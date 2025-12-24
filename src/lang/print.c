#include "lang/print.h"
#include "val.h"

// Implementations ------------------------------------------------------------
void print_exp(Port* out, Expr x) {
  Type* info = type_of(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    pprintf(out, "<%s>", type_name(info));
}
