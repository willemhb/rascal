#ifndef interpreter_h
#define interpreter_h

#include "value.h"

/* builtin types used predominantly by the compiler and virtual machine */

// C types --------------------------------------------------------------------
struct variable_t {
  HEADER;
  object_t*    ns;
  symbol_t*    name;
  string_t*    doc;
  type_t*      type;
  value_t      bind;
};

struct control_t {
  HEADER;
  control_t* caller;
  closure_t* func;
  uint16*    ip;
  alist_t*   rxs;
};

// globals --------------------------------------------------------------------
extern data_type_t VariableType, ControlType;

// API ------------------------------------------------------------------------
// variable -------------------------------------------------------------------
#define     is_variable(x) ISA(x, VariableType)
#define     as_variable(x) ASP(x, variable_t)

variable_t* variable(object_t* ns, symbol_t* name, string_t* doc, type_t* type);

// control --------------------------------------------------------------------
#define     is_control(x) ISA(x, ControlType)
#define     as_control(x) ASP(x, control_t)

#endif
