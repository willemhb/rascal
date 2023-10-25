#include "runtime.h"

#include "value.h"
#include "collection.h"
#include "array.h"
#include "table.h"
#include "stream.h"
#include "environment.h"

#include "eval.h"
#include "read.h"

// character classes
#define RLSPC  " \n\r\t\v,"
#define DELIM  RLSPC "()[]{}"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT  "0123456789"
#define PUNCT  "?!_-+*/=$^<>"

// globals
// pseudo accessors
static ReadFrame* read_frame(void);
static Table*     read_table(void);
static Stream*    read_source(void);
static ReadState  read_state(void);
static flags_t    read_flags(void);
static Alist*     read_stack(void);
static Buffer8*   read_buffer(void);
static char*      read_token(void);

// stream manipulation
static Glyph peek_gl(void);
static Glyph get_gl(void);
static Glyph unget_gl(Glyph gl);

// buffer manipulation
static size_t accumulate(int ch);
static void   clear_buffer(void);
static void   init_buffer(void);

// stack manipulation
static ReadFrame* push_frame(Stream* source);
static void       pop_frame(void);
static size_t     save_subx(Value x);
static Value      pop_subx(void);
static Value      consume_subx(size_t n, Value (*ctor)(Value* a, size_t n));

// misc utilities
static Value get_reader(int ch);
static void  give_expr(Value expr);
static Value take_expr(void);

// predicates and tests
static bool is_rl_space(int ch);
static bool is_sym_char(int ch);
static bool token_matches(const char* match);

// miscellaneous helpers
static Value read_sequence(int term, Value (*ctor)(Value* a, size_t n));

// toplevel read helper
static Value read_expr(void);

// adding dispatch handlers
static void add_reader_dispatch(int dispatch, ReadFn handler);
static void add_reader_dispatches(const char* dispatches, ReadFn handler);

// builtin readers
void read_eof(int dispatch);
void read_quote(int dispatch);
void read_space(int dispatch);
void read_line_comment(int dispatch);
void read_block_comment(int dispatch);
void read_atom(int dispatch);
void read_str(int dispatch);
void read_list(int dispatch);
void read_tuple(int dispatch);
void read_number(int dispatch);
void read_annot(int dispatch);

// implementations
// pseudo accessors
static ReadFrame* read_frame(void) {
  assert(RlVm.r.frame > ReadFrames);

  return RlVm.r.frame-1;
}

static Table* read_table(void) {
  return read_frame()->table;
}

static Stream* read_source(void) {
  return read_frame()->source;
}

static ReadState read_state(void) {
  return read_frame()->state;
}

static flags_t read_flags(void) {
  return read_frame()->flags;
}

static Alist* read_stack(void) {
  return &RlVm.r.stack;
}

static Buffer8* read_buffer(void) {
  return &read_frame()->buffer;
}

static char* read_token(void) {
  return read_buffer()->data;
}


// stream manipulation
static Glyph peek_gl(void) {
  Stream* s = read_source();

  return speek(s);
}

static Glyph get_gl(void) {
  Stream* s = read_source();

  return sget(s);
}

static Glyph unget_gl(Glyph gl) {
  Stream* s = read_source();

  return sunget(s, gl);
}

// token manipulation
static size_t accumulate(int ch) {
  return buffer8_push(read_buffer(), ch);
}

static void clear_buffer(void) {
  Buffer8* buffer = read_buffer();

  free_buffer8(buffer);
}

static void init_buffer(void) {
  Buffer8* buffer = read_buffer();

  obj_head(buffer, Buffer8, 0, .no_sweep=true);
  init_buffer8(buffer);
}

// stack manipulation
static ReadFrame* push_frame(Stream* source) {
  ReadFrame* out = RlVm.r.frame++;

  // initialize data
  out->source    = source;
  out->table     = RlVm.r.table;
  out->flags     = 0;
  out->state     = READER_READY;

  // initialize buffer
  init_buffer();

  return out;
}

static void pop_frame(void) {
  clear_buffer();
  RlVm.r.frame--;
}

static size_t save_subx(Value x) {
  return alist_push(read_stack(), x);
}

static Value pop_subx(void) {
  return alist_pop(read_stack());
}

static Value consume_subx(size_t n, Value (*ctor)(Value* a, size_t n)) {
  Value* buf = alist_peek(read_stack(), -n);
  Value out  = ctor(buf, n);
  
  alist_popn(read_stack(), n);

  return out;
}

// misc utilities
static Value get_reader(int ch) {
  Table* table = read_table();
  Value val;

  table_get(table, tag((Glyph)ch), &val);

  return val;
}

static void  give_expr(Value expr) {
  ReadFrame* frame = read_frame();
  
  assert(frame->state != READER_EXPRESSION);
  save_subx(expr);

  frame->state = READER_EXPRESSION;
}

static Value take_expr(void) {
  ReadFrame* frame = read_frame();
  Value out = pop_subx();
  frame->state = READER_READY;
  clear_buffer();

  return out;
}

