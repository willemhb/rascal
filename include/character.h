#ifndef rascal_character_h
#define rascal_character_h

#include "common.h"

// C types --------------------------------------------------------------------

// utilities ------------------------------------------------------------------
static inline value_t character(character_t ch) { return tag_sint(ch, CHARACTER); }

// implementation -------------------------------------------------------------
size_t character_print(value_t x, port_t *ios);
sint_t character_order(value_t x, value_t y);

// native functions -----------------------------------------------------------
value_t native_character(value_t *args, size_t n_args);
value_t native_ctype_p(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void character_init(void);

#endif
