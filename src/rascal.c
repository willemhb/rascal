#include "common.h"

#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Magic numbers
#define BUFFER_SIZE 2048
#define STACK_SIZE  65536
#define BUFFER_MAX  2046
#define INIT_HEAP   (4096 * sizeof(uintptr_t))

// Prompt/messages
#define PROMPT  "rl>"
#define VERSION "%d.%d.%d.%s"
#define WELCOME "Welcome to rascal version "VERSION"!"
#define MAJOR   0
#define MINOR   1
#define PATCH   0
#define RELEASE "a"

// Types
// Expression types
typedef enum {
    EXP_NUL=1,
    EXP_EOS,
    EXP_SYM,
    EXP_LIST,
    EXP_NUM
} ExpType;

typedef enum {
  OKAY,
  USER_ERROR,
  RUNTIME_ERROR,
  SYSTEM_ERROR
} Status;

typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double    Num;
typedef struct    Obj  Obj;
typedef struct    Sym  Sym;
typedef struct    List List;

typedef union {
  Expr expr;
  Num  num;
  Obj* obj;
  Nul  nul;
} Val;

#define HEAD                                    \
  Obj* heap;                                    \
  ExpType type;                                 \
  bool marked

struct Obj {
  HEAD;
};

struct Sym {
  HEAD;

  char* val;
};

struct List {
  HEAD;

  Expr   head;
  List*  tail;
  size_t count;
};

// Globals
char* ExpTypeName[] = {
  [EXP_NUL]  = "nul",
  [EXP_EOS]  = "eos",
  [EXP_SYM]  = "sym",
  [EXP_LIST] = "list",
  [EXP_NUM]  = "num"
};

size_t ExpTypeObjSize[] = {
  [EXP_NUL]  = 0,
  [EXP_EOS]  = 0,
  [EXP_SYM]  = sizeof(Sym),
  [EXP_LIST] = sizeof(List),
  [EXP_NUM]  = 0
};

char* ErrorNames[] = {
  [OKAY]          = "okay",
  [USER_ERROR]    = "user",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "sytem"
};

char Token[BUFFER_SIZE];
size_t TBOffset = 0;
Status VmStatus = OKAY;
jmp_buf Toplevel;
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;
Expr Stack[STACK_SIZE];
int Sp = 0;

// Expression tags
#define QNAN  0x7ffc000000000000ul

#define XTMSK 0xffff000000000000ul
#define XVMSK 0x0000fffffffffffful

#define OBJ     0xfffc000000000000ul
#define NUL     0xffff000000000000ul
#define EOS_T   0x7ffd000000000000ul
#define EOS     0x7ffd0000fffffffful

// forward declarations

void   panic(Status etype);
void   recover(void);
void   rascal_error(Status etype, char* fmt, ...);
void   reset_token(void);
size_t add_to_token(char c);
void   reset_stack(void);
Expr*  stack_ref(int i);
Expr*  push(Expr x);
Expr*  pushn(int n);
Expr   pop(void);
Expr   popn(int n);


#define safepoint() setjmp(Toplevel)

#define user_error(args...)    rascal_error(USER_ERROR, args)
#define runtime_error(args...) rascal_error(RUNTIME_ERROR, args)
#define system_error(args...)  rascal_error(SYSTEM_ERROR, args)

#define tos()  stack_ref(-1)

// error helpers
void panic(Status etype) {
  if ( etype == SYSTEM_ERROR )
    exit(1);

  longjmp(Toplevel, 1);
}

void recover(void) {
  if ( VmStatus ) {
    VmStatus = OKAY;
    reset_token();
    reset_stack();
    fseek(stdin, SEEK_SET, SEEK_END); // clear out invalid characters
  }
}

void rascal_error(Status etype, char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s error: ", ErrorNames[etype]);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);
  panic(etype);
}

// token API
void reset_token(void) {
  memset(Token, 0, BUFFER_SIZE);
  TBOffset = 0;
}

size_t add_to_token(char c) {
  if ( TBOffset < BUFFER_MAX )
    Token[TBOffset++] = c;

  else
    runtime_error("maximum token length exceeded");

  return TBOffset;
}

// stack API
void reset_stack(void) {
  memset(Stack, 0, STACK_SIZE * sizeof(Expr));
  Sp = 0;
}

