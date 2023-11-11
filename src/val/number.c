#include <math.h>

#include "environment.h"

#include "number.h"

Big*  new_big(int64_t value);
bool  fits(Value x, int64_t min, int64_t max);
Type* promote(Type* type, Value* x, Value* y);
Type* promote_n(Type* type, size_t n, Value* a);

void  init_numbers(void) {
  define(NULL, symbol("&pi"), tag(3.141592), FINAL);
  define(NULL, symbol("&e"),  tag(2.718281), FINAL);
}
