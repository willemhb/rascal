#include "print.h"

#include "value.h"
#include "port.h"
#include "types.h"

// utilities ------------------------------------------------------------------
size_t rsp_print(value_t x, port_t *ios) {
  data_type_t *xt = rsp_repr(x);

  if (xt->print)
    return xt->print(x, ios);

  return port_putf(ios, "<%s>", xt->name);
}