Expr* stack_ref(int i) {
  int j = i;
  
  if ( j < 0 )
    j += Sp;

  if ( j < 0 || j > Sp ) {
    runtime_error("stack reference %d out of bounds", i);
  }

  return &Stack[j];
}

Expr* push( Expr x ) {
  if ( Sp == STACK_SIZE )
    runtime_error("stack overflow");

  Stack[Sp] = x;

  return &Stack[Sp++];
}

Expr* pushn( int n ) {
  if ( Sp + n >= STACK_SIZE )
    runtime_error("stack overflow");

  Expr* base = &Stack[Sp]; Sp += n;

  return base;
}

Expr pop( void ) {
  if ( Sp == 0 )
    runtime_error("stack underflow");

  return Stack[--Sp];
}

Expr popn( int n ) {
  if ( n > Sp )
    runtime_error("stack underflow");

  Expr out = Stack[Sp-1]; Sp -= n;

  return out;
}

// forward declarations
void    run_gc(void);
void    free_object(Obj *obj);
ExpType expr_type(Expr x);
Status  read_expr(FILE* in, Expr* out);
Status  eval_expr(Expr x, Expr* out);
void    print_expr(FILE* out, Expr x);
void    repl(void);

// internal helpers
static bool check_gc(size_t n) {
  return HeapUsed + n >= HeapCap;
}

void run_gc(void) {
  HeapCap <<= 1;
}

void* allocate(bool h, size_t n) {
  void* out;

  if ( h ) {
    if ( check_gc(n) )
      run_gc();

    out = calloc(n, 1);
    HeapUsed += n;
  } else
    out = calloc(n, 1);

  if ( out == NULL )
    system_error("out of memory");

  return out;
}

// expression APIs
Num as_num(Expr x) {
  Val v = { .expr = x };

  return v.num;
}

Expr tag_num(Num n) {
  Val v = { .num = n };

  return v.expr;
}

Obj* as_obj(Expr x) {
  return (Obj*)(x & XVMSK);
}

Expr tag_obj(void* o) {
  return ((Expr)o) | OBJ;
}

#define as_sym(x)  ((Sym*)as_obj(x))
#define as_list(x) ((List*)as_obj(x))

void* mk_obj(ExpType type) {
  Obj* out = allocate(true, ExpTypeObjSize[type]);

  out->type   = type;
  out->marked = false;
  out->heap   = Heap;
  Heap        = out;

  return out;
}

Sym* mk_sym(char* val) {
  Sym* s = mk_obj(EXP_SYM);
  s->val = strdup(val);

  return s;
}

static List* empty_list(void) {
  List* l = mk_obj(EXP_LIST);

  l->head  = NUL;
  l->tail  = NULL;
  l->count = 0;

  return l;
}

static List* new_lists(size_t n) {
  assert(n > 0);

  size_t nb = (n+1) * sizeof(List);
  List* xs  = allocate(true, nb);

  // initialize terminal empty list
  for ( size_t i=0; i < n; i++ ) {
    List* cell = &xs[i];

    // initialize the list object
    cell->heap   = (Obj*)(&cell+1);
    cell->type   = EXP_LIST;
    cell->marked = false;
    cell->head   = NUL;
    cell->tail   = cell + 1;
    cell->count  = n - i;
  }

  // handle the terminal empty list specially
  List* cell  = &xs[n];
  cell->heap   = Heap;
  cell->type   = EXP_LIST;
  cell->marked = false;
  cell->head   = NUL;
  cell->tail   = NULL;
  cell->count  = 0;

  // add it all to the heap
  Heap = (Obj*)xs;

  return xs;
}

List* mk_list(size_t n, Expr* xs) {
  List* l;

  if ( n == 0 )
    l = empty_list();

  else {
    l = new_lists(n);

    for ( size_t i=0; i<n; i++ )
      l[i].head = xs[i];
  }

  return l;
}

// implmentations
ExpType expr_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NUL  : t = EXP_NUL;         break;
    case EOS_T: t = EXP_EOS;         break;
    case OBJ  : t = as_obj(x)->type; break;
    default   : t = EXP_NUM;         break;
  }

  return t;
}

// Function prototypes
bool is_sym_char(int c);
bool is_num_char(int c);
int peek(FILE *in);
char read_char(FILE *in);
void skip_space(FILE* in);
Expr read_list(FILE *in);
Expr read_atom(FILE* in);
Status read_expr(FILE *in, Expr* out);

