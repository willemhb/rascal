#include <stdio.h>

#include "print.h"
#include "object.h"

/* API */
void print_val(Val v, FILE *ios);
void print_obj(Obj *o, FILE *ios);

void print_sym(Sym *x, FILE *ios);
void print_list(List *xs, FILE *ios);
void print_pair(Pair *xs, FILE *ios);
void print_vec(Vec *xs, FILE *ios);
void print_bin(Bin *xs, FILE *ios);
void print_table(Table *xs, FILE *ios);

void print(Val x) {
  print_val(x, stdout);
}
