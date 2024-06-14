#include "val/text.h"
#include "val/table.h"

/* Globals */

/* Internal APIs */
void intern_in_scache(void* t, void* e, void* k,  void* s, hash_t h) {
  (void)s;
  (void)t;

  SCEntry* se   = e;
  se->val       = new_str(k);
  se->key       = se->val->chars;
  se->val->hash = h;
}

/* External APIs */
Str* get_str(char* cs) {
  SCEntry* e;
  
  scache_intern(&StrCache, cs, &e, intern_in_scache, NULL);

  return e->val;
}