// predicates and tests
static bool is_rl_space(int ch) {
  return strchr(RLSPC, ch);
}

static bool is_sym_char(int ch) {
  return strchr(LOWER UPPER DIGIT PUNCT, ch);
}

static bool token_matches(const char* str) {
  char* token = read_token();

  return strcmp(token, str) == 0;
}

// miscellaneous helpers
static Value read_sequence(int term, Value (*ctor)(Value* a, size_t n)) {
  Stream* source = read_source();
  size_t n = 0;
  Value xpr;
  
  while (peek_gl() != term) {
    require(!seof(source), "read", "unexpected EOF looking for \\%c", term);
    xpr = read_expr();
    save_subx(xpr);
    n++;
  }

  get_gl(); // clear terminal character

  return consume_subx(n, ctor);
}

// toplevel read helper
static Value read_expr(void) {
  Stream* stream = read_source();

  while (!read_state()) {
    int dispatch = peek_gl();
    Value reader = get_reader(dispatch);

    require(reader != NOTHING, "read", "Unreadable character \\%c", dispatch);

    if (is_fptr(reader)) {
      ReadFn cfun = (ReadFn)as_fptr(reader);
      cfun(dispatch);
    } else {
      Closure* cl = as_cls_s(reader, "read");
      Value val = apply_cl_v(cl, 2, tag(stream), tag((Glyph)dispatch));
      give_expr(val);
    }
  }

  if (read_state() == READER_EXPRESSION)
    return take_expr();

  else
    return NUL;
}

// builtin readers
void read_eof(int dispatch) {
  (void)dispatch;

  ReadFrame* frame = read_frame();
  frame->state = READER_DONE;
}

void read_quote(int dispatch) {
  (void)dispatch;
  Value quoted, form;
  
  get_gl(); // clear initial `'`
  save_subx(QuoteSym);
  quoted = read_expr();
  save_subx(quoted);
  form = consume_subx(2, mk_list);
  give_expr(form);
}

void read_space(int dispatch) {
  (void)dispatch;

  while (is_rl_space(peek_gl()))
    get_gl();
}

void read_line_comment(int dispatch) {
  while ((dispatch=peek_gl()) != EOF && dispatch != '\n')
    get_gl();
}

void read_block_comment(int dispatch) {
  int dispatch0;
  Stream* source;

  source = read_source();
  dispatch0 = dispatch;
  get_gl(); // clear opening '#'
  dispatch = peek_gl();

  if (dispatch != '|') {
    accumulate(dispatch0);
    read_atom(dispatch);
  } else {
    get_gl(); // clear opening '|'

    while (true) {
      require(!seof(source), "read", "unclosed block quote");

      dispatch = get_gl();

      if (dispatch == '#') {
        require(!seof(source), "read", "unclosed block quote");
        dispatch = get_gl();

        if (dispatch == '|') {
          get_gl();
          break;
        }
      }
    }
  }
}
void read_atom(int dispatch) {
  Stream* source = read_source();
  accumulate(dispatch);

  for (dispatch=peek_gl(); ! seof(source) && is_sym_char(dispatch); get_gl())
    accumulate(dispatch);

  if (token_matches("nul"))
    give_expr(NUL);

  else if (token_matches("true"))
    give_expr(TRUE);

  else if (token_matches("false"))
    give_expr(FALSE);

  else
    give_expr(tag(symbol(read_token())));
}

void read_str(int dispatch) {
  bool escaped = false;

  
}

void read_list(int dispatch) {
  get_gl(); // clear '('
  
  Value xpr = read_sequence(')', mk_list);
  give_expr(xpr);
}

void read_tuple(int dispatch) {
  get_gl(); // clear '['

  Value xpr = read_sequence(']', mk_tuple);
  give_expr(xpr);
}

void read_number(int dispatch);
void read_annot(int dispatch);

// adding dispatch handlers
static void add_reader_dispatch(int dispatch, ReadFn handler) {
  Table* table = read_table();

  table_set(table, tag((Glyph)dispatch), tag((funcptr_t)handler));
}

static void add_reader_dispatches(const char* dispatches, ReadFn handler) {
  for (char ch=*dispatches; ch != '\0'; dispatches++, ch=*dispatches)
    add_reader_dispatch(ch, handler);
}

// toplevel initialization
void init_builtin_readers(void) {
  // initialize builtin readers
  add_reader_dispatch(EOF, read_eof);
  add_reader_dispatch('\'', read_quote);
  add_reader_dispatch(';', read_line_comment);
  add_reader_dispatch('#', read_block_comment);
  add_reader_dispatch('@', read_annot);
  add_reader_dispatch('"', read_str);
  add_reader_dispatch('(', read_list);
  add_reader_dispatch('[', read_tuple);

  // initialize builtin readers (for ranges)
  add_reader_dispatches(RLSPC, read_space);
  add_reader_dispatches(DIGIT, read_number);
  add_reader_dispatches(LOWER UPPER PUNCT, read_atom);
}
