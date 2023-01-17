#include "read.h"

#include "list.h"

#include "reader.h"
#include "error.h"

#include "util/ios.h"

/* API */
val_t read(void) {
  val_t out = read_expression(&Reader);

  if (recover()) {
    sync_reader(&Reader, '\n');
    return NUL;
  }

  return out;
}

val_t readln(void) {
  extern int peekchr(reader_t *reader);
  
  val_t out = read();

  int ch;
  
  if ((ch=peekchr(&Reader)) != '\n') {
    printf("\nUnexpected character '%c' reading line.\n\n", ch);
    sync_reader(&Reader, '\n');
    return NUL;
  }

  newline();

  return out;
}

#include <ctype.h>

#include "reader.h"

#include "list.h"
#include "sym.h"
#include "vec.h"
#include "code.h"
#include "small.h"
#include "bool.h"
#include "func.h"

#include "val.h"
#include "obj.h"
#include "type.h"
#include "error.h"

#include "prin.h"
#include "apply.h"

#include "util/string.h"
#include "util/hashing.h"
#include "util/number.h"
#include "util/ios.h"

/* globals */
reader_t Reader;

extern val_t QuoteSym;

/* instantiations */
#include "tpl/impl/buffer.h"
#include "tpl/impl/hashmap.h"

BUFFER(readbuffer, ascii_t, int, pad_stack_size);

bool readt_cmp(int x, int y) { return x == y; }

read_fn_t readt_intern(int dispatch, void **space) {
  (void)dispatch;
  (void)space;
  return NULL;
}

void readt_noval(reader_t *reader, int dispatch) {
  (void)reader;

  panic(NOTHING, "Unexpected token '%c' reading file.\n", dispatch);
}

HASHMAP(readtable, int, read_fn_t, pad_table_size, hash_int32, readt_cmp, readt_intern, -2, readt_noval);

/* debugging */
#define show_dispatch(d) printf("%s:%d:%s: dispatch='%c'\n", __FILE__,__LINE__,__func__,(d))

/* API */
/* reader API */
void  give_expression(reader_t *reader, val_t value, read_state_t state);
void  give_expressions(reader_t *reader, val_t value, read_state_t state, size_t n);
void  save_expression(reader_t *reader, val_t value);
val_t take_expression(reader_t *reader);

char *token(reader_t *reader);
char* getln(reader_t *reader);

void  accumchr(reader_t *reader, int ch);
int   getchr(reader_t *reader);
int   ungetchr(reader_t *reader, int ch);
int   peekchr(reader_t *reader);
int   skipspc(reader_t *reader);

/* toplevel reader API */
val_t read_expression(reader_t *reader) {
  while (reader->state) {
    int dispatch = getchr(reader);
    read_fn_t handler = readtable_get(reader->table, dispatch);

    assert(handler);
    handler(reader, dispatch);
    repanic(NUL);
  }

  return take_expression(reader);
}

void clear_reader(reader_t *reader) {
  give_expression(reader, NUL, read_state_ready);
  reset_readbuffer(reader->buffer);
}

void  init_reader(reader_t *reader) {
  reader->infile  = stdin;
  reader->state   = read_state_ready;
  reader->buffer  = make_readbuffer(0);
  reader->table   = make_readtable(0, NULL);
  reader->subx    = make_vals(0, NULL);
}

void sync_reader(reader_t *reader, int sentinel) {
  int ch;

  while ((ch=peekchr(reader)) != sentinel)
    getchr(reader);

  clear_reader(reader);
  reset_vals(reader->subx);
}

/* reader helpers */
int read_seq(reader_t *reader, char* type, char *term) {
  int dispatch;

  while ((dispatch=skipspc(reader)) != EOF && !strchr(term, dispatch)) {
    val_t x = read_expression(reader);
    repanic(READERPANIC);
    save_expression(reader, x);
  }

  if (dispatch == EOF)
    panic(READERPANIC, "Unexpected eos reading %s.\n", type);

  getchr(reader); // take terminal token
  return dispatch;
}

void accumchr(reader_t *reader, int ch) {
  if (ch != EOF)
    readbuffer_push(reader->buffer, ch);
}

int getchr(reader_t *reader) {
  int ch = getc(reader->infile);

  return ch;
}

int ungetchr(reader_t *reader, int ch) {
  if (ch == EOF)
      give_expression(reader, NUL, read_state_eos);

  else
    return ungetc(ch, reader->infile);

  return ch;
}

int peekchr(reader_t *reader) {
  int ch = getc(reader->infile);
  
  ungetchr(reader, ch);

  return ch;
}

int skipspc(reader_t *reader) {
  void read_space(reader_t *reader, int dispatch);

  int dispatch = peekchr(reader);

  if (isrlspc(dispatch))
    read_space(reader, dispatch);

  dispatch = peekchr(reader);
  return dispatch;
}

char* token(reader_t *reader) {
  return reader->buffer->elements;
}

char *getln(reader_t *reader) {
  int ch;

  while ((ch=getchr(reader)) != '\n' && ch != EOF)
    accumchr(reader, ch);

  return token(reader);
}

val_t take_expression(reader_t *reader) {
  val_t out = reader->value;

  clear_reader(reader);

  return out;
}

void give_expression(reader_t *reader, val_t value, read_state_t state) {
  reader->value = value;
  reader->state = state;
}

