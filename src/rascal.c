#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Magic numbers
#define BUFFER_SIZE 2048
#define STACK_SIZE  8192
#define BUFFER_MAX  2046
#define INIT_HEAP   (4096 * sizeof(uintptr_t))

// Types
// Token types
typedef enum {
  TOKEN_READY,
  TOKEN_SYM,
  TOKEN_NUL,
  TOKEN_NUM,
  TOKEN_EOS
} Token;

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
  PROBLEM
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
const char* ExpTypeName[] = {
  [EXP_NUL]  = "nul",
  [EXP_EOS]  = "eos",
  [EXP_SYM]  = "sym",
  [EXP_LIST] = "list",
  [EXP_NUM]  = "num"
};

const size_t ExpTypeObjSize[] = {
  [EXP_NUL]  = 0,
  [EXP_EOS]  = 0,
  [EXP_SYM]  = sizeof(Sym),
  [EXP_LIST] = sizeof(List),
  [EXP_NUM]  = 0
};

char TokenBuffer[BUFFER_SIZE];
size_t TBOffset = 0;
Status VmStatus = OKAY;
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;

static void reset_tbuf(void) {
  memset(TokenBuffer, 0, BUFFER_SIZE);
  TBOffset = 0;
}

static size_t add_to_tbuf(char c) {
  if ( TBOffset < BUFFER_MAX )
    TokenBuffer[TBOffset++] = c;

  return TBOffset;
}

// Expression tags
#define QNAN  0x7ffc000000000000ul

#define XTMSK 0xffff000000000000ul
#define XVMSK 0x0000fffffffffffful

#define OBJ   0xfffc000000000000ul
#define NUL   0xffff000000000000ul
#define EOS   0x7ffd0000fffffffful

// miscellaneous helpers
#define error_message(fmt, ...) fprintf(stderr, fmt".\n" __VA_OPT__(,) __VA_ARGS__)

#define fatal_error(args...)                    \
  do {                                          \
    error_message(args);                        \
    exit(1);                                    \
  } while (false)

// forward declarations
void    run_gc(void);
ExpType expr_type(Expr x);
Status  read_expr(Expr* r);
Status  eval_expr(Expr* r, Expr x);
void    print_expr(Expr x);
void    rl_repl(void);

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
    fatal_error("out of memory");

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

Expr mk_sym(char* val) {
  Sym* s = mk_obj(EXP_SYM);
  s->val = strdup(val);

  return tag_obj(s);
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
    List cell = xs[i];

    // initialize the list object
    cell.heap   = (Obj*)(&cell+1);
    cell.type   = EXP_LIST;
    cell.marked = false;
    cell.head   = NUL;
    cell.tail   = &cell + 1;
    cell.count  = n - i;
  }

  // handle the terminal empty list specially
  List cell   = xs[n];
  cell.heap   = Heap;
  cell.type   = EXP_LIST;
  cell.marked = false;
  cell.head   = NUL;
  cell.tail   = NULL;
  cell.count  = 0;

  // add it all to the heap
  Heap = (Obj*)xs;

  return xs;
}

Expr mk_list(size_t n, Expr* xs) {
  List* l;
  
  if ( n == 0 )
    l = empty_list();

  else {
    l = new_lists(n);

    for ( size_t i=0; i<n; i++ )
      l[i].head = xs[i];
  }

  return tag_obj(l);
}

// implmentations

// constructors

ExpType expr_type(Expr x) {
  ExpType t;

  switch ( x & XTMSK ) {
    case NUL: t = EXP_NUL;         break;
    case EOS: t = EXP_EOS;         break;
    case OBJ: t = as_obj(x)->type; break;
    default:  t = EXP_NUM;         break;
  }

  return t;
}

// read helpers
void skip_space(FILE* f) {
  while ( !feof(f) ) {
    int c = fgetc(f);

    if ( isspace(c) )
      continue;

    else if ( c == EOF )
      break;

    else {
      ungetc(c, f);
      break;
    }
  }
}

