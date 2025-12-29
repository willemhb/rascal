#include "lang/print.h"
#include "val.h"

// Implementations ------------------------------------------------------------
void print_expr(Port* out, Expr x) {
  Type* info = type_of(x);

  if ( info->print_fn )
    info->print_fn(out, x);

  else
    pprintf(out, "<%s>", type_name(info));
}


void print_embed(Port* p, char* pre, char* sep, char* last, char* end, int n, ...) {
  // print a mix of C strings and Rascal expressions (useful for debugging)
  pprintf(p, pre);
  va_list va;
  va_start(va, n);

  for ( int i=0; i < n; i++ ) {
    Expr x = va_arg(va, Expr);
    print_expr(p, x);

    if ( sep != NULL && i+1 < n )
      pprintf(p, sep);

    else if ( last != NULL && i+1 == n )
      pprintf(p, last);
  }

  if ( end != NULL )
    pprintf(p, end);

  va_end(va);
}