void give_expressions(reader_t *reader, val_t value, read_state_t state, size_t n) {
  vals_popn(reader->subx, n);
  reader->value = value;
  reader->state = state;
}

void save_expression(reader_t *reader, val_t val) {
  vals_push(reader->subx, val);
}

/* handlers */
void read_eos(reader_t *reader, int dispatch) {
  (void)dispatch;
  give_expression(reader, NUL, read_state_eos);
}

void read_space(reader_t *reader, int dispatch) {
  while ( isspace(dispatch) )
    dispatch=getchr(reader);

  if (dispatch == EOF)
    read_eos(reader, dispatch);

  else
    ungetchr(reader, dispatch);
}

void read_symbol(reader_t *reader, int dispatch) {
  accumchr(reader, dispatch);

  while (isrlsymchr(dispatch=getchr(reader))) {
    accumchr(reader, dispatch);
  }

  ungetchr(reader, dispatch);

  val_t xpr;

  if (streq(token(reader), "true"))
    xpr = TRUE;

  else if (streq(token(reader), "false"))
    xpr = FALSE;

  else
    xpr = sym(token(reader));
  
  give_expression(reader, xpr, read_state_expr);
}

void read_number(reader_t *reader, int dispatch) {
  accumchr(reader, dispatch);

  bool valid = true;

  while ((dispatch=getchr(reader)) != EOF && !isrldlm(dispatch) ) {
    // show_dispatch(dispatch);
    switch (dispatch) {
    case '0' ... '9':
      break;

    case '.':
      valid = valid && !strchr(token(reader), '.');
      break;

    default:
      valid = false;
      break;
    }

    accumchr(reader, dispatch);
  }

  ungetchr(reader, dispatch);

  val_t result;

  if (valid) {
    char *buf;
    
    real_t x = strtod(token(reader), &buf);

    if (*buf == '\0') {
      if (fits_small(x))
        result = tag_val((small_t)x, SMALL);
      else
        result = as_val(x);
    }

    else
      result = sym(token(reader));

  } else {
    result = sym(token(reader));
  }

  give_expression(reader, result, read_state_expr);
}

void read_list(reader_t *reader, int dispatch) {
  size_t n = reader->subx->len; val_t out;
  
  dispatch = read_seq(reader, "list", ".)");

  repanic();

  n = reader->subx->len-n;

  if (dispatch == '.') {
    val_t tail = read_expression(reader);
    repanic();
    save_expression(reader, tail);
    dispatch = skipspc(reader);
  } else {
    save_expression(reader, NUL);
  }

  if (dispatch != ')')
    panic(NOTHING, "Unexpected eof reading list.");

  if (n == 0) {
    out = vals_pop(reader->subx);
    give_expression(reader, out, read_state_expr);
  } else {
    cons_t c = make_list(n+1, vals_at(reader->subx, -n-1));
    out = tag_val(c, OBJECT);
    give_expressions(reader, out, read_state_expr, n+1);
  }
}

void read_vec(reader_t *reader, int dispatch) {
  (void)dispatch;
  size_t n = reader->subx->len;

  read_seq(reader, "vec", "]");

  repanic();

  n = reader->subx->len-n;

  vec_t v = make_vec(n, n ? vals_at(reader->subx, -n) : NULL);
  val_t out = tag_val(v, OBJECT);

  give_expressions(reader, out, read_state_expr, n);
}

void read_obj(reader_t *reader, int dispatch) {
  (void)dispatch;
  size_t n = reader->subx->len;

  val_t head = read_expression(reader);

  if (!is_sym(head))
    panic(NOTHING, "Unexpected %s expression in object literal.", type_of(head)->name);

  if ((dispatch=getchr(reader)) != '(')
    panic(NOTHING, "Unexpected character in %s literal: '%c'.", as_sym(head), dispatch);

  read_seq(reader, as_sym(head), ")");

  n = reader->subx->len - n;
  val_t *base = vals_at(reader->subx, -n);
  val_t constructor = sym_head(head)->val;
  val_t expression = invoke(constructor, n, base);

  repanic();

  give_expressions(reader, expression, read_state_expr, n);
}

void read_quote(reader_t *reader, int dispatch) {
  (void)dispatch;
  val_t x = read_expression(reader);
  repanic();
  val_t f = list(2, QuoteSym, x);
  give_expression(reader, f, read_state_expr);
}

/* initialization */
void add_dispatch(reader_t *reader, int ch, read_fn_t handler) {
  readtable_add(reader->table, ch, handler);
}

void add_dispatches(reader_t *reader, char *chars, read_fn_t handler) {
  for (size_t i=0; i<strlen(chars); i++) {
    readtable_add(reader->table, chars[i], handler);
  }
}

/* initialization */
void reader_init(void)
{
  init_reader(&Reader);

  // add reader procedures
  add_dispatches(&Reader, SYMBOLS, read_symbol);
  add_dispatches(&Reader, DIGITS, read_number);
  add_dispatches(&Reader, RLSPACES, read_space);
  add_dispatch(&Reader, EOF, read_eos);
  add_dispatch(&Reader, '(', read_list);
  add_dispatch(&Reader, '[', read_vec);
  add_dispatch(&Reader, '#', read_obj);
  add_dispatch(&Reader, '\'', read_quote);
}
