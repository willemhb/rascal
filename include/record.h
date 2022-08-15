#ifndef rascal_record_h
#define rascal_record_h

#include "common.h"

// C types --------------------------------------------------------------------
struct record_t {
  uint_t       length;
  ushort_t     flags;
  ushort_t     tag;
  value_t      type;

  value_t      slots[0];
};

// utilities ------------------------------------------------------------------
bool_t is_record(value_t x);

#define as_record(x)    as_type(record_t*, opval, x)
#define record_type(x)  getf(record, x, type)
#define record_slots(x) getf(record, x, slots)

// implementation -------------------------------------------------------------
size_t record_print(value_t x, port_t *ios);
sint_t record_order(value_t x, value_t y);
hash_t record_hash(value_t x);

// native functions -----------------------------------------------------------

// initialization -------------------------------------------------------------

#endif
