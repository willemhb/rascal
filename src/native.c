#include "native.h"
#include "error.h"

// globals --------------------------------------------------------------------
value_t TypeNames[NUM_TYPES];

// local helpers --------------------------------------------------------------
static bool is_nonzero(value_t val) {
  return is_number(val) && as_number(val) != 0;
}

// API ------------------------------------------------------------------------
void define_native(char* name, native_t native) {
  value_t s = symbol(name);

  as_symbol(s)->bind = tag_ptr(native, NATIVETAG);
}

// native functions -----------------------------------------------------------
// arithmetic -----------------------------------------------------------------
value_t native_add(usize n, value_t* args) {
  argco(2, n);
  argtest(is_number, args[0], "not a number");
  argtest(is_number, args[1], "not a number");

  return tag_dbl(as_number(args[0]) + as_number(args[1]));
}

value_t native_sub(usize n, value_t* args) {
  argco(2, n);
  argtest(is_number, args[0], "not a number");
  argtest(is_number, args[1], "not a number");

  return tag_dbl(as_number(args[0]) - as_number(args[1]));
}

value_t native_mul(usize n, value_t* args) {
  argco(2, n);
  argtest(is_number, args[0], "not a number");
  argtest(is_number, args[1], "not a number");

  return tag_dbl(as_number(args[0]) * as_number(args[1]));
}

value_t native_div(usize n, value_t* args) {
  argco(2, n);
  argtest(is_number,  args[0], "not a number");
  argtest(is_number,  args[1], "not a number");
  argtest(is_nonzero, args[1], "0 in divisor");

  return tag_dbl(as_number(args[0]) / as_number(args[1]));
}


value_t native_eqp(usize n, value_t* args) {
  argco(2, n);
  argtest(is_number, args[0], "not a number");
  argtest(is_number, args[1], "not a number");

  return as_number(args[0]) == as_number(args[1]) ? TRUE_VAL : FALSE_VAL;
}

value_t native_ltp(usize n, value_t* args) {
  argco(2, n);
  argtest(is_number, args[0], "not a number");
  argtest(is_number, args[1], "not a number");

  return as_number(args[0]) < as_number(args[1]) ? TRUE_VAL : FALSE_VAL;
}

// constructors ---------------------------------------------------------------
value_t native_list(usize n, value_t* args) {
  return list(n, args);
}

value_t native_binary(usize n, value_t* args) {
  for (usize i=0; i<n; i++)
    argtest(is_byte, args[i], "not a number in the range 000-255");

  return binary(n, args);
}

// miscellaneous --------------------------------------------------------------
value_t native_idp(usize n, value_t* args) {
  argco(2, n);
  return args[0] == args[1] ? TRUE_VAL : FALSE_VAL;
}

value_t native_type_of(usize n, value_t* args) {
  argco(1, n);
  return TypeNames[type_of(args[0])];
}

// initialization -------------------------------------------------------------
void native_init(void) {
  for (type_t t = UNIT; t < NUM_TYPES; t++)
    TypeNames[t] = symbol(type_name(t));

  define_native("+", native_add);
  define_native("-", native_add);
  define_native("*", native_mul);
  define_native("/", native_div);
  define_native("=", native_eqp);
  define_native("<", native_ltp);
  define_native("list", native_list);
  define_native("binary", native_binary);
  define_native("id?", native_idp);
  define_native("type-of", native_type_of);
}
