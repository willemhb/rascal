#include "native.h"
#include "error.h"
#include "compare.h"
#include "eval.h"

// globals --------------------------------------------------------------------
value_t TypeNames[NUM_TYPES];

value_t LtSym, EqSym, GtSym;

// local helpers --------------------------------------------------------------
static bool is_nonzero(value_t val) {
  return is_number(val) && as_number(val) != 0;
}

// API ------------------------------------------------------------------------
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

value_t native_vector(usize n, value_t* args) {
  return vector(n, args);
}

value_t native_dict(usize n, value_t* args) {
  require(n % 2 == 0, APPLY_ERROR, NUL, "unpaired key in dict");
  return dict(n, args);
}

value_t native_set(usize n, value_t* args) {
  return set(n, args);
}

value_t native_tuple(usize n, value_t* args) {
  return tuple(n, args);
}

value_t native_binary(usize n, value_t* args) {
  for (usize i=0; i<n; i++)
    argtest(is_byte, args[i], "not a number in the range 000-255");

  return binary(n, args);
}

// accessors ------------------------------------------------------------------
value_t native_list_len(usize n, value_t* args) {
  argco(1, n);
  argtype(LIST, args[0]);
  return fixnum(as_list(args[0])->len);
}

value_t native_hd(usize n, value_t* args) {
  argco(1, n);
  argtype(LIST, args[0]);
  return as_list(args[0])->head;
}

value_t native_tl(usize n, value_t* args) {
  argco(1, n);
  argtype(LIST, args[0]);
  return object(as_list(args[0])->tail);
}

value_t native_nth_hd(usize n, value_t* args) {
  argco(2, n);
  argtype(LIST, args[0]);
  argtype(FIXNUM, args[1]);
  require(as_list(args[0])->len > as_fixnum(args[1]),
          APPLY_ERROR,
          NUL,
          "index out of range for list  of size %zu",
          as_list(args[0])->len);

  return nth_hd(as_list(args[0]), as_fixnum(args[1]));
}

value_t native_nth_tl(usize n, value_t* args) {
  argco(2, n);
  argtype(LIST, args[0]);
  argtype(FIXNUM, args[1]);
  require(as_list(args[0])->len > as_fixnum(args[1]),
          APPLY_ERROR,
          NUL,
          "index out of range for list  of size %zu",
          as_list(args[0])->len);

  return object(nth_tl(as_list(args[0]), as_fixnum(args[1])));
}


// comparison -----------------------------------------------------------------
value_t native_compare(usize n, value_t* args) {
  argco(2, n);

  int o = compare(args[0], args[1]);

  if (o < 0)
    return LtSym;

  if (o > 0)
    return GtSym;

  return EqSym;
}

value_t native_equal(usize n, value_t* args) {
  argco(2, n);

  return equal(args[0], args[1]) ? TRUE_VAL : FALSE_VAL;
}

value_t native_idp(usize n, value_t* args) {
  argco(2, n);
  return args[0] == args[1] ? TRUE_VAL : FALSE_VAL;
}

// value queries ------------------------------------------------------------
value_t native_hash(usize n, value_t* args) {
  argco(1, n);

  return fixnum(hash(args[0]));
}

value_t native_type_of(usize n, value_t* args) {
  argco(1, n);
  return TypeNames[type_of(args[0])];
}

// interpreter ----------------------------------------------------------------
value_t native_eval(usize n, value_t* args) {
  argco(1, n);
  return eval(args[0]);
}

value_t native_apply(usize n, value_t* args) {
  argco(2, n);
  argtest(is_function, args[0], "not a function");
  argtype(LIST, args[1]);

  return apply(args[0], as_list(args[1]), true);
}

// initialization -------------------------------------------------------------
void native_init(void) {
  for (type_t t = UNIT; t < NUM_TYPES; t++)
    TypeNames[t] = symbol(type_name(t));

  LtSym = symbol(":lt");
  EqSym = symbol(":eq");
  GtSym = symbol(":gt");

  // arithmetic ---------------------------------------------------------------
  native("+", native_add);
  native("-", native_add);
  native("*", native_mul);
  native("/", native_div);
  native("=", native_eqp);
  native("<", native_ltp);

  // constructors -------------------------------------------------------------
  native("list", native_list);
  native("vector", native_vector);
  native("dict", native_dict);
  native("set", native_set);
  native("binary", native_binary);
  native("tuple", native_tuple);

  // accessors ----------------------------------------------------------------
  native("list-len", native_list_len);
  native("hd", native_hd);
  native("tl", native_tl);
  native("nth-hd", native_nth_hd);
  native("nth-tl", native_nth_tl);

  // comparison ---------------------------------------------------------------
  native("id?", native_idp);
  native("eql?", native_equal);
  native("compare", native_compare);

  // value queries ------------------------------------------------------------
  native("hash", native_hash);
  native("type-of", native_type_of);

  // interpreter --------------------------------------------------------------
  native("eval", native_eval);
  native("apply", native_apply);
}