Status eval_expr(Expr* r, Expr x) {
  Expr v;
  Status s = OKAY;

  switch ( expr_type(x) ) {
    default: v = x; break;
  }

  *r = v;

  return s;
}

void print_expr(Expr x) {
  switch ( expr_type(x) ) {
    case EXP_LIST: {
      printf("(");

      List* xs = as_list(x);

      while ( xs->count > 0 ) {
        print_expr(xs->head);

        if ( xs->count > 1 )
          printf(" ");

        xs = xs->tail;
      }

      printf(")");

      break;
    }

    case EXP_NUL: printf("nul");                             break;
    case EXP_SYM: printf("%s", as_sym(x)->val);              break;
    case EXP_NUM: printf("%.2g", as_num(x));                 break;
    default:      printf("<%s>", ExpTypeName[expr_type(x)]); break;
  }
}

// Function prototypes
Expr read(FILE *in);
Expr read_list(FILE *in);
void skip_whitespace(FILE *in);
int peek(FILE *in);
char read_char(FILE *in);
Expr read_atom(FILE* in);
Expr read_symbol(FILE *in);
Expr read_number(FILE *in);
void print_object(Obj *obj);
void free_object(Obj *obj);

// Global symbol table could be added here
Expr read(FILE *in) {
    skip_whitespace(in);
    Expr x;
    int c = peek(in);
    
    if ( c == EOF )
      x = EOS;
    
    else if ( c == '(' ) {
      read_char(in); // consume the '('
      x = read_list(in);
    } else {
      x = read_atom(in);
    }

    return x;
}

Expr read_list(FILE *in) {
    skip_whitespace(in);

    
    
    if (peek(in) == ')') {
        read_char(in); // consume the ')'
        return NULL; // empty list
    }
    
    Obj *car = read(in);
    skip_whitespace(in);
    
    if (peek(in) == '.') {
        // Handle dotted pairs if needed
        read_char(in);
        Obj *cdr = read(in);
        skip_whitespace(in);
        if (read_char(in) != ')') {
            fprintf(stderr, "Expected ')' after dotted pair\n");
            exit(EXIT_FAILURE);
        }
        
        Obj *obj = malloc(sizeof(Obj));
        obj->type = TYPE_LIST;
        obj->data.list.car = car;
        obj->data.list.cdr = cdr;
        return obj;
    }
    
    Obj *cdr = read_list(in);
    
    Obj *obj = malloc(sizeof(Obj));
    obj->type = TYPE_LIST;
    obj->data.list.car = car;
    obj->data.list.cdr = cdr;
    return obj;
}

void skip_whitespace(FILE *in) {
    int c;
    while ((c = peek(in)) != EOF && isspace(c)) {
        read_char(in);
    }
}

int peek(FILE *in) {
    int c = fgetc(in);
    ungetc(c, in);
    return c;
}

char read_char(FILE *in) {
    return fgetc(in);
}

Obj *read_symbol(FILE *in) {
    char *symbol = malloc(256); // Fixed buffer for simplicity
    int i = 0;
    
    int c = peek(in);
    while (!isspace(c) && c != EOF && c != '(' && c != ')') {
        symbol[i++] = read_char(in);
        c = peek(in);
    }
    symbol[i] = '\0';
    
    Obj *obj = malloc(sizeof(Obj));
    obj->type = TYPE_SYMBOL;
    obj->data.symbol = symbol;
    return obj;
}

Obj *read_number(FILE *in) {
    int sign = 1;
    int value = 0;
    
    if (peek(in) == '-') {
        sign = -1;
        read_char(in);
    }
    
    while (isdigit(peek(in))) {
        value = value * 10 + (read_char(in) - '0');
    }

    value *= sign;
    
    Obj *obj = malloc(sizeof(Obj));
    obj->type = TYPE_NUMBER;
    obj->data.number = value;
    return obj;
}

// entry point
int main(int argc, const char* argv[argc]) {
  (void)argv;
  (void)argc;

  rl_repl();

  return 0;
}