// Global symbol table could be added here

// read helpers
bool is_sym_char(int c) {
  return !isspace(c) && !strchr("(){}[];", c);
}

bool is_num_char(int c) {
  return isdigit(c) || strchr(".+-", c);
}

int peek(FILE *in) {
    int c = fgetc(in);
    ungetc(c, in);
    return c;
}

char read_char(FILE *in) {
    return fgetc(in);
}

// read helpers
void skip_space(FILE* f) {
  int c;
  
  while ( !feof(f) ) {
    c = fgetc(f);

    if ( !isspace(c) ) {
      ungetc(c, f);
      break;
    }
  }
}

Status read_expr(FILE *in, Expr* out) {
  Status s = OKAY;
  reset_token();
  skip_space(in);
  Expr x;
  int c = peek(in);
  
  if ( c == EOF )
    x = EOS;
  else if ( c == '(' )
    x = read_list(in);
  else if ( is_sym_char(c) )
    x = read_atom(in);
  else if ( c == ')' )
    runtime_error("dangling ')'");
  else
    runtime_error("unrecognized character %c", c);

  *out = x;

  return s;
}

Expr read_list(FILE *in) {
  List* out;
  skip_space(in);
  read_char(in); // consume the '('
  Expr* base = &Stack[Sp], x;
  int n = 0, c;
  
  while ( (c=peek(in)) != ')' ) {
    if ( feof(in) )
      runtime_error("unterminated list");

    read_expr(in, &x);
    push(x);
    n++;
    skip_space(in);
  }

  read_char(in); // consume ')'

  out = mk_list(n, base);

  if ( n > 0 )
    popn(n);

  return tag_obj(out);
}

Expr read_atom(FILE* in) {
  int c;
  Expr x;
  
  while ( !feof(in) && is_sym_char(c=peek(in)) ) {
    add_to_token(c); // accumulate
    read_char(in);   // consume character
  }

  assert(TBOffset > 0);

  if ( is_num_char(Token[0])) {
    char* end;

    Num n = strtod(Token, &end);

    if ( end[0] != '\0' ) {   // Symbol that starts with numeric character like +, -, or digit
      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    } else {
      x      = tag_num(n);
    }
  } else {
    if ( strcmp(Token, "nul") == 0 )
      x = NUL;

    else if ( strcmp(Token, "<eos>" ) == 0 )
      x = EOS;

    else {
      Sym* s = mk_sym(Token);
      x      = tag_obj(s);
    }
  }

  return x;
}

// eval
Status eval_expr(Expr x, Expr* out) {
  Expr v;
  Status s = OKAY;

  switch ( expr_type(x) ) {
    default: v = x; break;
  }

  *out = v;

  return s;
}

// print
void print_num(FILE* out, Expr x) {
  
}

void print_expr(FILE* out, Expr x) {
  switch ( expr_type(x) ) {
    case EXP_LIST: {
      fprintf(out, "(");

      List* xs = as_list(x);

      while ( xs->count > 0 ) {
        print_expr(out, xs->head);

        if ( xs->count > 1 )
          fprintf(out, " ");

        xs = xs->tail;
      }

      fprintf(out, ")");

      break;
    }

    case EXP_NUL: fprintf(out, "nul");                             break;
    case EXP_SYM: fprintf(out, "%s", as_sym(x)->val);              break;
    case EXP_NUM: fprintf(out, "%g", as_num(x));                   break;
    default:      fprintf(out, "<%s>", ExpTypeName[expr_type(x)]); break;
  }
}

// void
void repl(void) {
  Expr x, v;

  for (;;) {
    if ( safepoint() )
      recover();

    else {
      fprintf(stdout, PROMPT" ");
      read_expr(stdin, &x);
      eval_expr(x, &v);
      fprintf(stdout, "\n>>> ");
      print_expr(stdout, x);
      fprintf(stdout, "\n\n");
    }
  }
}

// setup/teardown
void setup(void) {
  fprintf(stdout, WELCOME, MAJOR, MINOR, PATCH, RELEASE);
  fprintf(stdout, "\n\n");
}

void teardown(void) {}

// entry point
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  setup();
  repl();
  teardown();

  return 0;
}
